#pragma once

#include <future>
#include <memory>
#include <optional>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#ifndef Q_MOC_RUN
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/m64p_settings_window.hpp"
#include "qt_gui/multiplayer_settings_window.hpp"
#endif


namespace Ui {
class MainWindow;
}

namespace Frontend
{

struct ClientObject : QObject
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
        DISCONNECTING
    };

    ClientObject();

    static constexpr std::chrono::milliseconds INTERV{25};

public slots:
    void hook(Net64::Memory::MemHandle hdl);
    void connect(std::string addr, std::uint16_t port);
    void disconnect();
    void unhook();
    void cancel();

private slots:
    void tick();

signals:
    void state_changed(State, State, std::error_code);

private:
    std::optional<Net64::Client> client_;
    std::optional<Net64::Memory::MemHandle> mem_hdl_;
    State state_{State::STOPPED};
    QTimer timer_;

    CLASS_LOGGER_("frontend")
};

struct ClientThread : QObject
{
    Q_OBJECT

public:
    explicit ClientThread();
    ~ClientThread() override;

    [[nodiscard]]
    ClientObject::State state() const;

public slots:
    void hook(Net64::Memory::MemHandle hdl);
    void connect(std::string addr, std::uint16_t port);
    void disconnect();
    void unhook();
    void cancel();

signals:
    void state_changed(ClientObject::State, ClientObject::State, std::error_code);

private slots:
    void on_state_changed(ClientObject::State state, std::error_code ec);

signals:
    void s_hook(Net64::Memory::MemHandle);
    void s_connect(std::string, std::uint16_t);
    void s_disconnect();
    void s_unhook();
    void s_cancel();

private:
    QThread thread_;
    ClientObject::State state_{ClientObject::State::STOPPED};
};

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
    void emulator_state(ClientObject::State, ClientObject::State, std::error_code);

private slots:
    void on_join_host_changed(QAction* action);
    void on_emulator_settings();
    void on_start_server();
    void on_stop_server();
    void on_connect();
    void on_disconnect();
    void on_emulator_state(Net64::Emulator::State state);
    void on_client_state(ClientObject::State from, ClientObject::State to, std::error_code ec);

private:
    void setup_menus();
    void setup_signals();
    void start_emulator();
    void stop_emulator();
    void init_client();
    void connect_client();
    void disconnect_client();
    void destroy_client();
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
    Net64::Emulator::State emu_state_{Net64::Emulator::State::STOPPED};
    ClientThread client_;
    bool connect_after_hooking_{false};

    CLASS_LOGGER_("frontend")
};

} // Frontend
