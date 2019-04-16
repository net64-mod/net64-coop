#include "mainframe.hpp"
#include "ui_mainframe.h"

#include <QMessageBox>

#include <experimental/filesystem>
#include <fstream>
namespace Frontend
{

namespace fs = std::experimental::filesystem;

MainFrame::MainFrame(QWidget* parent)
:QWidget(parent), ui(new Ui::MainFrame)
{
    ui->setupUi(this);



    for(const auto& entry : fs::directory_iterator(plugin_dir))
    {
        const auto& path{entry.path()};
        switch(Core::Emulator::M64PPlugin::get_plugin_info(path.string()).type)
        {
        case M64PLUGIN_RSP:
            ui->cbx_rsp_plugin->addItem(QString::fromStdString(path.filename()));
            break;
        case M64PLUGIN_GFX:
            ui->cbx_gfx_plugin->addItem(QString::fromStdString(path.filename()));
            break;
        case M64PLUGIN_AUDIO:
            ui->cbx_audio_plugin->addItem(QString::fromStdString(path.filename()));
            break;
        case M64PLUGIN_INPUT:
            ui->cbx_input_plugin->addItem(QString::fromStdString(path.filename()));
        default: break;
        }
    }

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
}

MainFrame::~MainFrame()
{
    delete ui;
}

void MainFrame::on_tbx_emu_path_returnPressed()
{
    on_btn_start_emu_clicked();
}

void MainFrame::on_btn_start_emu_clicked()
{
    if(emu_.has_value())
    {
        emu_->stop();
        execution_thread_.get();
        emu_ = {};

        ui->btn_start_emu->setText("Start");

        return;
    }

    emu_ = Core::Emulator::M64Plus{{
{
    }};

    std::ifstream rom_file{ui->tbx_emu_path->text().toStdString(), std::ios::binary | std::ios::ate};
    if(!rom_file.is_open())
    {
        logger_->error("Failed to open rom\n");
        emu_ = {};
        return;
    }
    std::vector<char> rom_image(rom_file.tellg());
    rom_file.seekg(0);
    rom_file.read(rom_image.data(), rom_image.size());
    rom_file.close();

    emu_->load_rom(rom_image.data(), rom_image.size());

    emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_gfx_plugin->currentText().toStdString()});
    emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_audio_plugin->currentText().toStdString()});
    emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_rsp_plugin->currentText().toStdString()});
    emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_input_plugin->currentText().toStdString()});

    execution_thread_ = std::async(std::launch::async, [this]()
    {
        emu_->execute();
    });

    ui->btn_start_emu->setText("Stop");
}

} // Frontend
