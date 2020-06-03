#include "emulator_controller_configuration.hpp"
#include "ui_emulator_controller_configuration.h"

namespace Frontend
{
using namespace Net64::Emulator;

EmulatorControllerConfiguration::EmulatorControllerConfiguration(QWidget* parent):
    QWidget(parent), ui(new Ui::EmulatorControllerConfiguration)
{
    ui->setupUi(this);

    ui->pak_box->addItem(IControllerSettings::pak_to_string(ControllerPak::NONE));
    ui->pak_box->addItem(IControllerSettings::pak_to_string(ControllerPak::RUMBLE_PAK));
    ui->pak_box->addItem(IControllerSettings::pak_to_string(ControllerPak::MEM_PAK));

    ui->mode_box->addItem(IControllerSettings::config_mode_to_string(ConfigurationMode::AUTOMATIC));
    ui->mode_box->addItem(IControllerSettings::config_mode_to_string(ConfigurationMode::AUTO_NAMED_SDL_DEV));
    ui->mode_box->addItem(IControllerSettings::config_mode_to_string(ConfigurationMode::MANUAL));

    connect(ui->mode_box, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
        if(!controller_settings_)
            return;

        controller_settings_->set_config_mode(
            IControllerSettings::config_mode_from_string(ui->mode_box->currentText().toStdString()).value());
        update_interface();
    });

    connect(ui->pak_box, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
        if(!controller_settings_)
            return;

        controller_settings_->set_pak(
            IControllerSettings::pak_from_string(ui->pak_box->currentText().toStdString()).value());
    });

    connect(ui->device_box, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
        if(!controller_settings_ || ui->device_box->count() < 2)
            return;

        for(auto& btn : sdl_bind_buttons_)
        {
            btn->set_keyboard();
        }

        auto selected_dev{ui->device_box->currentText()};

        if(selected_dev == "[Disconnected]")
        {
            return;
        }
        else if(selected_dev == "Keyboard")
        {
            // Keyboard selected
            controller_settings_->set_device("Keyboard", -1);

            joystick_.reset();
        }
        else
        {
            // Gamepad selected
            auto joy_idx{ui->device_box->currentIndex() - 1};

            assert(joy_idx >= 0);

            joystick_.reset(SDL_JoystickOpen(joy_idx));

            if(!joystick_)
            {
                logger()->error("Failed to open joystick {}:{}: {}",
                                ui->device_box->currentText().toStdString(),
                                joy_idx,
                                SDL_GetError());
                return;
            }

            controller_settings_->set_device(ui->device_box->currentText().toStdString(), joy_idx);

            for(auto& btn : sdl_bind_buttons_)
            {
                btn->set_joystick(SDL_JoystickInstanceID(joystick_.get()));
            }
        }

        // Remove disconnected device item
        int disconnected_idx{ui->device_box->findText("[Disconnected]")};
        if(disconnected_idx > -1)
        {
            ui->device_box->removeItem(disconnected_idx);
        }
    });

    setup_sdl_buttons();
}

EmulatorControllerConfiguration::~EmulatorControllerConfiguration()
{
    delete ui;
}

void EmulatorControllerConfiguration::set_settings_handle(Net64::Emulator::IControllerSettings* settings)
{
    controller_settings_.reset(settings);

    for(auto& btn : sdl_bind_buttons_)
    {
        btn->set_settings_handle(settings);
    }

    update_interface();
}

bool EmulatorControllerConfiguration::has_settings_handle() const
{
    return bool(controller_settings_);
}

void EmulatorControllerConfiguration::handle_sdl_event(const SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_JOYDEVICEADDED:
        [[fallthrough]];
    case SDL_JOYDEVICEREMOVED:
        scan_devices();
        break;
    }
}

void EmulatorControllerConfiguration::scan_devices()
{
    auto set_combobox = [](QComboBox& box, const std::string& str) { box.setCurrentText(QString::fromStdString(str)); };

    if(!controller_settings_)
        return;

    // Load configured device
    std::string device_name;
    int device_idx;
    controller_settings_->get_device(device_name, device_idx);
    device_name.resize(std::strlen(device_name.c_str()));

    ui->device_box->clear();

    ui->device_box->addItem("Keyboard");

    for(int i{}; i < SDL_NumJoysticks(); ++i)
    {
        std::string name{SDL_JoystickNameForIndex(i)};

        if(!name.empty())
            ui->device_box->addItem(name.c_str());
    }

    set_combobox(*ui->device_box, device_name);
    if(ui->device_box->findText(QString::fromStdString(device_name)) <= -1)
    {
        // Configured device is not connected
        ui->device_box->addItem("[Disconnected]");
        set_combobox(*ui->device_box, "[Disconnected]");
    }
}

void EmulatorControllerConfiguration::update_interface()
{
    ui->device_box->setHidden(!controller_settings_);
    ui->buttons_box->setHidden(!controller_settings_);
    ui->analog_stick_box->setHidden(!controller_settings_);
    ui->c_box->setHidden(!controller_settings_);
    ui->d_pad_box->setHidden(!controller_settings_);

    if(!controller_settings_)
        return;

    std::string tmp_str;
    int tmp_int{};
    float tmp_float{};
    sdl_input_t sdl_input{};
    ControllerPak tmp_pak{};
    ConfigurationMode tmp_mode{};

    ui->device_box->setHidden(!controller_settings_->get_device(tmp_str, tmp_int));
    ui->device_box->setDisabled(!controller_settings_->set_device(tmp_str, tmp_int));
    ui->pak_box->setHidden(!controller_settings_->get_pak(tmp_pak));
    ui->pak_box->setDisabled(!controller_settings_->set_pak(tmp_pak));
    ui->mode_box->setHidden(!controller_settings_->get_config_mode(tmp_mode));
    ui->mode_box->setDisabled(!controller_settings_->set_config_mode(tmp_mode));
    ui->deadzone_sbx->setHidden(!controller_settings_->get_analog_deadzone(tmp_float));
    ui->deadzone_sbx->setDisabled(!controller_settings_->set_analog_deadzone(tmp_float));
    ui->peak_sbx->setHidden(!controller_settings_->get_analog_peak(tmp_float));
    ui->peak_sbx->setDisabled(!controller_settings_->set_analog_peak(tmp_float));

    bool read_mappings{controller_settings_->get_bind(N64Button::START, sdl_input)};
    bool set_mappings{controller_settings_->set_bind(N64Button::START, sdl_input)};
    ui->c_box->setHidden(!read_mappings);
    ui->analog_stick_box->setHidden(!read_mappings);
    ui->buttons_box->setHidden(!read_mappings);
    ui->d_pad_box->setHidden(!read_mappings);

    ui->c_box->setDisabled(!set_mappings);
    ui->analog_stick_box->setDisabled(!set_mappings);
    ui->buttons_box->setDisabled(!set_mappings);
    ui->d_pad_box->setDisabled(!set_mappings);

    load_settings();
}

void EmulatorControllerConfiguration::load_settings()
{
    if(!controller_settings_)
        return;

    auto set_combobox = [](QComboBox& box, const std::string& str) { box.setCurrentText(QString::fromStdString(str)); };

    ConfigurationMode mode{};
    controller_settings_->get_config_mode(mode);
    set_combobox(*ui->mode_box, IControllerSettings::config_mode_to_string(mode));
    if(mode != ConfigurationMode::MANUAL)
    {
        ui->buttons_box->setHidden(true);
        ui->analog_stick_box->setHidden(true);
        ui->c_box->setHidden(true);
        ui->d_pad_box->setHidden(true);
    }


    ControllerPak pak{ControllerPak::NONE};
    controller_settings_->get_pak(pak);
    set_combobox(*ui->pak_box, IControllerSettings::pak_to_string(pak));


    // Load button mappings
    for(auto& btn : sdl_bind_buttons_)
    {
        btn->set_settings_handle(controller_settings_.get());
        btn->load();
    }

    scan_devices();
}

void EmulatorControllerConfiguration::setup_sdl_buttons()
{
    sdl_bind_buttons_ = {ui->a_button_btn,
                         ui->b_button_btn,
                         ui->z_trigger_btn,
                         ui->l_trigger_btn,
                         ui->r_trigger_btn,
                         ui->start_btn,
                         ui->d_pad_up_btn,
                         ui->d_pad_down_btn,
                         ui->d_pad_left_btn,
                         ui->d_pad_right_btn,
                         ui->c_up_btn,
                         ui->c_down_btn,
                         ui->c_left_btn,
                         ui->c_right_btn,
                         ui->stick_up_btn,
                         ui->stick_down_btn,
                         ui->stick_left_btn,
                         ui->stick_right_btn};

    auto set = [this](N64Button btn) { sdl_bind_buttons_[static_cast<std::size_t>(btn)]->n64_button = btn; };

    for(std::size_t i{}; i < static_cast<std::size_t>(N64Button::NUM_BUTTONS); ++i)
    {
        set(static_cast<N64Button>(i));
    }
}

} // namespace Frontend
