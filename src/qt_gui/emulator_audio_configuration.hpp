#pragma once

#include <QWidget>

#include "net64/emulator/emulator.hpp"


namespace Ui
{
class EmulatorAudioConfiguration;
}


namespace Frontend
{
struct EmulatorAudioConfiguration : QWidget
{
    Q_OBJECT

public:
    explicit EmulatorAudioConfiguration(QWidget* parent = nullptr);
    ~EmulatorAudioConfiguration() override;

public slots:
    void set_settings_handle(Net64::Emulator::IAudioSettings* settings);

    bool has_settings_handle() const;

private:
    void update_interface();
    void load_settings();

    Ui::EmulatorAudioConfiguration* ui;
    std::unique_ptr<Net64::Emulator::IAudioSettings> audio_settings_;
};

} // namespace Frontend
