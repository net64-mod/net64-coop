#pragma once

#include <future>
#include <map>
#include <QMainWindow>
#include "mario20client.hpp"
#include "mario20server.hpp"
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

signals:
    void emulator_state(Net64::Emulator::State);

private slots:
    void on_action_emulator_settings_triggered();
    void on_start_emulator();
    void on_emulator_state(Net64::Emulator::State state);
    void on_pushButton_connect();
    void on_pushButton_debug();
    void on_client_state_changed(Mario20::Client::State state);
    void on_pushButton_host();

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

    void load_patches();
    void apply_patches();
    void check_patches();
    void patch_loop();
    void stop_emulator_threads();

    Ui::MainFrame* ui{};
    M64PSettings* m64p_settings_win_{};
    AppSettings* settings_{};
    Mario20::Client* client_{};
    Mario20::Server* server_{};
    std::unique_ptr<Net64::Emulator::Mupen64Plus> emulator_;
    std::future<void> emulation_thread_;
    std::future<void> patch_thread_;
    std::atomic<bool> patch_thread_active_{};
    std::map<uint32_t, std::vector<uint8_t>> patches_;
};

} // Frontend

Q_DECLARE_METATYPE(Net64::Emulator::State)
