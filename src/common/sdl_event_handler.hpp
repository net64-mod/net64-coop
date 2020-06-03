//
// Created by henrik on 28.04.20.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <mutex>
#include <SDL_events.h>


struct SDL_EventHandler
{
    SDL_EventHandler();

    virtual ~SDL_EventHandler();

    static void poll_events();

protected:
    virtual void handle_sdl_event(const SDL_Event&){}

private:
    static void register_obj(SDL_EventHandler& obj);
    static void unregister_obj(SDL_EventHandler& obj);

    static void dispatch_event(const SDL_Event& event);

    SDL_EventHandler* prev_handler{};
    SDL_EventHandler* next_handler{};

    inline static SDL_EventHandler* first_handler_s{};
    inline static std::mutex list_mutex_s;
};
