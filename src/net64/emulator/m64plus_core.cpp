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

const std::vector <std::string> Core::FORBIDDEN_HOTKEYS{
    "Kbd Mapping Load State", "Kbd Mapping Speed Down",
    "Kbd Mapping Speed Up", "Kbd Mapping Pause",
    "Kbd Mapping Fast Forward", "Kbd Mapping Frame Advance",
    "Kbd Mapping Gameshark"
};

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

void Core::prepare_config_file()
{
    list_config_sections(this, [](void* raw_this_ptr, const char* name)
    {
        auto this_ptr{reinterpret_cast<Core*>(raw_this_ptr)};

        if(std::strcmp(name, "CoreEvents") == 0)
        {
            auto hdl{this_ptr->open_config_section(name)};

            auto clear_param{[this_ptr, hdl](const char* name)
            {
                this_ptr->set_config_parameter(hdl, name, M64PTypes::M64TYPE_STRING, "");
            }};

            for(auto& hotkey : FORBIDDEN_HOTKEYS)
            {
                clear_param(hotkey.c_str());
            }
        }
        else if(std::strcmp(name, "Core") == 0)
        {
            auto hdl{this_ptr->open_config_section(name)};

            // Enable 8MB
            auto v{false};
            this_ptr->set_config_parameter(hdl, "DisableExtraMem", M64PTypes::M64TYPE_BOOL, &v);

            // Screenshot path
            std::string path{(fs::path(this_ptr->root_path_) / "screenshot").string()};
            this_ptr->set_config_parameter(hdl, "ScreenshotPath", M64PTypes::M64TYPE_STRING, path.c_str());
            // Savestate & Savegame path
            path = (fs::path(this_ptr->root_path_) / "save").string();
            this_ptr->set_config_parameter(hdl, "SaveStatePath", M64PTypes::M64TYPE_STRING, path.c_str());
            this_ptr->set_config_parameter(hdl, "SaveSRAMPath", M64PTypes::M64TYPE_STRING, path.c_str());
        }
    });

    save_config_file();
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

volatile void* Core::get_mem_ptr()
{
    return fn_.debug_get_mem_ptr(M64PTypes::M64P_DBG_PTR_RDRAM);
}

Error Core::do_cmd(M64PTypes::m64p_command cmd, int p1, void* p2)
{
    return fn_.core_do_cmd(cmd, p1, p2);
}

void Core::list_config_sections(void* context, void (* callback)(void*, const char*))
{
    auto ret{fn_.list_config_sections(context, callback)};

    if(failed(ret))
    {
        std::system_error err{make_error_code(ret), "Failed to list config sections"};
        logger()->error(err.what());
        throw err;
    }
}

M64PTypes::m64p_handle Core::open_config_section(const char* name)
{
    M64PTypes::m64p_handle hdl;

    auto ret{fn_.open_config_section(name, &hdl)};
    if(failed(ret))
    {
        std::system_error err{make_error_code(ret), "Failed to open config section " + std::string(name)};
        logger()->error(err.what());
        throw err;
    }

    return hdl;
}

void Core::save_config_file()
{
    return fn_.save_config_file();
}

void Core::set_config_parameter(M64PTypes::m64p_handle handle, const char* param_name, M64PTypes::m64p_type type,
                                const void* data)
{
    auto ret{fn_.set_config_parameter(handle, param_name, type, data)};
    if(failed(ret))
    {
        std::system_error err{make_error_code(ret), "Failed to set config parameter " + std::string(param_name)};
        logger()->error(err.what());
        throw err;
    }
}

void Core::set_state_callback(state_callback_f cb) noexcept
{
    *state_callback_ = std::move(cb);
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
    resolve_symbol(fn_.list_config_sections, "ConfigListSections");
    resolve_symbol(fn_.open_config_section, "ConfigOpenSection");
    resolve_symbol(fn_.save_config_file, "ConfigSaveFile");
    resolve_symbol(fn_.set_config_parameter, "ConfigSetParameter");

    if(!all_true(fn_.plugin_get_version, fn_.core_attach_plugin, fn_.core_detach_plugin, fn_.core_startup,
                 fn_.core_shutdown, fn_.core_do_cmd, fn_.debug_get_mem_ptr, fn_.list_config_sections,
                 fn_.open_config_section, fn_.save_config_file, fn_.set_config_parameter))
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

    ret = fn_.core_startup(API_VERSION, config_path.c_str(), data_path_.c_str(), nullptr, nullptr, state_callback_.get(), state_callback_c);
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
    logger()->info("Shutdown {}", Plugin::type_str(M64PTypes::M64PLUGIN_CORE));
}

void Core::state_callback_c(void* context, M64PTypes::m64p_core_param param_type, int new_value)
{
    auto cb = reinterpret_cast<state_callback_f*>(context);
    if (*cb)
        (*cb)(param_type, new_value);
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
