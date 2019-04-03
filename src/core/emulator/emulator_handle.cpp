//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "core/emulator/emulator_handle.hpp"
#include "core/error_codes.hpp"

#include "emulator_handle.hpp"


namespace Core
{

const std::array<u8, Emulator::PATTERN_SIZE> Emulator::RAM_PATTERN{
    0x32, 0x80, 0x1a, 0x3c, 0x50, 0x76, 0x5a, 0x27, 0x08, 0x00, 0x40, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x60, 0xa4, 0x01, 0x3c, 0x24, 0x48, 0x2a, 0x01
};

Emulator::~Emulator()
{
    close();
}

void Emulator::start(std::string_view path)
{
    process_.start(path);
}

void Emulator::close()
{
    process_.close();
    base_addr_ = 0;
}

bool Emulator::find_base_addr()
{
    if(!running())
        throw std::system_error(make_error_code(ErrorCode::EMU_NOT_RUNNING));

    // Get array of all memory regions
    std::vector<Process::MemoryRegion> mem_regions;
    try
    {
        mem_regions = process_.get_memory_regions().value();
    }
    catch(const std::bad_optional_access&)
    {
        return false;
    }

    // Search memory regions for RAM pattern
    std::vector<u8> buffer(Process::MAX_REGION_SIZE);

    for(auto& it : mem_regions)
    {
        if(!it.readable)
            continue;
        buffer.resize(it.end - it.begin);
        try
        {
            process_.read_memory(it.begin, buffer.data(), buffer.size());
        }
        catch(const std::runtime_error&)
        {
            continue;
        }
        auto pos = std::search(buffer.begin(), buffer.end(), RAM_PATTERN.begin(), RAM_PATTERN.end());
        if(pos == buffer.end())
            continue;
        base_addr_ = it.begin + static_cast<unsigned long>(std::distance(buffer.begin(), pos));
        break;
    }

    return (base_addr_ != 0);
}

void Emulator::reset_base_addr()
{
    base_addr_ = 0;
}

void Emulator::write_memory(n64_addr_t offset, const u8 data[], n64_usize_t n)
{
    if(offset + n > RAM_SIZE)
    {
        get_logger("emulator")->error("Invalid write to offset {:#x}-{:#x} (max: {:#x})",
                                      offset, offset + n, RAM_SIZE);

        throw std::system_error(make_error_code(ErrorCode::INVALID_OFFSET));
    }
    process_.write_memory(base_addr_ + offset, data, n);
}

void Emulator::read_memory(n64_addr_t offset, u8 data[], n64_usize_t n)
{
    if(offset + n > RAM_SIZE)
    {
        get_logger("emulator")->error("Invalid read from offset {:#x}-{:#x} (max: {:#x})",
                                      offset, offset + n, RAM_SIZE);

        throw std::system_error(make_error_code(ErrorCode::INVALID_OFFSET));
    }
    process_.read_memory(base_addr_ + offset, data, n);
}

bool Emulator::running()
{
    // Reset base address if emulator exited
    return process_.running() ? true : (reset_base_addr(), false);
}

std::uintptr_t Emulator::base_addr() const
{
    return base_addr_;
}

bool Emulator::valid() const
{
    return (base_addr_ != 0);
}

} // Core
