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

/**
 * Struct in SM64 which contains net64 relevant information & addresses
 */
struct net64_header_t
{
    u16 compat_version;
    u16 version;
    Memory::NestedPtr<MsgQueue::SharedState> receive_queue,
                                             send_queue;
};

/// Compatibility version of the client
constexpr u16 CLIENT_COMPAT_VER{0};

/// Magic number indicating net64 has been successfully initialized
constexpr u32 MAGIC_NUMBER{0x43303050}; // "C00P"

/// Fixed addresses in the game
/// These are all logical addresses (0x80000000 - 0x807FFFFF)
namespace FixedAddr
{

constexpr n64_addr_t HEADER_PTR{0x807FFFFC};      ///< Location of a pointer to net64's global state
constexpr n64_addr_t MAGIC_NUMBER{0x807FFFF8};    ///< Location of net64's magic number

}

} // Net64::Game
