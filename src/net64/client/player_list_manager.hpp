//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <unordered_map>

#include "net64/client/remote_player.hpp"
#include "net64/net/messages_server.hpp"
#include "net64/net/net_message.hpp"


namespace Net64
{
struct PlayerListManager :
    ClientMessageHandler::Derive<
        PlayerListManager>::Receive<Net::S_ClientConnected, Net::S_ClientDisconnected, Net::S_PlayerList>
{
    void on_message(const Net::S_ClientConnected& msg, Client& client);
    void on_message(const Net::S_ClientDisconnected& msg, Client& client);
    void on_message(const Net::S_PlayerList& msg, Client& client);
};

} // namespace Net64
