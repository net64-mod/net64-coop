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
#include "net64/emulator/emulator.hpp"
#include "net64/emulator/shared_library.hpp"
#include "net64/logging.hpp"


namespace Net64::Emulator
{

struct Mupen64Plus;

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

/// Overload for Mupen64Plus error codes
std::error_code make_error_code(Error e);


/// Contains information retrieved via PluginGetVersion
struct PluginInfo
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
struct Core final
{
    // Function pointer types
    using plugin_get_version_t = Error(CALL*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);
    using debug_context_t = void*;
    using debug_callback_t = void(*)(void*, int, const char*);
    using state_context_t = void*;
    using state_callback_t = void(*)(void*, M64PTypes::m64p_core_param, int);
    using core_startup_t = Error(CALL*)(int, const char*, const char*,
                                    debug_context_t, debug_callback_t,
                                    state_context_t, state_callback_t);
    using core_shutdown_t = Error(CALL*)();
    using core_attach_plugin_t = Error(CALL*)(M64PTypes::m64p_plugin_type, M64PTypes::m64p_dynlib_handle);
    using core_detach_plugin_t = Error(CALL*)(M64PTypes::m64p_plugin_type);
    using core_do_cmd_t = Error(CALL*)(M64PTypes::m64p_command, int, void*);
    using debug_get_mem_ptr_t = volatile void* (CALL*)(M64PTypes::m64p_dbg_memptr_type);

    using list_config_sections_t = Error(CALL*)(void*, void(*)(void*, const char*));
    using open_config_section_t = Error(CALL*)(const char*, M64PTypes::m64p_handle*);
    using save_config_file_t = void(CALL*)();
    using set_config_parameter_t = Error(CALL*)(M64PTypes::m64p_handle, const char*, M64PTypes::m64p_type, const void*);

    using state_callback_f = std::function<void(M64PTypes::m64p_core_param, int)>;

    /// Mupen64Plus API this frontend is compatible with
    static constexpr int API_VERSION{0x020001};

    /// Create core from current process
    Core(std::string root_path, std::string data_path);

    /// Create core from dynamic library handle
    Core(dynlib_t lib, std::string root_path, std::string data_path);

    /// Create core from library file
    Core(const std::string& lib_path, std::string root_path, std::string data_path);

    /// Non-copyable
    Core(const Core&) = delete;

    /// Moveable
    Core(Core&& other) noexcept;

    /// Move assignment
    Core& operator=(Core&& other) noexcept;

    ~Core();

    friend void swap(Core& first, Core& second) noexcept;

    void prepare_config_file();

    void attach_plugin(Plugin& plugin);

    void detach_plugin(M64PTypes::m64p_plugin_type type);

    /// Return pointer to n64 DRAM
    volatile void* get_mem_ptr();

    Error do_cmd(M64PTypes::m64p_command cmd, int p1, void* p2);

    void list_config_sections(void* context, void(*callback)(void* context, const char* name));

    M64PTypes::m64p_handle open_config_section(const char* name);

    void save_config_file();

    void set_config_parameter(M64PTypes::m64p_handle handle, const char* param_name,
                              M64PTypes::m64p_type type, const void* data);

    void set_state_callback(state_callback_f cb);

    /// Return native library handle
    dynlib_t handle();

    /// Return general information about the core
    const PluginInfo& info() const;

private:
    Core() = default;

    void init_symbols();
    void init_core();
    void create_folder_structure();
    void destroy_core();

    static void state_callback_c(void* context, M64PTypes::m64p_core_param param_type, int new_value);

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
        list_config_sections_t list_config_sections;
        open_config_section_t open_config_section;
        save_config_file_t save_config_file;
        set_config_parameter_t set_config_parameter;
    }fn_{};
    PluginInfo info_;
    std::unique_ptr<state_callback_f> state_callback_ = std::make_unique<state_callback_f>();

    std::string root_path_,
                data_path_;

    static const std::vector<std::string> FORBIDDEN_HOTKEYS;

    CLASS_LOGGER_("mupen64plus");
};

/**
 * Dynamically loaded Mupen64Plus plugin
 */
struct Plugin
{
    // Function pointer types
    using plugin_startup_t = Error(CALL*)(M64PTypes::m64p_dynlib_handle, void*, void* (*)(void*, int, const char*));
    using plugin_shutdown_t = Error(CALL*)();
    using plugin_get_version_t = Error(CALL*)(M64PTypes::m64p_plugin_type*, int*, int*, const char**, int*);

    /// Create plugin from dynamic library handle
    Plugin(Core& core, dynlib_t lib);

    /// Create plugin from library path
    Plugin(Core& core, const std::string& lib_path);

    /// Non-copyable
    Plugin(const Plugin&) = delete;

    /// Moveable
    Plugin(Plugin&& other) noexcept;

    /// Move assignment
    Plugin& operator=(Plugin&& other) noexcept;

    ~Plugin();

    friend void swap(Plugin& first, Plugin& second) noexcept;

    /// Return general information about the plugin
    const PluginInfo& info() const;

    /// Return native library handle
    dynlib_t handle();

    /**
     * Load file as plugin and return information about it.
     * Returns type = M64PLUGIN_NULL if not a plugin
     */
    static PluginInfo get_plugin_info(const std::string& file);
    static PluginInfo get_plugin_info(dynlib_t lib);

    /// Get string representation of plugin type id
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

    CLASS_LOGGER_("mupen64plus");
};

#undef CALL

} // M64PlusHelper

/**
 * Mupen64Plus instance
 */
struct Mupen64Plus final : IEmulator
{
    using Core = M64PlusHelper::Core;
    using Plugin = M64PlusHelper::Plugin;
    using PluginInfo = M64PlusHelper::PluginInfo;
    using Error = M64PlusHelper::Error;

    static constexpr usize_t BSWAP_SIZE{4};


    explicit Mupen64Plus(Core&& core);

    /// Non copyable
    Mupen64Plus(Mupen64Plus&) = delete;

    /// Moveable
    Mupen64Plus(Mupen64Plus&& other) noexcept;

    /// Move assignment
    Mupen64Plus& operator=(Mupen64Plus&& other) noexcept;

    ~Mupen64Plus() override;

    friend void swap(Mupen64Plus& first, Mupen64Plus& second) noexcept;

    /// Register a plugin
    void add_plugin(Plugin&& plugin);

    /// Remove a plugin
    void remove_plugin(M64PTypes::m64p_plugin_type type);

    /// Load ROM image
    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    void execute() override;

    void stop() override;

    // Read & Write implementations
    void read_memory(addr_t addr, void* data, usize_t n) override;
    void write_memory(addr_t addr, const void* data, usize_t n) override;

    void read(addr_t addr, u8& val) final;
    void read(addr_t addr, u16& val) final;
    void read(addr_t addr, u32& val) final;
    void read(addr_t addr, u64& val) final;
    void read(addr_t addr, f32& val) final;
    void read(addr_t addr, f64& val) final;

    void write(addr_t addr, u8 val) final;
    void write(addr_t addr, u16 val) final;
    void write(addr_t addr, u32 val) final;
    void write(addr_t addr, u64 val) final;
    void write(addr_t addr, f32 val) final;
    void write(addr_t addr, f64 val) final;

    Core& core();

    bool running() const override;

    const char* name() const override
    {
        return "mupen64plus";
    }

    bool rom_loaded() const;

    /// Check if plugin of a type is already registered
    bool has_plugin(M64PTypes::m64p_plugin_type type) const;

private:
    void attach_plugins();
    void detach_plugins();
    inline static void check_bounds(addr_t addr, usize_t size);

    template<typename T>
    volatile T* get_mem_ptr()
    {
        volatile auto ptr{core_.get_mem_ptr()};

        if(!ptr)
        {
            // Memory not initalized
            std::system_error err{make_error_code(Error::INVALID_STATE), "Failed to access emulator memory"};
            logger()->error(err.what());
            throw err;
        }

        return reinterpret_cast<volatile T*>(ptr);
    }

    Core core_;
    std::array<std::optional<Plugin>, 6> plugins_{};
    std::atomic_bool running_{};
    bool rom_loaded_{};

    CLASS_LOGGER_("mupen64plus")
};

} // Net64::Emulator


/// Specialization for Mupen64Plus error codes
template<>
struct std::is_error_code_enum<::Net64::Emulator::Mupen64Plus::Error> : std::true_type{};
