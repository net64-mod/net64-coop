//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"

#include <algorithm>
#include <string>


namespace Core::Emulator
{

namespace
{

template<typename... TArgs>
bool all_true(const TArgs& ... args)
{
    return (args && ...);
}

} // anonymous

M64PCore::M64PCore(std::string_view config_path, std::string_view data_path)
:M64PCore(get_current_process(), config_path, data_path)
{
}

M64PCore::M64PCore(dynlib_t lib, std::string_view config_path, std::string_view data_path)
:handle_{lib}
{
    init_symbols();
    init_core(config_path, data_path);
}

M64PCore::M64PCore(std::string_view lib_path, std::string_view config_path, std::string_view data_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_)
    {
        // Library file does not exist: log dlerror & throw @todo
        assert(false);
    }
    init_symbols();
    init_core(config_path, data_path);
}

M64PCore::M64PCore(M64PCore&& other) noexcept
:handle_{other.handle_}, fn_{other.fn_}
{
    other.handle_ = nullptr;
}

M64PCore& M64PCore::operator=(M64PCore&& other) noexcept
{
    swap(*this, other);

    return *this;
}

M64PCore::~M64PCore()
{
    if(!handle_)
        return;

    destroy_core();
    free_library(handle_);
}

void swap(M64PCore& first, M64PCore& second) noexcept
{
    using std::swap;

    swap(first.handle_, second.handle_);
    swap(first.fn_, second.fn_);
}

void M64PCore::attach_plugin(M64PPlugin& plugin)
{
    auto ret{fn_.core_attach_plugin(plugin.info().type, plugin.handle())};
    if(ret)
    {
        // Failed to attach plugin: log & throw @todo
        assert(false);
    }
}

void M64PCore::detach_plugin(m64p_plugin_type type)
{
    auto ret{fn_.core_detach_plugin(type)};
    if(ret)
    {
        // Failed to correctly detach plugin: ONLY log @todo
        assert(false);
    }
}

void* M64PCore::get_mem_ptr()
{
    return fn_.debug_get_mem_ptr(M64P_DBG_PTR_RDRAM);
}

m64p_error M64PCore::do_cmd(m64p_command cmd, int p1, void* p2)
{
    return fn_.core_do_cmd(cmd, p1, p2);
}

void M64PCore::init_symbols()
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
        // @todo log & throw
        assert(false);
    }
}

void M64PCore::init_core(std::string_view config_path, std::string_view data_path)
{
    const char* name_ptr{};
    auto ret{fn_.plugin_get_version(&info_.type, &info_.plugin_version,
             &info_.api_version, &name_ptr, &info_.capabilities)};
    if(ret)
    {
        // Failed to get core info: log & throw @todo
        assert(false);
    }

    info_.name = name_ptr;

    // @todo: Correct params
    ret = fn_.core_startup(131841, std::string{config_path}.c_str(), std::string{data_path}.c_str(),
                           nullptr, nullptr, nullptr, nullptr);
    if(ret)
    {
        // Failed to startup core: log & throw @todo
        info_ = {};
        assert(false);
    }
}

void M64PCore::destroy_core()
{
    auto ret{fn_.core_shutdown()};
    if(ret)
    {
        // Failed to correctly shutdown core: ONLY log @todo
        assert(false);
    }
}

dynlib_t M64PCore::handle()
{
    return handle_;
}

const M64PPluginInfo& M64PCore::info() const
{
    return info_;
}


M64PPlugin::M64PPlugin(M64PCore& core, dynlib_t lib)
:handle_{lib}
{
    init_symbols();
    init_plugin(core.handle());
}

M64PPlugin::M64PPlugin(M64PCore& core, std::string_view lib_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_)
    {
        // Library file does not exist: log & throw @todo
        assert(false);
    }
    init_symbols();
    init_plugin(core.handle());
}

M64PPlugin::M64PPlugin(M64PPlugin&& other) noexcept
:handle_{other.handle_}, fn_{other.fn_}, info_{other.info_}
{
    other.handle_ = nullptr;
}

M64PPlugin& M64PPlugin::operator=(M64PPlugin&& other) noexcept
{
    swap(*this, other);

    return *this;
}

M64PPlugin::~M64PPlugin()
{
    if(!handle_)
        return;

    destroy_plugin();
    free_library(handle_);
}

void swap(M64PPlugin& first, M64PPlugin& second) noexcept
{
    using std::swap;

    swap(first.handle_, second.handle_);
    swap(first.info_, second.info_);
    swap(first.fn_, second.fn_);
}

const M64PPluginInfo& M64PPlugin::info() const
{
    return info_;
}

dynlib_t M64PPlugin::handle()
{
    return handle_;
}

M64PPluginInfo M64PPlugin::get_plugin_info(std::string_view lib_path)
{
    auto lib{load_library(std::string(lib_path).c_str())};

    if(!lib)
        return {};

    auto ret{get_plugin_info(lib)};
    free_library(lib);
    return ret;
}

M64PPluginInfo M64PPlugin::get_plugin_info(dynlib_t lib)
{
    M64PPluginInfo info;
    auto get_version{reinterpret_cast<plugin_get_version_t>(get_symbol(lib, "PluginGetVersion"))};
    if(!get_version)
        return {};

    const char* name_ptr{};
    get_version(&info.type, &info.plugin_version, &info.api_version, &name_ptr, &info.capabilities);
    info.name = name_ptr;

    return info;
}

void M64PPlugin::init_symbols()
{
    resolve_symbol(fn_.startup, "PluginStartup");
    resolve_symbol(fn_.shutdown, "PluginShutdown");
    resolve_symbol(fn_.get_version, "PluginGetVersion");

    if(!all_true(fn_.startup, fn_.shutdown, fn_.get_version))
    {
        // Failed to resolve symbol: log & throw @todo
        assert(false);
    }
}

void M64PPlugin::init_plugin(dynlib_t core_lib)
{
    auto ret{fn_.startup(core_lib, nullptr, nullptr)};
    if(ret)
    {
        // Plugin failed to initialize: log & throw @todo
        assert(false);
    }

    const char* name_ptr{};
    ret = fn_.get_version(&info_.type, &info_.plugin_version, &info_.api_version, &name_ptr, &info_.capabilities);
    if(ret)
    {
        // Failed to retrieve version info: log & throw @todo
        assert(false);
    }

    info_.name = name_ptr;
}

void M64PPlugin::destroy_plugin()
{
    auto ret{fn_.shutdown()};
    if(ret)
    {
        // Failed to destroy plugin: ONLY log @todo
        assert(false);
    }
}


M64Plus::M64Plus(M64PCore&& core)
:EmulatorBase("Mupen64Plus"), core_{std::move(core)}
{
}

M64Plus::M64Plus(M64Plus&& other) noexcept
:EmulatorBase("Mupen64Plus"), core_{std::move(other.core_)},
plugins_{std::move(other.plugins_)},
running_{other.running_.load()}
{
}

M64Plus& M64Plus::operator=(M64Plus&& other) noexcept
{
    swap(*this, other);

    return *this;
}

M64Plus::~M64Plus()
{
    if(core_.handle() != nullptr && rom_loaded())
    {
        if(running_)
            stop();
        unload_rom();
    }
}

void swap(M64Plus& first, M64Plus& second) noexcept
{
    using std::swap;

    swap(first.core_, second.core_);
    swap(first.plugins_, second.plugins_);
    swap(first.rom_loaded_, second.rom_loaded_);
    first.running_.exchange(second.running_);
}

void M64Plus::add_plugin(M64PPlugin&& plugin)
{
    assert(!running_);

    plugins_[plugin.info().type] = std::move(plugin);
}

void M64Plus::remove_plugin(m64p_plugin_type type)
{
    assert(!running_);

    plugins_[type] = {};
}

void M64Plus::load_rom(void* rom_data, std::size_t n)
{
    auto ret {core_.do_cmd(M64CMD_ROM_OPEN, static_cast<int>(n), rom_data)};
    if(ret)
    {
        // @todo: log & throw
        assert(false);
    }
    rom_loaded_ = true;
}

void M64Plus::unload_rom()
{
    rom_loaded_ = false;

    auto ret{core_.do_cmd(M64CMD_ROM_CLOSE, 0, nullptr)};
    if(ret)
    {
        // @todo: log
        assert(false);
    }
}

void M64Plus::execute()
{
    running_ = true;
    attach_plugins();
    auto ret{core_.do_cmd(M64CMD_EXECUTE, 0, nullptr)};
    detach_plugins();
    running_ = false;

    if(ret)
    {
        // @todo: log & throw
        assert(false);
    }
}

void M64Plus::stop()
{
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

void M64Plus::read_memory(std::size_t addr, void* data, std::size_t n)
{
    auto base{reinterpret_cast<std::uintptr_t>(core_.get_mem_ptr())};
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds: log & throw @todo
        assert(false);
    }
    if(base == 0)
    {
        // Memory not initalized: log & throw @todo
        assert(false);
    }
    std::copy_n(reinterpret_cast<u8*>(base + addr), n, reinterpret_cast<u8*>(data));
}

void M64Plus::write_memory(std::size_t addr, const void* data, std::size_t n)
{
    auto base{reinterpret_cast<std::uintptr_t>(core_.get_mem_ptr())};
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds: log & throw @todo
        assert(false);
    }
    if(base == 0)
    {
        // Memory not initalized: log & throw @todo
        assert(false);
    }
    std::copy_n(reinterpret_cast<const u8*>(data), n, reinterpret_cast<u8*>(base + addr));
}

M64PCore& M64Plus::core()
{
    return core_;
}

bool M64Plus::running() const
{
    return running_;
}

bool M64Plus::rom_loaded() const
{
    return rom_loaded_;
}

void M64Plus::attach_plugins()
{
    core_.attach_plugin(plugins_[M64PLUGIN_GFX].value());
    core_.attach_plugin(plugins_[M64PLUGIN_AUDIO].value());
    core_.attach_plugin(plugins_[M64PLUGIN_INPUT].value());
    core_.attach_plugin(plugins_[M64PLUGIN_RSP].value());
}

void M64Plus::detach_plugins()
{
    core_.detach_plugin(plugins_[M64PLUGIN_GFX]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_AUDIO]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_INPUT]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_RSP]->info().type);
}

bool M64Plus::has_plugin(m64p_plugin_type type) const
{
    return plugins_[type].has_value();
}

} // Core::Emulator


namespace
{

const struct M64PErrorCategoryInterface : std::error_category
{
    const char* name() const noexcept override
    {
        return "mupen64plus_interface";
    }
    std::string message(int ev) const override
    {
        using Core::Emulator::M64PError;

        switch(static_cast<M64PError>(ev))
        {
        case M64PError::LIB_NOT_FOUND:
            return "";
        case M64PError::SYM_NOT_FOUND:
            return "";
        case M64PError::BASE_ADDR_NOT_FOUND:
            return "";
        default:
            return "[Unknown Error]";
        }
    }
}m64p_error_category_interface_g;

const struct M64PErrorCategoryInternal : std::error_category
{
    const char* name() const noexcept override
    {
        return "mupen64plus_internal";
    }
    std::string message(int ev) const override
    {
        switch(static_cast<m64p_error>(ev))
        {
        case M64ERR_NOT_INIT:
            return "Function is disallowed before InitMupen64Plus() is called";
        case M64ERR_ALREADY_INIT:
            return "InitMupen64Plus() was called twice";
        case M64ERR_INCOMPATIBLE:
            return "API versions between components are incompatible";
        case M64ERR_INPUT_ASSERT:
            return "Invalid parameters for function call";
        case M64ERR_INPUT_INVALID:
            return "Invalid input data";
        case M64ERR_INPUT_NOT_FOUND:
            return "The input parameter(s) specified a particular item which was not found";
        case M64ERR_NO_MEMORY:
            return "Memory allocation failed";
        case M64ERR_FILES:
            return "Error opening, creating, reading, or writing to a file";
        case M64ERR_INTERNAL:
            return "Internal error (bug)";
        case M64ERR_INVALID_STATE:
            return "Current program state does not allow operation";
        case M64ERR_PLUGIN_FAIL:
            return "A plugin function returned a fatal error";
        case M64ERR_SYSTEM_FAIL:
            return "A system function call, such as an SDL or file operation, failed";
        case M64ERR_UNSUPPORTED:
            return "Function call is not supported";
        case M64ERR_WRONG_TYPE:
            return "A given input type parameter cannot be used for desired operation";
        default:
            return "[Unknown Error]";
        }
    }
}m64p_error_category_internal_g;

} // anonymous

std::error_code make_error_code(m64p_error e)
{
    return {static_cast<int>(e), m64p_error_category_internal_g};
}
