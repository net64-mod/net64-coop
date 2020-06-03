#pragma once

#include <QDialog>
#ifndef Q_MOC_RUN
#include "net64/emulator/emulator.hpp"
#endif
#include "qt_gui/app_settings.hpp"
#include "qt_gui/m64p_settings_window.hpp"


namespace Ui {
class EmulatorSettings;
}

namespace Frontend
{

struct EmulatorSettings : QDialog
{
Q_OBJECT

public:

    struct Tab
    {
        enum
        {
            EMULATOR,
            CONTROLLER,
            VIDEO,
            AUDIO
        };
    };

    explicit EmulatorSettings(AppSettings& settings, QWidget* parent = nullptr);
    ~EmulatorSettings();

signals:
    void reload_emulator();

public slots:
    void on_core_lib_changed();
    void set_emulator_object(Net64::Emulator::IEmulator* emu);
    void update_interface();

protected:
    void showEvent(QShowEvent* event) override;

private:
    template<typename T, typename... TArgs>
    void show_window(T*& win_ptr, TArgs&&... args)
    {
        if(!win_ptr)
        {
            win_ptr = new T(std::forward<TArgs>(args)..., this);
        }

        win_ptr->show();
        win_ptr->raise();
        win_ptr->activateWindow();
    }

    Ui::EmulatorSettings* ui;
    AppSettings* config_;
    M64PSettings* m64p_config_win_{};
    Net64::Emulator::IEmulator* emulator_{};
};

} // Frontend
