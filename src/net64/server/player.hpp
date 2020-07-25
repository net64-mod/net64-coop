//
// Created by henrik on 06.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <chrono>

#include <enet/enet.h>

#include "common/id_manager.hpp"
#include "net64/net/net_message.hpp"


namespace Net64
{
struct Server;

struct Player
{
    enum struct ConnectionState
    {
        INITIAL,
        HANDSHAKED,
        IN_GAME
    };

    Player(Server& server, ENetPeer& peer);

    bool handshaked() const;
    void disconnect(Net::S_DisconnectCode reason);
    void send(const INetMessage& msg);
    void broadcast(const INetMessage& msg);

    ENetPeer& peer();


    std::chrono::steady_clock::time_point connect_time{};
    IdHandle<Net::player_id_t> id{};
    std::string name{};
    ConnectionState connection_state{ConnectionState::INITIAL};

private:
    Server* server_{};
    ENetPeer* peer_{};
};

} // namespace Net64
