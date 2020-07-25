//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "chat_client.hpp"
#include "net64/client.hpp"
#include "net64/net/messages_client.hpp"


namespace Net64
{
void ChatClient::on_message(const Net::S_ChatMessage& msg, Client& client)
{
    if(!callback_)
        return;

    try
    {
        if(msg.sender == 0)
        {
            // Server message, leave sender empty
            callback_("", msg.message);
        }
        else
        {
            callback_(client.remote_players().at(msg.sender).name, msg.message);
        }
    }
    catch(const std::out_of_range& e)
    {
        logger()->warn("Unknown sender ID in chat message");
    }
}

void ChatClient::set_chat_callback(ChatClient::ChatCallback fn)
{
    callback_ = std::move(fn);
}

void ChatClient::send(Client& client, std::string message)
{
    Net::C_ChatMessage snd_msg;
    snd_msg.message = std::move(message);
    client.send(snd_msg);
}

void ChatClient::on_message(const Net::S_ClientConnected& msg, Client&)
{
    if(!callback_)
        return;

    callback_("", msg.name + " joined");
}

void ChatClient::on_message(const Net::S_ClientDisconnected& msg, Client& client)
{
    if(!callback_)
        return;

    try
    {
        callback_("", client.remote_players().at(msg.player).name + " left");
    }
    catch(const std::out_of_range& e)
    {
        logger()->warn("Unknown player id left");
    }
}

} // namespace Net64
