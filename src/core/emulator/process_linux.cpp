//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "core/emulator/process_linux.hpp"

#ifdef USE_PROCESS_HANDLE_LINUX_

#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
#include <spawn.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include "core/error_codes.hpp"
#include "core/logging.hpp"


namespace Core
{

ProcessLinux::~ProcessLinux()
{
    // Wait for child process to prevent zombie
    (void)running();
}

void ProcessLinux::start(std::string_view executable)
{
    if(running())
        close();

    char* argv[]{nullptr};
    auto ret = posix_spawnp(&pid_, std::string{executable}.c_str(), nullptr, nullptr, argv, environ);
    if(ret != 0)
    {
        std::error_code errc{ret, std::generic_category()};
        logger_->error("Failed to start process. Error code {}", errc);
        throw std::system_error(errc);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if(!running())
    {
        logger_->error("Failed to start process");
        throw std::system_error(make_error_code(ErrorCode::FAILED_PROC_START));
    }
    logger_->info("Started emulator (PID: {})", pid_);
}

void ProcessLinux::close()
{
    pid_ = -1;
}

std::optional<std::vector<ProcessLinux::MemoryRegion>>
ProcessLinux::get_memory_regions()
{
    std::vector<MemoryRegion> mem_regions;
    std::ifstream map_file;

    // Parse /proc/pid/maps file
    map_file.open("/proc/" + std::to_string(pid_) + "/maps");
    if(!map_file.is_open())
    {
        logger_->error("Failed to load memory regions");
        return std::nullopt;
    }

    for(std::string line; !map_file.eof();)
    {
        std::getline(map_file, line);
        MemoryRegion region;
        try
        {
            region.begin = static_cast<std::uintptr_t>(std::stoul(line.substr(0, line.find_first_of('-')),
                           nullptr, 16));
            region.end   = static_cast<std::uintptr_t>(std::stoul(line.substr(line.find_first_of('-') + 1,
                           line.find_first_of(' ')), nullptr, 16));
            region.readable = ((line[line.find_first_of(' ', line.find_first_of('-')) + 1]) == 'r');
        }
        catch(...)
        {
            logger_->debug("Invalid memory region parsed: \"{}\"", line);
            continue;
        }

        if(region.end == 0)
            continue;
        if(region.end - region.begin > MAX_REGION_SIZE)
        {
            logger_->debug("Memory region {:#x} - {:#x} is larger than MAX_REGION_SIZE", region.begin, region.end);
            continue;
        }

        mem_regions.push_back(region);
    }

    return mem_regions;
}

bool ProcessLinux::running()
{
    if(pid_ == -1)
        return false;

    return ((waitpid(pid_, nullptr, WNOHANG)) == 0) ? true : (pid_ = -1, false);
}

void ProcessLinux::write_memory(std::uintptr_t addr, const u8* data, std::size_t n)
{
    iovec local{(void*)data, n},    // const_cast required here
          remote{reinterpret_cast<void*>(addr), n};

    if(process_vm_writev(pid_, &local, 1, &remote, 1, 0) == -1)
    {
        std::error_code errc{errno, std::generic_category()};
        logger_->error("Failed memory access at {:#x}-{:#x}. Error code: {}", addr, addr + n, errc);
        throw std::system_error(errc);
    }
}

void ProcessLinux::read_memory(std::uintptr_t addr, u8* data, std::size_t n)
{
    iovec local{reinterpret_cast<void*>(data), n},
          remote{reinterpret_cast<void*>(addr), n};

    if(process_vm_readv(pid_, &local, 1, &remote, 1, 0) == -1)
    {
        std::error_code errc{errno, std::generic_category()};
        logger_->error("Failed memory access at {:#x}-{:#x}. Error code: {}", addr, addr + n, errc);
        throw std::system_error(errc);
    }
}

bool ProcessLinux::valid() const
{
    return pid_ != -1;
}

} // Core

#endif
