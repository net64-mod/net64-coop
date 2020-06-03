//
// Created by henrik on 28.04.20.
// Copyright 2020 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include <unordered_map>

#include <QInputEvent>
#include <SDL_joystick.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>

#include "sdl_bind_button.hpp"


namespace Frontend
{
extern const std::unordered_map<Qt::Key, SDL_Keycode> QT_SDL_KEYMAP;

SDL_BindButton::SDL_BindButton(QWidget* parent): QPushButton(parent)
{
}

void SDL_BindButton::set_joystick(SDL_JoystickID joy_id)
{
    joy_id_ = joy_id;
}

void SDL_BindButton::set_keyboard()
{
    joy_id_ = -1;
}

void SDL_BindButton::set_settings_handle(Net64::Emulator::IControllerSettings* handle)
{
    settings_handle_ = handle;
}

void SDL_BindButton::load()
{
    if(!settings_handle_)
        return;

    if(!settings_handle_->get_bind(n64_button, input_))
        input_.type = sdl_input_t::Type::NONE;

    set_bind(input_);
}

void SDL_BindButton::save()
{
    if(!settings_handle_)
        return;

    settings_handle_->set_bind(n64_button, input_);
}

void SDL_BindButton::set_bind(sdl_input_t input)
{
    input_ = input;

    listening_ = false;

    update_text();
    save();
}

auto SDL_BindButton::get_bind() -> sdl_input_t
{
    return input_;
}

void SDL_BindButton::clear()
{
    input_.type = sdl_input_t::Type::NONE;
    set_bind(input_);

    update_text();
}

void SDL_BindButton::keyPressEvent(QKeyEvent* event)
{
    QPushButton::keyPressEvent(event);

    if(!listening_)
        return;

    if(joy_id_ >= 0)
    {
        listening_ = false;
        return;
    }

    input_.type = sdl_input_t::Type::KEY;
    try
    {
        input_.key_code = QT_SDL_KEYMAP.at((Qt::Key)event->key());
    }
    catch(const std::out_of_range&)
    {
        return;
    }

    set_bind(input_);
}

void SDL_BindButton::mouseReleaseEvent(QMouseEvent* event)
{
    QPushButton::mouseReleaseEvent(event);

    if(!listening_)
    {
        if(event->button() == Qt::RightButton)
            clear();
        else if(event->button() == Qt::LeftButton)
            listening_ = true;
    }
    else
    {
        if(joy_id_ >= 0 && event->button() == Qt::LeftButton)
            listening_ = false;
        else if(joy_id_ < 0)
        {
            input_.type = sdl_input_t::Type::MOUSE_BUTTON;
            switch(event->button())
            {
            case Qt::LeftButton:
                input_.mouse_button = SDL_BUTTON_LEFT;
                break;
            case Qt::RightButton:
                input_.mouse_button = SDL_BUTTON_RIGHT;
                break;
            case Qt::MidButton:
                input_.mouse_button = SDL_BUTTON_MIDDLE;
                break;
            }
            set_bind(input_);
        }
    }

    update_text();
}

void SDL_BindButton::handle_sdl_event(const SDL_Event& event)
{
    if(!listening_)
        return;

    switch(event.type)
    {
    case SDL_JOYBUTTONDOWN:
        if(joy_id_ != event.jbutton.which)
            return;
        input_.type = sdl_input_t::Type::JOY_BUTTON;
        input_.joy.index = event.jbutton.button;
        set_bind(input_);
        break;
    case SDL_JOYHATMOTION:
        if(joy_id_ != event.jhat.which)
            return;
        if(event.jhat.value == 0)
            return;
        input_.type = sdl_input_t::Type::JOY_HAT;
        input_.joy.index = event.jhat.hat;
        input_.joy.value = event.jhat.value;
        set_bind(input_);
        break;
    case SDL_JOYAXISMOTION:
        if(joy_id_ != event.jaxis.which)
            return;
        if(std::abs(event.jaxis.value) < 9830) // Register axis event only when moved 30%+
            return;
        input_.type = sdl_input_t::Type::JOY_AXIS;
        input_.joy.index = event.jaxis.axis;
        input_.joy.value = (event.jaxis.value > 0 ? 1 : -1);
        set_bind(input_);
        break;
    }
}

void SDL_BindButton::update_text()
{
    if(listening_)
    {
        setText("...");
        return;
    }

    auto hat_dir{[](int val) -> std::string {
        if(val & SDL_HAT_UP)
            return "UP";
        if(val & SDL_HAT_DOWN)
            return "DOWN";
        if(val & SDL_HAT_LEFT)
            return "LEFT";
        if(val & SDL_HAT_RIGHT)
            return "RIGHT";
        else
            return "CENTERED";
    }};

    std::string label;

    switch(input_.type)
    {
    case sdl_input_t::Type::NONE:
        label = "";
        break;
    case sdl_input_t::Type::KEY:
        label = "Key(" + std::string(SDL_GetKeyName(input_.key_code)) + ")";
        break;
    case sdl_input_t::Type::JOY_AXIS:
        label = "Axis(" + std::to_string(input_.joy.index) + (input_.joy.value > 0 ? '+' : '-') + std::string(")");
        break;
    case sdl_input_t::Type::JOY_BUTTON:
        label = "Button(" + std::to_string(input_.joy.index) + ")";
        break;
    case sdl_input_t::Type::JOY_HAT:
        label = "Hat(" + std::to_string(input_.joy.index) + ' ' + hat_dir(input_.joy.value) + ")";
        break;
    case sdl_input_t::Type::MOUSE_BUTTON:
        label = "Mouse(" + std::to_string(input_.mouse_button) + ")";
        break;
    }

    setText(QString::fromStdString(label));
}

const std::unordered_map<Qt::Key, SDL_Keycode> QT_SDL_KEYMAP{{Qt::Key_A, SDLK_a},
                                                             {Qt::Key_B, SDLK_b},
                                                             {Qt::Key_C, SDLK_c},
                                                             {Qt::Key_D, SDLK_d},
                                                             {Qt::Key_E, SDLK_e},
                                                             {Qt::Key_F, SDLK_f},
                                                             {Qt::Key_G, SDLK_g},
                                                             {Qt::Key_H, SDLK_h},
                                                             {Qt::Key_I, SDLK_i},
                                                             {Qt::Key_J, SDLK_j},
                                                             {Qt::Key_K, SDLK_k},
                                                             {Qt::Key_L, SDLK_l},
                                                             {Qt::Key_M, SDLK_m},
                                                             {Qt::Key_N, SDLK_n},
                                                             {Qt::Key_O, SDLK_o},
                                                             {Qt::Key_P, SDLK_p},
                                                             {Qt::Key_Q, SDLK_q},
                                                             {Qt::Key_R, SDLK_r},
                                                             {Qt::Key_S, SDLK_s},
                                                             {Qt::Key_T, SDLK_t},
                                                             {Qt::Key_U, SDLK_u},
                                                             {Qt::Key_V, SDLK_v},
                                                             {Qt::Key_W, SDLK_w},
                                                             {Qt::Key_X, SDLK_x},
                                                             {Qt::Key_Y, SDLK_y},
                                                             {Qt::Key_Z, SDLK_z},
                                                             {Qt::Key_0, SDLK_0},
                                                             {Qt::Key_1, SDLK_1},
                                                             {Qt::Key_2, SDLK_2},
                                                             {Qt::Key_3, SDLK_3},
                                                             {Qt::Key_4, SDLK_4},
                                                             {Qt::Key_5, SDLK_5},
                                                             {Qt::Key_6, SDLK_6},
                                                             {Qt::Key_7, SDLK_7},
                                                             {Qt::Key_8, SDLK_8},
                                                             {Qt::Key_9, SDLK_9},
                                                             {Qt::Key_F1, SDLK_F1},
                                                             {Qt::Key_F2, SDLK_F2},
                                                             {Qt::Key_F3, SDLK_F3},
                                                             {Qt::Key_F4, SDLK_F4},
                                                             {Qt::Key_F5, SDLK_F5},
                                                             {Qt::Key_F6, SDLK_F6},
                                                             {Qt::Key_F7, SDLK_F7},
                                                             {Qt::Key_F8, SDLK_F8},
                                                             {Qt::Key_F9, SDLK_F9},
                                                             {Qt::Key_F10, SDLK_F10},
                                                             {Qt::Key_F11, SDLK_F11},
                                                             {Qt::Key_F12, SDLK_F12},
                                                             {Qt::Key_F13, SDLK_F13},
                                                             {Qt::Key_F14, SDLK_F14},
                                                             {Qt::Key_F15, SDLK_F15},
                                                             {Qt::Key_Insert, SDLK_INSERT},
                                                             {Qt::Key_Delete, SDLK_DELETE},
                                                             {Qt::Key_Home, SDLK_HOME},
                                                             {Qt::Key_End, SDLK_END},
                                                             {Qt::Key_PageUp, SDLK_PAGEUP},
                                                             {Qt::Key_PageDown, SDLK_PAGEDOWN},
                                                             {Qt::Key_Up, SDLK_UP},
                                                             {Qt::Key_Down, SDLK_DOWN},
                                                             {Qt::Key_Left, SDLK_LEFT},
                                                             {Qt::Key_Right, SDLK_RIGHT},
                                                             {Qt::Key_Return, SDLK_RETURN},
                                                             {Qt::Key_Enter, SDLK_RETURN2},
                                                             {Qt::Key_Escape, SDLK_ESCAPE},
                                                             {Qt::Key_Pause, SDLK_PAUSE},
                                                             {Qt::Key_QuoteLeft, SDLK_QUOTEDBL},
                                                             {Qt::Key_Backspace, SDLK_BACKSPACE},
                                                             {Qt::Key_Tab, SDLK_TAB},
                                                             {Qt::Key_CapsLock, SDLK_CAPSLOCK},
                                                             {Qt::Key_Space, SDLK_SPACE},
                                                             {Qt::Key_Slash, SDLK_SLASH},
                                                             {Qt::Key_Backslash, SDLK_BACKSLASH},
                                                             {Qt::Key_Minus, SDLK_MINUS},
                                                             {Qt::Key_Plus, SDLK_UNKNOWN},
                                                             {Qt::Key_Equal, SDLK_EQUALS},
                                                             {Qt::Key_BracketLeft, SDLK_LEFTBRACKET},
                                                             {Qt::Key_BracketRight, SDLK_RIGHTBRACKET},
                                                             {Qt::Key_Semicolon, SDLK_SEMICOLON},
                                                             {Qt::Key_Apostrophe, SDLK_QUOTE},
                                                             {Qt::Key_Comma, SDLK_COMMA},
                                                             {Qt::Key_Period, SDLK_PERIOD},
                                                             {Qt::Key_Alt, SDLK_LALT},
                                                             {Qt::Key_Control, SDLK_LCTRL},
                                                             {Qt::Key_Shift, SDLK_LSHIFT},
                                                             {Qt::Key_Print, SDLK_PRINTSCREEN},
                                                             {Qt::Key_ScrollLock, SDLK_SCROLLLOCK},
                                                             {Qt::Key_Meta, SDLK_LGUI},
                                                             {Qt::Key_Super_L, SDLK_LGUI},
                                                             {Qt::Key_Super_R, SDLK_RGUI},
                                                             {Qt::Key_unknown, SDLK_UNKNOWN}};

} // namespace Frontend
