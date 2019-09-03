//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "client.hpp"

namespace Core
{

Client::Client(Memory::MemHandle mem_hdl):
    mem_hdl_{mem_hdl},
    net64_header_{mem_hdl_, mem_hdl_.read<n64_addr_t>(Game::FixedAddr::HEADER_PTR)},
    rcv_queue_{net64_header_->field(&Game::net64_header_t::receive_queue)},
    snd_queue_{net64_header_->field(&Game::net64_header_t::send_queue)}
{

}

bool Client::game_initialized(Memory::MemHandle mem_hdl)
{
    return (mem_hdl.read<u32>(Game::FixedAddr::MAGIC_NUMBER) == Game::MAGIC_NUMBER);
}

}
