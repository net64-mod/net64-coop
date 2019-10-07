#pragma once

#include <QDialog>

namespace Ui {
class MultiplayerSettingsWindow;
}

namespace Frontend
{

struct MultiplayerSettingsWindow : QDialog
{
    Q_OBJECT

public:
    explicit MultiplayerSettingsWindow(QWidget* parent = nullptr);
    ~MultiplayerSettingsWindow();

private:
    Ui::MultiplayerSettingsWindow *ui;
};

} // Frontend
