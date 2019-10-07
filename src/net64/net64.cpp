//
// Created by henrik on 07.10.19.
// Copyright 2019 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "net64.hpp"


namespace Net64
{

bool initialize()
{
    if(enet_initialize() != 0)
    {
        get_logger("root")->critical("Failed to initialize ENet library");
        return false;
    }

    return true;
}

void deinitialize()
{
    enet_deinitialize();
}

} // Net64
