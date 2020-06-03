#pragma once

#include <QWidget>
#include "qt_gui/sdl_bind_button.hpp"
#ifndef Q_MOC_RUN
#include "common/deleter.hpp"
#include "net64/emulator/emulator.hpp"
#endif

namespace Ui {
class EmulatorControllerConfiguration;
}

namespace Frontend
{

struct EmulatorControllerConfiguration : QWidget, SDL_EventHandler
{
    Q_OBJECT

public:
    using N64Button = Net64::Emulator::IControllerSettings::N64Button;
    using ConfigurationMode = Net64::Emulator::IControllerSettings::ConfigurationMode;
    using ControllerPak = Net64::Emulator::IControllerSettings::ControllerPak;
    using sdl_input_t = Net64::Emulator::IControllerSettings::sdl_input_t;

    explicit EmulatorControllerConfiguration(QWidget* parent = nullptr);
    ~EmulatorControllerConfiguration();

public slots:
    void set_settings_handle(Net64::Emulator::IControllerSettings* settings);

    bool has_settings_handle() const;

private:
    void handle_sdl_event(const SDL_Event& event) override;

    void scan_devices();

    void update_interface();
    void load_settings();
    void setup_sdl_buttons();

    Ui::EmulatorControllerConfiguration *ui;
    std::unique_ptr<SDL_Joystick, Deleter<&SDL_JoystickClose>> joystick_;
    std::vector<SDL_BindButton*> sdl_bind_buttons_;
    std::unique_ptr<Net64::Emulator::IControllerSettings> controller_settings_;

    CLASS_LOGGER_("frontend")
};

}
