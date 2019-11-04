#include "patch_dialog.hpp"
#include "ui_patch_dialog.h"
#include <fstream>
#include <vector>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>


namespace Frontend
{

PatchDialog::PatchDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::PatchDialog)
{
    ui->setupUi(this);
    adjustSize();
    setFixedSize(size());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->btn_browse_orig, &QPushButton::clicked, this, &PatchDialog::on_browse_original);
    connect(ui->btn_browse_sec, &QPushButton::clicked, this, &PatchDialog::on_browse_second);
    connect(ui->btn_operate, &QPushButton::clicked, this, &PatchDialog::on_create_patch);
    connect(ui->radio_apply, &QRadioButton::toggled, this, &PatchDialog::on_mode_changed);
    connect(&vcdiff_thread_, &VCdiffThread::finished, this, &PatchDialog::on_finished);

    ui->radio_apply->setChecked(true);
    ui->cbx_embed_checksum->setChecked(true);
}

PatchDialog::~PatchDialog()
{
    delete ui;
}

void PatchDialog::on_browse_original()
{
    auto file{
        QFileDialog::getOpenFileName(
            this,
            "Original file",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        )
    };
    if(!file.isEmpty())
    {
        ui->tbx_orig_file->setText(file);
    }
}

void PatchDialog::on_browse_second()
{
    auto file{
        QFileDialog::getOpenFileName(
            this,
            apply_patch_ ? "Delta file" : "Modified file",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            apply_patch_ ? "VCDIFF Delta (*.vcdiff *.VCDIFF);;All files (*)" : "All files (*)"
        )
    };
    if(!file.isEmpty())
    {
        ui->tbx_sec_file->setText(file);
    }
}

void PatchDialog::on_finished(bool success, std::string result)
{
    ui->btn_operate->setEnabled(true);

    if(!success)
    {
        QMessageBox box;
        box.setWindowTitle(apply_patch_ ? "Failed to apply patch" : "Failed to create patch");
        box.setText("See the standard error output for details");
        box.exec();
        return;
    }

    auto file{
        QFileDialog::getSaveFileName(this,
                                     apply_patch_ ? "Result file" : "Delta file",
                                     QFileInfo(ui->tbx_orig_file->text()).path())
    };
    if(file.isEmpty())
        return;
    // Append *.vcdiff file extension
    if((!apply_patch_) && QFileInfo(file).completeSuffix().isEmpty())
        file += ".vcdiff";

    std::ofstream out_file(file.toStdString(), std::ios::binary | std::ios::trunc);
    if(!out_file)
    {
        QMessageBox box;
        box.setWindowTitle("Error");
        box.setText("Failed to write delta patch");
        box.exec();
        return;
    }

    out_file.write(result.data(), static_cast<long>(result.size()));
}

void PatchDialog::on_create_patch()
{
    std::ifstream orig_file(ui->tbx_orig_file->text().toStdString(), std::ios::binary | std::ios::ate),
                  second_file(ui->tbx_sec_file->text().toStdString(), std::ios::binary | std::ios::ate);
    if(!orig_file)
    {
        QMessageBox box;
        box.setWindowTitle("Error");
        box.setText("Failed to open original file");
        box.exec();
        return;
    }
    if(!second_file)
    {
        QMessageBox box;
        box.setWindowTitle("Error");
        box.setText(apply_patch_ ? "Failed to open delta file" : "Failed to open modified file");
        box.exec();
        return;
    }

    std::vector<std::byte> orig_data(static_cast<std::size_t>(orig_file.tellg()));
    std::string second_data(static_cast<std::size_t>(second_file.tellg()), 0);

    orig_file.seekg(0);
    second_file.seekg(0);

    orig_file.read(reinterpret_cast<char*>(orig_data.data()), static_cast<long>(orig_data.size()));
    second_file.read(reinterpret_cast<char*>(second_data.data()), static_cast<long>(second_data.size()));

    if(apply_patch_)
    {
        vcdiff_thread_.start_decode(orig_data.data(), orig_data.size(), second_data);
    }
    else
    {
        auto flags = ui->cbx_embed_checksum->isChecked()
                     ? open_vcdiff::VCD_FORMAT_CHECKSUM
                     : open_vcdiff::VCD_STANDARD_FORMAT;

        vcdiff_thread_.start_encode(orig_data.data(),
                                    orig_data.size(),
                                    second_data.data(),
                                    second_data.size(),
                                    flags);
    }

    ui->btn_operate->setDisabled(true);
}

void PatchDialog::on_mode_changed(bool apply_patch)
{
    apply_patch_ = apply_patch;
    ui->cbx_embed_checksum->setDisabled(apply_patch);

    if(apply_patch)
    {
        ui->grp_sec_file->setTitle("Delta patch");
        ui->btn_operate->setText("Apply");
    }
    else
    {
        ui->grp_sec_file->setTitle("Modified file");
        ui->btn_operate->setText("Create");
    }
}

}
