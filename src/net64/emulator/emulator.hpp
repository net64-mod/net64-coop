//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <SDL_joystick.h>
#include <SDL_keycode.h>

#include "net64/logging.hpp"
#include "types.hpp"


namespace Net64::Emulator
{
enum struct State
{
    RUNNING,
    PAUSED,
    JOINABLE,
    STOPPED
};

struct IAudioSettings;
struct IVideoSettings;
struct IControllerSettings;

/// Interface for n64 emulators
struct IEmulator
{
    using addr_t = n64_addr_t;
    using saddr_t = n64_saddr_t;
    using usize_t = n64_usize_t;
    using ssize_t = n64_ssize_t;

    static constexpr addr_t LOGICAL_BASE{0x80000000};

    using StateCallback = std::function<void(State)>;

    virtual ~IEmulator() = default;

    /// Get audio settings object
    virtual std::unique_ptr<IAudioSettings> audio_settings() { return nullptr; }

    /// Get video settings object
    virtual std::unique_ptr<IVideoSettings> video_settings() { return nullptr; }

    /// Get controller settings object for controller [controller 0 - 3]
    virtual std::unique_ptr<IControllerSettings> controller_settings(unsigned short controller)
    {
        (void)controller;
        return nullptr;
    }

    /// Load uncompressed rom image
    virtual void load_rom(void* rom_data, std::size_t n) = 0;

    /// Unload rom
    virtual void unload_rom() = 0;

    /// Start emulation
    virtual void start(const StateCallback& fn = {}) = 0;

    /// Stop execution
    virtual void stop() = 0;

    /// Join emulator thread
    virtual void join(std::error_code& exit_code) = 0;

    /// Read n bytes from addr
    virtual void read_memory(addr_t addr, void* data, usize_t n) = 0;

    /// Write n bytes at addr
    virtual void write_memory(addr_t addr, const void* data, usize_t n) = 0;

    // Converting read functions
    virtual void read(addr_t addr, u8& val) = 0;
    virtual void read(addr_t addr, u16& val) = 0;
    virtual void read(addr_t addr, u32& val) = 0;
    virtual void read(addr_t addr, u64& val) = 0;
    virtual void read(addr_t addr, f32& val) = 0;
    virtual void read(addr_t addr, f64& val) = 0;

    // Converting write functions
    virtual void write(addr_t addr, u8 val) = 0;
    virtual void write(addr_t addr, u16 val) = 0;
    virtual void write(addr_t addr, u32 val) = 0;
    virtual void write(addr_t addr, u64 val) = 0;
    virtual void write(addr_t addr, f32 val) = 0;
    virtual void write(addr_t addr, f64 val) = 0;

    /// Check if emulator is running
    virtual State state() const = 0;

    /// Return name of emulator
    virtual const char* name() const = 0;

    static constexpr usize_t RAM_SIZE{0x800000};
};

struct IAudioSettings
{
    virtual ~IAudioSettings() = default;

    /**
     * Reads current emulator volume
     * @param vol Volume (0.0 - 1.0)
     * @return true if reading volume is supported, false otherwise
     */
    virtual bool get_volume(float& vol) = 0;

    /**
     * Sets emulator volume
     * @param vol Volume (0.0 - 1.0)
     * @return true if setting volume is supported, false otherwise
     */
    virtual bool set_volume(float vol) = 0;
};

struct IVideoSettings
{
    virtual ~IVideoSettings() = default;

    /**
     * Reads current fullscreen setting
     * @param fullscreen true if fullscreen enabled
     * @return true if reading fullscreen setting is supported
     */
    virtual bool get_fullscreen(bool& fullscreen) = 0;

    /**
     * Reads current emulator screen dimensions
     * @param width Current width
     * @param height Current height
     * @return true if reading screen diemensions is supported
     */
    virtual bool get_dimensions(unsigned& width, unsigned& height) = 0;

    /**
     * Reads current vsync setting
     * @param vsync true if vsync enabled
     * @return true if reading vsync setting is supported
     */
    virtual bool get_vsync(bool& vsync) = 0;

    /**
     * Enables/disables fullscreen
     * @param fullscreen
     * @return true if setting fullscreen setting is supported
     */
    virtual bool set_fullscreen(bool fullscreen) = 0;

    /**
     * Set emulator screen dimensions
     * @param width
     * @param height
     * @return true if setting emulator screen dimensions is supported
     */
    virtual bool set_dimensions(unsigned width, unsigned height) = 0;

    /**
     * Set vsync setting
     * @param vsync
     * @return true if setting vsync setting is supported
     */
    virtual bool set_vsync(bool vsync) = 0;
};

struct IControllerSettings
{
    struct sdl_input_t
    {
        enum struct Type
        {
            NONE,
            KEY,
            MOUSE_BUTTON,
            JOY_BUTTON,
            JOY_AXIS,
            JOY_HAT,
            JOY_BALL
        } type;

        union {
            struct
            {
                int index;
                int value;
            } joy;
            SDL_Keycode key_code;
            unsigned mouse_button;
        };
    };

    /// Specifies which type of expansion pak is in the controller
    enum struct ControllerPak
    {
        NONE = 1,
        MEM_PAK = 2,
        RUMBLE_PAK = 3
    };

    /// Controller configuration mode
    enum struct ConfigurationMode
    {
        MANUAL = 0,
        AUTO_NAMED_SDL_DEV = 1,
        AUTOMATIC = 2
    };

    enum struct N64Button
    {
        A,
        B,
        Z,
        L,
        R,
        START,
        D_UP,
        D_DOWN,
        D_LEFT,
        D_RIGHT,
        C_UP,
        C_DOWN,
        C_LEFT,
        C_RIGHT,
        ANALOG_UP,
        ANALOG_DOWN,
        ANALOG_LEFT,
        ANALOG_RIGHT,

        NUM_BUTTONS
    };


    virtual ~IControllerSettings() = default;

    static std::optional<ControllerPak> pak_from_string(const std::string& str)
    {
        if(str == pak_to_string(ControllerPak::NONE))
            return ControllerPak::NONE;
        if(str == pak_to_string(ControllerPak::MEM_PAK))
            return ControllerPak::MEM_PAK;
        if(str == pak_to_string(ControllerPak::RUMBLE_PAK))
            return ControllerPak::RUMBLE_PAK;
        return {};
    }

    static std::optional<ConfigurationMode> config_mode_from_string(const std::string& str)
    {
        if(str == config_mode_to_string(ConfigurationMode::AUTOMATIC))
            return ConfigurationMode::AUTOMATIC;
        if(str == config_mode_to_string(ConfigurationMode::MANUAL))
            return ConfigurationMode::MANUAL;
        if(str == config_mode_to_string(ConfigurationMode::AUTO_NAMED_SDL_DEV))
            return ConfigurationMode::AUTO_NAMED_SDL_DEV;
        return {};
    }

    static const char* config_mode_to_string(ConfigurationMode mode)
    {
        switch(mode)
        {
        case ConfigurationMode::AUTOMATIC:
            return "Automatic";
        case ConfigurationMode::MANUAL:
            return "Manual";
        case ConfigurationMode::AUTO_NAMED_SDL_DEV:
            return "Auto with named SDL device";
        }
        assert(false);
        return nullptr;
    }

    static const char* pak_to_string(ControllerPak pak)
    {
        switch(pak)
        {
        case ControllerPak::NONE:
            return "None";
        case ControllerPak::MEM_PAK:
            return "Memory pak";
        case ControllerPak::RUMBLE_PAK:
            return "Rumble pak";
        }
        assert(false);
        return nullptr;
    }

    /**
     * Reads current device bound to this n64 controller
     * @param device SDL device name or "Keyboard"
     * @param device_index SDL joystick device index or -1 for keyboard or -2 for none
     * @return true if reading device is supported
     */
    virtual bool get_device(std::string& device, int& device_index) = 0;

    /**
     * Reads currently plugged in expansion pak
     * @param pak The expansion pak type that is plugged in
     * @return true if reading expansion pak is supported
     */
    virtual bool get_pak(ControllerPak& pak) = 0;

    /**
     * Reads configuration mode
     * @param mode Currently used configuration mode
     * @return true if reading configuration mode is supported
     */
    virtual bool get_config_mode(ConfigurationMode& mode) = 0;

    /**
     * Reads if the virtual n64 controller is plugged in
     * @param plugged true if plugged in
     * @return true if reading plugged in status is supported
     */
    virtual bool get_plugged(bool& plugged) = 0;

    /**
     * Reads if the mouse is used as input method for this n64 controller
     * @param mouse_enabled true if enabled
     * @return true if reading mouse enabled status is supported
     */
    virtual bool get_mouse_enabled(bool& mouse_enabled) = 0;

    /**
     * Reads the current analog deadzone
     * @param deadzone current deadzone (0.0 - 1.0)
     * @return true if reading deadzone is supported
     */
    virtual bool get_analog_deadzone(float& deadzone) = 0;

    /**
     * Reads the current analog peak value
     * @param peak current peak (0.0 - 1.0)
     * @return true if reading analog peak is supported
     */
    virtual bool get_analog_peak(float& peak) = 0;

    /**
     * Reads the current SDL thing bound to this n64 button
     * @param btn N64 button
     * @param sdl_input sdl input used for this virtual n64 button
     * @return true if reading this binding is supported
     */
    virtual bool get_bind(N64Button btn, sdl_input_t& sdl_input) = 0;

    /**
     * Sets the SDL device bound to this n64 controller
     * @param device SDL device name or "Keyboard"
     * @param device_index SDL joystick device_index or -1 for keyboard or -2 for none
     * @return true if setting the device is supported
     */
    virtual bool set_device(const std::string& device, int device_index) = 0;

    /**
     * Sets the expansion pak plugged into this controller
     * @param pak Expansion pak to plug in
     * @return true if setting the expansion pak is supported
     */
    virtual bool set_pak(ControllerPak pak) = 0;

    /**
     * Sets the configuration mode
     * @param mode configuration mode
     * @return true if setting the configuration mode is supported
     */
    virtual bool set_config_mode(ConfigurationMode mode) = 0;

    /**
     * Enables / disables this virtual controller
     * @param plugged Wether this controller is plugged in
     * @return true if enabling / disabling this controller is supported
     */
    virtual bool set_plugged(bool plugged) = 0;

    /**
     * Enables / disables the mouse as input mode for this controller
     * @param mouse_enabled Wether to enable the mouse for this controller
     * @return true if enabling / disabling the mouse for this controller is supported
     */
    virtual bool set_mouse_enabled(bool mouse_enabled) = 0;

    /**
     * Sets the deadzone of the analog stick
     * @param deadzone deadzone of analog stick (0.0 - 1.0)
     * @return
     */
    virtual bool set_analog_deadzone(float deadzone) = 0;

    /**
     * Sets the peak of the analog stick
     * @param peak peak of the analog stick (0.0 - 1.0)
     * @return
     */
    virtual bool set_analog_peak(float peak) = 0;

    /**
     * Binds a button to the virtual n64 button
     * @param btn N64 button to set
     * @param sdl_input Input used for this virtual n64 button
     * @return true if binding this n64 button is supported
     */
    virtual bool set_bind(N64Button btn, sdl_input_t sdl_input) = 0;
};

} // namespace Net64::Emulator
