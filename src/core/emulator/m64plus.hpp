//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <atomic>
#include <experimental/filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <optional>
#include "core/emulator/emulator.hpp"
#include "core/emulator/shared_library.hpp"
#include "core/logging.hpp"


namespace Core::Emulator
{

namespace fs = std::experimental::filesystem;

/// Mupen64Plus API this frontend is compatible with
constexpr int CORE_API_VERSION{0x020001};

// Don't pollute global namespace
namespace M64PTypes
{
#include <mupen64plus/m64p_types.h>
} // M64PTypes

namespace M64PlusHelper
{

struct Plugin;

/// Mupen64Plus error codes
enum struct Error
{
    // Mupen64Plus internal errors
    SUCCESS = 0,
    NOT_INIT,        ///< Function is disallowed before InitMupen64Plus() is called
    ALREADY_INIT,    ///< InitMupen64Plus() was called twice
    INCOMPATIBLE,    ///< API versions between components are incompatible
    INPUT_ASSERT,    ///< Invalid parameters for function call, such as ParamValue=NULL for GetCoreParameter()
    INPUT_INVALID,   ///< Invalid input data, such as ParamValue="maybe" for SetCoreParameter() to set a BOOL-type value
    INPUT_NOT_FOUND, ///< The input parameter(s) specified a particular item which was not found
    NO_MEMORY,       ///< Memory allocation failed
    FILES,           ///< Error opening, creating, reading, or writing to a file
    INTERNAL,        ///< Internal error (bug)
    INVALID_STATE,   ///< Current program state does not allow operation
    PLUGIN_FAIL,     ///< A plugin function returned a fatal error
    SYSTEM_FAIL,     ///< A system function call, such as an SDL or file operation, failed
    UNSUPPORTED,     ///< Function call is not supported (ie, core not built with debugger)
    WRONG_TYPE,      ///< A given input type parameter cannot be used for desired operation
    // Interface errors
    LIB_LOAD_FAILED, ///< Failed to load library file
    INVALID_ADDR,    ///< Tried to access out of bounds memory
    SYM_NOT_FOUND    ///< A symbol required by the API could not be located in the specified module
};

/// Contains information retrieved via PluginGetVersion
struct PluginInfo
{
    M64PTypes::m64p_plugin_type type{M64PTypes::M64PLUGIN_NULL};
    int plugin_version{}, api_version{}, capabilities{};
    std::string name;
};

/**
 * Dynamically loaded Mupen64Plus core
 */
struct Core
{
    // Function pointer types
    using plugin_get_version_t = Error(*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);
    using core_startup_t = Error(*)(int, const char*, const char*, void*, void(*)(void*, int, const char*), void*,
                                    void(*)(void*, M64PTypes::m64p_core_param, int));
    using core_shutdown_t = Error(*)();
    using core_attach_plugin_t = Error(*)(M64PTypes::m64p_plugin_type, M64PTypes::m64p_dynlib_handle);
    using core_detach_plugin_t = Error(*)(M64PTypes::m64p_plugin_type);
    using core_do_cmd_t = Error(*)(M64PTypes::m64p_command, int, void*);
    using debug_get_mem_ptr_t = void* (*)(M64PTypes::m64p_dbg_memptr_type);

    /// Create core from current process
    Core(std::string config_path, std::string data_path);

    /// Create core from dynamic library handle
    Core(dynlib_t lib, std::string config_path, std::string data_path);

    /// Create core from library path
    Core(const fs::directory_entry& lib_file, std::string config_path, std::string data_path);

    /// Non-copyable
    Core(const Core&) = delete;

    /// Moveable
    Core(Core&& other) noexcept;

    /// Move assignment
    Core& operator=(Core&& other) noexcept;

    ~Core();

    friend void swap(Core& first, Core& second) noexcept;

    void attach_plugin(Plugin& plugin);

    void detach_plugin(M64PTypes::m64p_plugin_type type);

    void* get_mem_ptr();

    Error do_cmd(M64PTypes::m64p_command cmd, int p1, void* p2);

    dynlib_t handle();

    const PluginInfo& info() const;

private:
    void init_symbols();
    void init_core();
    void destroy_core();

    template<typename T>
    void resolve_symbol(T& fn_ptr, const char* name)
    {
        fn_ptr = reinterpret_cast<T>(get_symbol(handle_.lib, name));
    }

    UniqueLib handle_{};
    struct
    {
        plugin_get_version_t plugin_get_version;
        core_startup_t core_startup;
        core_shutdown_t core_shutdown;
        core_attach_plugin_t core_attach_plugin;
        core_detach_plugin_t core_detach_plugin;
        core_do_cmd_t core_do_cmd;
        debug_get_mem_ptr_t debug_get_mem_ptr;
    }fn_{};
    PluginInfo info_;

    std::string config_path_,
                data_path_;

    CLASS_LOGGER_("emulator");
};

/**
 * Dynamically loaded Mupen64Plus plugin
 */
struct Plugin
{
    // Function pointer types
    using plugin_startup_t = Error(*)(M64PTypes::m64p_dynlib_handle, void*, void* (*)(void*, int, const char*));
    using plugin_shutdown_t = Error(*)();
    using plugin_get_version_t = Error(*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);

    /// Create plugin from dynamic library handle
    Plugin(Core& core, dynlib_t lib);

    /// Create plugin from library path
    Plugin(Core& core, const fs::directory_entry& lib_file);

    /// Non-copyable
    Plugin(const Plugin&) = delete;

    /// Moveable
    Plugin(Plugin&& other) noexcept;

    /// Move assignment
    Plugin& operator=(Plugin&& other) noexcept;

    ~Plugin();

    friend void swap(Plugin& first, Plugin& second) noexcept;

    const PluginInfo& info() const;

    dynlib_t handle();

    static PluginInfo get_plugin_info(const fs::directory_entry& file);
    static PluginInfo get_plugin_info(dynlib_t lib);

    static const char* type_str(M64PTypes::m64p_plugin_type type_id);


private:
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

    CLASS_LOGGER_("emulator");
};

} // M64PlusHelper

/**
 * Mupen64Plus instance
 */
struct Mupen64Plus final : EmulatorBase
{
    using Core = M64PlusHelper::Core;
    using Plugin = M64PlusHelper::Plugin;
    using PluginInfo = M64PlusHelper::PluginInfo;
    using Error = M64PlusHelper::Error;

    explicit Mupen64Plus(Core&& core);

    /// Non copyable
    Mupen64Plus(Mupen64Plus&) = delete;

    /// Moveable
    Mupen64Plus(Mupen64Plus&& other) noexcept;

    /// Move assignment
    Mupen64Plus& operator=(Mupen64Plus&& other) noexcept;

    ~Mupen64Plus() override;

    friend void swap(Mupen64Plus& first, Mupen64Plus& second) noexcept;

    void add_plugin(Plugin&& plugin);

    void remove_plugin(M64PTypes::m64p_plugin_type type);

    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    void execute() override;

    void stop() override;

    void read_memory(std::size_t addr, void* data, std::size_t n) override;

    void write_memory(std::size_t addr, const void* data, std::size_t n) override;

    Core& core();

    bool running() const override;

    bool rom_loaded() const;

    bool has_plugin(M64PTypes::m64p_plugin_type type) const;

private:
    void attach_plugins();
    void detach_plugins();

    Core core_;
    std::array<std::optional<Plugin>, 6> plugins_{};
    std::atomic_bool running_{};
    bool rom_loaded_{};
};

} // Core::Emulator


/// Overload for Mupen64Plus error codes
std::error_code make_error_code(Core::Emulator::Mupen64Plus::Error e);

/// Specialization for Mupen64Plus error codes
template<>
struct std::is_error_code_enum<::Core::Emulator::Mupen64Plus::Error> : std::true_type{};
