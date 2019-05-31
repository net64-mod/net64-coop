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

    load_config();
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
	save_config();

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

void MainFrame::load_config()
{
    using json = nlohmann::json;

    std::ifstream config_file;
    // Don't bother opening the file if it's size is zero or it doesn't exist
    if(fs::exists(user_config_path / MAIN_CONFIG_FILE_SUB_PATH) && fs::file_size(user_config_path / MAIN_CONFIG_FILE_SUB_PATH) != 0)
    {
        config_file.open(user_config_path / MAIN_CONFIG_FILE_SUB_PATH);

        if(config_file.good())
        {
            try
            {
                json last_save;

                config_file >> last_save;
                // messy conversion
                ui->tbx_emu_path->setText(QString::fromStdString((std::string)last_save["rom"]));

                // find index of last GFX
                QString to_find{ QString::fromStdString((std::string)last_save["gfx"]) };
                int index{ ui->cbx_gfx_plugin->findText(to_find) };

                // if that index was valid
                if(index != -1)
                {
                    // set it as the current index
                    ui->cbx_gfx_plugin->setCurrentIndex(index);
                }

                to_find = QString::fromStdString((std::string)last_save["audio"]);
                index = ui->cbx_audio_plugin->findText(to_find);

                if(index != -1)
                {
                    ui->cbx_audio_plugin->setCurrentIndex(index);
                }

                to_find = QString::fromStdString((std::string)last_save["input"]);
                index = ui->cbx_input_plugin->findText(to_find);

                if(index != -1)
                {
                    ui->cbx_input_plugin->setCurrentIndex(index);
                }

                to_find = QString::fromStdString((std::string)last_save["core"]);
                index = ui->cbx_core_plugin->findText(to_find);

                if(index != -1)
                {
                    ui->cbx_core_plugin->setCurrentIndex(index);
                }

                to_find = QString::fromStdString((std::string)last_save["rsp"]);
                index = ui->cbx_rsp_plugin->findText(to_find);

                if(index != -1)
                {
                    ui->cbx_rsp_plugin->setCurrentIndex(index);
                }
            }
            catch(const std::exception& e)
            {
                logger()->warn("Exception while reading config.json");
            }
        }	
    }
}

void MainFrame::save_config()
{
    using json = nlohmann::json;

    // write values to config file
    json configSaveData;
    configSaveData["audio"] = ui->cbx_audio_plugin->currentText().toStdString();
    configSaveData["gfx"] = ui->cbx_gfx_plugin->currentText().toStdString();
    configSaveData["core"] = ui->cbx_core_plugin->currentText().toStdString();
    configSaveData["input"] = ui->cbx_input_plugin->currentText().toStdString();
    configSaveData["rsp"] = ui->cbx_rsp_plugin->currentText().toStdString();
    configSaveData["rom"] = ui->tbx_emu_path->text().toStdString();

    std::ofstream config_file;

    config_file.open(user_config_path / MAIN_CONFIG_FILE_SUB_PATH);

    if(config_file.good())
    {
        config_file << configSaveData.dump(4);
    }
}

} // Frontend
