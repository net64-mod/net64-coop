//
// Created by henrik on 28.04.20.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <QPushButton>

#include "common/sdl_event_handler.hpp"
#include "net64/emulator/emulator.hpp"


namespace Frontend
{
struct SDL_BindButton : QPushButton, SDL_EventHandler
{
    Q_OBJECT

public:
    using sdl_input_t = Net64::Emulator::IControllerSettings::sdl_input_t;
    using N64Button = Net64::Emulator::IControllerSettings::N64Button;

    SDL_BindButton(QWidget* parent = nullptr);

    void set_joystick(SDL_JoystickID joy_id);

    void set_keyboard();

    void set_settings_handle(Net64::Emulator::IControllerSettings* handle);

    void load();

    void save();

    void set_bind(sdl_input_t input);

    sdl_input_t get_bind();

    void clear();


    N64Button n64_button{};

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void handle_sdl_event(const SDL_Event& event) override;

private:
    void update_text();

    sdl_input_t input_{};
    bool listening_{};
    SDL_JoystickID joy_id_{-1};
    Net64::Emulator::IControllerSettings* settings_handle_{};
};

} // namespace Frontend
