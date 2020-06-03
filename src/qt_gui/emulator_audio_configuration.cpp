#include "emulator_audio_configuration.hpp"
#include "ui_emulator_audio_configuration.h"


namespace Frontend
{
EmulatorAudioConfiguration::EmulatorAudioConfiguration(QWidget* parent):
    QWidget(parent), ui(new Ui::EmulatorAudioConfiguration)
{
    ui->setupUi(this);

    connect(ui->volume_slider, &QSlider::valueChanged, [this](int position) {
        ui->volume_percentage_lbl->setText(QString::fromStdString(std::to_string(position) + "%"));

        if(audio_settings_)
            audio_settings_->set_volume(static_cast<float>(position) / 100);
    });

    update_interface();
}

EmulatorAudioConfiguration::~EmulatorAudioConfiguration()
{
    delete ui;
}

void EmulatorAudioConfiguration::set_settings_handle(Net64::Emulator::IAudioSettings* settings)
{
    audio_settings_.reset(settings);

    update_interface();
}

bool EmulatorAudioConfiguration::has_settings_handle() const
{
    return (audio_settings_ != nullptr);
}

void EmulatorAudioConfiguration::update_interface()
{
    if(!audio_settings_)
    {
        ui->volume_box->setHidden(true);
        return;
    }

    float volume{};
    ui->volume_box->setHidden(!audio_settings_->get_volume(volume));
    ui->volume_box->setDisabled(!audio_settings_->set_volume(volume));

    load_settings();
}

void EmulatorAudioConfiguration::load_settings()
{
    if(!audio_settings_)
        return;

    float volume{};
    audio_settings_->get_volume(volume);
    ui->volume_slider->setValue(static_cast<int>(volume * 100));
}

} // namespace Frontend
