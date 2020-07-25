//
// Created by henrik on 05.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <cereal/types/string.hpp>

#include "net64/net/net_message.hpp"


namespace Net64::Net
{
NET_MESSAGE_(C_Handshake, NetMessageId::CLIENT_HANDSHAKE, Channel::META)
{
    std::string name, client_version_str;

    NET_SERIALIZE_(name, client_version_str)
};

NET_MESSAGE_(C_ChatMessage, NetMessageId::CLIENT_CHAT_MESSAGE, Channel::META)
{
    std::string message;

    NET_SERIALIZE_(message)
};

} // namespace Net64::Net
