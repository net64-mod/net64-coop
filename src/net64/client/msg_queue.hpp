//
// Created by henrik on 05.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <optional>

#include "net64/memory/pointer.hpp"
#include "types.hpp"


/**
 * A MsgQueue allows either sending messages from the client to the game
 * or from the game to the client.
 * Messages are sent reliable and in order.
 */
namespace Net64::Game::MsgQueue
{
using message_type_t = u16;
constexpr n64_usize_t MSG_DATA_LEN{14};

/// Message type
struct n64_message_t
{
    message_type_t msg_type;   //< Reserved for type of message
    u8 msg_data[MSG_DATA_LEN]; //< Message contents
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
    [[maybe_unused]] u8 game_index;                //< Index of game, unused in net code
    u8 client_index,                               //< Current read/write index
        size;                                      //< Size of the queue
    Memory::NestedPtr<SlotState> descriptor_array; //< Pointer to descriptor array
    Memory::NestedPtr<n64_message_t> msg_array;    //< Pointer to message array
};

/**
 * Receiver for message queue
 */
struct Receiver
{
    explicit Receiver(Memory::Ptr<SharedState> queue);

    /**
     * Receive message from queue
     * @param received message
     * @return true if message received, false if queue empty
     */
    bool poll(n64_message_t& msg);

private:
    Memory::Ptr<SharedState> state_;
};

/**
 * Sender for message queue
 */
struct Sender
{
    explicit Sender(Memory::Ptr<SharedState> queue);

    /**
     * Send one message to the queue
     * @param msg message to send
     * @return false if queue full, try again later
     */
    bool send(n64_message_t msg);

private:
    Memory::Ptr<SharedState> state_;
};

} // namespace Net64::Game::MsgQueue
