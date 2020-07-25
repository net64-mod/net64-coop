//
// Created by henrik on 01.10.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "net64/net/errors.hpp"
#include "net64/server/chat_server.hpp"
#include "net64/server/player.hpp"
#include "net64/server/player_manager.hpp"
#include "server.hpp"


namespace Net64
{
Server::Server(std::uint16_t port, std::size_t max_peers)
{
    max_peers_ = max_peers;
    ENetAddress addr{ENET_HOST_ANY, port};
    host_.reset(enet_host_create(&addr, ALLOCATED_PEERS, Net::channel_count(), 0, 0));

    if(!host_)
        throw std::system_error(make_error_code(Net::Error::ENET_HOST_CREATION));

    add_component(new PlayerManager);
    add_component(new ChatServer);

    logger()->info("Created server (port={} max_peers={})", port, max_peers);
}

Server::~Server()
{
    message_handlers_.clear();
    connection_event_handlers_.clear();
    tick_handlers_.clear();

    reset_all();

    for(auto& iter : components_)
    {
        iter.deleter(iter.component);
    }
}

void Server::tick()
{
    for(ENetEvent evt{}; enet_host_service(host_.get(), &evt, 0) > 0;)
    {
        switch(evt.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            if(!accept_new_)
            {
                logger()->info("Refusing connection attempt from {}. Server not accepting new clients",
                               Net::format_ip4(evt.peer->address));
                enet_peer_disconnect(evt.peer, static_cast<std::uint32_t>(Net::S_DisconnectCode::NOT_ACCEPTED));
                break;
            }
            if(host_->connectedPeers >= max_peers_)
            {
                logger()->info("Refusing connection attempt from {}. Server full", Net::format_ip4(evt.peer->address));
                enet_peer_disconnect(evt.peer, static_cast<std::uint32_t>(Net::S_DisconnectCode::SERVER_FULL));
                break;
            }
            if(evt.data != Net::PROTO_VER)
            {
                logger()->info("Refusing connection attempt from {}. Incompatible version (Client={} Server={})",
                               Net::format_ip4(evt.peer->address),
                               evt.data,
                               Net::PROTO_VER);
                enet_peer_disconnect(evt.peer, static_cast<std::uint32_t>(Net::S_DisconnectCode::INCOMPATIBLE));
                break;
            }
            evt.peer->data = reinterpret_cast<void*>(peer_ids_.acquire_id());
            players_[reinterpret_cast<std::uintptr_t>(evt.peer->data)] = std::make_unique<Player>(*this, *evt.peer);
            logger()->info("New connection from {}", Net::format_ip4(evt.peer->address));
            on_connect(*evt.peer);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            logger()->info("{} disconnected", Net::format_ip4(evt.peer->address));
            if(!evt.peer->data)
                break;
            logger()->info("Player {} (id={}) left the game", player(*evt.peer).name, player(*evt.peer).id.id());
            on_disconnect(*evt.peer, static_cast<Net::C_DisconnectCode>(evt.data));
            destroy_player(*evt.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            PacketHandle packet(evt.packet);
            on_net_message(*packet, *evt.peer);
            break;
        }
    }

    for(auto handler : tick_handlers_)
    {
        handler->on_tick(*this);
    }
}

void Server::send(ENetPeer& peer, const INetMessage& msg)
{
    std::ostringstream strm;
    {
        cereal::PortableBinaryOutputArchive ar(strm);

        msg.serialize_msg(ar);
    }

    PacketHandle packet(enet_packet_create(strm.str().data(), strm.str().size(), Net::channel_flags(msg.channel())));

    enet_peer_send(&peer, static_cast<std::uint8_t>(msg.channel()), packet.release());
}

void Server::broadcast(const INetMessage& msg)
{
    std::ostringstream strm;
    {
        cereal::PortableBinaryOutputArchive ar(strm);

        msg.serialize_msg(ar);
    }

    PacketHandle packet(enet_packet_create(strm.str().data(), strm.str().size(), Net::channel_flags(msg.channel())));

    enet_host_broadcast(host_.get(), static_cast<std::uint8_t>(msg.channel()), packet.release());
}

void Server::disconnect(ENetPeer& peer, Net::S_DisconnectCode reason)
{
    enet_peer_disconnect(&peer, static_cast<std::uint32_t>(reason));
}

void Server::disconnect_all(Net::S_DisconnectCode code)
{
    for(auto& player : players_)
    {
        player.second->disconnect(code);
    }
}

void Server::reset_all()
{
    for(auto& player : players_)
    {
        enet_peer_reset(&player.second->peer());
    }

    players_.clear();
}

void Server::set_max_peers(std::size_t max_peers)
{
    max_peers_ = max_peers;
}

void Server::accept_new(bool v)
{
    accept_new_ = v;
}

std::size_t Server::connected_peers() const
{
    return host_->connectedPeers;
}

std::size_t Server::max_peers() const
{
    return max_peers_;
}

std::uint16_t Server::port() const
{
    return host_->address.port;
}

bool Server::accept_new() const
{
    return accept_new_;
}

Player& Server::player(ENetPeer& peer)
{
    return *players_.at(reinterpret_cast<std::uintptr_t>(peer.data));
}

void Server::destroy_player(ENetPeer& peer)
{
    players_.erase(reinterpret_cast<std::uintptr_t>(peer.data));
    peer_ids_.return_id(reinterpret_cast<std::uintptr_t>(peer.data));
    peer.data = nullptr;
}

void Server::on_connect(ENetPeer& peer)
{
    for(auto handler : connection_event_handlers_)
    {
        handler->on_connect(*this, player(peer));
    }
}

void Server::on_disconnect(ENetPeer& peer, Net::C_DisconnectCode code)
{
    for(auto handler : connection_event_handlers_)
    {
        handler->on_disconnect(*this, player(peer), code);
    }
}

void Server::on_net_message(const ENetPacket& packet, ENetPeer& sender)
{
    std::istringstream strm;

    strm.str({reinterpret_cast<const char*>(packet.data), packet.dataLength});
    try
    {
        std::unique_ptr<INetMessage> msg{INetMessage::parse_message(strm)};

        for(auto handler : message_handlers_)
        {
            handler->handle_message(*msg, *this, player(sender));
        }
    }
    catch(const std::exception& e)
    {
        logger()->warn("Failed to parse network message: {}", e.what());
    }
}

ServerSharedData Server::get_server_shared_data(Badge<ServerDataAccess>)
{
    return ServerSharedData{players_};
}

} // namespace Net64
