//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"

#include <algorithm>
#include <SDL_joystick.h>
#include <net64/error_codes.hpp>
#include "net64/memory/util.hpp"


namespace Net64::Emulator
{

namespace
{

bool failed(Mupen64Plus::Error err)
{
    return err != Mupen64Plus::Error::SUCCESS;
}

const std::array<const char*, 7> FORBIDDEN_HOTKEYS{
    "Kbd Mapping Load State", "Kbd Mapping Speed Down",
    "Kbd Mapping Speed Up", "Kbd Mapping Pause",
    "Kbd Mapping Fast Forward", "Kbd Mapping Frame Advance",
    "Kbd Mapping Gameshark"
};

}

struct Mupen64PlusAudioSettings : IAudioSettings
{
    explicit Mupen64PlusAudioSettings(M64PlusHelper::Core& core):
        core_(&core)
    {}

    bool get_volume(float& vol) override
    {
        try
        {
            auto config{core_->config()};

            auto section{config.open_section("Audio-SDL")};

            vol = static_cast<float>(section.get_int("VOLUME_DEFAULT")) / 100;
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_volume(float vol) override
    {
        // @todo: Adjust volume in realtime with AudioPlugin::VolumeSetLevel(int)

        float tmp{};
        if(!get_volume(tmp))
            return false;

        try
        {
            auto config{core_->config()};

            auto section{config.open_section("Audio-SDL")};

            section.set("VOLUME_DEFAULT", static_cast<int>(vol * 100));

            config.save("Audio-SDL");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

private:
    M64PlusHelper::Core* core_;
};

struct Mupen64PlusVideoSettings : IVideoSettings
{
    explicit Mupen64PlusVideoSettings(M64PlusHelper::Core& core):
        core_(&core)
    {}

    bool get_fullscreen(bool& fullscreen) override
    {
        try
        {
            auto section{core_->config().open_section("Video-General")};

            fullscreen = section.get_bool("Fullscreen");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_dimensions(unsigned& width, unsigned& height) override
    {
        try
        {
            auto section{core_->config().open_section("Video-General")};

            width = static_cast<unsigned>(section.get_int("ScreenWidth"));
            height = static_cast<unsigned>(section.get_int("ScreenHeight"));
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_vsync(bool& vsync) override
    {
        try
        {
            auto section{core_->config().open_section("Video-General")};

            vsync = section.get_bool("VerticalSync");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_fullscreen(bool fullscreen) override
    {
        bool tmp{};
        if(!get_fullscreen(tmp))
            return false;

        try
        {
            auto section{core_->config().open_section("Video-General")};

            section.set("Fullscreen", fullscreen);

            core_->config().save("Video-General");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_dimensions(unsigned width, unsigned height) override
    {
        unsigned tmp1{}, tmp2{};
        if(!get_dimensions(tmp1, tmp2))
            return false;

        try
        {
            auto section{core_->config().open_section("Video-General")};

            section.set("ScreenWidth", (int)width);
            section.set("ScreenHeight", (int)height);

            core_->config().save("Video-General");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_vsync(bool vsync) override
    {
        bool tmp{};
        if(!get_vsync(tmp))
            return false;

        try
        {
            auto section{core_->config().open_section("Video-General")};

            section.set("VerticalSync", vsync);

            core_->config().save("Video-General");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

private:
    M64PlusHelper::Core* core_;
};

struct Mupen64PlusControllerSettings : IControllerSettings
{
    explicit Mupen64PlusControllerSettings(M64PlusHelper::Core& core, unsigned short joy_index):
        core_{&core}, joy_index_{joy_index}
    {}

    bool get_device(std::string& device, int& device_index) override
    {
        try
        {
            auto section{open_section()};

            device = section.get_string("name");
            device_index = section.get_int("device");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_pak(ControllerPak& pak) override
    {
        try
        {
            auto section{open_section()};

            switch(section.get_int("plugin"))
            {
            case 1:
                pak = ControllerPak::NONE;
                break;
            case 2:
                pak = ControllerPak::MEM_PAK;
                break;
            case 5:
                pak = ControllerPak::RUMBLE_PAK;
                break;
            default:
                return false;
            }
        }
        catch(const std::system_error& e)
        {
            get_logger("mupen64plus")->error(e.what());
            return false;
        }

        return true;
    }

    bool get_config_mode(ConfigurationMode& mode) override
    {
        try
        {
            auto section{open_section()};

            switch(section.get_int("mode"))
            {
            case 0:
                mode = ConfigurationMode::MANUAL;
                break;
            case 1:
                mode = ConfigurationMode::AUTO_NAMED_SDL_DEV;
                break;
            case 2:
                mode = ConfigurationMode::AUTOMATIC;
                break;
            default:
                return false;
            }
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_plugged(bool& plugged) override
    {
        try
        {
            auto section{open_section()};

            plugged = section.get_bool("plugged");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_mouse_enabled(bool& mouse_enabled) override
    {
        try
        {
            auto section{open_section()};

            mouse_enabled = section.get_bool("mouse");
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_analog_deadzone(float& deadzone) override
    {
        try
        {
            auto section{open_section()};

            auto[x, y]{parse_analog_xy(section.get_string("AnalogDeadzone"))};

            // Ignore the  y deadzone
            (void)y;
            deadzone = static_cast<float>(x) / static_cast<float>(std::numeric_limits<std::int16_t>::max());
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_analog_peak(float& peak) override
    {
        try
        {
            auto section{open_section()};

            auto[x, y]{parse_analog_xy(section.get_string("AnalogPeak"))};

            // Ignore the  y peak
            (void)y;

            peak = static_cast<float>(x) / static_cast<float>(std::numeric_limits<std::uint16_t>::max() / 2);
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool get_bind(N64Button btn, sdl_input_t& sdl_input) override
    {
        try
        {
            auto section{open_section()};

            sdl_input = get_button(section, btn);
        }
        catch(...)
        {
            return false;
        }

        return true;
    }

    bool set_device(const std::string& device, int device_index) override
    {
        std::string tmp1;
        int tmp2{};
        if(!get_device(tmp1, tmp2))
            return false;

        try
        {
            auto section{open_section()};

            if(device_index == -1 || device == "Keyboard")
            {
                // Keyboard
                section.set("name", "Keyboard");
                section.set("device", -1);
            }
            else
            {
                section.set("name", device.c_str());
                section.set("device", device_index);
            }

            save();
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_pak(ControllerPak pak) override
    {
        ControllerPak tmp{};
        if(!get_pak(tmp))
            return false;

        try
        {
            auto section{open_section()};

            switch(pak)
            {
            case ControllerPak::NONE:
                section.set("plugin", 1);
                break;
            case ControllerPak::MEM_PAK:
                section.set("plugin", 2);
                break;
            case ControllerPak::RUMBLE_PAK:
                section.set("plugin", 5);
                break;
            default:
                return false;
            }

            save();
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_config_mode(ConfigurationMode mode) override
    {
        ConfigurationMode tmp{};
        if(!get_config_mode(tmp))
            return false;

        try
        {
            auto section{open_section()};

            switch(mode)
            {
            case ConfigurationMode::MANUAL:
                section.set("mode", 0);
                break;
            case ConfigurationMode::AUTO_NAMED_SDL_DEV:
                section.set("mode", 1);
                break;
            case ConfigurationMode::AUTOMATIC:
                section.set("mode", 2);
                break;
            default:
                return false;
            }

            save();
        }
        catch(const std::system_error& e)
        {
            return false;
        }

        return true;
    }

    bool set_plugged(bool plugged) override
    {
        bool tmp{};
        if(!get_plugged(tmp))
            return false;

        try
        {
            auto section{open_section()};

            section.set("plugged", plugged);

            save();
        }
        catch(const std::system_error&)
        {
            return false;
        }

        return true;
    }

    bool set_mouse_enabled(bool mouse_enabled) override
    {
        bool tmp{};
        if(!get_mouse_enabled(tmp))
            return false;

        try
        {
            auto section{open_section()};

            section.set("mouse", mouse_enabled);

            save();
        }
        catch(const std::system_error&)
        {
            return false;
        }

        return true;
    }

    bool set_analog_deadzone(float deadzone) override
    {
        float tmp{};
        if(!get_analog_deadzone(tmp))
            return false;

        try
        {
            auto section{open_section()};

            auto absolute_deadzone{std::to_string(static_cast<unsigned>(deadzone * std::numeric_limits<std::int16_t>::max()))};

            section.set("AnalogDeadzone", std::string(absolute_deadzone + ',' + absolute_deadzone).c_str());

            save();
        }
        catch(const std::system_error&)
        {
            return false;
        }

        return true;
    }

    bool set_analog_peak(float peak) override
    {
        float tmp{};
        if(!get_analog_peak(tmp))
            return false;

        try
        {
            auto section{open_section()};

            auto absolute_peak{std::to_string(static_cast<unsigned>(peak * (std::numeric_limits<std::uint16_t>::max() / 2)))};

            section.set("AnalogPeak", std::string(absolute_peak + ',' + absolute_peak).c_str());

            save();
        }
        catch(const std::system_error&)
        {
            return false;
        }

        return true;
    }

    bool set_bind(N64Button btn, sdl_input_t sdl_input) override
    {
        N64Button tmp1{};
        sdl_input_t tmp2{};
        if(!get_bind(tmp1, tmp2))
            return false;

        try
        {
            auto section{open_section()};

            set_button(section, btn, sdl_input);

            save();
        }
        catch(const std::system_error&)
        {
            return false;
        }

        return true;
    }

private:
    M64PlusHelper::ConfigSection open_section()
    {
        return core_->config().open_section(
            std::string("Input-SDL-Control" + std::to_string(joy_index_ + 1)).c_str()
        );
    }

    void save()
    {
        core_->config().save(std::string("Input-SDL-Control" + std::to_string(joy_index_ + 1)).c_str());
    }

    static std::string format_analog_xy(std::int16_t x, std::int16_t y)
    {
        return std::to_string(x) + ',' + std::to_string(y);
    }

    static std::pair<std::int16_t, std::int16_t> parse_analog_xy(const std::string& str)
    {
        auto c{str.find(',')};

        if(c == std::string::npos)
            throw std::system_error(make_error_code(std::errc::invalid_argument));

        try
        {
            auto xy{std::make_pair<unsigned long , unsigned long>(std::stoul(str), std::stoul(str.c_str() + c + 1))};
            std::pair<std::int16_t, std::int16_t> ret(
                std::clamp(xy.first, 0ul, (unsigned long)std::numeric_limits<std::int16_t>::max()),
                std::clamp(xy.second, 0ul, (unsigned long)std::numeric_limits<std::int16_t>::max())
            );

            return ret;
        }
        catch(...)
        {
            throw std::system_error(make_error_code(std::errc::invalid_argument));
        }
    }

    static const char* n64_button_str(N64Button btn)
    {
        return btn_enum2str_s[static_cast<unsigned>(btn)];
    }

    static std::pair<sdl_input_t, sdl_input_t> parse_sdl_input(const std::string& str)
    {
        std::pair<sdl_input_t, sdl_input_t> input;
        input.first.type = sdl_input_t::Type::NONE;
        input.second.type = input.first.type;

        if(str.empty())
            return input;

        /// Splits string at ',' & ' '
        auto get_args{[](std::string_view str) -> std::vector<std::string>
        {
            std::vector<std::string> args;

            auto begin{str.find_first_of('(') + 1};

            if(begin == std::string::npos || begin == str.size() - 1)
                return args;

            str.remove_prefix(begin);

            while(!str.empty())
            {
                auto arg_begin{str.find_first_not_of(", ")};
                if(arg_begin == std::string::npos)
                    break;
                str.remove_prefix(arg_begin);
                auto arg_end{str.find_first_of(", )")};
                if(arg_end == std::string::npos || arg_end == 0)
                    break;
                args.emplace_back(str, 0, arg_end);
                str.remove_prefix(arg_end);
            }

            return args;
        }};

        auto begin{std::string::npos};
        auto args{get_args(str)};

        if(args.empty())
            return input;

        if((begin = str.find("key(")) != std::string::npos)
        {
            input.first.type = sdl_input_t::Type::KEY;
            input.first.key_code = std::stoi(args[0]);

            if(args.size() > 1)
            {
                input.second.key_code = std::stoi(args[1]);
                input.second.type = input.first.type;
            }
        }
        else if((begin = str.find("button(")) != std::string::npos)
        {
            input.first.type = sdl_input_t::Type::JOY_BUTTON;
            input.first.joy.index = std::stoi(args[0]);

            if(args.size() > 1)
            {
                input.second.joy.index = std::stoi(args[1]);
                input.second.type = input.first.type;
            }
        }
        else if((begin = str.find("axis(")) != std::string::npos)
        {
            input.first.type = sdl_input_t::Type::JOY_AXIS;
            input.first.joy.index = std::stoi(args[0]);
            input.first.joy.value = (args[0].back() == '+' ? 1 : -1);
            
            if(args.size() > 1)
            {
                input.second.type = input.first.type;
                input.second.joy.index = std::stoi(args[1]);
                input.second.joy.value = (args[1].back() == '+' ? 1 : -1);
            }
        }
        else if((begin = str.find("hat(")) != std::string::npos)
        {
            auto parse_dir = [](std::string_view str, int& c)
            {
                if(str == "Up")
                    c = SDL_HAT_UP;
                else if(str == "Down")
                    c = SDL_HAT_DOWN;
                else if(str == "Left")
                    c = SDL_HAT_LEFT;
                else if(str == "Right")
                    c = SDL_HAT_RIGHT;
                else
                    return false;
                return true;
            };

            input.first.type = sdl_input_t::Type::JOY_HAT;
            input.first.joy.index = std::stoi(args[0]);
            if(!parse_dir(args[1], input.first.joy.value))
                throw std::runtime_error("Invalid joyhat direction");

            if(args.size() > 2)
            {
                input.second.type = input.first.type;
                input.second.joy.index = input.first.joy.index;
                if(!parse_dir(args[2], input.second.joy.value))
                    throw std::runtime_error("Invalid joyhat direction");
            }
        }
        else if((begin = str.find("mouse(")) != std::string::npos)
        {
            input.first.type = sdl_input_t::Type::MOUSE_BUTTON;
            input.first.mouse_button = (unsigned)std::stoul(args[0]);
        }

        return input;
    }

    static std::string sdl_hat_direction(int joy_value)
    {
        if(joy_value == SDL_HAT_UP)
            return "Up";
        if(joy_value == SDL_HAT_DOWN)
            return "Down";
        if(joy_value == SDL_HAT_LEFT)
            return "Left";
        if(joy_value == SDL_HAT_RIGHT)
            return "Right";
        else
            throw std::system_error(make_error_code(std::errc::invalid_argument));
    }

    static void set_button(M64PlusHelper::ConfigSection section, N64Button n64_button, sdl_input_t input)
    {
        // Special treatment for analog stick
        if(n64_button == N64Button::ANALOG_DOWN  ||
           n64_button == N64Button::ANALOG_UP ||
           n64_button == N64Button::ANALOG_LEFT ||
           n64_button == N64Button::ANALOG_RIGHT)
        {
            auto set{[&section](N64Button n64_button, const std::string& str)
            {
                section.set(n64_button_str(n64_button), str.c_str());
            }};

            auto get_pair{[&section, input, n64_button]() -> std::pair<sdl_input_t, sdl_input_t>
            {
                N64Button first{n64_button}, second{};

                switch(n64_button)
                {
                case N64Button::ANALOG_RIGHT:
                    second = N64Button::ANALOG_LEFT;
                    break;
                case N64Button::ANALOG_LEFT:
                    second = N64Button::ANALOG_RIGHT;
                    break;
                case N64Button::ANALOG_UP:
                    second = N64Button::ANALOG_DOWN;
                    break;
                case N64Button::ANALOG_DOWN:
                    second = N64Button::ANALOG_UP;
                    break;
                }

                auto opposite_bind{get_button(section, second)};

                bool swap{};
                if(first == N64Button::ANALOG_RIGHT || first == N64Button::ANALOG_DOWN)
                    swap = true;

                // Mupen64Plus doesn't support binding two different input types to the same axis (or two different hats)
                if(opposite_bind.type != input.type ||
                   (input.type == sdl_input_t::Type::JOY_HAT && input.joy.index != opposite_bind.joy.index))
                {
                    opposite_bind.type = sdl_input_t::Type::NONE;
                }

                if(swap)
                    return {opposite_bind, input};

                return {input, opposite_bind};
            }};


            auto[first, second]{get_pair()};

            switch(input.type != sdl_input_t::Type::NONE ? input.type : second.type)
            {
            case sdl_input_t::Type::KEY:
                set(n64_button, "key(" +
                    (first.type == sdl_input_t::Type::NONE ? "" : std::to_string(first.key_code)) + ',' +
                    (second.type == sdl_input_t::Type::NONE ? "" : std::to_string(second.key_code)) + ")");
                break;
            case sdl_input_t::Type::JOY_AXIS:
                set(n64_button, "axis(" +
                    (first.type == sdl_input_t::Type::NONE ? "" :
                        std::to_string(first.joy.index) + (first.joy.value < 0 ? "-" : "+")) + std::string(",") +
                    (second.type == sdl_input_t::Type::NONE ? "" :
                        std::to_string(second.joy.index) + (second.joy.value < 0 ? "-" : "+")) + std::string(")"));
                break;
            case sdl_input_t::Type::JOY_BUTTON:
                set(n64_button, "button(" +
                    (first.type == sdl_input_t::Type::NONE ? "" : std::to_string(first.joy.index)) + ',' +
                    (second.type == sdl_input_t::Type::NONE ? "" : std::to_string(second.joy.index)) + ")");
                break;
            case sdl_input_t::Type::JOY_HAT:
                set(n64_button, "hat(" +
                    (first.type == sdl_input_t::Type::NONE ? std::to_string(second.joy.index) + " " + sdl_hat_direction(second.joy.value) :
                        std::to_string(first.joy.index) + " " + sdl_hat_direction(first.joy.value)) +
                    (second.type == sdl_input_t::Type::NONE ? "" :
                        " " + sdl_hat_direction(second.joy.value)) + ")");
                break;
            case sdl_input_t::Type::MOUSE_BUTTON:
                set(n64_button, "mouse(" +
                    (first.type == sdl_input_t::Type::NONE ? "" : std::to_string(first.mouse_button)) + ',' +
                    (second.type == sdl_input_t::Type::NONE ? "" : std::to_string(second.mouse_button)) + ")");
                break;
            case sdl_input_t::Type::NONE:
                set(n64_button, "");
                break;
            }
        }
        else
        {
            if(input.type == sdl_input_t::Type::NONE)
            {
                section.set(n64_button_str(n64_button), "");
                return;
            }

            auto set{[&section](N64Button n64_button, const std::string& str)
            {
                section.set(n64_button_str(n64_button), str.c_str());
            }};

            switch(input.type)
            {
            case sdl_input_t::Type::KEY:
                set(n64_button, "key(" + std::to_string(input.key_code) + ")");
                break;
            case sdl_input_t::Type::JOY_AXIS:
                set(n64_button, "axis(" + std::to_string(input.joy.index) +
                    (input.joy.value < 0 ? "-" : "+") +
                    std::string(",9830)")); // We'll use a default analog to digital trigger of 30%
                break;
            case sdl_input_t::Type::JOY_BUTTON:
                set(n64_button, "button(" + std::to_string(input.joy.index) + ")");
                break;
            case sdl_input_t::Type::JOY_HAT:
                set(n64_button, "hat(" + std::to_string(input.joy.index) + ' ' + sdl_hat_direction(input.joy.value) + ")");
                break;
            case sdl_input_t::Type::MOUSE_BUTTON:
                set(n64_button, "mouse(" + std::to_string(input.mouse_button) + ")");
                break;
            }
        }
    }

    static sdl_input_t get_button(M64PlusHelper::ConfigSection section, N64Button n64_button)
    {
        auto inputs{parse_sdl_input(section.get_string(n64_button_str(n64_button)))};

        if(n64_button == N64Button::ANALOG_DOWN || n64_button == N64Button::ANALOG_RIGHT)
            return inputs.second;
        else
            return inputs.first;
    }

    M64PlusHelper::Core* core_;
    unsigned short joy_index_;

    inline static const std::array<const char*, static_cast<unsigned>(N64Button::NUM_BUTTONS)> btn_enum2str_s{
            "A Button", "B Button", "Z Trig", "L Trig", "R Trig",
            "Start", "DPad U", "DPad D", "DPad L", "DPad R",
            "C Button U", "C Button D", "C Button L", "C Button R",
            "Y Axis", "Y Axis",
            "X Axis", "X Axis"
    };
};


Mupen64Plus::Mupen64Plus(const std::string& lib_path, std::string root_path, std::string data_path):
    core_{lib_path, root_path, std::move(data_path)},
    emulator_root_{std::move(root_path)}
{
    assert(!has_instance_s);
    has_instance_s = true;
}

Mupen64Plus::~Mupen64Plus()
{
    if(core_.handle())
    {
        std::error_code ec;
        stop();
        join(ec);
        if(rom_loaded_)
            unload_rom();
    }
    has_instance_s = false;
}

std::unique_ptr<IAudioSettings> Mupen64Plus::audio_settings()
{
    return std::make_unique<Mupen64PlusAudioSettings>(core_);
}

std::unique_ptr<IVideoSettings> Mupen64Plus::video_settings()
{
    return std::make_unique<Mupen64PlusVideoSettings>(core_);
}

std::unique_ptr<IControllerSettings> Mupen64Plus::controller_settings(unsigned short controller)
{
    assert(controller < 4);
    return std::make_unique<Mupen64PlusControllerSettings>(core_, controller);
}

void Mupen64Plus::add_plugin(const std::string& lib_path)
{
    std::lock_guard g(mutex_);
    assert(state_ == MupenState::Stopped);

    // Unload the previous plugin
    plugins_[Plugin::get_plugin_info(lib_path).type].reset();

    auto plugin = new Plugin(core_, lib_path);
    plugins_[plugin->info().type] = std::unique_ptr<Plugin>(plugin);
}

void Mupen64Plus::remove_plugin(m64p_plugin_type type)
{
    std::lock_guard g(mutex_);
    assert(state_ == MupenState::Stopped);

    plugins_[type].reset();
}

void Mupen64Plus::load_rom(void* rom_data, std::size_t n)
{
    std::lock_guard g(mutex_);
    assert(!rom_loaded_);

    prepare_config();

    auto ret {core_.do_cmd(M64CMD_ROM_OPEN, static_cast<int>(n), rom_data)};
    if(failed(ret))
    {
        std::system_error err(make_error_code(ret), "Failed to load ROM image");
        logger()->error(err.what());
        throw err;
    }
    rom_loaded_ = true;
}

void Mupen64Plus::unload_rom()
{
    std::lock_guard g(mutex_);
    assert(rom_loaded_);
    assert(state_ == MupenState::Stopped);

    rom_loaded_ = false;

    auto ret{core_.do_cmd(M64CMD_ROM_CLOSE, 0, nullptr)};
    if(failed(ret))
    {
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly unload ROM: {}", errc.message());
    }
}

void Mupen64Plus::start(const StateCallback& fn)
{
    // NOTE: This code is unfortunately complicated. Take care to not violate the assumptions
    // documented below and in the stop() function.

    assert(rom_loaded_);

    // Safely call callback
    auto notify{[fn](Emulator::State state) noexcept
    {
        try{if(fn) fn(state);}
        catch(...)
        {
            log_noexcept(spdlog::level::warn, "Exception in user state callback");
        }
    }};

    // Take ownership of mutex_, this guarantees nobody will change state_ while we own mutex_.
    // Because we are manually managing mutex_ no exceptions can be thrown while it's locked.
    mutex_.lock();

    // Do nothing if the emulator is not stopped
    if(state_ != MupenState::Stopped)
    {
        mutex_.unlock();
        return;
    }

    log_noexcept(spdlog::level::info, "Starting n64 emulation");
    state_ = MupenState::Starting;

    emulation_thread_ = std::async([this, notify]()
    {
        try
        {
            attach_plugins();
        }
        catch(...)
        {
            state_ = MupenState::Joinable;
            mutex_.unlock();
            throw;
        }

        core_.set_state_callback([this, &notify](m64p_core_param param_type, int new_value) noexcept
        {
            if(param_type != M64CORE_EMU_STATE)
                return;

            switch(new_value)
            {
            case M64EMU_RUNNING:
                if(state_ == MupenState::Starting)
                {
                    // To guarantee that stop() cannot be entered while starting we kept mutex_
                    // now that the emulator is running we want to be able to stop, so unlock it
                    state_ = MupenState::Running;
                    mutex_.unlock();
                    log_noexcept(spdlog::level::info, "Started n64 emulation");
                }
                notify(Emulator::State::RUNNING);
                break;
            case M64EMU_PAUSED:
                mutex_.lock();
                state_ = MupenState::Paused;
                mutex_.unlock();
                notify(Emulator::State::PAUSED);
                break;
            case M64EMU_STOPPED:
                // This is to guarantee we own mutex_ when do_cmd returns
                mutex_.lock();
                state_ = MupenState::Joinable;
                break;
            default:
                assert(false);
                break;
            }
        });

        // Blocking call to execute rom
        auto ret{core_.do_cmd(M64CMD_EXECUTE, 0, nullptr)};

        detach_plugins();

        if(failed(ret))
        {
            // If do_cmd failed before reaching the "running" state we still need to notify about it
            bool notify_about_running{state_ == MupenState::Starting};
            std::system_error err{make_error_code(ret), "Failed to execute ROM image"};

            state_ = MupenState::Joinable;
            mutex_.unlock();

            if(notify_about_running)
                notify(Emulator::State::RUNNING);

            logger()->error(err.what());
            notify(Emulator::State::JOINABLE);
            throw err;
        }

        assert(state_ == MupenState::Joinable);
        log_noexcept(spdlog::level::info, "Stopped n64 emulation");
        mutex_.unlock();
        notify(Emulator::State::JOINABLE);
    });
}

void Mupen64Plus::stop()
{
    // Take ownership of mutex_, this guarantees nobody will change state_
    // while we own mutex_.
    mutex_.lock();

    // Do nothing if already stopped, stopping or joinable
    if(state_ == MupenState::Stopped || state_ == MupenState::Stopping || state_ == MupenState::Joinable)
    {
        mutex_.unlock();
        return;
    }

    // We cannot be starting because the emulation thread owns mutex_ until state_ == State::Started,
    // because we own mutex_ the emulation thread cannot be between Starting and Started.
    assert(state_ == MupenState::Running || state_ == MupenState::Paused);

    // Emulator running, stop it
    log_noexcept(spdlog::level::info, "Stopping n64 emulation");
    core_.do_cmd(M64CMD_STOP, 0, nullptr);
    state_ = MupenState::Stopping;
    mutex_.unlock();

    // Wait for the emulator to be stopped
    while(state_ != MupenState::Joinable)
    {
        std::this_thread::yield();
        // Workaround for: https://github.com/mupen64plus/mupen64plus-core/issues/681
        core_.do_cmd(M64CMD_STOP, 0, nullptr);
    }
}

void Mupen64Plus::join(std::error_code& exit_code)
{
    if(state_ == MupenState::Stopped)
        return;

    assert(state_ == MupenState::Joinable);

    // Emulator is stopped, join the emulation thread
    mutex_.lock();
    try{emulation_thread_.get();}
    catch(const std::system_error& e)
    {
        exit_code = e.code();
    }
    catch(const std::exception& e)
    {
        exit_code = make_error_code(Net64::ErrorCode::UNKNOWN);
    }
    catch(...)
    {
        exit_code = make_error_code(Net64::ErrorCode::UNKNOWN);
    }

    state_ = MupenState::Stopped;
    mutex_.unlock();
    logger()->info("Joined emulation thread");
}

State Mupen64Plus::state() const
{
    std::scoped_lock m(mutex_);

    switch(state_)
    {
    case MupenState::Running:
        return Emulator::State::RUNNING;
    case MupenState::Joinable:
        return Emulator::State::JOINABLE;
    case MupenState::Paused:
        return Emulator::State::PAUSED;
    case MupenState::Stopped:
        return Emulator::State::STOPPED;
    default:
        assert(false);
        break;
    }

    // Silence warning
    return Emulator::State::STOPPED;
}

bool Mupen64Plus::rom_loaded() const
{
    return rom_loaded_;
}

void Mupen64Plus::attach_plugins() noexcept
{
    // We already own mutex_
    core_.attach_plugin(*plugins_[M64PLUGIN_GFX]);
    core_.attach_plugin(*plugins_[M64PLUGIN_AUDIO]);
    core_.attach_plugin(*plugins_[M64PLUGIN_INPUT]);
    core_.attach_plugin(*plugins_[M64PLUGIN_RSP]);
}

void Mupen64Plus::detach_plugins() noexcept
{
    // We already own mutex_
    core_.detach_plugin(plugins_[M64PLUGIN_GFX]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_AUDIO]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_INPUT]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_RSP]->info().type);
}

void Mupen64Plus::logical2physical(addr_t& addr)
{
    if(addr < LOGICAL_BASE)
    {
        logger()->error("Logical address {:#x} does not map to any physical address", addr);
        throw std::system_error(Error::INVALID_ADDR);
    }
    addr -= LOGICAL_BASE;
}

void Mupen64Plus::prepare_config()
{
#define IGNR(x) try{x}catch(const std::system_error&){}

    auto config{core_.config()};

    try
    {
        config.list_sections([this, &config](const char* name)
        {
            // Disable certain hotkeys
            if(std::strcmp(name, "CoreEvents") == 0)
            {
                auto section{config.open_section("CoreEvents")};

                for(const auto hotkey : FORBIDDEN_HOTKEYS)
                {
                    IGNR(section.set(hotkey, "");)
                }
            }
            else if(std::strcmp(name, "Core") == 0)
            {
                auto section{config.open_section("Core")};

                // Enable 8MB
                IGNR(section.set("DisableExtraMem", false);)
                // Set screenshot path
                IGNR(section.set("ScreenshotPath", (fs::path(emulator_root_) / "screenshot").string().c_str());)
                // Set savestate path
                IGNR(section.set("SaveStatePath", (fs::path(emulator_root_) / "save").string().c_str());)
                // Set savegame path
                IGNR(section.set("SaveSRAMPath", (fs::path(emulator_root_) / "save").string().c_str());)
            }
        });

        config.save();
    }
    catch(const std::system_error& e)
    {
        logger()->error("Failed to prepare configuration: {}", e.what());
    }

#undef IGNR
}

void Mupen64Plus::check_bounds(addr_t addr, usize_t size)
{
    if(addr + size > RAM_SIZE)
    {
        // Out of bounds
        auto errc{make_error_code(Error::INVALID_ADDR)};
        logger()->error("Out of bounds memory access at address {:#x} (size: {:#x})", addr, size);
        throw std::system_error(errc);
    }
}

void Mupen64Plus::log_noexcept(spdlog::level::level_enum lvl, const char* msg) noexcept
{
    try
    {
        logger()->log(lvl, msg);
    }
    catch (...)
    {
    }
}

bool Mupen64Plus::has_plugin(m64p_plugin_type type) const
{
    std::lock_guard g(const_cast<Mupen64Plus*>(this)->mutex_);

    if (plugins_[type])
        return true;
    return false;
}

void Mupen64Plus::read_memory(addr_t addr, void* data, usize_t n)
{
    logical2physical(addr);
    check_bounds(addr, n);

    auto ptr{get_mem_ptr<u8>()};

    // First copy the aligned chunk

    // Calc unaligned leftovers
    usize_t begin_left{BSWAP_SIZE - (addr % BSWAP_SIZE)},
            end_left{(addr + n) % BSWAP_SIZE};

    // Address and length of aligned part
    addr_t aligned_addr{addr + begin_left},
           aligned_len{n - end_left - (aligned_addr - addr)};


    // Copy & bswap the aligned chunk
    std::copy_n(ptr + aligned_addr, aligned_len, reinterpret_cast<u8*>(data) + (aligned_addr - addr));
    Memory::bswap32(reinterpret_cast<u8*>(data) + (aligned_addr - addr), aligned_len);


    // Take care of the unaligned beginning
    auto begin_addr{aligned_addr - BSWAP_SIZE};
    u32 begin_word{};
    read(begin_addr, begin_word);
    Memory::bswap32(&begin_word, sizeof(begin_word));
    std::copy_n(reinterpret_cast<u8*>(&begin_word) + (BSWAP_SIZE - begin_left),
                begin_left, reinterpret_cast<u8*>(data));

    // Unaligned ending
    auto end_addr{aligned_addr + aligned_len};
    u32 end_word{};
    read(end_addr, end_word);
    Memory::bswap32(&end_word, sizeof(end_word));
    std::copy_n(reinterpret_cast<u8*>(&end_word), end_left, reinterpret_cast<u8*>(data) + begin_left + aligned_len);
}

void Mupen64Plus::write_memory(addr_t addr, const void* data, usize_t n)
{
    auto ptr{get_mem_ptr<u8>()};

    // Calc unaligned padding
    usize_t begin_padding{addr % BSWAP_SIZE},
            end_padding{BSWAP_SIZE - ((addr + n) % BSWAP_SIZE)};

    // Aligned chunk
    addr_t aligned_addr{addr - begin_padding},
           aligned_len{n + begin_padding + end_padding};


    logical2physical(addr);
    check_bounds(aligned_addr, aligned_len);


    std::vector<u8> buf(aligned_len);

    // Read leftovers from ram
    u32 leftovers[2];
    read(aligned_addr, leftovers[0]);
    read(aligned_addr + aligned_len - BSWAP_SIZE, leftovers[1]);

    Memory::bswap32(leftovers, sizeof(leftovers));

    // Write leftovers into buffer
    std::copy_n(reinterpret_cast<u8*>(&leftovers), sizeof(leftovers[0]), buf.begin());
    std::copy_n(reinterpret_cast<u8*>(&leftovers[1]), sizeof(leftovers[1]), buf.end() - BSWAP_SIZE);

    // Write actual data into the buffer
    std::copy_n(reinterpret_cast<const u8*>(data), n, buf.begin() + begin_padding);
    Memory::bswap32(buf.data(), buf.size());

    std::copy_n(buf.begin(), buf.size(), ptr + aligned_addr);
}

void Mupen64Plus::read(addr_t addr, u8& val)
{
    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    addr_t real_addr{addr - (2 * (addr % BSWAP_SIZE)) + (BSWAP_SIZE - 1)};

    val = ptr[real_addr];
}

void Mupen64Plus::read(addr_t addr, u16& val)
{
    u8 tmp[2];
    read(addr, tmp[0]);
    read(addr + 1, tmp[1]);

    val = static_cast<u16>((static_cast<u16>(tmp[0]) & 0xffu) << 8u | static_cast<u16>(tmp[1]));
}

void Mupen64Plus::read(addr_t addr, u32& val)
{
    u8 tmp[4];
    read(addr, tmp[0]);
    read(addr + 1, tmp[1]);
    read(addr + 2, tmp[2]);
    read(addr + 3, tmp[3]);

    val = static_cast<u32>(static_cast<u32>(tmp[0]) << 24u | static_cast<u32>(tmp[1]) << 16u |
                           static_cast<u32>(tmp[2]) << 8u | static_cast<u32>(tmp[3]));
}

void Mupen64Plus::read(addr_t addr, u64& val)
{
    u32 tmp[2];
    read(addr, tmp[0]);
    read(addr + 4, tmp[1]);

    val = static_cast<u64>(static_cast<u64>(tmp[0]) << 32u | static_cast<u64>(tmp[1]));
}

void Mupen64Plus::read(addr_t addr, f32& val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    std::copy_n(ptr + addr, sizeof(val), reinterpret_cast<u8*>(&val));

    Memory::bswap32(&val, sizeof(val));
}

void Mupen64Plus::read(addr_t addr, f64& val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    std::copy_n(ptr + addr, sizeof(val), reinterpret_cast<u8*>(&val));

    Memory::bswap32(&val, sizeof(val));
}

void Mupen64Plus::write(addr_t addr, u8 val)
{
    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    addr_t real_addr{addr - (2 * (addr % BSWAP_SIZE)) + (BSWAP_SIZE - 1)};

    ptr[real_addr] = val;
}

void Mupen64Plus::write(addr_t addr, u16 val)
{
    write(addr + 1, static_cast<u8>(val & 0xffu));
    write(addr, static_cast<u8>((val & 0xff00u) >> 8u));
}

void Mupen64Plus::write(addr_t addr, u32 val)
{
    write(addr + 3, static_cast<u8>(val & 0xffu));
    write(addr + 2, static_cast<u8>((val & 0xff00u) >> 8u));
    write(addr + 1, static_cast<u8>((val & 0xff0000u) >> 16u));
    write(addr, static_cast<u8>((val & 0xff000000u) >> 24u));
}

void Mupen64Plus::write(addr_t addr, u64 val)
{
    write(addr + 4, static_cast<u32>(val & 0xffffffffu));
    write(addr, static_cast<u32>((val & 0xffffffff00000000u) >> 32u));
}

void Mupen64Plus::write(addr_t addr, f32 val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    Memory::bswap32(&val, sizeof(val));

    std::copy_n(reinterpret_cast<u8*>(&val), sizeof(val), ptr + addr);
}

void Mupen64Plus::write(addr_t addr, f64 val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    logical2physical(addr);
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    Memory::bswap32(&val, sizeof(val));

    std::copy_n(reinterpret_cast<u8*>(&val), sizeof(val), ptr + addr);
}

} // Net64::Emulator
