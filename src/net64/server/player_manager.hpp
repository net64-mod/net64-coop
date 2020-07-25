//
// Created by henrik on 05.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "common/id_manager.hpp"
#include "net64/logging.hpp"
#include "net64/net/messages_client.hpp"


namespace Net64
{
struct PlayerManager :
    ServerMessageHandler::Derive<PlayerManager>::Receive<Net::C_Handshake>,
    ServerConnectionEventHandler
{
    void on_connect(Server& server, Player& player) final;

    void on_disconnect(Server& server, Player& player, Net::C_DisconnectCode code) final;

    void on_message(const Net::C_Handshake& msg, Server& server, Player& sender);

private:
    IdManager<Net::player_id_t> player_ids_;

    CLASS_LOGGER_("server")
};

} // namespace Net64
