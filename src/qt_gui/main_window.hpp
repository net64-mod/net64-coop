#pragma once

#include <future>
#include <memory>
#include <optional>

#include <QLabel>
#include <QMainWindow>
#include <QThread>
#include <QTimer>

#include "common/sdl_event_handler.hpp"
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/emulatorsettings.hpp"
#include "qt_gui/multiplayer_settings_window.hpp"


namespace Ui
{
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
    void emulator_object_changed(Net64::Emulator::IEmulator*);
    void emulator_started();
    void emulator_paused();
    void emulator_unpaused();
    void emulator_joinable();

public slots:
    void reload_emulator();

private slots:
    void on_handle_sdl_events();

    void on_emulator_settings();

    void on_connect_btn_pressed();
    void on_disconnect_btn_pressed();
    void on_stop_server_btn_pressed();
    void on_host_server_checked(int state);
    void on_host_port_changed(int port);
    void on_send_chat_message();

    // Emulator events
    void on_emulator_state(Net64::Emulator::State state);
    void on_emulator_started();
    void on_emulator_paused();
    void on_emulator_unpaused();
    void on_emulator_joinable();

    // Client events
    void on_hooked(std::error_code ec);
    void on_chat_message(std::string sender, std::string message);
    void on_connected(std::error_code ec);
    void on_disconnected(std::error_code ec);


    void setup_menus();
    void setup_signals();

    void set_page(Page page);

    bool create_emulator();
    void start_emulation();
    void stop_emulation();
    void destroy_emulator();

    void connect_net64();

private:
    void update_statustext();

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
        QObject::connect(sender, signal, context_ptr, [this, fn, context_ptr](auto&&... args) mutable {
            fn(std::forward<decltype(args)>(args)...);
            delete context_ptr;
        });
    }

    Ui::MainWindow* ui;
    QLabel* statustext_{};
    AppSettings* settings_;
    EmulatorSettings* emu_settings_win_{};
    MultiplayerSettingsWindow* multiplayer_cfg_win_{};
    Page last_page_{Page::SETUP};
    QTimer* sdl_event_timer_{};

    std::vector<std::byte> rom_image_;
    std::unique_ptr<Net64::Emulator::Mupen64Plus> emulator_;
    std::atomic<Net64::Emulator::State> emu_state_{Net64::Emulator::State::STOPPED};
    Net64::Client client_;
    std::optional<Net64::Server> server_;
    // Net64Thread net64_thread_;
    bool reload_emulator_after_stop_{};

    CLASS_LOGGER_("frontend")
};

} // namespace Frontend
