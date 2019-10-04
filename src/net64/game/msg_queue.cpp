//
// Created by henrik on 05.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "msg_queue.hpp"


namespace Net64::Game::MsgQueue
{

inline void inc_loop(u8& index, u8 size)
{
    if(++index == size)
        index = 0;
}

Receiver::Receiver(Memory::Ptr<SharedState> queue)
:state_{queue}
{}

bool Receiver::poll(n64_message_t& msg)
{
    u8 index{state_->field(&SharedState::client_index)};

    // If current slot is empty then the whole queue is empty
    if(state_->field(&SharedState::descriptor_array)[index] != SlotState::IN_USE)
    {
        return false; // Queue empty, try again later
    }

    // Read message from slot
    Memory::Ptr<n64_message_t> msg_ptr{
        state_->field(&SharedState::msg_array) + index
    };

    msg.msg_type = msg_ptr->field(&n64_message_t::msg_type);
    auto hdl{msg_ptr.hdl()};
    hdl.read_raw(msg_ptr->field(&n64_message_t::msg_data).ptr().offset(), msg.msg_data, sizeof(msg.msg_data));

    // Slot is empty now
    state_->field(&SharedState::descriptor_array)[index] = SlotState::FREE;

    // Increment index
    inc_loop(index, state_->field(&SharedState::size));
    state_->field(&SharedState::client_index) = index;

    return true;
}


Sender::Sender(Memory::Ptr<SharedState> queue)
:state_{queue}
{}

bool Sender::send(n64_message_t msg)
{
    u8 index{state_->field(&SharedState::client_index)};

    // If current slot is still in use the queue is full
    if(state_->field(&SharedState::descriptor_array)[index] != SlotState::FREE)
    {
        return false; // Queue full, try again later
    }

    // Write message into slot
    Memory::Ptr<n64_message_t> msg_ptr{
        state_->field(&SharedState::msg_array) + index
    };

    msg_ptr->field(&n64_message_t::msg_type) = msg.msg_type;
    auto hdl{msg_ptr.hdl()};
    hdl.write_raw((msg_ptr->field(&n64_message_t::msg_data)).ptr().offset(), msg.msg_data, sizeof(msg.msg_data));

    // Slot is in use now
    state_->field(&SharedState::descriptor_array)[index] = SlotState::IN_USE;

    // Increment index
    inc_loop(index, state_->field(&SharedState::size));
    state_->field(&SharedState::client_index) = index;

    return true;
}

}
