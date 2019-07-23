//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "core/game/msg_queue.hpp"
#include "types.hpp"


namespace Core::Game
{

struct ClientMessage
{
    enum : MsgQueue::message_type_t
    {
        RESERVED = 0,

        NUM_MESSAGES
    };
};

} // Core::Game
