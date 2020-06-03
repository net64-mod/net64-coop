//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "net64/emulator/m64plus_core.hpp"


namespace Net64::Emulator::M64PlusHelper
{

/**
 * Dynamically loaded Mupen64Plus plugin
 */
class Plugin final
{
    friend struct ::Net64::Emulator::Mupen64Plus;
    friend class ::Net64::Emulator::M64PlusHelper::Core;

public:
    /// Non copyable
    Plugin(const Plugin&) = delete;

    /// Non movable
    Plugin(Plugin&& other) = delete;

    ~Plugin();

    /**
     * Load file as plugin and return information about it.
     * Returns type = M64PLUGIN_NULL if not a plugin
     */
    static PluginInfo get_plugin_info(const std::string& file);
    static PluginInfo get_plugin_info(dynlib_t lib);

    /// Get string representation of plugin type id
    static const char* type_str(m64p_plugin_type type_id);

private:
    // Function pointer types
    using plugin_startup_t = Error(CALL*)(m64p_dynlib_handle, void*, void* (*)(void*, int, const char*));
    using plugin_shutdown_t = Error(CALL*)();
    using plugin_get_version_t = Error(CALL*)(m64p_plugin_type*, int*, int*, const char**, int*);

    /// Create plugin from dynamic library handle
    Plugin(Core& core, dynlib_t lib);

    /// Create plugin from library path
    Plugin(Core& core, const std::string& lib_path);

    /// Return general information about the plugin
    const PluginInfo& info() const;

    /// Return native library handle
    dynlib_t handle();

    void init_symbols();
    void init_plugin(dynlib_t core_lib);
    void destroy_plugin();

    template<typename T>
    void resolve_symbol(T& fn_ptr, const char* name)
    {
        fn_ptr = reinterpret_cast<T>(get_symbol(handle_.lib, name));
    }

    UniqueLib handle_{};
    struct
    {
        plugin_startup_t startup;
        plugin_shutdown_t shutdown;
        plugin_get_version_t get_version;
    }fn_{};
    PluginInfo info_;

    CLASS_LOGGER_("mupen64plus");
};

#undef CALL

} // M64PlusHelper
