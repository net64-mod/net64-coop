#include "m64p_settings_window.hpp"
#include "ui_m64p_settings_window.h"
#include <experimental/filesystem>
#include <QDesktopServices>
#include <QFileDialog>
#include <QUrl>
#include "core/emulator/m64plus.hpp"


namespace Frontend
{

namespace fs = std::experimental::filesystem;

M64PSettings::M64PSettings(QWidget* parent, AppSettings& settings)
:QMainWindow(parent), ui(new Ui::M64PSettings), settings_{&settings}
{
    ui->setupUi(this);
    ui->folder_path_field->setText(QString::fromStdString(settings_->m64p_plugin_dir.string()));
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
    QDesktopServices::openUrl(QUrl(ui->folder_path_field->text()));
}

void M64PSettings::on_set_plugin_folder()
{
    QFileDialog dialog{this};
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.selectUrl(QUrl(ui->folder_path_field->text()));

    if(dialog.exec())
    {
        ui->folder_path_field->setText(dialog.selectedFiles()[0]);
        refresh_plugins();
    }
}

void M64PSettings::on_reset_plugin_folder()
{
    ui->folder_path_field->setText(QString::fromStdString(AppSettings::M64P_DEFAULT_PLUGIN_DIR));
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

    settings_->m64p_plugin_dir = ui->folder_path_field->text().toStdString();
}

void M64PSettings::refresh_plugins()
{
    using namespace Core::Emulator::M64PTypes;

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
            switch(Core::Emulator::Mupen64Plus::Plugin::get_plugin_info(entry.path()).type)
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
