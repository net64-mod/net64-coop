//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <functional>
#include <string>
#include "net64/logging.hpp"
#include "types.hpp"


namespace Net64::Emulator
{

enum struct State
{
    RUNNING,
    PAUSED,
    JOINABLE,
    STOPPED
};

/// Interface for n64 emulators
struct IEmulator
{
    using addr_t = n64_addr_t;
    using saddr_t = n64_saddr_t;
    using usize_t = n64_usize_t;
    using ssize_t = n64_ssize_t;

    static constexpr addr_t LOGICAL_BASE{0x80000000};

    using StateCallback = std::function<void(State)>;

    virtual ~IEmulator() = default;

    /// Load uncompressed rom image
    virtual void load_rom(void* rom_data, std::size_t n) = 0;

    /// Unload rom
    virtual void unload_rom() = 0;

    /// Execute rom, blocking
    //virtual void execute(const StateCallback& fn = {}) = 0;

    /// Start emulation
    virtual void start(const StateCallback& fn = {}) = 0;

    /// Stop execution
    virtual void stop() = 0;

    /// Join emulator thread
    virtual void join(std::error_code& exit_code) = 0;

    /// Read n bytes from addr
    virtual void read_memory(addr_t addr, void* data, usize_t n) = 0;

    /// Write n bytes at addr
    virtual void write_memory(addr_t addr, const void* data, usize_t n) = 0;

    // Converting read functions
    virtual void read(addr_t addr, u8& val) = 0;
    virtual void read(addr_t addr, u16& val) = 0;
    virtual void read(addr_t addr, u32& val) = 0;
    virtual void read(addr_t addr, u64& val) = 0;
    virtual void read(addr_t addr, f32& val) = 0;
    virtual void read(addr_t addr, f64& val) = 0;

    // Converting write functions
    virtual void write(addr_t addr, u8 val) = 0;
    virtual void write(addr_t addr, u16 val) = 0;
    virtual void write(addr_t addr, u32 val) = 0;
    virtual void write(addr_t addr, u64 val) = 0;
    virtual void write(addr_t addr, f32 val) = 0;
    virtual void write(addr_t addr, f64 val) = 0;

    /// Check if emulator is running
    virtual State state() const = 0;

    /// Return name of emulator
    virtual const char* name() const = 0;

    static constexpr usize_t RAM_SIZE{0x800000};
};

} // Net64::Emulator
