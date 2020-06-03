#pragma once

#include <QWidget>

#include "net64/emulator/emulator.hpp"

namespace Ui
{
class EmulatorVideoConfiguration;
}

namespace Frontend
{
struct EmulatorVideoConfiguration : QWidget
{
    Q_OBJECT

public:
    explicit EmulatorVideoConfiguration(QWidget* parent = nullptr);
    ~EmulatorVideoConfiguration();

public slots:
    void set_settings_handle(Net64::Emulator::IVideoSettings* settings);

    bool has_settings_handle() const;

private:
    void update_interface();
    void load_settings();

    Ui::EmulatorVideoConfiguration* ui;
    std::unique_ptr<Net64::Emulator::IVideoSettings> video_settings_;
};

} // namespace Frontend
