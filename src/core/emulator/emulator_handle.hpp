//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <array>
#include <string_view>
#include <vector>
#include "core/emulator/process_handle.hpp"
#include "core/logging.hpp"
#include "types.hpp"


namespace Core
{

/// Class encapsulating management and memory access to an emulator instance
struct Emulator : NonCopyable
{
    ~Emulator();

    /// Start executable at path
    void start(std::string_view path);

    /// Close connection to emulator
    void close();

    /// Search for base address in emulator memory
    bool find_base_addr();

    void reset_base_addr();

    /// Write n bytes at base address + offset
    void write_memory(n64_addr_t offset, const u8 data[], n64_usize_t n);

    /// Read n bytes from base address + offset
    void read_memory(n64_addr_t offset, u8 data[], n64_usize_t n);

    /// Check if emulator process is running
    bool running();

    std::uintptr_t base_addr() const;

    /// Check if instance found the base address
    bool valid() const;

    static constexpr n64_usize_t RAM_SIZE{0x800000},       //< RAM size of n64 (8Mb)
                                 MAX_OFFSET{RAM_SIZE - 1}; //< Maximum address that is accessible

    static constexpr n64_usize_t PATTERN_SIZE{24};         //< Size of Pattern
    static const std::array<u8, PATTERN_SIZE> RAM_PATTERN; //< Pattern to search for in emulator memory

private:
    Process process_;
    std::uintptr_t base_addr_{};
    mutable LoggerPtr logger_{get_logger("emulator")};
};

} // Core
