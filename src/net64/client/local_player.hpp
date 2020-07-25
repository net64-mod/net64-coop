//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "net64/net/protocol.hpp"


namespace Net64
{
struct LocalPlayer
{
    bool handshaked() const;

    std::string name;
    Net::player_id_t id{};
};

} // namespace Net64
