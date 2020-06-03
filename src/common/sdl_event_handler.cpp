//
// Created by henrik on 28.04.20.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "sdl_event_handler.hpp"


SDL_EventHandler::SDL_EventHandler()
{
    register_obj(*this);
}

SDL_EventHandler::~SDL_EventHandler()
{
    unregister_obj(*this);
}

void SDL_EventHandler::poll_events()
{
    SDL_Event event{};
    while(SDL_PollEvent(&event))
    {
        dispatch_event(event);
    }
}

void SDL_EventHandler::register_obj(SDL_EventHandler& obj)
{
    std::scoped_lock g(list_mutex_s);

    if(first_handler_s)
    {
        first_handler_s->prev_handler = &obj;
        obj.next_handler = first_handler_s;
    }
    first_handler_s = &obj;
}

void SDL_EventHandler::unregister_obj(SDL_EventHandler& obj)
{
    std::scoped_lock g(list_mutex_s);

    if(obj.prev_handler)
        obj.prev_handler->next_handler = obj.next_handler;
    if(obj.next_handler)
        obj.next_handler->prev_handler = obj.prev_handler;
}

void SDL_EventHandler::dispatch_event(const SDL_Event& event)
{
    std::unique_lock g(list_mutex_s);

    auto next_handler{first_handler_s};

    while(next_handler)
    {
        next_handler->handle_sdl_event(event);
        next_handler = next_handler->next_handler;
    }
}
