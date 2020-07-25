//
// Created by henrik on 06.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "net64/server.hpp"
#include "player.hpp"

namespace Net64
{
Player::Player(Server& server, ENetPeer& peer): server_(&server), peer_(&peer)
{
}

bool Player::handshaked() const
{
    return id.has_id();
}

void Player::disconnect(Net::S_DisconnectCode reason)
{
    Server::disconnect(*peer_, reason);
}

void Player::send(const INetMessage& msg)
{
    Server::send(*peer_, msg);
}

void Player::broadcast(const INetMessage& msg)
{
    server_->broadcast(msg);
}

ENetPeer& Player::peer()
{
    return *peer_;
}

} // namespace Net64
