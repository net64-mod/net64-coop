//
// Created by henrik on 04.10.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "errors.hpp"


namespace
{

const struct Net64NetErrorCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "net64_net_error";
    }

    std::string message(int ev) const override
    {
        using namespace Net64::Net;

        switch(static_cast<Error>(ev))
        {
        case Error::INVALID_IP:
            return "Failed to connect to IP";
        case Error::UNKOWN_HOSTNAME:
            return "Could not resolve hostname";
        case Error::ALREADY_CONNECTED:
            return "Client already connected";
        case Error::ENET_HOST_CREATION:
            return "Failed to create ENet host";
        case Error::ENET_PEER_CREATION:
            return "Failed to create ENet peer";
        case Error::TIMED_OUT:
            return "Connection timed out";
        }

        return "[Unkown Error]";
    }

}g_net64_net_error_category;

}

namespace Net64::Net
{

std::error_code make_error_code(Net64::Net::Error e)
{
    return {static_cast<int>(e), g_net64_net_error_category};
}

};
