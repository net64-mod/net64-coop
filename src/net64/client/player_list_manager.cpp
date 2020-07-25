//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "net64/client.hpp"
#include "player_list_manager.hpp"


namespace Net64
{
void PlayerListManager::on_message(const Net::S_ClientConnected& msg, Client& client)
{
    RemotePlayer player;
    player.id = msg.id;
    player.name = msg.name;
    client_data(client).remote_players.insert(std::pair(player.id, player));
}

void PlayerListManager::on_message(const Net::S_ClientDisconnected& msg, Client& client)
{
    client_data(client).remote_players.erase(msg.player);
}

void PlayerListManager::on_message(const Net::S_PlayerList& msg, Client& client)
{
    for(auto& iter : msg.players)
    {
        RemotePlayer player;
        player.id = iter.id;
        player.name = iter.name;
        client_data(client).remote_players.insert(std::pair(iter.id, player));
    }
}

} // namespace Net64
