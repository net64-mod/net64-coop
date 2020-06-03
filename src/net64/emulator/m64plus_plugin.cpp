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
bool all_true(const TArgs&... args)
{
    return (args && ...);
}

} // namespace

Plugin::Plugin(Core& core, shared_object_t lib): handle_{lib}
{
    init_symbols();
    init_plugin(core.handle());
}

Plugin::Plugin(Core& core, const std::string& lib_path): handle_{lib_path.c_str()}
{
    if(!handle_)
    {
        // Library file does not exist
        std::system_error err(make_error_code(Error::LIB_LOAD_FAILED),
                              "Failed to load plugin library file " + std::string(get_shared_object_error()));
        logger()->error(err.what());
        throw err;
    }
    init_symbols();
    init_plugin(core.handle());
}

Plugin::~Plugin()
{
    if(!handle_)
        return;

    destroy_plugin();
}

const PluginInfo& Plugin::info() const
{
    return info_;
}

shared_object_t Plugin::handle()
{
    return handle_.get();
}

PluginInfo Plugin::get_plugin_info(const std::string& file)
{
    SharedObjectHandle lib(file);

    if(!lib)
    {
        logger()->debug("Failed to load file {} as library: {}", file, get_shared_object_error());
        return {};
    }

    return get_plugin_info(lib.get());
}

PluginInfo Plugin::get_plugin_info(shared_object_t lib)
{
    PluginInfo info{};
    plugin_get_version_t get_version{};
    load_function(lib, get_version, "PluginGetVersion");
    if(!get_version)
        return {};

    const char* name_ptr{};
    get_version(&info.type, &info.plugin_version, &info.api_version, &name_ptr, &info.capabilities);
    info.name = name_ptr;

    return info;
}

const char* Plugin::type_str(m64p_plugin_type type_id)
{
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
    handle_.load_function(fn_.startup, "PluginStartup");
    handle_.load_function(fn_.shutdown, "PluginShutdown");
    handle_.load_function(fn_.get_version, "PluginGetVersion");
}

void Plugin::init_plugin(shared_object_t core_lib)
{
    const char* name_ptr{};
    auto ret{fn_.get_version(&info_.type, &info_.plugin_version, &info_.api_version, &name_ptr, &info_.capabilities)};
    if(failed(ret))
    {
        // Failed to retrieve version info
        std::system_error err{make_error_code(ret), "Failed to retrieve plugin info"};
        logger()->error(err.what());
        info_ = {};
        throw err;
    }

    ret = fn_.startup(core_lib, nullptr, nullptr);
    if(failed(ret))
    {
        // Plugin failed to initialize
        auto errc{make_error_code(ret)};
        logger()->error("Failed to start {} plugin {} v{}, api: {} (capabilities: {:#x}): {}",
                        Plugin::type_str(info_.type),
                        name_ptr,
                        info_.plugin_version,
                        info_.api_version,
                        info_.capabilities,
                        errc.message());
        info_ = {};
        throw std::system_error(errc, "Failed to start " + std::string(Plugin::type_str(info_.type)) + " plugin");
    }

    info_.name = name_ptr;

    logger()->info("Initialized {} plugin {} v{}, api: {:#x} (capabilities: {:#x})",
                   Plugin::type_str(info_.type),
                   name_ptr,
                   info_.plugin_version,
                   info_.api_version,
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

    logger()->info("Shutdown {} plugin", Plugin::type_str(info_.type));
}

} // namespace Net64::Emulator::M64PlusHelper
