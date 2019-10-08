#pragma once

#include <QMainWindow>
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

private slots:
    void on_join_host_changed(QAction* action);

private:
    void setup_menus();

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
};

} // Frontend
