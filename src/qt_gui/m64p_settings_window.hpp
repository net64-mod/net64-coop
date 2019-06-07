#pragma once

#include <QMainWindow>
#include "core/logging.hpp"
#include "qt_gui/app_settings.hpp"


namespace Ui {
class M64PSettings;
}

namespace Frontend
{

class M64PSettings : public QMainWindow
{
    Q_OBJECT

public:
    M64PSettings(QWidget* parent, AppSettings& settings);
    ~M64PSettings() override;

private slots:
    void on_close_pressed();
    void on_open_plugin_folder();
    void on_set_plugin_folder();
    void on_reset_plugin_folder();

private:
    void closeEvent(QCloseEvent*) override;
    void refresh_plugins();

    Ui::M64PSettings* ui;
    AppSettings* settings_{};

    CLASS_LOGGER_("frontend")
};

} // Frontend
