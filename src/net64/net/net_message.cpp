//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//


#include "net64/client.hpp"
#include "net64/server.hpp"
#include "net_message.hpp"


namespace Net64
{
ClientSharedData ClientDataAccess::client_data(Client& client)
{
    return client.get_client_shared_data({});
}
ServerSharedData ServerDataAccess::server_data(Server& server)
{
    return server.get_server_shared_data({});
}
} // namespace Net64
