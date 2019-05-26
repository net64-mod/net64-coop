//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <string>
#include "core/logging.hpp"
#include "types.hpp"


namespace Core::Emulator
{

/// Interface for n64 emulators
struct EmulatorBase
{
    virtual ~EmulatorBase() = default;

    /// Load uncompressed rom image
    virtual void load_rom(void* rom_data, std::size_t n) = 0;

    /// Unload rom
    virtual void unload_rom() = 0;

    /// Execute rom, blocking
    virtual void execute() = 0;

    /// Stop execution
    virtual void stop() = 0;

    /// Read n bytes from addr
    virtual void read_memory(std::size_t addr, void* data, std::size_t n) = 0;

    /// Write n bytes at addr
    virtual void write_memory(std::size_t addr, const void* data, std::size_t n) = 0;

    /// Check if emulator is running
    virtual bool running() const = 0;

    /// Return name of emulator
    std::string_view name() const;

    static constexpr n64_usize_t RAM_SIZE{0x800000},
                                 MAX_OFFSET{RAM_SIZE - 1};

protected:
    explicit EmulatorBase(std::string emu_name);

    spdlog::logger* logger();

    std::string name_;

    CLASS_LOGGER_N_("emulator", base_logger)
};

} // Core::Emulator
