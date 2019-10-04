//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include <net64/net/errors.hpp>
#include "client.hpp"

namespace Net64
{

Client::Client(Memory::MemHandle mem_hdl):
    mem_hdl_{mem_hdl},
    net64_header_{mem_hdl_, mem_hdl_.read<n64_addr_t>(Game::FixedAddr::HEADER_PTR)},
    rcv_queue_{net64_header_->field(&Game::net64_header_t::receive_queue)},
    snd_queue_{net64_header_->field(&Game::net64_header_t::send_queue)},
    host_{enet_host_create(nullptr, 1, Net::CHANNEL_COUNT, 0, 0)}
{
    if(!host_)
        throw std::system_error(make_error_code(Net::Error::ENET_HOST_CREATION));
}

Client::~Client()
{
    peer_.reset();
    host_.reset();
}

std::error_code Client::connect(const char* ip, std::uint16_t port)
{
    if(peer_)
        return make_error_code(Net::Error::ALREADY_CONNECTED);

    disconnect_code_ = 0;

    ENetAddress addr;
    // @todo: check if ip is a domain or ip address
    if(enet_address_set_host(&addr, ip) != 0)
    {
        return make_error_code(Net::Error::UNKOWN_HOSTNAME);
    }
    addr.port = port;

    PeerHandle peer{enet_host_connect(host_.get(), &addr, Net::CHANNEL_COUNT, Net::PROTO_VER)};
    if(!peer)
    {
        return make_error_code(Net::Error::ENET_PEER_CREATION);
    }

    ENetEvent evt;
    if(enet_host_service(host_.get(), &evt, Net::CONNECT_TIMEOUT) > 0 &&
       evt.type == ENET_EVENT_TYPE_CONNECT)
    {
        peer_ = std::move(peer);
        on_connect();

        return {0, std::generic_category()};
    }

    return make_error_code(Net::Error::TIMED_OUT);
}

void Client::disconnect()
{
    if(!peer_)
        return;

    enet_peer_disconnect(peer_.get(), 0);

    ENetEvent evt;
    while(enet_host_service(host_.get(), &evt, Net::DISCONNECT_TIMEOUT) > 0)
    {
        switch(evt.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(evt.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            // ENet already took care of deallocation
            (void)peer_.release();
            return;
        }
    }

    // Force disconnect
    peer_.reset();

    on_disconnect();
}

void Client::tick()
{
    if(peer_)
    {
        for(ENetEvent evt; enet_host_service(host_.get(), &evt, Net::CLIENT_SERVICE_WAIT) > 0;)
        {
            switch(evt.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                on_net_message(*evt.packet);
                enet_packet_destroy(evt.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                // ENet already took care of deallocation
                (void)peer_.release();
                disconnect_code_ = evt.data;
                break;
            }
        }
    }

    for(Game::MsgQueue::n64_message_t msg{}; rcv_queue_.poll(msg);)
    {
        on_game_message(msg);
    }
}

bool Client::connected() const
{
    return (peer_ != nullptr);
}

std::uint32_t Client::disconnect_code() const
{
    return disconnect_code_;
}

bool Client::game_initialized(Memory::MemHandle mem_hdl)
{
    return (mem_hdl.read<u32>(Game::FixedAddr::MAGIC_NUMBER) == Game::MAGIC_NUMBER);
}

void Client::on_connect()
{

}

void Client::on_disconnect()
{

}

void Client::on_net_message(const ENetPacket& packet)
{

}

void Client::on_game_message(const Game::MsgQueue::n64_message_t& message)
{

}

}
