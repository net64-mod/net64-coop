//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"

#include <algorithm>
#include <string>


namespace Core::Emulator::M64Plus
{

template<typename... TArgs>
bool all_true(const TArgs& ... args)
{
    return (args && ...);
}

Core::Core(std::string_view config_path, std::string_view data_path)
:Core(get_current_process(), config_path, data_path)
{
}

Core::Core(dynlib_t lib, std::string_view config_path, std::string_view data_path)
:handle_{lib}
{
    init_symbols();
    init_core(config_path, data_path);
}

Core::Core(std::string_view lib_path, std::string_view config_path, std::string_view data_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_.lib)
    {
        // Library file does not exist
        logger()->error("Failed to open library file: \"{}\"", get_lib_error_msg());
        throw std::system_error(make_error_code(Error::LIB_LOAD_FAILED));
    }
    init_symbols();
    init_core(config_path, data_path);
}

Core::Core(Core&& other) noexcept
:handle_{std::move(other.handle_)}, fn_{other.fn_}
{
}

Core& Core::operator=(Core&& other) noexcept
{
    swap(*this, other);

    return *this;
}

Core::~Core()
{
    if(!handle_.lib)
        return;

    destroy_core();
}

void swap(Core& first, Core& second) noexcept
{
    using std::swap;

    swap(first.handle_, second.handle_);
    swap(first.fn_, second.fn_);
}

void Core::attach_plugin(Plugin& plugin)
{
    auto ret{fn_.core_attach_plugin(plugin.info().type, plugin.handle())};
    if(failed(ret))
    {
        // Failed to attach plugin
        auto errc{make_error_code(ret)};
        logger()->error("Failed to attach plugin {}: {}", plugin.info().name, errc.message());
        throw std::system_error(errc);
    }
}

void Core::detach_plugin(M64PTypes::m64p_plugin_type type)
{
    auto ret{fn_.core_detach_plugin(type)};
    if(failed(ret))
    {
        // Failed to correctly detach plugin
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly detach plugin of type {}: {}", type, errc.message());
    }
}

void* Core::get_mem_ptr()
{
    return fn_.debug_get_mem_ptr(M64PTypes::M64P_DBG_PTR_RDRAM);
}

Error Core::do_cmd(M64PTypes::m64p_command cmd, int p1, void* p2)
{
    return fn_.core_do_cmd(cmd, p1, p2);
}

void Core::init_symbols()
{
    resolve_symbol(fn_.plugin_get_version, "PluginGetVersion");
    resolve_symbol(fn_.core_startup, "CoreStartup");
    resolve_symbol(fn_.core_shutdown, "CoreShutdown");
    resolve_symbol(fn_.core_attach_plugin, "CoreAttachPlugin");
    resolve_symbol(fn_.core_detach_plugin, "CoreDetachPlugin");
    resolve_symbol(fn_.core_do_cmd, "CoreDoCommand");
    resolve_symbol(fn_.debug_get_mem_ptr, "DebugMemGetPointer");

    if(!all_true(fn_.plugin_get_version, fn_.core_attach_plugin, fn_.core_detach_plugin, fn_.core_startup,
                 fn_.core_shutdown, fn_.core_do_cmd, fn_.debug_get_mem_ptr))
    {
        auto errc{make_error_code(Error::SYM_NOT_FOUND)};
        logger()->error("Failed to resolve symbols of mupen64plus core");
        throw std::system_error(errc);
    }
}

void Core::init_core(std::string_view config_path, std::string_view data_path)
{
    const char* name_ptr{};
    auto ret{fn_.plugin_get_version(&info_.type, &info_.plugin_version,
             &info_.api_version, &name_ptr, &info_.capabilities)};
    if(failed(ret))
    {
        // Failed to get core info
        auto errc{make_error_code(ret)};
        logger()->error("Failed to retrieve core info: {}", errc.message());
        throw std::system_error(errc);
    }

    info_.name = name_ptr;

    ret = fn_.core_startup(CORE_API_VERSION, std::string{config_path}.c_str(), std::string{data_path}.c_str(),
                           nullptr, nullptr, nullptr, nullptr);
    if(failed(ret))
    {
        // Failed to startup core
        info_ = {};
        auto errc{make_error_code(ret)};
        logger()->error("Failed to start {} v{}, api: {} (capabilities: {:#x}): {}",
                        name_ptr, info_.plugin_version, info_.api_version, info_.capabilities, errc.message());
        throw std::system_error(errc);
    }

    logger()->info("Initialized {} v{}, api: {:#x} (capabilities: {:#x})",
                  name_ptr, info_.plugin_version, info_.api_version, info_.capabilities);
}

void Core::destroy_core()
{
    auto ret{fn_.core_shutdown()};
    if(failed(ret))
    {
        // Failed to correctly shutdown core
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly shutdown core: ", errc.message());
    }
    logger()->info("Shutdown mupen64plus {}", Plugin::type_str(M64PTypes::M64PLUGIN_CORE));
}

dynlib_t Core::handle()
{
    return handle_.lib;
}

const PluginInfo& Core::info() const
{
    return info_;
}


Plugin::Plugin(Core& core, dynlib_t lib)
:handle_{lib}
{
    init_symbols();
    init_plugin(core.handle());
}

Plugin::Plugin(Core& core, std::string_view lib_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_.lib)
    {
        // Library file does not exist
        logger()->error("Failed to open library file: \"{}\"", get_lib_error_msg());
        throw std::system_error(make_error_code(Error::LIB_LOAD_FAILED));
    }
    init_symbols();
    init_plugin(core.handle());
}

Plugin::Plugin(Plugin&& other) noexcept
:handle_{std::move(other.handle_)}, fn_{other.fn_}, info_{other.info_}
{}

Plugin& Plugin::operator=(Plugin&& other) noexcept
{
    swap(*this, other);

    return *this;
}

Plugin::~Plugin()
{
    if(!handle_.lib)
        return;

    destroy_plugin();
}

void swap(Plugin& first, Plugin& second) noexcept
{
    using std::swap;

    swap(first.handle_, second.handle_);
    swap(first.info_, second.info_);
    swap(first.fn_, second.fn_);
}

const PluginInfo& Plugin::info() const
{
    return info_;
}

dynlib_t Plugin::handle()
{
    return handle_.lib;
}

PluginInfo Plugin::get_plugin_info(std::string_view lib_path)
{
    auto lib{load_library(std::string(lib_path).c_str())};

    if(!lib)
        return {};

    auto ret{get_plugin_info(lib)};
    free_library(lib);
    return ret;
}

PluginInfo Plugin::get_plugin_info(dynlib_t lib)
{
    PluginInfo info;
    auto get_version{reinterpret_cast<plugin_get_version_t>(get_symbol(lib, "PluginGetVersion"))};
    if(!get_version)
        return {};

    const char* name_ptr{};
    get_version(&info.type, &info.plugin_version, &info.api_version, &name_ptr, &info.capabilities);
    info.name = name_ptr;

    return info;
}

const char* Plugin::type_str(M64PTypes::m64p_plugin_type type_id)
{
    using namespace M64PTypes;

    switch(type_id)
    {
    case M64PLUGIN_RSP:
        return "RSP";
    case M64PLUGIN_GFX:
        return "VIDEO";
    case M64PLUGIN_AUDIO:
        return "AUDIO";
    case M64PLUGIN_INPUT:
        return "INPUT";
    case M64PLUGIN_CORE:
        return "CORE";
    case M64PLUGIN_NULL:
        return "NULL";
    default:
        return "Invalid plugin type";
    }
}

void Plugin::init_symbols()
{
    resolve_symbol(fn_.startup, "PluginStartup");
    resolve_symbol(fn_.shutdown, "PluginShutdown");
    resolve_symbol(fn_.get_version, "PluginGetVersion");

    if(!all_true(fn_.startup, fn_.shutdown, fn_.get_version))
    {
        // Failed to resolve symbol
        auto errc{make_error_code(Error::SYM_NOT_FOUND)};
        logger()->error("Failed to resolve symbols of plugin");
        throw std::system_error(errc);
    }
}

void Plugin::init_plugin(dynlib_t core_lib)
{
    const char* name_ptr{};
    auto ret {fn_.get_version(&info_.type, &info_.plugin_version, &info_.api_version, &name_ptr, &info_.capabilities)};
    if(failed(ret))
    {
        // Failed to retrieve version info
        auto errc{make_error_code(ret)};
        logger()->error("Failed to retrieve core info: {}", errc.message());
        info_ = {};
        throw std::system_error(errc);
    }

    ret = fn_.startup(core_lib, nullptr, nullptr);
    if(failed(ret))
    {
        // Plugin failed to initialize
        auto errc{make_error_code(ret)};
        logger()->error("Failed to start mupen64plus {} plugin {} v{}, api: {} (capabilities: {:#x}): {}",
                        Plugin::type_str(info_.type), name_ptr, info_.plugin_version, info_.api_version,
                        info_.capabilities, errc.message());
        info_ = {};
        throw std::system_error(errc);
    }

    info_.name = name_ptr;

    logger()->info("Initialized mupen64plus {} plugin {} v{}, api: {:#x} (capabilities: {:#x})",
                   Plugin::type_str(info_.type), name_ptr, info_.plugin_version, info_.api_version,
                   info_.capabilities);
}

void Plugin::destroy_plugin()
{
    auto ret{fn_.shutdown()};
    if(failed(ret))
    {
        // Failed to destroy plugin
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly shutdown plugin {}: {}", info_.name, errc.message());
    }

    logger()->info("Shutdown mupen64plus {} plugin", Plugin::type_str(info_.type));
}


Instance::Instance(Core&& core)
:EmulatorBase("Mupen64Plus"), core_{std::move(core)}
{
}

Instance::Instance(Instance&& other) noexcept
:EmulatorBase("Mupen64Plus"), core_{std::move(other.core_)},
plugins_{std::move(other.plugins_)},
running_{other.running_.load()}
{
}

Instance& Instance::operator=(Instance&& other) noexcept
{
    swap(*this, other);

    return *this;
}

Instance::~Instance()
{
    if(core_.handle() != nullptr && rom_loaded())
    {
        if(running_)
            stop();
        unload_rom();
    }
}

void swap(Instance& first, Instance& second) noexcept
{
    using std::swap;

    swap(first.core_, second.core_);
    swap(first.plugins_, second.plugins_);
    swap(first.rom_loaded_, second.rom_loaded_);
    first.running_.exchange(second.running_);
}

void Instance::add_plugin(Plugin&& plugin)
{
    assert(!running_);

    plugins_[plugin.info().type] = std::move(plugin);
}

void Instance::remove_plugin(M64PTypes::m64p_plugin_type type)
{
    assert(!running_);

    plugins_[type] = {};
}

void Instance::load_rom(void* rom_data, std::size_t n)
{
    auto ret {core_.do_cmd(M64PTypes::M64CMD_ROM_OPEN, static_cast<int>(n), rom_data)};
    if(failed(ret))
    {
        auto errc{make_error_code(ret)};
        logger()->error("Failed to load ROM image: {}", errc.message());
        throw std::system_error(errc);
    }
    rom_loaded_ = true;
}

void Instance::unload_rom()
{
    rom_loaded_ = false;

    auto ret{core_.do_cmd(M64PTypes::M64CMD_ROM_CLOSE, 0, nullptr)};
    if(failed(ret))
    {
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly unload ROM: {}", errc.message());
    }
}

void Instance::execute()
{
    running_ = true;
    attach_plugins();
    logger()->info("Started n64 emulation");
    auto ret{core_.do_cmd(M64PTypes::M64CMD_EXECUTE, 0, nullptr)};
    detach_plugins();
    running_ = false;

    if(failed(ret))
    {
        auto errc{make_error_code(ret)};
        logger()->error("Error executing ROM image: {}", errc.message());
        throw std::system_error(errc);
    }

    logger()->info("Stopped n64 emulation");
}

void Instance::stop()
{
    using namespace M64PTypes;

    if(!running_)
        return;

    // Wait for emulator to be fully started
    m64p_emu_state state{};
    do
    {
        core_.do_cmd(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE, &state);
    }while(state != M64EMU_RUNNING);

    // Wait for emulator to be fully stopped
    do
    {
        core_.do_cmd(M64CMD_STOP, 0, nullptr);
        core_.do_cmd(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE, &state);
    }while(state != M64EMU_STOPPED);
}

void Instance::read_memory(std::size_t addr, void* data, std::size_t n)
{
    auto base{reinterpret_cast<std::uintptr_t>(core_.get_mem_ptr())};
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds
        auto errc{make_error_code(Error::INVALID_ADDR)};
        logger()->error("Out of bounds memory read at address {:#x} (size: {:#x})", addr, n);
        throw std::system_error(errc);
    }
    if(base == 0)
    {
        // Memory not initalized
        auto errc{make_error_code(Error::INVALID_STATE)};
        logger()->error("Tried to read from uninitialized emulator memory (addr: {:#x}, size: {:#x})", addr, n);
        throw std::system_error(errc);
    }
    std::copy_n(reinterpret_cast<u8*>(base + addr), n, reinterpret_cast<u8*>(data));
}

void Instance::write_memory(std::size_t addr, const void* data, std::size_t n)
{
    auto base{reinterpret_cast<std::uintptr_t>(core_.get_mem_ptr())};
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds
        auto errc{make_error_code(Error::INVALID_ADDR)};
        logger()->error("Out of bounds memory write at address {:#x} (size: {:#x})", addr, n);
        throw std::system_error(errc);
    }
    if(base == 0)
    {
        // Memory not initalized
        auto errc{make_error_code(Error::INVALID_STATE)};
        logger()->error("Tried to write to uninitialized emulator memory (addr: {:#x}, size: {:#x})", addr, n);
        throw std::system_error(errc);
    }
    std::copy_n(reinterpret_cast<const u8*>(data), n, reinterpret_cast<u8*>(base + addr));
}

Core& Instance::core()
{
    return core_;
}

bool Instance::running() const
{
    return running_;
}

bool Instance::rom_loaded() const
{
    return rom_loaded_;
}

void Instance::attach_plugins()
{
    using namespace M64PTypes;

    core_.attach_plugin(plugins_[M64PLUGIN_GFX].value());
    core_.attach_plugin(plugins_[M64PLUGIN_AUDIO].value());
    core_.attach_plugin(plugins_[M64PLUGIN_INPUT].value());
    core_.attach_plugin(plugins_[M64PLUGIN_RSP].value());
}

void Instance::detach_plugins()
{
    using namespace M64PTypes;

    core_.detach_plugin(plugins_[M64PLUGIN_GFX]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_AUDIO]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_INPUT]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_RSP]->info().type);
}

bool Instance::has_plugin(M64PTypes::m64p_plugin_type type) const
{
    return plugins_[type].has_value();
}

} // Core::Emulator::M64Plus


namespace
{

static const struct ErrorCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "mupen64plus";
    }
    std::string message(int ev) const override
    {
        using Core::Emulator::M64Plus::Error;
        
        switch(static_cast<Error>(ev))
        {
        case Error::NOT_INIT:
            return "Function is disallowed before InitMupen64Plus() is called";
        case Error::ALREADY_INIT:
            return "InitMupen64Plus() was called twice";
        case Error::INCOMPATIBLE:
            return "API versions between components are incompatible";
        case Error::INPUT_ASSERT:
            return "Invalid parameters for function call";
        case Error::INPUT_INVALID:
            return "Invalid input data";
        case Error::INPUT_NOT_FOUND:
            return "The input parameter(s) specified a particular item which was not found";
        case Error::NO_MEMORY:
            return "Memory allocation failed";
        case Error::FILES:
            return "Error opening, creating, reading, or writing to a file";
        case Error::INTERNAL:
            return "Internal error (bug)";
        case Error::INVALID_STATE:
            return "Current program state does not allow operation";
        case Error::PLUGIN_FAIL:
            return "A plugin function returned a fatal error";
        case Error::SYSTEM_FAIL:
            return "A system function call, such as an SDL or file operation, failed";
        case Error::UNSUPPORTED:
            return "Function call is not supported";
        case Error::WRONG_TYPE:
            return "A given input type parameter cannot be used for desired operation";

        case Error::LIB_LOAD_FAILED:
            return "Failed to load library file";
        case Error::INVALID_ADDR:
            return "Tried to access out of bounds memory";
        case Error::SYM_NOT_FOUND:
            return "A symbol required by the API could not be located in the specified module";
        default:
            return "[Unknown Error]";
        }
    }
}error_category_g;

} // anonymous

std::error_code make_error_code(Core::Emulator::M64Plus::Error e)
{
    return {static_cast<int>(e), error_category_g};
}
