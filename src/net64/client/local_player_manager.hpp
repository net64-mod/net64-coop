//
// Created by henrik on 15.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include "net64/logging.hpp"
#include "net64/net/messages_server.hpp"

namespace Net64
{
struct LocalPlayerManager :
    ClientMessageHandler::Derive<LocalPlayerManager>::Receive<Net::S_Handshake>,
    ClientConnectionEventHandler
{
    void on_connect(Client& client) final;
    void on_disconnect(Client& client) final;

    void on_message(const Net::S_Handshake& msg, Client& client);

    CLASS_LOGGER_("client")
};

} // namespace Net64
