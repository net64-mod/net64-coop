//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <memory>
#include <enet/enet.h>
#include <common/deleter.hpp>
#include "net64/net/errors.hpp"
#include "net64/net/protocol.hpp"
#include "net64/game/msg_queue.hpp"
#include "net64/game/net64_header.hpp"
#include "net64/memory/pointer.hpp"
#include "types.hpp"


namespace Net64
{

struct Client
{
    using HostHandle = std::unique_ptr<ENetHost, Deleter<&enet_host_destroy>>;
    using PeerHandle = std::unique_ptr<ENetPeer, Deleter<&enet_peer_reset>>;

    explicit Client(Memory::MemHandle mem_hdl);

    std::error_code connect(const char* ip, std::uint16_t port);
    void disconnect();

    void tick();

    [[nodiscard]]
    bool connected() const;

    [[nodiscard]]
    std::uint32_t disconnect_code() const;

    static bool game_initialized(Memory::MemHandle mem_hdl);

private:
    void on_connect();
    void on_disconnect();
    void on_net_message(const ENetPacket& packet);
    void on_game_message(const Game::MsgQueue::n64_message_t& message);

    Memory::MemHandle mem_hdl_;
    Memory::Ptr<Game::net64_header_t> net64_header_;
    Game::MsgQueue::Receiver rcv_queue_;
    Game::MsgQueue::Sender snd_queue_;

    HostHandle host_;
    PeerHandle peer_;
    std::uint32_t disconnect_code_{};
};

} // Net64
