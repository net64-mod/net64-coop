//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <atomic>
#include <string>
#include <string_view>
#include <system_error>
#include <optional>
#include "core/emulator/emulator.hpp"
#include "core/emulator/shared_library.hpp"
#include "core/logging.hpp"


namespace Core::Emulator
{

constexpr int M64P_CORE_API_VERSION{0x020001};

// Don't pollute global namespace
namespace M64PTypes
{
#include <mupen64plus/m64p_types.h>
}

struct M64PPlugin;
enum struct M64PError;

struct M64PPluginInfo
{
    M64PTypes::m64p_plugin_type type{M64PTypes::M64PLUGIN_NULL};
    int plugin_version{},
        api_version{},
        capabilities{};
    std::string name;
};

/**
 * Dynamically loaded Mupen64Plus core
 */
struct M64PCore
{
    // Function pointer types
    using plugin_get_version_t = M64PError(*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);
    using core_startup_t = M64PError(*)(int, const char*, const char*, void*, void(*)(void*, int, const char*),
                                         void*, void(*)(void*, M64PTypes::m64p_core_param, int));
    using core_shutdown_t = M64PError(*)();
    using core_attach_plugin_t = M64PError(*)(M64PTypes::m64p_plugin_type, M64PTypes::m64p_dynlib_handle);
    using core_detach_plugin_t = M64PError(*)(M64PTypes::m64p_plugin_type);
    using core_do_cmd_t = M64PError(*)(M64PTypes::m64p_command, int, void*);
    using debug_get_mem_ptr_t = void*(*)(M64PTypes::m64p_dbg_memptr_type);

    /// Create core from current process
    M64PCore(std::string_view config_path, std::string_view data_path);

    /// Create core from dynamic library handle
    M64PCore(dynlib_t lib, std::string_view config_path, std::string_view data_path);

    /// Create plugin from library path
    M64PCore(std::string_view lib_path, std::string_view config_path, std::string_view data_path);

    /// Non-copyable
    M64PCore(const M64PCore&) = delete;

    /// Moveable
    M64PCore(M64PCore&& other) noexcept;

    /// Move assignment
    M64PCore& operator=(M64PCore&& other) noexcept;

    ~M64PCore();

    friend void swap(M64PCore& first, M64PCore& second) noexcept;

    void attach_plugin(M64PPlugin& plugin);

    void detach_plugin(M64PTypes::m64p_plugin_type type);

    void* get_mem_ptr();

    M64PError do_cmd(M64PTypes::m64p_command cmd, int p1, void* p2);

    dynlib_t handle();

    const M64PPluginInfo& info() const;

private:
    void init_symbols();
    void init_core(std::string_view config_path, std::string_view data_path);
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
    M64PPluginInfo info_;

    CLASS_LOGGER_("emulator");
};

/**
 * Dynamically loaded Mupen64Plus plugin
 */
struct M64PPlugin
{
    // Function pointer types
    using plugin_startup_t = M64PError(*)(M64PTypes::m64p_dynlib_handle, void*, void*(*)(void*, int, const char*));
    using plugin_shutdown_t = M64PError(*)();
    using plugin_get_version_t = M64PError(*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);

    /// Create plugin from dynamic library handle
    M64PPlugin(M64PCore& core, dynlib_t lib);

    /// Create plugin from library path
    M64PPlugin(M64PCore& core, std::string_view lib_path);

    /// Non-copyable
    M64PPlugin(const M64PPlugin&) = delete;

    /// Moveable
    M64PPlugin(M64PPlugin&& other) noexcept;

    /// Move assignment
    M64PPlugin& operator=(M64PPlugin&& other) noexcept;

    ~M64PPlugin();

    friend void swap(M64PPlugin& first, M64PPlugin& second) noexcept;

    const M64PPluginInfo& info() const;

    dynlib_t handle();

    static M64PPluginInfo get_plugin_info(std::string_view lib_path);
    static M64PPluginInfo get_plugin_info(dynlib_t lib);

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
    M64PPluginInfo info_;

    CLASS_LOGGER_("emulator");
};

/**
 * Mupen64Plus instance
 */
struct M64Plus final : EmulatorBase
{
    explicit M64Plus(M64PCore&& core);

    /// Non copyable
    M64Plus(M64Plus&) = delete;

    /// Moveable
    M64Plus(M64Plus&& other) noexcept;

    /// Move assignment
    M64Plus& operator=(M64Plus&& other) noexcept;

    ~M64Plus() override;

    friend void swap(M64Plus& first, M64Plus& second) noexcept;

    void add_plugin(M64PPlugin&& plugin);

    void remove_plugin(M64PTypes::m64p_plugin_type type);

    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    void execute() override;

    void stop() override;

    void read_memory(std::size_t addr, void* data, std::size_t n) override;

    void write_memory(std::size_t addr, const void* data, std::size_t n) override;

    M64PCore& core();

    bool running() const override;

    bool rom_loaded() const;

    bool has_plugin(M64PTypes::m64p_plugin_type type) const;

private:
    void attach_plugins();
    void detach_plugins();

    M64PCore core_;
    std::array<std::optional<M64PPlugin>, 6> plugins_{};
    std::atomic_bool running_{};
    bool rom_loaded_{};

    CLASS_LOGGER_("emulator");
};

/// Mupen64Plus error codes
enum struct M64PError
{
    // Mupen64Plus internal errors
    SUCCESS = 0,
    NOT_INIT,        //< Function is disallowed before InitMupen64Plus() is called
    ALREADY_INIT,    //< InitMupen64Plus() was called twice
    INCOMPATIBLE,    //< API versions between components are incompatible
    INPUT_ASSERT,    //< Invalid parameters for function call, such as ParamValue=NULL for GetCoreParameter()
    INPUT_INVALID,   //< Invalid input data, such as ParamValue="maybe" for SetCoreParameter() to set a BOOL-type value
    INPUT_NOT_FOUND, //< The input parameter(s) specified a particular item which was not found
    NO_MEMORY,       //< Memory allocation failed
    FILES,           //< Error opening, creating, reading, or writing to a file
    INTERNAL,        //< Internal error (bug)
    INVALID_STATE,   //< Current program state does not allow operation
    PLUGIN_FAIL,     //< A plugin function returned a fatal error
    SYSTEM_FAIL,     //< A system function call, such as an SDL or file operation, failed
    UNSUPPORTED,     //< Function call is not supported (ie, core not built with debugger)
    WRONG_TYPE,      //< A given input type parameter cannot be used for desired operation
    // Interface errors
    LIB_LOAD_FAILED, //< Failed to load library file
    INVALID_ADDR,    //< Tried to access out of bounds memory
    SYM_NOT_FOUND    //< A symbol required by the API could not be located in the specified module
};

inline bool failed(M64PError err)
{
    return err != M64PError::SUCCESS;
}

inline bool success(M64PError err)
{
    return err == M64PError::SUCCESS;
}

} // Core::Emulator


/// Overload for Mupen64Plus error codes
std::error_code make_error_code(Core::Emulator::M64PError e);

/// Specialization for Mupen64Plus interface error codes
template<>
struct std::is_error_code_enum<::Core::Emulator::M64PError> : std::true_type{};
