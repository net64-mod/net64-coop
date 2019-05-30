#include "mainframe.hpp"
#include "ui_mainframe.h"

#include <QMessageBox>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Frontend
{

using namespace std::string_literals;


MainFrame::MainFrame(QWidget* parent)
:QWidget(parent), ui(new Ui::MainFrame)
{
    using namespace Core::Emulator::M64PTypes;
    using json = nlohmann::json;

    ui->setupUi(this);

    ui->tbx_emu_path->setText("");

    for(const auto& entry : fs::directory_iterator(M64P_PLUGIN_PATH))
    {
		auto path{ entry.path().string() };
        switch(Core::Emulator::Mupen64Plus::Plugin::get_plugin_info(entry).type)
        {
        case M64PLUGIN_CORE:
            ui->cbx_core_plugin->addItem(QString::fromStdString(path));
        break;
        case M64PLUGIN_RSP:
            ui->cbx_rsp_plugin->addItem(QString::fromStdString(path));
            break;
        case M64PLUGIN_GFX:
            ui->cbx_gfx_plugin->addItem(QString::fromStdString(path));
            break;
        case M64PLUGIN_AUDIO:
            ui->cbx_audio_plugin->addItem(QString::fromStdString(path));
            break;
        case M64PLUGIN_INPUT:
            ui->cbx_input_plugin->addItem(QString::fromStdString(path));
        default: break;
        }
    }

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
	
    std::fstream configFile;
    //Don't bother opening the file if it's size is zero
    if (fs::file_size(user_config_path.string() + "/" + M64P_USER_CONFIG_SUB_PATH) != 0)
    {
        configFile.open(user_config_path.string() + "/" + M64P_USER_CONFIG_SUB_PATH);

        if (configFile.good())
        {
            json lastSave;

            configFile >> lastSave;
            //messy conversion
            ui->tbx_emu_path->setText(QString::fromStdString((std::string)lastSave["rom"]));

            //find index of last GFX
            QString toFind = QString::fromStdString((std::string)lastSave["gfx"]);
            int index = ui->cbx_gfx_plugin->findText(toFind);

            //if that index was valid
            if (index != -1)
            {
                //set it as the current index
                ui->cbx_gfx_plugin->setCurrentIndex(index);
            }

            toFind = QString::fromStdString((std::string)lastSave["audio"]);
            index = ui->cbx_audio_plugin->findText(toFind);

            if (index != -1)
            {
                ui->cbx_audio_plugin->setCurrentIndex(index);
            }

            toFind = QString::fromStdString((std::string)lastSave["input"]);
            index = ui->cbx_input_plugin->findText(toFind);

            if (index != -1)
            {
                ui->cbx_input_plugin->setCurrentIndex(index);
            }

            toFind = QString::fromStdString((std::string)lastSave["core"]);
            index = ui->cbx_core_plugin->findText(toFind);

            if (index != -1)
            {
                ui->cbx_core_plugin->setCurrentIndex(index);
            }

            toFind = QString::fromStdString((std::string)lastSave["rsp"]);
            index = ui->cbx_rsp_plugin->findText(toFind);

            if (index != -1)
            {
                ui->cbx_rsp_plugin->setCurrentIndex(index);
            }
        }
    }
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
    using json = nlohmann::json;

    //write values to config file
    json configSaveData;
    configSaveData["audio"] = ui->cbx_audio_plugin->currentText().toStdString();
	configSaveData["gfx"] = ui->cbx_gfx_plugin->currentText().toStdString();
	configSaveData["core"] = ui->cbx_core_plugin->currentText().toStdString();
	configSaveData["input"] = ui->cbx_input_plugin->currentText().toStdString();
	configSaveData["rsp"] = ui->cbx_rsp_plugin->currentText().toStdString();
	configSaveData["rom"] = ui->tbx_emu_path->text().toStdString();

    std::fstream configFile;

	configFile.open(user_config_path.string() + "/" + M64P_USER_CONFIG_SUB_PATH);

	if (configFile.good())
    {
		configFile << configSaveData;
    }
	
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
        emu_ = Core::Emulator::Mupen64Plus{
            Core::Emulator::Mupen64Plus::Core{
                fs::directory_entry{ui->cbx_core_plugin->currentText().toStdString()},
                fs::path{user_config_path / M64P_CONFIG_SUB_PATH}.string(),
                fs::path{user_config_path / M64P_DATA_SUB_PATH}.string()
            }
        };

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

		emu_->add_plugin({emu_->core(), fs::directory_entry{ui->cbx_gfx_plugin->currentText().toStdString()}});
		emu_->add_plugin({emu_->core(), fs::directory_entry{ui->cbx_audio_plugin->currentText().toStdString()}});
		emu_->add_plugin({emu_->core(), fs::directory_entry{ui->cbx_rsp_plugin->currentText().toStdString()}});
		emu_->add_plugin({emu_->core(), fs::directory_entry{ui->cbx_input_plugin->currentText().toStdString()}});

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
