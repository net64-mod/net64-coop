//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "process_windows.hpp"

#ifdef USE_PROCESS_HANDLE_WINDOWS_

#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
#include <Windows.h>


namespace Core
{

ProcessWindows::~ProcessWindows()
{
    close();
}

void ProcessWindows::start(std::string_view executable)
{
    if(running())
        close();

    STARTUPINFOA start_info;
    memset(&start_info, 0, sizeof(start_info));
    memset(&proc_info_, 0, sizeof(proc_info_));
    start_info.cb = sizeof(start_info);

    if(CreateProcessA(std::string(executable).c_str(), nullptr, nullptr, nullptr, false,
                      0, nullptr, nullptr, &start_info, &proc_info_)
       == 0)
    {
        std::error_code errc{(int)GetLastError(), std::generic_category()};
        logger_->error("Failed to start process \"{}\". Error code: ", executable, errc);
        throw std::system_error(errc);
    }
    valid_handle_ = true;
}

void ProcessWindows::close()
{
    if(!valid_handle_)
        return;

    CloseHandle(proc_info_.hProcess);
    CloseHandle(proc_info_.hThread);

    valid_handle_ = false;
}

std::optional<std::vector<ProcessWindows::MemoryRegion>>
ProcessWindows::get_memory_regions()
{
    std::vector<MemoryRegion> mem_regions;
    MEMORY_BASIC_INFORMATION info;

    for(std::uintptr_t addr{0};
        VirtualQueryEx(proc_info_.hProcess, reinterpret_cast<void*>(addr), &info, sizeof(info))
        == sizeof(info);
        addr += info.RegionSize)
    {
        MemoryRegion region;
        region.begin = reinterpret_cast<std::uintptr_t>(info.BaseAddress);
        region.end = region.begin + info.RegionSize;
        region.readable = true;

        if(region.end - region.begin > MAX_REGION_SIZE)
        {
            logger_->debug("Memory region {:#x} - {:#x} is larger than MAX_REGION_SIZE",
                            region.begin, region.end);
            continue;
        }
        mem_regions.push_back(region);
    }

    return mem_regions.size() == 0 ? std::nullopt :
           std::optional<std::vector<MemoryRegion>>{std::move(mem_regions)};
}

bool ProcessWindows::running()
{
    if(!valid_handle_)
        return false;

    unsigned long exit_code{};
    if(GetExitCodeProcess(proc_info_.hProcess, &exit_code) == 0 || exit_code != STILL_ACTIVE)
        return false;
    return true;
}

void ProcessWindows::write_memory(std::uintptr_t addr, const u8* data, std::size_t n)
{
    if(WriteProcessMemory(proc_info_.hProcess, reinterpret_cast<void*>(addr), data, n, nullptr) == 0)
    {
        std::error_code errc{(int)GetLastError(), std::generic_category()};
        logger_->error("Failed memory access at {:#x}-{:#x}. Error code: {}", addr, addr + n, errc);
        throw std::system_error(errc);
    }
}

void ProcessWindows::read_memory(std::uintptr_t addr, u8* data, std::size_t n)
{
    if(ReadProcessMemory(proc_info_.hProcess, reinterpret_cast<void*>(addr), data, n, nullptr) == 0)
    {
        std::error_code errc{(int)GetLastError(), std::generic_category()};
        logger_->error("Failed memory access at {:#x}-{:#x}. Error code: {}", addr, addr + n, errc);
        throw std::system_error(errc);
    }
}

bool ProcessWindows::valid() const
{
    return valid_handle_;
}

} // Core

#endif
