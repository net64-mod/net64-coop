#include "mainframe.hpp"
#include "ui_mainframe.h"
#include <fstream>
#include <QMessageBox>


namespace Frontend
{

using namespace std::string_literals;

MainFrame::MainFrame(QWidget* parent, AppSettings& settings)
:QMainWindow(parent), ui(new Ui::MainFrame), settings_{&settings}
{
    ui->setupUi(this);

    connect(ui->menubar->addMenu("Utilities")->addAction("VCDIFF Patcher"), &QAction::triggered, this, &MainFrame::on_open_patch_dialog);

    setWindowIcon(QIcon{":/icons/net64-icon128.png"});

    ui->lineEdit->setText(QString::fromStdString(settings_->rom_file_path.string()));

    qRegisterMetaType<Net64::Emulator::State>();
    connect(this, &MainFrame::emulator_state, this, &MainFrame::on_emulator_state);
}

MainFrame::~MainFrame()
{
    settings_->rom_file_path = ui->lineEdit->text().toStdString();

    delete ui;
}

void MainFrame::closeEvent(QCloseEvent* event)
{
    if(emulator_)
    {
        emulator_.reset();
        emulation_thread_.get();
    }
    QMainWindow::closeEvent(event);
}

void MainFrame::on_action_emulator_settings_triggered()
{
    show_window(m64p_settings_win_, *settings_);
}

void MainFrame::on_start_emulator()
{
    if(emulator_)
    {
        emulator_.reset();
        emulation_thread_.get();
        return;
    }
    try
    {
        emulator_ = std::make_unique<Net64::Emulator::Mupen64Plus>(
            (settings_->m64p_plugin_dir() / settings_->m64p_core_plugin).string(),
            settings_->m64p_dir().string(),
            settings_->m64p_plugin_dir().string()
        );

        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_video_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_audio_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_input_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_rsp_plugin).string());

        std::ifstream rom_file(ui->lineEdit->text().toStdString(), std::ios::ate | std::ios::binary);
        if(!rom_file)
            throw std::runtime_error("Failed to open ROM file");

        std::vector<std::uint8_t> rom_image(rom_file.tellg());
        rom_file.seekg(0);
        rom_file.read(reinterpret_cast<char*>(rom_image.data()), rom_image.size());
        rom_file.close();

        emulator_->load_rom(rom_image.data(), rom_image.size());

        emulation_thread_ = std::async([this]()
        {
            emulator_->execute([this](auto state)
            {
                this->emulator_state(state);
            });
            emulator_.reset();
        });
    }
    catch(const std::system_error& e)
    {
        QMessageBox box;
        box.setWindowTitle("Error in " + QString::fromStdString(e.code().category().name()));
        box.setText(QString::fromStdString("An error has occurred: "s + e.what() + "\nError code: " +
                                           e.code().category().name() + ":"s + std::to_string(e.code().value())));
        box.exec();
    }
}

void MainFrame::on_emulator_state(Net64::Emulator::State state)
{
    using Net64::Emulator::State;

    switch(state)
    {
    case State::STARTING:
        ui->pushButton->setDisabled(true);
        break;
    case State::RUNNING:
        ui->pushButton->setEnabled(true);
        ui->pushButton->setText("Stop");
        break;
    case State::STOPPED:
        ui->pushButton->setText("Start");
        break;
    default: break;
    }
}

void MainFrame::on_open_patch_dialog()
{
    show_window(patch_dialog_);
}

} // Frontend
