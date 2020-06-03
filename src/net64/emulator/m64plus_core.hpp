//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "net64/emulator/m64plus_config.hpp"
#include "net64/emulator/m64plus_error.hpp"
#include "net64/emulator/shared_library.hpp"
#include "net64/logging.hpp"


namespace Net64::Emulator
{
struct Mupen64Plus;

}

namespace Net64::Emulator::M64PlusHelper
{
class Plugin;

/// Contains information retrieved via PluginGetVersion
struct PluginInfo
{
    m64p_plugin_type type{M64PLUGIN_NULL};
    int plugin_version{}, api_version{}, capabilities{};
    std::string name;
};

/**
 * Dynamically loaded Mupen64Plus core
 */
class Core final
{
    friend struct ::Net64::Emulator::Mupen64Plus;
    friend class ::Net64::Emulator::M64PlusHelper::Plugin;

    // Function pointer types
    using plugin_get_version_t = Error(CALL*)(m64p_plugin_type*, int*, int*, const char**, int*);
    using debug_context_t = void*;
    using debug_callback_t = void (*)(void*, int, const char*);
    using state_context_t = void*;
    using state_callback_t = void (*)(void*, m64p_core_param, int);
    using core_startup_t = Error(CALL*)(
        int, const char*, const char*, debug_context_t, debug_callback_t, state_context_t, state_callback_t);
    using core_shutdown_t = Error(CALL*)();
    using core_attach_plugin_t = Error(CALL*)(m64p_plugin_type, m64p_dynlib_handle);
    using core_detach_plugin_t = Error(CALL*)(m64p_plugin_type);
    using core_do_cmd_t = Error(CALL*)(m64p_command, int, void*);
    using debug_get_mem_ptr_t = volatile void*(CALL*)(m64p_dbg_memptr_type);

    using state_callback_f = std::function<void(m64p_core_param, int)>;
    using debug_callback_f = std::function<void(int, const char*)>;

    /// Mupen64Plus API this frontend is compatible with
    static constexpr int API_VERSION{0x020001};

    /// Create core from current process
    Core(std::string root_path, std::string data_path);

    /// Create core from dynamic library handle
    Core(dynlib_t lib, std::string root_path, std::string data_path);

    /// Create core from library file
    Core(const std::string& lib_path, std::string root_path, std::string data_path);

    /// Non copyable
    Core(const Core&) = delete;

    /// Non movable
    Core(Core&& other) = delete;

    ~Core();

    // @todo:
public:
    Config config();

private:
    void attach_plugin(Plugin& plugin);

    void detach_plugin(m64p_plugin_type type);

    /// Return pointer to n64 DRAM
    volatile void* get_mem_ptr();

    Error do_cmd(m64p_command cmd, int p1, void* p2);

    void set_state_callback(state_callback_f cb) noexcept;
    void set_debug_callback(debug_callback_f cb) noexcept;

    /// Return native library handle
    dynlib_t handle();

    /// Return general information about the core
    const PluginInfo& info() const;

    void init_symbols();
    void init_core();
    void create_folder_structure();
    void destroy_core();

    static void state_callback_c(void* context, m64p_core_param param_type, int new_value);
    static void debug_callback_c(void* context, int level, const char* message);

    template<typename T>
    void resolve_symbol(T& fn_ptr, const char* name)
    {
        fn_ptr = reinterpret_cast<T>(get_symbol(handle_.lib, name));
    }

    UniqueLib handle_{};
    struct CoreFunctions
    {
        plugin_get_version_t plugin_get_version;
        core_startup_t core_startup;
        core_shutdown_t core_shutdown;
        core_attach_plugin_t core_attach_plugin;
        core_detach_plugin_t core_detach_plugin;
        core_do_cmd_t core_do_cmd;
        debug_get_mem_ptr_t debug_get_mem_ptr;
    } fn_{};
    PluginInfo info_;
    std::unique_ptr<state_callback_f> state_callback_ = std::make_unique<state_callback_f>();
    std::unique_ptr<debug_callback_f> debug_callback_ = std::make_unique<debug_callback_f>();

    std::string root_path_, data_path_;

    CLASS_LOGGER_("mupen64plus");
};

} // namespace Net64::Emulator::M64PlusHelper
