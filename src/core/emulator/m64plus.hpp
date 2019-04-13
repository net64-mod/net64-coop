//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <string>
#include <optional>
#include <mupen64plus/m64p_types.h>
#include "core/emulator/emulator.hpp"
#include "core/emulator/shared_library.hpp"


namespace Core::Emulator
{

struct M64PPlugin;

struct M64PCore
{
    // Function pointer types
    using core_startup_t = m64p_error(*)(int, const char*, const char*, void*, void(*)(void*, int, const char*),
                                         void*, void(*)(void*, m64p_core_param, int));
    using core_shutdown_t = m64p_error(*)();
    using core_attach_plugin_t = m64p_error(*)(m64p_plugin_type, m64p_dynlib_handle);
    using core_detach_plugin_t = m64p_error(*)(m64p_plugin_type);
    using core_do_cmd_t = m64p_error(*)(m64p_command, int, void*);
    using debug_get_mem_ptr_t = void*(*)(m64p_dbg_memptr_type);

    /// Create core from current process
    M64PCore();

    /// Create core from dynamic library handle
    M64PCore(dynlib_t lib);

    /// Create plugin from library path
    M64PCore(std::string_view lib_path);

    /// Non-copyable
    M64PCore(const M64PCore&) = delete;

    /// Moveable
    M64PCore(M64PCore&& other) noexcept;

    /// Move assignment
    M64PCore& operator=(M64PCore&& other) noexcept;

    ~M64PCore();

    friend void swap(M64PCore& first, M64PCore& second) noexcept;

    void attach_plugin(M64PPlugin& plugin);

    void detach_plugin(m64p_plugin_type type);

    void* get_mem_ptr();

    void do_cmd(m64p_command cmd, int p1, void* p2);

    dynlib_t handle();

private:
    void init_symbols();
    void init_core();
    void destroy_core();

    template<typename T>
    T resolve_symbol(const char* name)
    {
        return reinterpret_cast<T>(get_symbol(handle_, name));
    }

    dynlib_t handle_{};
    struct
    {
        core_startup_t core_startup;
        core_shutdown_t core_shutdown;
        core_attach_plugin_t core_attach_plugin;
        core_detach_plugin_t core_detach_plugin;
        core_do_cmd_t core_do_cmd;
        debug_get_mem_ptr_t debug_get_mem_ptr;
    }fn_{};
};

/**
 * Dynamically loaded mupen64plus plugin
 */
struct M64PPlugin
{
    // Function pointer types
    using plugin_startup_t = m64p_error(*)(m64p_dynlib_handle, void*, void*(*)(void*, int, const char*));
    using plugin_shutdown_t = m64p_error(*)();
    using plugin_get_version_t = m64p_error(*)(m64p_plugin_type*, int*, int*, const char**, int*);

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

    /// Return plugin type
    m64p_plugin_type type() const;

    std::string_view name() const;

    int plugin_version() const;

    int api_version() const;

    int capabilities() const;

    dynlib_t handle();

private:
    void init_symbols();
    void init_plugin(dynlib_t core_lib);
    void destroy_plugin();

    template<typename T>
    T resolve_symbol(const char* name)
    {
        return reinterpret_cast<T>(get_symbol(handle_, name));
    }

    dynlib_t handle_{};
    m64p_plugin_type type_{};
    std::string name_;
    int plugin_version_{},
        api_version_{},
        capabilities_{};
    struct
    {
        plugin_startup_t startup;
        plugin_shutdown_t shutdown;
        plugin_get_version_t get_version;
    }fn_{};
};

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

    void remove_plugin(m64p_plugin_type type);

    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    void execute() override;

    void read_memory(std::size_t addr, void* data, std::size_t n) override;

    void write_memory(std::size_t addr, const void* data, std::size_t n) override;

    M64PCore& core();

    bool running() const override;

    bool rom_loaded() const;

    bool has_plugin(m64p_plugin_type type) const;

private:
    void attach_plugins();
    void detach_plugins();

    M64PCore core_;
    std::array<std::optional<M64PPlugin>, 6> plugins_{};
    std::uintptr_t rdram_addr_{};
    bool running_{};
};

} // Core::Emulator
