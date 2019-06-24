//
// Created by henrik on 20.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "memory_handle.hpp"


namespace Core::Memory
{

Handle::Handle(Emulator::IEmulator& hdl)
{
    set_emulator(hdl);
}

bool Handle::operator==(const Handle& other) const
{
    return (emu_ == other.emu_);
}

bool Handle::operator!=(const Handle& other) const
{
    return !(*this == other);
}

void Handle::set_emulator(Emulator::IEmulator& hdl)
{
    emu_ = &hdl;
}

const Emulator::IEmulator& Handle::emulator() const
{
    return *emu_;
}

void Handle::read_raw(addr_t offset, u8* data, usize_t n)
{
    emu_->read_memory(offset, data, n);
}

void Handle::write_raw(addr_t offset, const u8* data, usize_t n)
{
    emu_->write_memory(offset, data, n);
}

bool Handle::valid_offset(addr_t offset)
{
    return offset < Emulator::IEmulator::RAM_SIZE;
}

} // Core::Memory
