//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include <cstring>

#include "game_logger.hpp"


namespace Net64::Game
{
static std::size_t strnlen(const char* str, std::size_t max_len)
{
    std::size_t len{};
    for(; len < max_len && str[len] != '\0'; ++len)
    {
    }
    return len;
}


Logger::Logger(): MessageHandler{Message::LOG_APPEND, Message::LOG_END}
{
}

void Logger::handle_message(Client&, const n64_message_t& message)
{
    switch(message.msg_type)
    {
    case Message::LOG_APPEND:
        text_buf_ += std::string(reinterpret_cast<const char*>(message.msg_data),
                                 strnlen(reinterpret_cast<const char*>(message.msg_data), MSG_DATA_LEN));
        break;
    case Message::LOG_END:
        text_buf_ += std::string(reinterpret_cast<const char*>(message.msg_data),
                                 strnlen(reinterpret_cast<const char*>(message.msg_data), MSG_DATA_LEN));
        logger()->info(text_buf_);
        text_buf_.clear();
        break;
    default:
        break;
    }
}

} // namespace Net64::Game
