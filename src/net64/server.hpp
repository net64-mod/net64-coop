//
// Created by henrik on 01.10.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <chrono>
#include <map>
#include <memory>

#include <enet/enet.h>

#include "common/badge.hpp"
#include "common/id_manager.hpp"
#include "common/resource_handle.hpp"
#include "net64/logging.hpp"
#include "net64/net/net_message.hpp"
#include "net64/net/protocol.hpp"
#include "net64/server/player.hpp"


namespace Net64
{
struct Player;

struct ServerSharedData
{
    std::unordered_map<std::uintptr_t, std::unique_ptr<Player>>& players;
};

struct Server
{
    // Types
    using Clock = std::chrono::steady_clock;
    using peer_id_t = std::uintptr_t;
    using HostHandle = ResourceHandle<&enet_host_destroy>;
    using PacketHandle = ResourceHandle<&enet_packet_destroy>;

    //
    Server(std::uint16_t port, std::size_t max_peers);
    ~Server();
    Server(Server&& other) = default;
    Server& operator=(Server&& other) = default;

    // Update
    void tick();

    // Connection
    static void send(ENetPeer& peer, const INetMessage& msg);
    void broadcast(const INetMessage& msg);
    static void disconnect(ENetPeer& peer, Net::S_DisconnectCode reason);
    void disconnect_all(Net::S_DisconnectCode code);
    void reset_all();

    // Set properties
    void set_max_peers(std::size_t max_peers);
    void accept_new(bool v);

    // Get properties
    [[nodiscard]] std::size_t connected_peers() const;
    [[nodiscard]] std::size_t max_peers() const;
    [[nodiscard]] std::uint16_t port() const;
    [[nodiscard]] bool accept_new() const;


    ServerSharedData get_server_shared_data(Badge<ServerDataAccess>);
    Player& player(ENetPeer& peer);

    static constexpr std::size_t ALLOCATED_PEERS{32};

private:
    void destroy_player(ENetPeer& peer);

    // Events
    void on_connect(ENetPeer& peer);
    void on_disconnect(ENetPeer& peer, Net::C_DisconnectCode code);
    void on_net_message(const ENetPacket& packet, ENetPeer& sender);

    template<typename T>
    void add_component(T* component)
    {
        components_.push_back({component, [](void* component) { delete reinterpret_cast<T*>(component); }});

        if constexpr(std::is_base_of_v<ServerMessageHandler, T>)
            message_handlers_.push_back(component);
        if constexpr(std::is_base_of_v<ServerConnectionEventHandler, T>)
            connection_event_handlers_.push_back(component);
        if constexpr(std::is_base_of_v<ServerTickHandler, T>)
            tick_handlers_.push_back(component);
    }


    HostHandle host_;
    bool accept_new_{true};
    std::size_t max_peers_;

    std::unordered_map<std::uintptr_t, std::unique_ptr<Player>> players_;
    IdManager<std::uintptr_t> peer_ids_;

    // Server components
    struct ComponentPtr
    {
        void* component;
        void (*deleter)(void*);
    };
    std::vector<ComponentPtr> components_;
    std::vector<ServerMessageHandler*> message_handlers_;
    std::vector<ServerConnectionEventHandler*> connection_event_handlers_;
    std::vector<ServerTickHandler*> tick_handlers_;

    CLASS_LOGGER_("server")
};

} // namespace Net64
