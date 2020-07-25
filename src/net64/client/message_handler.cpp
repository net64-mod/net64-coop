//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "message_handler.hpp"
#include "net64/client.hpp"
#include "net64/client/local_player.hpp"


namespace Net64::Game
{
void MessageHandler::push_message(Client& client, const n64_message_t& message)
{
    if(subscribed_messages_[message.msg_type])
        handle_message(client, message);
}

MessageHandler::MessageHandler(std::initializer_list<message_type_t> subscribed_messages)
{
    for(auto message_type : subscribed_messages)
    {
        subscribed_messages_[message_type] = true;
    }
}

void MessageHandler::set_subsribed_messages(std::initializer_list<message_type_t> subscribed_messages)
{
    subscribed_messages_.reset();

    for(auto message_type : subscribed_messages)
    {
        subscribed_messages_[message_type] = true;
    }
}

void MessageHandler::subscribe(message_type_t message)
{
    subscribed_messages_[message] = true;
}

void MessageHandler::unsubscribe(message_type_t message)
{
    subscribed_messages_[message] = false;
}

bool MessageHandler::is_subscribed(message_type_t message) const
{
    return subscribed_messages_[message];
}

} // namespace Net64::Game
