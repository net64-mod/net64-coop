//
// Created by henrik on 05.06.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once


namespace Net64
{
enum struct NetMessageId
{
    CLIENT_HANDSHAKE,
    SERVER_HANDSHAKE,
    SERVER_CHAT_MESSAGE, ///< Chat message sent by server to client
    CLIENT_CHAT_MESSAGE, ///< Chat message from client to server
    SERVER_PLAYER_LIST,
    SERVER_CLIENT_DISCONNECTED,
    SERVER_CLIENT_CONNECTED,

    COUNT
};

}
