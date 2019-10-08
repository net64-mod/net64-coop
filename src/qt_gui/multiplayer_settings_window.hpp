#pragma once

#include <QDialog>
#include "qt_gui/app_settings.hpp"


namespace Ui {
class MultiplayerSettingsWindow;
}

namespace Frontend
{

struct MultiplayerSettingsWindow : QDialog
{
    Q_OBJECT

public:
    explicit MultiplayerSettingsWindow(AppSettings& settings, QWidget* parent = nullptr);
    ~MultiplayerSettingsWindow() override;

private slots:
    void on_cancel();
    void on_ok();
    void on_browse_rom();

private:
    void save();
    void load_settings();
    void closeEvent(QCloseEvent*) override;

    Ui::MultiplayerSettingsWindow *ui;
    AppSettings* settings_;
};

} // Frontend
