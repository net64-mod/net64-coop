//
// Created by henrik on 20.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <algorithm>
#include <limits>
#include "common/integer.hpp"
#include "core/emulator/emulator.hpp"


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

    explicit Handle(Emulator::IEmulator& hdl);

    Handle(const Handle&) = default;

    bool operator==(const Handle& other) const;
    bool operator!=(const Handle& other) const;

    /// Set the referenced emulator
    void set_emulator(Emulator::IEmulator& hdl);

    /// Return referenced emulator (const)
    const Emulator::IEmulator& emulator() const;

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

private:

    /**
     * Helper alias to find the readable / writeable version of a type.
     * Casts signed integers to their unsigned counterpart
     * Casts enums to their underlying (unsigned) type
     */
    template<typename T>
    using Casted = std::conditional_t<std::is_signed_v<T>,
                       Common::unsigned_t<T>,
                   std::conditional_t<std::is_enum_v<T>,
                       Common::Uint<sizeof(T) * 8>,
                   T>>;

    Emulator::IEmulator* emu_;
};


template<typename T>
T Handle::read(addr_t offset)
{
    Casted<T> val{};
    emu_->read(offset, val);
    return static_cast<T>(val);
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
    emu_->write(offset, static_cast<Casted<T>>(val));
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
