//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"

#include <string>


namespace Core::Emulator
{

M64PPlugin::M64PPlugin(dynlib_t lib)
:handle_{lib}
{
    init_symbols();
    init_plugin();
}

M64PPlugin::M64PPlugin(std::string_view lib_path)
:handle_{load_library(std::string(lib_path).c_str())}
{
    if(!handle_)
    {
        // Library file does not exist: log & throw @todo
    }
    init_symbols();
    init_plugin();
}

M64PPlugin::~M64PPlugin()
{
    destroy_plugin();
    free_library(handle_);
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

dynlib_t M64PPlugin::handle() const
{
    return handle_;
}

void M64PPlugin::init_symbols()
{
    startup_ = resolve_symbol<plugin_startup_t>("PluginStartup");
    shutdown_ = resolve_symbol<plugin_shutdown_t>("PluginShutdown");
    get_version_ = resolve_symbol<plugin_get_version_t>("PluginGetVersion");

    if(!(startup_ && shutdown_ && get_version_))
    {
        // Failed to resolve symbol: log & throw @todo
    }
}

void M64PPlugin::init_plugin()
{
    auto ret{startup_(get_current_library(), nullptr, nullptr)};
    if(!ret)
    {
        // Plugin failed to initialize: log & throw @todo
    }

    const char* name_ptr{};
    ret = get_version_(&type_, &plugin_version_, &api_version_, &name_ptr, &capabilities_);
    if(!ret)
    {
        // Failed to retrieve version info: log & throw @todo
    }

    name_ = name_ptr;
}

void M64PPlugin::destroy_plugin()
{
    auto ret{shutdown_()};
    if(!ret)
    {
        // Failed to destroy plugin: ONLY log @todo
    }
}



M64Plus::M64Plus(M64PPlugin p1, M64PPlugin p2, M64PPlugin p3, M64PPlugin p4)
:EmulatorBase("Mupen64Plus")
{
    plugins_[p1.type()] = p1;
    plugins_[p2.type()] = p2;
    plugins_[p3.type()] = p3;
    plugins_[p4.type()] = p4;
}

void M64Plus::load_rom(void* rom_data, std::size_t n)
{

}

void M64Plus::unload_rom()
{

}

void M64Plus::execute()
{

}

void M64Plus::read_memory(std::size_t addr, void* data, std::size_t n)
{

}

void M64Plus::write_memory(std::size_t addr, const void* data, std::size_t n)
{

}

bool M64Plus::running() const
{
    return false;
}

void M64Plus::load_plugin(dynlib_t& plugin_slot, const M64Plus::plugin_variant_t& plugin_var)
{
    if(std::get_if<dynlib_t>(&plugin_var))
    {
        plugin_slot = std::get<dynlib_t>(plugin_var);
    }
    else
    {
        const auto& str{std::get<std::string>(plugin_var)};
        if(str.empty())
        {
            // Load current module
            plugin_slot = get_current_library();
            return;
        }
        plugin_slot = load_library(str.c_str());
        if(!plugin_slot)
        {
            // @todo: throw
        }
    }
}


} // Core::Emulator
