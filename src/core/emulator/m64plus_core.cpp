//
// Created by henrik on 28.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"


namespace Core::Emulator::M64PlusHelper
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

Core::Core(std::string root_path)
:Core(get_current_process(), std::move(root_path))
{
    init_symbols();
    init_core();
}

Core::Core(dynlib_t lib, std::string root_path)
:handle_{lib}, root_path_{std::move(root_path)}
{
    init_symbols();
    init_core();
}

Core::Core(const std::string& lib_path, std::string root_path)
:handle_{load_library(lib_path.c_str())}, root_path_{std::move(root_path)}
{
    if(!handle_.lib)
    {
        // Library file does not exist
        logger()->error("Failed to open library file: \"{}\"", get_lib_error_msg());
        throw std::system_error(make_error_code(Error::LIB_LOAD_FAILED));
    }
    init_symbols();
    init_core();
}

Core::Core(Core&& other) noexcept
:handle_{std::move(other.handle_)}, fn_{other.fn_}, root_path_{std::move(other.root_path_)}
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
    swap(first.root_path_, second.root_path_);
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

void Core::list_config_sections(void* context, void (* callback)(void*, const char*))
{
    auto ret{fn_.list_config_sections(context, callback)};

    if(failed(ret))
    {
        logger()->error("Failed to list config sections");
        throw std::system_error(make_error_code(ret));
    }
}

M64PTypes::m64p_handle Core::open_config_section(const char* name)
{
    M64PTypes::m64p_handle hdl;

    auto ret{fn_.open_config_section(name, &hdl)};
    if(failed(ret))
    {
        logger()->error("Failed to open config section '{}'", name);
        throw std::system_error(make_error_code(ret));
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
        logger()->error("Failed to set config parameter '{}'", param_name);
        throw std::system_error(make_error_code(ret));
    }
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
        auto errc{make_error_code(Error::SYM_NOT_FOUND)};
        logger()->error("Failed to resolve symbols of core");
        throw std::system_error(errc);
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
        auto errc{make_error_code(ret)};
        logger()->error("Failed to retrieve core info: {}", errc.message());
        throw std::system_error(errc);
    }

    info_.name = name_ptr;

    std::string config_path{(fs::path(root_path_) / "config").string()}, data_path{
    (fs::path(root_path_) / "data").string()};

    ret = fn_.core_startup(API_VERSION, config_path.c_str(), data_path.c_str(), nullptr, nullptr, nullptr, nullptr);
    if(failed(ret))
    {
        // Failed to startup core
        info_ = {};
        auto errc{make_error_code(ret)};
        logger()->error("Failed to start {} v{}, api: {} (capabilities: {:#x}): {}", name_ptr, info_.plugin_version,
                        info_.api_version, info_.capabilities, errc.message());
        throw std::system_error(errc);
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
    create_dir(dir / "data");
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

dynlib_t Core::handle()
{
    return handle_.lib;
}

const PluginInfo& Core::info() const
{
    return info_;
}

} // Core::Emulator::M64PlusHelpers
