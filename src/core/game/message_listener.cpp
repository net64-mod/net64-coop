//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "message_listener.hpp"


namespace Core::Game
{

void MessageListener::push_message(const n64_message_t& message)
{
    if(subscribed_messages_[message.msg_type])
        handle_message(message);
}

MessageListener::MessageListener(std::initializer_list<message_type_t> subscribed_messages)
{
    for(auto message_type : subscribed_messages)
    {
        subscribed_messages_[message_type] = true;
    }
}

void MessageListener::set_subsribed_messages(std::initializer_list<message_type_t> subscribed_messages)
{
    subscribed_messages_.reset();

    for(auto message_type : subscribed_messages)
    {
        subscribed_messages_[message_type] = true;
    }
}

void MessageListener::subscribe(message_type_t message)
{
    subscribed_messages_[message] = true;
}

void MessageListener::unsubscribe(message_type_t message)
{
    subscribed_messages_[message] = false;
}

bool MessageListener::is_subscribed(message_type_t message) const
{
    return subscribed_messages_[message];
}

}
