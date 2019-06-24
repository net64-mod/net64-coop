//
// Created by henrik on 05.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "msg_queue.hpp"


namespace Core::Game::MsgQueue
{

Receiver::Receiver(Memory::Ptr<SharedState> queue)
:state_{queue}
{}

std::optional<n64_message_t> Receiver::receive()
{
    u8 index{state_->field(&SharedState::client_index)};

    // If current slot is empty then the whole queue is empty
    if(state_->field(&SharedState::info_array)[index] != SlotState::IN_USE)
    {
        return {}; // Queue empty, try again later
    }

    // Read message from slot
    auto msg_ptr{
        static_cast<Memory::CPtr<n64_message_t>>(state_->field_ptr(&SharedState::msg_array)[index])
    };
    n64_message_t msg{
        msg_ptr->field(&n64_message_t::msg_id),
        msg_ptr->field(&n64_message_t::arg0),
        msg_ptr->field(&n64_message_t::arg1),
        msg_ptr->field(&n64_message_t::arg2),
        msg_ptr->field(&n64_message_t::arg2)
    };

    // Slot is empty now
    state_->field(&SharedState::info_array)[index] = SlotState::FREE;

    // Increment index
    if(++index == state_->field(&SharedState::size))
    {
        index = 0;
    }
    state_->field(&SharedState::client_index) = index;

    return {msg};
}


Sender::Sender(Memory::Ptr<SharedState> queue)
:state_{queue}
{}

bool Sender::send(n64_message_t msg)
{
    u8 index{state_->field(&SharedState::client_index)};

    // If current slot is still in use the queue is full
    if(state_->field(&SharedState::info_array)[index] != SlotState::FREE)
    {
        return false; // Queue full, try again later
    }

    // Write message into slot
    Memory::Ptr<n64_message_t> msg_ptr{
        state_->field_ptr(&SharedState::msg_array)[index]
    };
    msg_ptr->field(&n64_message_t::msg_id) = msg.msg_id;
    msg_ptr->field(&n64_message_t::arg0) = msg.arg0;
    msg_ptr->field(&n64_message_t::arg1) = msg.arg1;
    msg_ptr->field(&n64_message_t::arg2) = msg.arg2;
    msg_ptr->field(&n64_message_t::arg3) = msg.arg3;

    // Slot is in use now
    state_->field(&SharedState::info_array)[index] = SlotState::IN_USE;

    // Increment index
    if(++index == state_->field(&SharedState::size))
    {
        index = 0;
    }
    state_->field(&SharedState::client_index) = index;

    return true;
}

}
