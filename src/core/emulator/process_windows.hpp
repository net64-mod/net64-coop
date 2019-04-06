//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "core/emulator/handle_config.hpp"

#ifdef USE_PROCESS_HANDLE_WINDOWS_

#include <optional>
#include <string_view>
#include <vector>
#include <Windows.h>
#include <Memoryapi.h>
#include "core/logging.hpp"
#include "core/emulator/process_handle_base.hpp"
#include "types.hpp"


namespace Core
{

/// Provides functionality to access a process on Linux
struct ProcessWindows
{
    ~ProcessWindows();

    /// Start process
    void start(std::string_view executable);

    /// Close process handle
    void close();

    /// Return memory regions of process
    std::optional<std::vector<MemoryRegion>> get_memory_regions();

    /// Write n bytes at addr
    void write_memory(std::uintptr_t addr, const u8 data[], std::size_t n);

    /// Read n bytes from addr
    void read_memory(std::uintptr_t addr, u8 data[], std::size_t n);

    /// Check if process exited and close handle if so
    bool running();

    /// Check if process handle is valid
    [[nodiscard]] bool valid() const;

private:
    PROCESS_INFORMATION proc_info_{};
    bool valid_handle_{};
    mutable LoggerPtr logger_{get_logger("process")};
};

} // Core

#endif
