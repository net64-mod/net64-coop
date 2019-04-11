//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <string>
#include <variant>
#include <mupen64plus/m64p_types.h>
#include "core/emulator/emulator.hpp"
#include "core/emulator/shared_library.hpp"


namespace Core::Emulator
{

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
    explicit M64PPlugin(dynlib_t lib);

    /// Create plugin from library path
    explicit M64PPlugin(std::string_view lib_path);

    /// Non-copyable
    M64PPlugin(const M64PPlugin&) = delete;

    /// Moveable
    M64PPlugin(M64PPlugin&& other) = default;

    ~M64PPlugin();

    /// Return plugin type
    m64p_plugin_type type() const;

    std::string_view name() const;

    int plugin_version() const;

    int api_version() const;

    int capabilities() const;

    dynlib_t handle() const;

private:
    void init_symbols();
    void init_plugin();
    void destroy_plugin();

    template<typename T>
    T resolve_symbol(const char* name)
    {
        return reinterpret_cast<T>(get_symbol(handle_, name));
    }

    m64p_plugin_type type_{};
    std::string name_;
    int plugin_version_{},
        api_version_{},
        capabilities_{};

    dynlib_t handle_{};
    plugin_startup_t startup_{};
    plugin_shutdown_t shutdown_{};
    plugin_get_version_t get_version_{};
};

struct M64Plus : EmulatorBase
{
    M64Plus(M64PPlugin p1, M64PPlugin p2, M64PPlugin p3, M64PPlugin p4);

    ~M64Plus() override;

    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    void execute() override;

    void read_memory(std::size_t addr, void* data, std::size_t n) override;

    void write_memory(std::size_t addr, const void* data, std::size_t n) override;

    bool running() const override;

private:
    std::array<M64PPlugin, 6> plugins_{};
};

} // Core::Emulator
