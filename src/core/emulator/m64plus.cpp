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

template<typename... TArgs>
bool all_true(const TArgs&... args)
{
    return (args && ...);
}

M64PCore::M64PCore()
:M64PCore(get_current_process())
{
}

M64PCore::M64PCore(dynlib_t lib)
:handle_{lib}
{
    init_symbols();
    init_core();
}

M64PCore::M64PCore(std::string_view lib_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_)
    {
        // Library file does not exist: log & throw @todo
        assert(false);
    }
    init_symbols();
    init_core();
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
    auto ret{fn_.core_attach_plugin(plugin.type(), plugin.handle())};
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

void M64PCore::do_cmd(m64p_command cmd, int p1, void* p2)
{
    auto ret{fn_.core_do_cmd(cmd, p1, p2)};
    if(ret)
    {
        assert(false);
        // Failed to execute cmd: log & throw @todo or return error code?
    }
}

void M64PCore::init_symbols()
{
    fn_.core_startup = resolve_symbol<core_startup_t>("CoreStartup");
    fn_.core_shutdown = resolve_symbol<core_shutdown_t>("CoreShutdown");
    fn_.core_attach_plugin = resolve_symbol<core_attach_plugin_t>("CoreAttachPlugin");
    fn_.core_detach_plugin = resolve_symbol<core_detach_plugin_t>("CoreDetachPlugin");
    fn_.core_do_cmd = resolve_symbol<core_do_cmd_t>("CoreDoCommand");
    fn_.debug_get_mem_ptr = resolve_symbol<debug_get_mem_ptr_t>("DebugMemGetPointer");

    if(!all_true(fn_.core_attach_plugin, fn_.core_detach_plugin, fn_.core_startup, fn_.core_shutdown,
                 fn_.core_do_cmd, fn_.debug_get_mem_ptr))
    {
        get_logger("emulator")->info("Dang");   // @todo log & throw
    }
}

void M64PCore::init_core()
{
    // @todo: Correct params
    auto ret{fn_.core_startup(131841, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)};
    if(ret)
    {
        // Failed to startup core: log & throw @todo
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
:handle_{other.handle_}, type_{other.type_}, name_{std::move(other.name_)},
plugin_version_{other.plugin_version_}, api_version_{other.api_version_},
capabilities_{other.capabilities_}, fn_{other.fn_}
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
    swap(first.type_, second.type_);
    swap(first.name_, second.name_);
    swap(first.plugin_version_, second.plugin_version_);
    swap(first.api_version_, second.api_version_);
    swap(first.capabilities_, second.capabilities_);
    swap(first.fn_, second.fn_);
}

m64p_plugin_type M64PPlugin::type() const
{
    return type_;
}

std::string_view M64PPlugin::name() const
{
    return name_;
}

int M64PPlugin::plugin_version() const
{
    return plugin_version_;
}

int M64PPlugin::api_version() const
{
    return api_version_;
}

int M64PPlugin::capabilities() const
{
    return capabilities_;
}

dynlib_t M64PPlugin::handle()
{
    return handle_;
}

void M64PPlugin::init_symbols()
{
    fn_.startup = resolve_symbol<plugin_startup_t>("PluginStartup");
    fn_.shutdown = resolve_symbol<plugin_shutdown_t>("PluginShutdown");
    fn_.get_version = resolve_symbol<plugin_get_version_t>("PluginGetVersion");

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
    ret = fn_.get_version(&type_, &plugin_version_, &api_version_, &name_ptr, &capabilities_);
    if(ret)
    {
        // Failed to retrieve version info: log & throw @todo
        assert(false);
    }

    name_ = name_ptr;
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
plugins_{std::move(other.plugins_)}, rdram_addr_{other.rdram_addr_},
running_{other.running_}
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
        unload_rom();
}

void swap(M64Plus& first, M64Plus& second) noexcept
{
    using std::swap;

    swap(first.core_, second.core_);
    swap(first.plugins_, second.plugins_);
    swap(first.rdram_addr_, second.rdram_addr_);
    swap(first.running_, second.running_);
}

void M64Plus::add_plugin(M64PPlugin&& plugin)
{
    assert(!rom_loaded());

    plugins_[plugin.type()] = std::move(plugin);
}

void M64Plus::remove_plugin(m64p_plugin_type type)
{
    assert(!rom_loaded());

    plugins_[type] = {};
}

void M64Plus::load_rom(void* rom_data, std::size_t n)
{
    core_.do_cmd(M64CMD_ROM_OPEN, static_cast<int>(n), rom_data);
    rdram_addr_ = reinterpret_cast<std::uintptr_t>(core_.get_mem_ptr());
    attach_plugins();
}

void M64Plus::unload_rom()
{
    detach_plugins();
    rdram_addr_ = 0;

    try
    {
        core_.do_cmd(M64CMD_ROM_CLOSE, 0, nullptr);
    }
    catch(std::system_error& e)
    {
        // @todo: log
        assert(false);
    }
}

void M64Plus::execute()
{
    running_ = true;
    core_.do_cmd(M64CMD_EXECUTE, 0, nullptr);
    running_ = false;
}

void M64Plus::read_memory(std::size_t addr, void* data, std::size_t n)
{
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds: log & throw @todo
        assert(false);
    }
    std::copy_n(reinterpret_cast<u8*>(rdram_addr_ + addr), n, reinterpret_cast<u8*>(data));
}

void M64Plus::write_memory(std::size_t addr, const void* data, std::size_t n)
{
    if(addr + n > RAM_SIZE)
    {
        // Out of bounds: log & throw @todo
        assert(false);
    }
    std::copy_n(reinterpret_cast<const u8*>(data), n, reinterpret_cast<u8*>(rdram_addr_ + addr));
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
    return (rdram_addr_ != 0);
}

void M64Plus::attach_plugins()
{
    core_.attach_plugin(plugins_[2].value());
    core_.attach_plugin(plugins_[3].value());
    core_.attach_plugin(plugins_[4].value());
    core_.attach_plugin(plugins_[1].value());
}

void M64Plus::detach_plugins()
{
    core_.detach_plugin(plugins_[2]->type());
    core_.detach_plugin(plugins_[3]->type());
    core_.detach_plugin(plugins_[4]->type());
    core_.detach_plugin(plugins_[1]->type());
}

bool M64Plus::has_plugin(m64p_plugin_type type) const
{
    return plugins_[type].has_value();
}

} // Core::Emulator
