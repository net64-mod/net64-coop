#include "emulatorsettings.hpp"
#include "ui_emulatorsettings.h"

namespace Frontend
{
EmulatorSettings::EmulatorSettings(AppSettings& settings, QWidget* parent):
    QDialog(parent), ui(new Ui::EmulatorSettings), config_{&settings}
{
    ui->setupUi(this);
    setFixedSize(sizeHint());

    m64p_config_win_ = new M64PSettings(*config_, this);

    connect(m64p_config_win_, &M64PSettings::core_lib_changed, this, &EmulatorSettings::on_core_lib_changed);

    connect(ui->config_emu_btn, &QPushButton::pressed, [this]() { show_window(m64p_config_win_, *config_); });

    update_interface();
}

EmulatorSettings::~EmulatorSettings()
{
    delete ui;
}

void EmulatorSettings::set_emulator_object(Net64::Emulator::IEmulator* emu)
{
    emulator_ = emu;
    update_interface();
}

void EmulatorSettings::update_interface()
{
    if(!emulator_)
    {
        ui->tabWidget->setTabEnabled(Tab::AUDIO, false);
        ui->tabWidget->setTabEnabled(Tab::CONTROLLER, false);
        ui->tabWidget->setTabEnabled(Tab::VIDEO, false);
        return;
    }

    ui->audio_widget->set_settings_handle(emulator_->audio_settings().release());
    ui->tabWidget->setTabEnabled(Tab::AUDIO, ui->audio_widget->has_settings_handle());
    ui->video_widget->set_settings_handle(emulator_->video_settings().release());
    ui->tabWidget->setTabEnabled(Tab::VIDEO, ui->video_widget->has_settings_handle());
    ui->controller_widget->set_settings_handle(emulator_->controller_settings(0).release());
    ui->tabWidget->setTabEnabled(Tab::CONTROLLER, ui->controller_widget->has_settings_handle());
}

void EmulatorSettings::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    update_interface();
}

void EmulatorSettings::on_core_lib_changed()
{
    reload_emulator();
}

} // namespace Frontend
