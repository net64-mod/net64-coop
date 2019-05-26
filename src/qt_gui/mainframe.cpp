#include "mainframe.hpp"
#include "ui_mainframe.h"

#include <QMessageBox>

#include <experimental/filesystem>
#include <fstream>
namespace Frontend
{

namespace fs = std::experimental::filesystem;
using namespace std::string_literals;

MainFrame::MainFrame(QWidget* parent)
:QWidget(parent), ui(new Ui::MainFrame)
{
    using namespace Core::Emulator::M64PTypes;

    ui->setupUi(this);

    ui->tbx_emu_path->setText("");

    const char* plugin_dir{""};

    for(const auto& entry : fs::directory_iterator(plugin_dir))
    {
        const auto& path{entry.path()};
        switch(Core::Emulator::Mupen64Plus::Plugin::get_plugin_info(path.string()).type)
        {
        case M64PLUGIN_RSP:
            ui->cbx_rsp_plugin->addItem(QString::fromStdString(path.filename().string()));
            break;
        case M64PLUGIN_GFX:
            ui->cbx_gfx_plugin->addItem(QString::fromStdString(path.filename().string()));
            break;
        case M64PLUGIN_AUDIO:
            ui->cbx_audio_plugin->addItem(QString::fromStdString(path.filename().string()));
            break;
        case M64PLUGIN_INPUT:
            ui->cbx_input_plugin->addItem(QString::fromStdString(path.filename().string()));
        default: break;
        }
    }

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
}

MainFrame::~MainFrame()
{
    if(emu_.has_value())
    {
        emu_ = {};
        try{emulation_thread_.get();}
        catch(...){}
    }
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
        emu_ = {};
        try
        {
            emulation_thread_.get();
        }
        catch(const std::system_error& e)
        {
            QMessageBox box;
            box.setWindowTitle("Error in " + QString::fromStdString(e.code().category().name()));
            box.setText(QString::fromStdString("An error has occurred: "s + e.what() + "\nError code: " +
                                               e.code().category().name() + ":"s + std::to_string(e.code().value())));
            box.exec();
        }

        ui->btn_start_emu->setText("Start");

        return;
    }

    try
    {
        emu_ = Core::Emulator::Mupen64Plus{{
                "",
                "",
                ""
        }};

        std::ifstream rom_file{ui->tbx_emu_path->text().toStdString(), std::ios::binary | std::ios::ate};
        if(!rom_file.is_open())
        {
            logger()->error("Failed to open rom");
            emu_ = {};
            return;
        }
        std::vector<char> rom_image(rom_file.tellg());
        rom_file.seekg(0);
        rom_file.read(rom_image.data(), rom_image.size());
        rom_file.close();

        emu_->load_rom(rom_image.data(), rom_image.size());

        const char* plugin_dir{""};
        emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_gfx_plugin->currentText().toStdString()});
        emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_audio_plugin->currentText().toStdString()});
        emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_rsp_plugin->currentText().toStdString()});
        emu_->add_plugin({emu_->core(), plugin_dir + ui->cbx_input_plugin->currentText().toStdString()});

        emulation_thread_ = std::async(std::launch::async, [this]()
        {
            emu_->execute();
        });

        ui->btn_start_emu->setText("Stop");
    }
    catch(const std::system_error& e)
    {
        emu_ = {};

        QMessageBox box;
        box.setWindowTitle("Error in " + QString::fromStdString(e.code().category().name()));
        box.setText(QString::fromStdString("An error has occurred: "s + e.what() + "\nError code: " +
                    e.code().category().name() + ":"s + std::to_string(e.code().value())));
        box.exec();
    }
}

} // Frontend
