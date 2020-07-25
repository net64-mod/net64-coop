//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "local_player.hpp"


namespace Net64
{
bool LocalPlayer::handshaked() const
{
    return id != 0;
}
} // namespace Net64
