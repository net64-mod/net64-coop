//
// Created by henrik on 20.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "memory_handle.hpp"


namespace Net64::Memory
{

MemHandle::MemHandle(Emulator::IEmulator& hdl)
{
    set_emulator(hdl);
}

bool MemHandle::operator==(const MemHandle& other) const
{
    return (emu_ == other.emu_);
}

bool MemHandle::operator!=(const MemHandle& other) const
{
    return !(*this == other);
}

void MemHandle::set_emulator(Emulator::IEmulator& hdl)
{
    emu_ = &hdl;
}

const Emulator::IEmulator& MemHandle::emulator() const
{
    return *emu_;
}

void MemHandle::read_raw(addr_t offset, u8* data, usize_t n)
{
    emu_->read_memory(offset, data, n);
}

void MemHandle::write_raw(addr_t offset, const u8* data, usize_t n)
{
    emu_->write_memory(offset, data, n);
}

} // Net64::Memory
