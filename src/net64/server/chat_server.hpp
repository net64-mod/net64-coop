//
// Created by henrik on 06.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "net64/logging.hpp"
#include "net64/net/messages_client.hpp"
#include "net64/net/net_message.hpp"


namespace Net64
{
struct ChatServer : ServerMessageHandler::Derive<ChatServer>::Receive<Net::C_ChatMessage>
{
    void on_message(const Net::C_ChatMessage& msg, Server& server, Player& sender);
};

} // namespace Net64
