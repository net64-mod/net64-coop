//
// Created by henrik on 06.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "chat_server.hpp"
#include "net64/net/messages_server.hpp"
#include "net64/server/player.hpp"


namespace Net64
{
void ChatServer::on_message(const Net::C_ChatMessage& msg, Server&, Player& sender)
{
    if(!sender.handshaked())
        return;

    Net::S_ChatMessage snd_msg;
    snd_msg.message = msg.message;
    snd_msg.sender = sender.id.id();
    sender.broadcast(snd_msg);
}
} // namespace Net64
