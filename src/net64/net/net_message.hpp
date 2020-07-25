//
// Created by henrik on 05.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <enet/enet.h>

#include "common/message_handler.hpp"
#include "common/message_interface.hpp"
#include "net64/net/message_ids.hpp"
#include "net64/net/protocol.hpp"


#define NET_MESSAGE_(name, id, channel) struct name : NetMessage<name, id, channel>
#define NET_SERIALIZE_(...) \
    template<typename T>    \
    void serialize(T& a)    \
    {                       \
        a(__VA_ARGS__);     \
    }

namespace Net64
{
struct Server;
struct Client;
struct Player;
struct LocalPlayer;
struct ClientSharedData;
struct ServerSharedData;

struct INetMessage : IMessage<INetMessage, NetMessageId>
{
    explicit INetMessage(Net::Channel channel): channel_(channel) {}

    Net::Channel channel() const { return channel_; }

private:
    Net::Channel channel_;
};

template<typename Derived, NetMessageId ID, Net::Channel CHANNEL>
struct NetMessage : INetMessage::Derive<Derived, ID>
{
    friend Derived;

private:
    NetMessage(): INetMessage::Derive<Derived, ID>(CHANNEL) {}
};

struct ServerDataAccess
{
protected:
    static ServerSharedData server_data(Server& server);
};

using ServerMessageHandler = IMessageHandler<INetMessage, Server&, Player&>::Base<ServerDataAccess>;

struct ClientDataAccess
{
protected:
    static ClientSharedData client_data(Client& client);
};

using ClientMessageHandler = IMessageHandler<INetMessage, Client&>::Base<ClientDataAccess>;

struct ServerConnectionEventHandler
{
    virtual ~ServerConnectionEventHandler() = default;

    virtual void on_connect(Server&, Player&) {}
    virtual void on_disconnect(Server&, Player&, Net::C_DisconnectCode) {}
};

struct ClientConnectionEventHandler : ClientDataAccess
{
    virtual ~ClientConnectionEventHandler() = default;

    virtual void on_connect(Client&) {}
    virtual void on_disconnect(Client&) {}
};

struct ServerTickHandler
{
    virtual ~ServerTickHandler() = default;

    virtual void on_tick(Server& server) = 0;
};

struct ClientTickHandler : ClientDataAccess
{
    virtual ~ClientTickHandler() = default;

    virtual void on_tick(Client& client) = 0;
};

} // namespace Net64
