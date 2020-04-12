#pragma once

#include <future>
#include <memory>
#include <optional>
#include <QLabel>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#ifndef Q_MOC_RUN
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/m64p_settings_window.hpp"
#include "qt_gui/multiplayer_settings_window.hpp"
#include "qt_gui/net64_thread.hpp"
#endif


namespace Ui {
class MainWindow;
}

namespace Frontend
{

struct MainWindow : QMainWindow
{
    Q_OBJECT

public:

    enum struct Page : int
    {
        SETUP,
        IN_GAME
    };

    explicit MainWindow(AppSettings& settings, QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void emulator_started();
    void emulator_paused();
    void emulator_unpaused();
    void emulator_joinable();

private slots:
    void on_emulator_settings();

    void on_start_server_btn_pressed();
    void on_connect_btn_pressed();
    void on_disconnect_btn_pressed();
    void on_stop_server_btn_pressed();

    void on_emulator_state(Net64::Emulator::State state);
    void on_emulator_started();
    void on_emulator_paused();
    void on_emulator_unpaused();
    void on_emulator_joinable();


    void setup_menus();
    void setup_signals();

    void set_page(Page page);

    void start_emulation();
    void stop_emulation();

    void connect_net64();

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

    template<typename Sender, typename Signal, typename Functor>
    void connect_once(Sender sender, Signal signal, Functor fn)
    {
        auto context_ptr{new QObject};
        QObject::connect(sender, signal, context_ptr, [this, fn, context_ptr](auto&&... args) mutable
        {
            fn(std::forward<decltype(args)>(args)...);
            delete context_ptr;
        });
    }

    Ui::MainWindow* ui;
    QLabel* statustext_{};
    AppSettings* settings_;
    M64PSettings* m64p_cfg_win_{};
    MultiplayerSettingsWindow* multiplayer_cfg_win_{};
    Page last_page_{Page::SETUP};

    std::vector<std::byte> rom_image_;
    std::unique_ptr<Net64::Emulator::IEmulator> emulator_;
    std::atomic<Net64::Emulator::State> emu_state_{Net64::Emulator::State::STOPPED};
    Net64Thread net64_thread_;

    CLASS_LOGGER_("frontend")
};

} // Frontend
