//
// Created by henrik on 20.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <limits>
#include "core/emulator/emulator_handle.hpp"
#include "core/memory/conversion.hpp"


namespace Core::Memory
{

/**
 * Memory handle to n64 RAM
 *
 * Provides interface for raw memory access, type-safe memory access + conversion and aligned memory access
 */
struct Handle
{
    using addr_t = n64_addr_t;
    using saddr_t = n64_saddr_t;
    using usize_t = n64_usize_t;
    using ssize_t = n64_ssize_t;

    static constexpr addr_t INVALID_OFFSET{std::numeric_limits<addr_t>::max()};

    explicit Handle(Emulator& hdl);

    Handle(const Handle&) = default;

    bool operator==(const Handle& other) const;
    bool operator!=(const Handle& other) const;

    /// Set the referenced emulator
    void set_emulator(Emulator& hdl);

    /// Return referenced emulator (const)
    const Emulator& emulator() const;

    /// Check if referenced emulator is valid
    bool valid() const;

    /// Read n bytes from offset
    void read_raw(addr_t offset, u8 data[], usize_t n);

    /// Write n bytes to offset
    void write_raw(addr_t offset, const u8 data[], usize_t n);

    /// Read and convert type T from offset
    template<typename T>
    T read(addr_t offset);

    /// Convert and write type T to offset
    template<typename T>
    void write(addr_t offset, T val);

    /// Same as read but guess the alignment
    template<typename T>
    T read_aligned(addr_t offset);

    /// Same as write but guess the alignment
    template<typename T>
    void write_aligned(addr_t offset, T val);

    /// Same as read but advance offset pointer to point to potential next value
    template<typename T>
    T readc(addr_t& offset);

    /// Same as write but advance offset pointer to point to potential next value
    template<typename T>
    void writec(addr_t& offset, T val);

    /// Same as read_aligned but advance offset pointer to point to potential next value
    template<typename T>
    T readc_aligned(addr_t& offset);

    /// Same as write_aligned but advance offset pointer to point to potential next value
    template<typename T>
    void writec_aligned(addr_t& offset, T val);

    /// Check if offset is a valid offset into n64 memory
    static bool valid_offset(addr_t offset);

protected:
    Emulator* emu_;
};


template<typename T>
T Handle::read(addr_t offset)
{
    T val{};
    read_raw(offset, reinterpret_cast<u8*>(&val), sizeof(val));
    return to_native(val);
}

template<typename T>
T Handle::read_aligned(addr_t offset)
{
    offset += offset % sizeof(T);
    return read<T>(offset);
}

template<typename T>
T Handle::readc(addr_t& offset)
{
    T val{};
    val = read<T>(offset);
    offset += sizeof(val);
    return val;
}

template<typename T>
T Handle::readc_aligned(addr_t& offset)
{
    offset += offset % sizeof(T);
    return readc<T>(offset);
}

template<typename T>
void Handle::write(addr_t offset, T val)
{
    val = to_n64(val);
    write_raw(offset, reinterpret_cast<u8*>(&val), sizeof(val));
}

template<typename T>
void Handle::write_aligned(addr_t offset, T val)
{
    offset += offset % sizeof(T);
    write(offset, val);
}

template<typename T>
void Handle::writec(addr_t& offset, T val)
{
    write<T>(offset, val);
    offset += sizeof(val);
}

template<typename T>
void Handle::writec_aligned(addr_t& offset, T val)
{
    offset += offset % sizeof(T);
    writec(offset, val);
}

} // Core::Memory
