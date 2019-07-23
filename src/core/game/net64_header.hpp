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

/**
 * Struct in SM64 which contains net64 relevant information & addresses
 */
struct net64_header_t
{
    u16 compat_version;
    u16 version;
    Memory::N64Ptr<MsgQueue::SharedState> receive_queue,
                                          send_queue;
};

/// Compatibility version of the client
constexpr u16 CLIENT_COMPAT_VER{0};

}
