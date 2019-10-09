#pragma once

#include <future>
#include <QMainWindow>
#ifndef Q_MOC_RUN
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/m64p_settings_window.hpp"
#endif


namespace Ui {
class MainFrame;
}

namespace Frontend
{

struct MainFrame : QMainWindow
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget* parent, AppSettings& settings);
    ~MainFrame();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_action_emulator_settings_triggered();
    void on_start_emulator();

private:
    template<typename T, typename... TArgs>
    void show_window(T*& win_ptr, TArgs&&... args)
    {
        if(!win_ptr)
        {
            win_ptr = new T(this, std::forward<TArgs>(args)...);
        }

        win_ptr->show();
        win_ptr->raise();
        win_ptr->activateWindow();
    }

    Ui::MainFrame* ui;
    M64PSettings* m64p_settings_win_{};
    AppSettings* settings_;
    std::unique_ptr<Net64::Emulator::Mupen64Plus> emulator_;
    std::future<void> emulation_thread_;
};

} // Frontend
