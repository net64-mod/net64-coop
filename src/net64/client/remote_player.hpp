//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <string>

#include "net64/net/protocol.hpp"


namespace Net64
{
struct RemotePlayer
{
    Net::player_id_t id{};
    std::string name;
};

} // namespace Net64
