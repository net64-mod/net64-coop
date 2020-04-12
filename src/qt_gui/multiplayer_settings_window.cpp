#include "multiplayer_settings_window.hpp"
#include "ui_multiplayer_settings_window.h"
#include <QFileDialog>
#include <QStandardPaths>


namespace Frontend
{

MultiplayerSettingsWindow::MultiplayerSettingsWindow(AppSettings& settings, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MultiplayerSettingsWindow), settings_{&settings}
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    adjustSize();
    setFixedSize(size());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MultiplayerSettingsWindow::on_ok);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MultiplayerSettingsWindow::on_cancel);
    connect(ui->browse_rom_btn, &QPushButton::clicked, this, &MultiplayerSettingsWindow::on_browse_rom);

    load_settings();
}

MultiplayerSettingsWindow::~MultiplayerSettingsWindow()
{
    delete ui;
}

void MultiplayerSettingsWindow::on_cancel()
{
    close();
}

void MultiplayerSettingsWindow::on_ok()
{
    save();
    close();
}

void MultiplayerSettingsWindow::on_browse_rom()
{
    auto rom_file{QFileDialog::getOpenFileName(this, "Super Mario 64 ROM",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
    };

    if(!rom_file.isEmpty())
    {
        ui->rom_path_tbx->setText(rom_file);
    }
}

void MultiplayerSettingsWindow::save()
{
    settings_->username = ui->username_tbx->text().toStdString();
    settings_->rom_file_path = ui->rom_path_tbx->text().toStdString();
}

void MultiplayerSettingsWindow::load_settings()
{
    ui->username_tbx->setText(QString::fromStdString(settings_->username));
    ui->rom_path_tbx->setText(QString::fromStdString(settings_->rom_file_path.string()));
}

void MultiplayerSettingsWindow::closeEvent(QCloseEvent *)
{
    load_settings();
}

} // Frontend
