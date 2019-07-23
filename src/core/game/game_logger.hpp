//
// Created by henrik on 12.07.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <string>
#include "core/game/message_listener.hpp"
#include "core/logging.hpp"


namespace Core::Game
{

/**
 * Allows SM64 to send log messages into the client's console
 */
struct GameLogger : MessageListener
{
    GameLogger();

private:
    void handle_message(const n64_message_t& message) override;

    std::string text_buf_;

    CLASS_LOGGER_("SM64")
};

}
