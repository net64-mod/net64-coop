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

    using OptionalMemHandle = std::optional<Net64::Memory::MemHandle>;

    ClientObject();

    static constexpr std::chrono::milliseconds INTERV{25};

public slots:
    void hook(Frontend::ClientObject::OptionalMemHandle hdl);
    void connect(std::string addr, std::uint16_t port);
    void disconnect();
    void unhook();

private slots:
    void tick();

signals:
    void state_changed(Frontend::ClientObject::State, std::error_code);

private:
    std::optional<Net64::Client> client_;
    std::optional<Net64::Memory::MemHandle> mem_hdl_;
    State state_{State::STOPPED};
    QTimer* timer_{new QTimer(this)};

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
    void hook(Frontend::ClientObject::OptionalMemHandle hdl);
    void connect(std::string addr, std::uint16_t port);
    void disconnect();
    void unhook();

signals:
    void hooked(std::error_code);
    void connected(std::error_code);
    void disconnected(std::error_code);
    void unhooked(std::error_code);

private slots:
    void on_state_changed(Frontend::ClientObject::State state, std::error_code ec);

signals:
    void s_hook(Frontend::ClientObject::OptionalMemHandle);
    void s_connect(std::string, std::uint16_t);
    void s_disconnect();
    void s_unhook();

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
    void emulator_state(Net64::Emulator::State);

private slots:
    void on_join_host_changed(QAction* action);
    void on_emulator_settings();
    void on_start_server();
    void on_stop_server();
    void on_connect();
    void on_disconnect();
    void on_emulator_state(Net64::Emulator::State state);
    void on_client_hooked(std::error_code ec);
    void on_client_connected(std::error_code ec);
    void on_client_unhooked(std::error_code ec);

private:
    void setup_menus();
    void setup_signals();
    bool start_emulator();
    void stop_emulator();
    void init_client();
    void connect_client();
    void disconnect_client();
    void destroy_client();
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

    std::unique_ptr<Net64::Emulator::IEmulator> emulator_;
    std::future<void> emulation_thread_;
    Net64::Emulator::State emu_state_{Net64::Emulator::State::STOPPED};
    ClientThread client_;
    bool connect_after_hooking_{false},
         hook_after_emu_start_{false};

    CLASS_LOGGER_("frontend")
};

} // Frontend
