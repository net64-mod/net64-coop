//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <enet/enet.h>
#include "core/game/msg_queue.hpp"
#include "core/game/net64_header.hpp"
#include "core/memory/pointer.hpp"
#include "types.hpp"


namespace Core
{

struct Client
{
    explicit Client(Memory::MemHandle mem_hdl);


    static bool game_initialized(Memory::MemHandle mem_hdl);

private:
    Memory::MemHandle mem_hdl_;
    Memory::Ptr<Game::net64_header_t> net64_header_;
    Game::MsgQueue::Receiver rcv_queue_;
    Game::MsgQueue::Sender snd_queue_;
};

} // Core
