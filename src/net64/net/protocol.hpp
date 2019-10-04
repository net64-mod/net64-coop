//
// Created by henrik on 30.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <cstdint>
#include <system_error>


namespace Net64::Net
{

constexpr std::size_t CHANNEL_COUNT{1};
constexpr std::uint32_t CONNECT_TIMEOUT{5000}, // ms
                        DISCONNECT_TIMEOUT{3000}, // ms
                        CLIENT_SERVICE_WAIT{100}, // ms
                        SERVER_SERVICE_WAIT{100}; // ms

constexpr std::uint32_t PROTO_VER{0x0};

struct S_DisconnectCode
{
    enum _s_disconnect_code_enum : std::uint32_t
    {
        KICKED = 1,
        BANNED,
        SERVER_SHUTDOWN,
        NOT_ACCEPTED,
        INCOMPATIBLE
    };
};

struct C_DisconnectCode
{
    enum : std::uint32_t
    {
        USER_DISCONNECT = 1
    };
};

std::error_code make_error_code(Net64::Net::S_DisconnectCode::_s_disconnect_code_enum e);

}

namespace std
{

template<>
struct is_error_code_enum<Net64::Net::S_DisconnectCode::_s_disconnect_code_enum> : std::true_type{};

}
