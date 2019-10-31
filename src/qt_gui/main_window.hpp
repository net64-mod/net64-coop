#pragma once

#include <future>
#include <memory>
#include <optional>
#include <QMainWindow>
#include <QThread>
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/m64p_settings_window.hpp"
#include "qt_gui/multiplayer_settings_window.hpp"


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
    ~MainWindow();

signals:
    void emulator_state(Net64::Emulator::State state);

private slots:
    void on_join_host_changed(QAction* action);
    void on_emulator_settings();
    void on_start_server();
    void on_stop_server();
    void on_connect();
    void on_disconnect();
    void on_emulator_state(Net64::Emulator::State state);

private:
    void setup_menus();
    void setup_signals();
    void start_emulator();
    void stop_emulator();
    void set_page(int page);

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

    Ui::MainWindow* ui;
    AppSettings* settings_;
    QMenu* join_host_menu_;
    M64PSettings* m64p_cfg_win_{};
    MultiplayerSettingsWindow* multiplayer_cfg_win_{};
    int last_page_{Page::JOIN};

    std::unique_ptr<Net64::Emulator::IEmulator> emulator_;
    std::future<void> emulation_thread_;
    std::optional<Net64::Client> client_;
    std::optional<Net64::Server> server_;

    CLASS_LOGGER_("frontend")
};

struct ClientThread : QObject
{
    Q_OBJECT

public:
    enum struct State
    {
        STOPPED,
        HOOKING,
        HOOKED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        DISCONNECTED
    };

    ClientThread(Net64::Memory::MemHandle hdl);
    ~ClientThread();



signals:
    void state_changed(State);

private slots:
    void tick();
    void check_initialized();
    void connect();
    void disconnect();

private:
    std::optional<Net64::Client> client_;
    QThread thread_;
};

} // Frontend
