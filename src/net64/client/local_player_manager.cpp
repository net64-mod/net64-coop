//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "build_info.hpp"
#include "local_player_manager.hpp"
#include "net64/client.hpp"
#include "net64/net/messages_client.hpp"


namespace Net64
{
void LocalPlayerManager::on_connect(Client& client)
{
    Net::C_Handshake snd_msg;

    snd_msg.name = client.username();
    snd_msg.client_version_str = BuildInfo::GIT_DESC;

    client.send(snd_msg);
}

void LocalPlayerManager::on_disconnect(Client& client)
{
    client_data(client).local_player = {};
}

void LocalPlayerManager::on_message(const Net::S_Handshake& msg, Client& client)
{
    client_data(client).local_player.id = msg.local_player_id;

    logger()->info("Received handshake from server (player_id={})", client_data(client).local_player.id);
}

} // namespace Net64
