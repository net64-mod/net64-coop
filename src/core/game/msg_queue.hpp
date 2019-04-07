//
// Created by henrik on 05.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <optional>
#include "core/memory/mem_ptr.hpp"
#include "types.hpp"


/**
 * A MsgQueue allows either sending messages from the client to the game
 * or from the game to the client.
 * Messages are sent reliable and in order.
 */
namespace Core::Game::MsgQueue
{

/// Message type
struct n64_message_t
{
    u16 msg_id, //< Reserved for type of message
        arg0;
    u32 arg1,
        arg2,
        arg3;
};

/// State of one queue slot
enum struct SlotState : u8
{
    FREE = 0, //< Message slot is empty
    IN_USE    //< Message slot is used
};

/// State of queue
struct SharedState
{
    [[maybe_unused]] u8 game_index;   //< Index of game, unused in client code
    u8 client_index, //< Current read/write index
       size;         //< Size of the queue
    Memory::N64Ptr<SlotState> info_array; //< Pointer to info array
    Memory::N64Ptr<n64_message_t> msg_array;      //< Pointer to message array
};

/**
 * Receiver for message queue
 */
struct Receiver
{
    Receiver(Memory::Ptr<SharedState> queue);

    /**
     * Receive message from queue
     * @return Received message, empty if queue empty
     */
    std::optional<n64_message_t> receive();

private:
    Memory::Ptr<SharedState> state_;
};

/**
 * Sender for message queue
 */
struct Sender
{
    Sender(Memory::Ptr<SharedState> queue);

    /**
     * Send one message to the queue
     * @param msg message to send
     * @return false if queue full, try again later
     */
    bool send(n64_message_t msg);

private:
    Memory::Ptr<SharedState> state_;
};

}
