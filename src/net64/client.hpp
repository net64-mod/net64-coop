//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <memory>
#include <unordered_map>

#include <enet/enet.h>

#include "common/badge.hpp"
#include "common/resource_handle.hpp"
#include "net/net_message.hpp"
#include "net64/client/chat_client.hpp"
#include "net64/client/local_player.hpp"
#include "net64/client/message_handler.hpp"
#include "net64/client/msg_queue.hpp"
#include "net64/client/net64_header.hpp"
#include "net64/client/remote_player.hpp"
#include "net64/memory/pointer.hpp"
#include "net64/net/errors.hpp"
#include "net64/net/messages_server.hpp"
#include "net64/net/protocol.hpp"
#include "types.hpp"


namespace Net64
{
enum struct ClientError
{
    INCOMPATIBLE_GAME = 1
};

std::error_code make_error_code(Net64::ClientError e);

} // namespace Net64

namespace std
{
template<>
struct is_error_code_enum<Net64::ClientError> : std::true_type
{
};

} // namespace std

namespace Net64
{
struct ClientSharedData
{
    std::optional<Memory::MemHandle> mem_hdl;
    std::optional<Game::MsgQueue::Sender>& send_queue;
    LocalPlayer& local_player;
    std::unordered_map<Net::player_id_t, RemotePlayer>& remote_players;
};

struct Client
{
    enum struct ConnectionState
    {
        INITIAL,
        HANDSHAKED,
        IN_GAME
    };

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    using HostHandle = ResourceHandle<&enet_host_destroy>;
    using PeerHandle = ResourceHandle<&enet_peer_reset>;
    using PacketHandle = ResourceHandle<&enet_packet_destroy>;

    using HookedCallback = std::function<void(std::error_code)>;
    using ConnectCallback = std::function<void(std::error_code)>;
    using DisconnectCallback = std::function<void(std::error_code)>;
    using ChatCallback = ChatClient::ChatCallback;

    Client();

    Client(Client&&) = default;
    Client& operator=(Client&&) = default;

    ~Client();

    void set_chat_callback(ChatCallback fn);

    void hook(Memory::MemHandle mem_hdl, HookedCallback fn);
    void connect(const char* ip,
                 std::uint16_t port,
                 std::string username,
                 std::chrono::seconds timeout,
                 ConnectCallback connect_callback,
                 DisconnectCallback disconnect_callback);
    void disconnect(std::chrono::seconds timeout);
    void unhook();

    void send(const INetMessage& msg);
    void send_chat_message(std::string msg);

    void tick();

    ClientSharedData get_client_shared_data(Badge<ClientDataAccess>);

    [[nodiscard]] bool hooked() const;
    [[nodiscard]] bool connecting() const;
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool disconnecting() const;
    [[nodiscard]] bool disconnected() const;

    std::string username() const;
    const std::unordered_map<Net::player_id_t, RemotePlayer>& remote_players() const;

private:
    void try_hook();
    void on_connect();
    void on_disconnect();
    void on_net_message(const ENetPacket& packet);
    void on_game_message(const Game::MsgQueue::n64_message_t& message);

    template<typename T>
    void add_component(T* component)
    {
        components_.push_back({component, [](void* component) { delete reinterpret_cast<T*>(component); }});

        if constexpr(std::is_base_of_v<ClientMessageHandler, T>)
            net_message_handlers_.push_back(component);
        if constexpr(std::is_base_of_v<ClientConnectionEventHandler, T>)
            connection_event_handlers_.push_back(component);
        if constexpr(std::is_base_of_v<ClientTickHandler, T>)
            tick_handlers_.push_back(component);
        if constexpr(std::is_base_of_v<Game::MessageHandler, T>)
            game_message_handlers_.push_back(component);
    }

    // SM64
    std::optional<Memory::MemHandle> mem_hdl_;
    Memory::Ptr<Game::net64_header_t> net64_header_;
    std::optional<Game::MsgQueue::Receiver> rcv_queue_;
    std::optional<Game::MsgQueue::Sender> snd_queue_;
    bool hooking_{};

    // ENet
    HostHandle host_;
    PeerHandle peer_;

    // Time
    TimePoint connection_start_time_;
    std::chrono::seconds connect_timeout_;
    TimePoint disconnection_start_time_;
    std::chrono::seconds disconnect_timeout_;

    // Callbacks
    HookedCallback hooked_callback_;
    ConnectCallback connect_callback_;
    DisconnectCallback disconnect_callback_;

    LocalPlayer player_;
    std::unordered_map<Net::player_id_t, RemotePlayer> remote_players_;

    std::string username_;

    // Components
    struct ComponentPtr
    {
        void* component;
        void (*deleter)(void*);
    };
    std::vector<ComponentPtr> components_;
    std::vector<ClientMessageHandler*> net_message_handlers_;
    std::vector<Game::MessageHandler*> game_message_handlers_;
    std::vector<ClientConnectionEventHandler*> connection_event_handlers_;
    std::vector<ClientTickHandler*> tick_handlers_;

    ChatClient* chat_client_{};

    CLASS_LOGGER_("client")
};

} // namespace Net64
