#include "multiplayer_settings_window.hpp"
#include "ui_multiplayer_settings_window.h"


namespace Frontend
{

MultiplayerSettingsWindow::MultiplayerSettingsWindow(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MultiplayerSettingsWindow)
{
    ui->setupUi(this);
}

MultiplayerSettingsWindow::~MultiplayerSettingsWindow()
{
    delete ui;
}

} // Frontend
