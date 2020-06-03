#include "emulator_video_configuration.hpp"
#include "ui_emulator_video_configuration.h"

#include <QCheckBox>
#include <QSpinBox>


namespace Frontend
{

EmulatorVideoConfiguration::EmulatorVideoConfiguration(QWidget* parent):
    QWidget(parent),
    ui(new Ui::EmulatorVideoConfiguration)
{
    ui->setupUi(this);

    connect(ui->vsync_cbx, &QCheckBox::stateChanged, [this](int state)
    {
        if(!video_settings_)
            return;

        video_settings_->set_vsync(static_cast<bool>(state));
    });

    connect(ui->fullscreen_cbx, &QCheckBox::stateChanged, [this](int state)
    {
        if(!video_settings_)
            return;

        video_settings_->set_fullscreen(static_cast<bool>(state));
    });

    connect(ui->width_sbx, qOverload<int>(&QSpinBox::valueChanged), [this](int value)
    {
        if(!video_settings_)
            return;

        video_settings_->set_dimensions(static_cast<unsigned>(value), static_cast<unsigned>(ui->height_sbx->value()));
    });

    connect(ui->height_sbx, qOverload<int>(&QSpinBox::valueChanged), [this](int value)
    {
        if(!video_settings_)
            return;

        video_settings_->set_dimensions(static_cast<unsigned>(ui->width_sbx->value()), static_cast<unsigned>(value));
    });

    update_interface();
}

EmulatorVideoConfiguration::~EmulatorVideoConfiguration()
{
    delete ui;
}

void EmulatorVideoConfiguration::set_settings_handle(Net64::Emulator::IVideoSettings* settings)
{
    video_settings_.reset(settings);

    update_interface();
}

bool EmulatorVideoConfiguration::has_settings_handle() const
{
    return (bool)video_settings_;
}

void EmulatorVideoConfiguration::update_interface()
{
    if(!video_settings_)
    {
        ui->fullscreen_cbx->setHidden(true);
        ui->vsync_cbx->setHidden(true);
        ui->resolution_box->setHidden(true);
        return;
    }

    bool tmp{};
    unsigned w{}, h{};
    ui->fullscreen_cbx->setHidden(!video_settings_->get_fullscreen(tmp));
    ui->fullscreen_cbx->setDisabled(!video_settings_->set_fullscreen(tmp));

    ui->vsync_cbx->setHidden(!video_settings_->get_vsync(tmp));
    ui->vsync_cbx->setDisabled(!video_settings_->set_vsync(tmp));

    ui->resolution_box->setHidden(!video_settings_->get_dimensions(w, h));
    ui->resolution_box->setDisabled(!video_settings_->set_dimensions(w, h));

    load_settings();
}

void EmulatorVideoConfiguration::load_settings()
{
    if(!video_settings_)
        return;

    bool vsync{}, fullscreen{};
    unsigned width{}, height{};
    video_settings_->get_vsync(vsync);
    video_settings_->get_fullscreen(fullscreen);
    video_settings_->get_dimensions(width, height);

    ui->vsync_cbx->setChecked(vsync);
    ui->fullscreen_cbx->setChecked(fullscreen);
    ui->height_sbx->setValue((int)height);
    ui->width_sbx->setValue((int)width);
}

} // Frontend
