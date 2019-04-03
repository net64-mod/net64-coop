#include "mainframe.hpp"
#include "ui_mainframe.h"

#include <QMessageBox>


namespace Frontend
{

MainFrame::MainFrame(QWidget* parent)
:QWidget(parent), ui(new Ui::MainFrame)
{
    ui->setupUi(this);

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
    if(!emu_.running())
    {
        emu_.start(ui->tbx_emu_path->text().toStdString());
    }
}

void MainFrame::on_btn_base_addr_clicked()
{
    try
    {
        emu_.find_base_addr();
    }
    catch(const std::runtime_error& e)
    {
    }

    QMessageBox box;
    box.setWindowTitle("Base Address");
    box.setText("Base address: " + QString::fromStdString(std::to_string(emu_.base_addr())));
    box.exec();
}

} // Frontend
