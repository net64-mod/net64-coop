//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "net64/game/msg_queue.hpp"
#include "types.hpp"


namespace Net64::Game
{

struct GameMessage
{
    enum : MsgQueue::message_type_t
    {
        RESERVED = 0,
        LOG_APPEND,     ///< Append text to next log message
        LOG_END,        ///< Append text to log message and print it

        NUM_MESSAGES
    };
};

} // Net64::Game
