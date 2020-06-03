//
// Created by henrik on 28.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"


namespace Net64::Emulator::M64PlusHelper
{

namespace
{

bool failed(Mupen64Plus::Error err)
{
    return err != Mupen64Plus::Error::SUCCESS;
}

template<typename... TArgs>
bool all_true(const TArgs& ... args)
{
    return (args && ...);
}

}


Core::Core(std::string root_path, std::string data_path)
:Core(get_current_process(), std::move(root_path), std::move(data_path))
{
    init_symbols();
    init_core();
}

Core::Core(dynlib_t lib, std::string root_path, std::string data_path)
:handle_{lib}, root_path_{std::move(root_path)}, data_path_{std::move(data_path)}
{
    init_symbols();
    init_core();
}

Core::Core(const std::string& lib_path, std::string root_path, std::string data_path)
:handle_{load_library(lib_path.c_str())}, root_path_{std::move(root_path)}, data_path_{std::move(data_path)}
{
    if(!handle_.lib)
    {
        // Library file does not exist
        logger()->error("Failed to open library file: \"{}\"", get_lib_error_msg());
        throw std::system_error(make_error_code(Error::LIB_LOAD_FAILED), "Failed to init Mupen64Plus Core");
    }
    init_symbols();
    init_core();
}

Core::~Core()
{
    if(!handle_.lib)
        return;

    destroy_core();
}

Config Core::config()
{
    return Config(handle());
}

void Core::attach_plugin(Plugin& plugin)
{
    auto ret{fn_.core_attach_plugin(plugin.info().type, plugin.handle())};
    if(failed(ret))
    {
        // Failed to attach plugin
        std::system_error err{make_error_code(ret), "Failed to attach plugin " + plugin.info().name};
        logger()->error(err.what());
        throw err;
    }
}

void Core::detach_plugin(m64p_plugin_type type)
{
    auto ret{fn_.core_detach_plugin(type)};
    if(failed(ret))
    {
        // Failed to correctly detach plugin
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly detach plugin of type {}: {}", type, errc.message());
    }
}

volatile void* Core::get_mem_ptr()
{
    return fn_.debug_get_mem_ptr(M64P_DBG_PTR_RDRAM);
}

Error Core::do_cmd(m64p_command cmd, int p1, void* p2)
{
    return fn_.core_do_cmd(cmd, p1, p2);
}

void Core::set_state_callback(state_callback_f cb) noexcept
{
    *state_callback_ = std::move(cb);
}

void Core::set_debug_callback(debug_callback_f cb) noexcept
{
    *debug_callback_ = std::move(cb);
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
        std::system_error err{make_error_code(Error::SYM_NOT_FOUND), "Failed to resolve core symbole"};
        logger()->error(err.what());
        throw err;
    }
}

void Core::init_core()
{
    const char* name_ptr{};
    auto ret{
    fn_.plugin_get_version(&info_.type, &info_.plugin_version, &info_.api_version, &name_ptr, &info_.capabilities)};
    if(failed(ret))
    {
        // Failed to get core info
        std::system_error err{make_error_code(ret), "Failed to retrieve core info"};
        logger()->error(err.what());
        throw err;
    }

    info_.name = name_ptr;

    std::string config_path{(fs::path(root_path_) / "config").string()};

    ret = fn_.core_startup(API_VERSION, config_path.c_str(), data_path_.c_str(), debug_callback_.get(), debug_callback_c, state_callback_.get(), state_callback_c);
    if(failed(ret))
    {
        // Failed to startup core
        auto errc{make_error_code(ret)};
        logger()->error("Failed to start {} v{}, api: {} (capabilities: {:#x}): {}", name_ptr, info_.plugin_version,
                        info_.api_version, info_.capabilities, errc.message());
        info_ = {};
        throw std::system_error(errc, "Failed to start Mupen64Plus Core " + std::string(name_ptr));
    }

    create_folder_structure();

    logger()->info("Initialized {} v{}, api: {:#x} (capabilities: {:#x})", name_ptr, info_.plugin_version,
                   info_.api_version, info_.capabilities);
}

void Core::create_folder_structure()
{
    auto create_dir{[](const fs::path& dir)
    {
        if(!fs::exists(dir))
        {
            fs::create_directories(dir);
        }
    }};

    fs::path dir{root_path_};
    create_dir(dir / "config");
    create_dir(dir / "screenshot");
    create_dir(dir / "save");
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
    logger()->info("Shutdown {}", Plugin::type_str(M64PLUGIN_CORE));
}

void Core::state_callback_c(void* context, m64p_core_param param_type, int new_value)
{
    auto cb = reinterpret_cast<state_callback_f*>(context);
    if (*cb)
        (*cb)(param_type, new_value);
}

void Core::debug_callback_c(void* context, int level, const char* message)
{
    auto cb = reinterpret_cast<debug_callback_f*>(context);
    if (*cb)
        (*cb)(level, message);
}

dynlib_t Core::handle()
{
    return handle_.lib;
}

const PluginInfo& Core::info() const
{
    return info_;
}

} // Net64::Emulator::M64PlusHelpers
