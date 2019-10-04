//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <bitset>
#include <initializer_list>
#include "net64/game/game_messages.hpp"
#include "net64/game/msg_queue.hpp"


namespace Net64::Game
{

/**
 * Base class for receiving game messages
 */
struct MessageListener
{
    using n64_message_t = MsgQueue::n64_message_t;
    using message_type_t = MsgQueue::message_type_t;
    static constexpr n64_usize_t MSG_DATA_LEN{MsgQueue::MSG_DATA_LEN};

    virtual ~MessageListener() = default;

    void push_message(const n64_message_t& message);

protected:
    MessageListener(std::initializer_list<message_type_t> subscribed_messages);

    void set_subsribed_messages(std::initializer_list<message_type_t> subscribed_messages);

    void subscribe(message_type_t message);

    void unsubscribe(message_type_t message);

    bool is_subscribed(message_type_t message) const;

    virtual void handle_message(const n64_message_t& message) = 0;

private:
    std::bitset<GameMessage::NUM_MESSAGES> subscribed_messages_;
};

} // Net64::Game
