//
// Created by henrik on 04.10.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <system_error>


namespace Net64::Net
{

enum struct Error
{
    INVALID_IP = 1,
    UNKOWN_HOSTNAME,
    ALREADY_CONNECTED,
    ENET_PEER_CREATION,
    ENET_HOST_CREATION,
    TIMED_OUT
};

std::error_code make_error_code(Net64::Net::Error e);

} // Net64::Net

namespace std
{

template<>
struct is_error_code_enum<Net64::Net::Error> : std::true_type{};

}
