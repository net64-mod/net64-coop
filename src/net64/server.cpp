//
// Created by henrik on 01.10.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "server.hpp"

#include "net64/net/errors.hpp"


namespace Net64
{

template<typename Fn>
static void iterate_peers(ENetHost& host, const Fn& fn)
{
    for(std::size_t i{}; i < host.peerCount; ++i)
    {
        if(host.peers[i].state != ENET_PEER_STATE_DISCONNECTED)
            fn(host.peers + i);
    }
}

Server::Server(std::uint16_t port, std::size_t max_clients)
{
    ENetAddress addr{ENET_HOST_ANY, port};
    host_.reset(enet_host_create(&addr, max_clients, Net::CHANNEL_COUNT, 0, 0));

    if(!host_)
    {
        throw std::system_error(make_error_code(Net::Error::ENET_HOST_CREATION));
    }
}

Server::~Server()
{
    reset_all();
}

void Server::set_port(std::uint16_t port)
{
    auto addr{host_->address};
    addr.port = port;
    auto max_clients{host_->peerCount};
    reset_all();
    host_.reset(enet_host_create(&addr, max_clients, Net::CHANNEL_COUNT, 0, 0));
}

void Server::set_max_clients(std::size_t max_clients)
{
    auto addr{host_->address};
    reset_all();
    host_.reset(enet_host_create(&addr, max_clients, Net::CHANNEL_COUNT, 0, 0));
}

void Server::tick(std::chrono::milliseconds max_tick_time)
{
    auto tick_start{Clock::now()};
    for(ENetEvent evt{}; enet_host_service(host_.get(), &evt, Net::SERVER_SERVICE_WAIT) > 0
        && max_tick_time.count() == 0 ? true : Clock::now() - tick_start < max_tick_time;)
    {
        switch(evt.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            if(accept_new_)
            {
                if(evt.data != Net::PROTO_VER)
                {
                    enet_peer_disconnect(evt.peer, Net::S_DisconnectCode::INCOMPATIBLE);
                    break;
                }
                client(*evt.peer) = new client_t;
                client(*evt.peer)->connect_time = Clock::now();
                on_connect(*evt.peer, evt.data);
            }
            else
            {
                enet_peer_disconnect_now(evt.peer, Net::S_DisconnectCode::NOT_ACCEPTED);
            }
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            if(!evt.peer->data)
                break;
            on_disconnect(*evt.peer, evt.data);
            destroy_client(*evt.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            {
            PacketHandle packet{evt.packet};
            on_net_message(*evt.packet);
            break;
            }
        }
    }
}

bool& Server::accept_new_peers()
{
    return accept_new_;
}

const bool& Server::accept_new_peers() const
{
    return accept_new_;
}

std::size_t Server::max_clients() const
{
    return host_->peerCount;
}

std::uint16_t Server::port() const
{
    return host_->address.port;
}

void Server::disconnect_all(std::uint32_t code)
{
    iterate_peers(*host_, [code](auto peer)
    {
        enet_peer_disconnect(peer, code);
    });
}

void Server::reset_all()
{
    iterate_peers(*host_, [this](auto peer)
    {
        destroy_client(*peer);
        enet_peer_reset(peer);
    });
}

std::size_t Server::connected_peers() const
{
    return host_->connectedPeers;
}

void Server::on_connect(ENetPeer& peer, std::uint32_t userdata)
{

}

void Server::on_disconnect(ENetPeer& peer, std::uint32_t userdata)
{

}

void Server::on_net_message(const ENetPacket& packet)
{

}

void Server::destroy_client(ENetPeer& peer)
{
    delete client(peer);
    client(peer) = nullptr;
}

Server::client_t*& Server::client(ENetPeer& peer)
{
    return *reinterpret_cast<client_t**>(&peer.data);
}

}
