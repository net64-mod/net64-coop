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
    struct Page
    {
        enum
        {
            HOST,
            JOIN,
            IN_GAME
        };
    };

    explicit MainWindow(AppSettings& settings, QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void emulator_starting();
    void emulator_running();
    void emulator_paused();
    void emulator_stopped();

private slots:
    void on_join_host_changed(QAction* action);
    void on_emulator_settings();
    void on_client_hooked(std::error_code ec);
    void on_client_connected(std::error_code ec);
    void on_client_unhooked(std::error_code ec);

    void on_start_server_btn_pressed();
    void on_connect_btn_pressed();
    void on_disconnect_btn_pressed();
    void on_stop_server_btn_pressed();

private:
    void setup_menus();
    void setup_signals();

    void set_page(int page);

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
    template<typename Sender, typename Signal, typename Receiver, typename Slot>
    void connect_once(Sender sender, Signal signal, Receiver receiver, Slot slot)
    {
        auto context_ptr{new QObject};
        QObject::connect(sender, signal, context_ptr, [this, receiver, slot, context_ptr](auto&&... args) mutable
        {
            receiver->*slot(std::forward<decltype(args)>(args)...);
            delete context_ptr;
        });
    }

    Ui::MainWindow* ui;
    QLabel* statustext_{};
    AppSettings* settings_;
    QMenu* join_host_menu_;
    M64PSettings* m64p_cfg_win_{};
    MultiplayerSettingsWindow* multiplayer_cfg_win_{};
    int last_page_{Page::JOIN};

    Net64Thread net64_thread_{*settings_};
    std::vector<std::byte> rom_image_;

    CLASS_LOGGER_("frontend")
};

} // Frontend
