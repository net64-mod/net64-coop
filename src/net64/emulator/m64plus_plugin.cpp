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

Plugin::Plugin(Core& core, dynlib_t lib)
:handle_{lib}
{
    init_symbols();
    init_plugin(core.handle());
}

Plugin::Plugin(Core& core, const std::string& lib_path)
:handle_{load_library(lib_path.c_str())}
{
    if(!handle_.lib)
    {
        // Library file does not exist
        std::system_error err(make_error_code(Error::LIB_LOAD_FAILED), "Failed to load plugin library file " +
                              std::string(get_lib_error_msg()));
        logger()->error(err.what());
        throw err;
    }
    init_symbols();
    init_plugin(core.handle());
}

Plugin::~Plugin()
{
    if(!handle_.lib)
        return;

    destroy_plugin();
}

const PluginInfo& Plugin::info() const
{
    return info_;
}

dynlib_t Plugin::handle()
{
    return handle_.lib;
}

PluginInfo Plugin::get_plugin_info(const std::string& file)
{
    auto lib{load_library(file.c_str())};

    if(!lib)
    {
        logger()->debug("Failed to load file {} as library: {}", file, get_lib_error_msg());
        return {};
    }

    auto ret{get_plugin_info(lib)};
    free_library(lib);
    return ret;
}

PluginInfo Plugin::get_plugin_info(dynlib_t lib)
{
    PluginInfo info{};
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
        std::system_error err{make_error_code(Error::SYM_NOT_FOUND), "Failed to resolve plugin symbol"};
        logger()->error(err.what());
        throw err;
    }
}

void Plugin::init_plugin(dynlib_t core_lib)
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
                        Plugin::type_str(info_.type), name_ptr, info_.plugin_version, info_.api_version,
                        info_.capabilities, errc.message());
        info_ = {};
        throw std::system_error(errc, "Failed to start " + std::string(Plugin::type_str(info_.type)) + " plugin");
    }

    info_.name = name_ptr;

    logger()->info("Initialized {} plugin {} v{}, api: {:#x} (capabilities: {:#x})",
                   Plugin::type_str(info_.type), name_ptr, info_.plugin_version, info_.api_version, info_.capabilities);
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

} // Net64::Emulator::M64PlusHelper
