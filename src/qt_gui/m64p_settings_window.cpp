#include "m64p_settings_window.hpp"
#include "ui_m64p_settings_window.h"
#include "filesystem.hpp"
#include <QDesktopServices>
#include <QFileDialog>
#include <QUrl>


namespace Frontend
{

M64PSettings::M64PSettings(AppSettings& settings, QWidget* parent)
:QMainWindow(parent), ui(new Ui::M64PSettings), settings_{&settings}
{
    ui->setupUi(this);
    adjustSize();
    setFixedSize(size());
    ui->folder_path_field->setText(QString::fromStdString(settings_->m64p_plugin_dir().string()));
    refresh_plugins();
}

M64PSettings::~M64PSettings()
{
    delete ui;
}

void Frontend::M64PSettings::on_close_pressed()
{
    close();
}

void M64PSettings::on_open_plugin_folder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->folder_path_field->text()));
}

void M64PSettings::on_set_plugin_folder()
{
    auto dir{QFileDialog::getExistingDirectory(this, "Mupen64Plus Directory",
        ui->folder_path_field->text(), QFileDialog::ShowDirsOnly)
    };

    if(!dir.isEmpty())
    {
        ui->folder_path_field->setText(dir);
        refresh_plugins();
    }
}

void M64PSettings::on_reset_plugin_folder()
{
    settings_->m64p_custom_pugin_dir.clear();
    ui->folder_path_field->setText(QString::fromStdString(settings_->m64p_plugin_dir().string()));
    refresh_plugins();
}

void M64PSettings::closeEvent(QCloseEvent*)
{
    auto save = [](auto& to, const QComboBox& from)
    {
        to = from.currentText().toStdString();
    };

    save(settings_->m64p_core_plugin, *ui->core_plugin_box);
    save(settings_->m64p_video_plugin, *ui->video_plugin_box);
    save(settings_->m64p_audio_plugin, *ui->audio_plugin_box);
    save(settings_->m64p_rsp_plugin, *ui->rsp_plugin_box);
    save(settings_->m64p_input_plugin, *ui->input_plugin_box);

    settings_->m64p_custom_pugin_dir = ui->folder_path_field->text().toStdString();
}

void M64PSettings::refresh_plugins()
{
    using namespace Net64::Emulator::M64PTypes;

    ui->audio_plugin_box->clear();
    ui->core_plugin_box->clear();
    ui->video_plugin_box->clear();
    ui->input_plugin_box->clear();
    ui->rsp_plugin_box->clear();

    try
    {
        for(const auto& entry : fs::directory_iterator(ui->folder_path_field->text().toStdString()))
        {
            auto file{QString::fromStdString(entry.path().filename().string())};
            switch(Net64::Emulator::Mupen64Plus::Plugin::get_plugin_info(entry.path().string()).type)
            {
            case M64PLUGIN_CORE:
                ui->core_plugin_box->addItem(file);
            break;
            case M64PLUGIN_RSP:
                ui->rsp_plugin_box->addItem(file);
                break;
            case M64PLUGIN_GFX:
                ui->video_plugin_box->addItem(file);
                break;
            case M64PLUGIN_AUDIO:
                ui->audio_plugin_box->addItem(file);
                break;
            case M64PLUGIN_INPUT:
                ui->input_plugin_box->addItem(file);
            default: break;
            }
        }
    }
    catch(const std::exception& e)
    {
        logger()->warn("Failed to inspect plugin folder {}: {}", ui->folder_path_field->text().toStdString(), e.what());
    }

    auto set_index_if_found = [](QComboBox& widget, const std::string& text)
    {
        int index{-1};
        if((index = widget.findText(QString::fromStdString(text))) != -1)
            widget.setCurrentIndex(index);
    };

    set_index_if_found(*ui->core_plugin_box, settings_->m64p_core_plugin);
    set_index_if_found(*ui->audio_plugin_box, settings_->m64p_audio_plugin);
    set_index_if_found(*ui->video_plugin_box, settings_->m64p_video_plugin);
    set_index_if_found(*ui->input_plugin_box, settings_->m64p_input_plugin);
    set_index_if_found(*ui->rsp_plugin_box, settings_->m64p_rsp_plugin);
}

} // Frontend
