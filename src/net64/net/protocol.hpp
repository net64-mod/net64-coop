//
// Created by henrik on 30.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <cstdint>
#include <system_error>

#include <enet/enet.h>


namespace Net64::Net
{
std::string format_ip4(ENetAddress addr);

constexpr std::uint32_t CONNECT_TIMEOUT{5000}, // ms
    DISCONNECT_TIMEOUT{3000},                  // ms
    CLIENT_SERVICE_WAIT{100},                  // ms
    SERVER_SERVICE_WAIT{100};                  // ms

constexpr std::uint32_t PROTO_VER{0x0};

using player_id_t = std::uint16_t;


enum struct Channel : std::uint8_t
{
    DEFAULT,
    META,

    COUNT
};

std::uint8_t channel_count();

inline std::array<std::uint32_t, static_cast<std::size_t>(Channel::COUNT)> CHANNEL_FLAGS{ENET_PACKET_FLAG_RELIABLE,
                                                                                         ENET_PACKET_FLAG_RELIABLE};

std::uint32_t channel_flags(Channel channel);

enum struct S_DisconnectCode
{
    KICKED = 1,
    BANNED,
    SERVER_SHUTDOWN,
    NOT_ACCEPTED,
    INCOMPATIBLE,
    PROTOCOL_VIOLATION,
    SERVER_FULL
};

enum struct C_DisconnectCode
{
    USER_DISCONNECT = 1
};

std::error_code make_error_code(Net64::Net::S_DisconnectCode e);

} // namespace Net64::Net

namespace std
{
template<>
struct is_error_code_enum<Net64::Net::S_DisconnectCode> : std::true_type
{
};

} // namespace std
