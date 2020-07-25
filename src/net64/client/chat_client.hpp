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
struct ChatClient :
    ClientMessageHandler::Derive<
        ChatClient>::Receive<Net::S_ChatMessage, Net::S_ClientConnected, Net::S_ClientDisconnected>
{
    using ChatCallback = std::function<void(const std::string& sender, const std::string& msg)>;

    void set_chat_callback(ChatCallback fn);

    void send(Client& client, std::string message);

    void on_message(const Net::S_ChatMessage& msg, Client& client);

    void on_message(const Net::S_ClientConnected& msg, Client& client);

    void on_message(const Net::S_ClientDisconnected& msg, Client& client);

private:
    ChatCallback callback_;

    CLASS_LOGGER_("client")
};

} // namespace Net64
