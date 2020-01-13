#include "main_window.hpp"
#include "ui_main_window.h"
#include <fstream>
#include <string>
#include <QDesktopServices>
#include <QMessageBox>


namespace Frontend
{

using namespace std::string_literals;

static QString format_error_msg(std::error_code ec)
{
    return "[" + QString::fromStdString(ec.category().name()) + ":" + QString(ec.value()) + "] " + QString::fromStdString(ec.message());
}

static void error_popup(const char* action, const QString& reason)
{
    QMessageBox box;
    box.setIcon(QMessageBox::Critical);
    box.setWindowTitle(action);
    box.setText(reason);
    box.exec();
}

MainWindow::MainWindow(AppSettings& settings, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings_(&settings)
{
    qRegisterMetaType<Net64::Emulator::State>("Net64::Emulator::State");

    ui->setupUi(this);
    setFixedSize(size());
    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->addWidget(statustext_ = new QLabel);
    statustext_->setText("Ready");

    set_page(Page::HOST);
    ui->btn_stop->setDisabled(true);

    setup_menus();
    setup_signals();

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());

    // temp
    std::ifstream rom_file(settings_->rom_file_path.string(), std::ios::ate | std::ios::binary);
    if(!rom_file)
        return;

    rom_image_.resize(static_cast<std::size_t>(rom_file.tellg()));
    rom_file.seekg(0);
    rom_file.read(reinterpret_cast<char*>(rom_image_.data()), static_cast<long>(rom_image_.size()));
    rom_file.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_join_host_changed(QAction* action)
{
    if(ui->stackedWidget->currentIndex() == Page::JOIN)
    {
        set_page(Page::HOST);
        action->setText("Join game");
        join_host_menu_->setTitle("Host game");
    }
    else if(ui->stackedWidget->currentIndex() == Page::HOST)
    {
        set_page(Page::JOIN);
        action->setText("Host game");
        join_host_menu_->setTitle("Join game");
    }
}

void MainWindow::on_emulator_settings()
{
}

void MainWindow::on_client_hooked(std::error_code ec)
{
    if(ec)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle("Failed to initialize Net64 client");
        box.setText(format_error_msg(ec));
        box.exec();
        return;
    }

    statustext_->setText("Initialized");
}

void MainWindow::on_client_connected(std::error_code ec)
{
    if(ec)
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setWindowTitle("Failed to connect to server");
        box.setText(format_error_msg(ec));
        box.exec();
        return;
    }

    statustext_->setText("Connected");
}

void MainWindow::on_client_unhooked(std::error_code)
{
    statustext_->setText("Ready");
}

void MainWindow::on_start_server_btn_pressed()
{

}

void MainWindow::on_connect_btn_pressed()
{
    if(!net64_thread_.is_emulator_initialized())
    {
        connect_once(&net64_thread_, &Net64Thread::emulator_initialized, [this](auto ec)
        {
            if(ec)
            {
                error_popup("Failed to initialize emulator", format_error_msg(ec));
            }
            else
            {
                on_connect_btn_pressed();
            }
        });
        net64_thread_.initialize_emulator();
        return;
    }

    if(!net64_thread_.is_emulation_running())
    {
        connect_once(&net64_thread_, &Net64Thread::emulation_started, [this](auto ec)
        {
            if(ec)
            {
                error_popup("Failed to launch emulator", format_error_msg(ec));
            }
            else
            {
                on_connect_btn_pressed();
            }
        });
        net64_thread_.start_emulation(rom_image_);
        return;
    }

    if(!net64_thread_.is_net64_initialized())
    {
        connect_once(&net64_thread_, &Net64Thread::net64_initialized, [this](auto ec)
        {
            if(ec)
            {
                error_popup("Failed to initialize Net64", format_error_msg(ec));
            }
            else
            {
                on_connect_btn_pressed();
            }
        });
        return;
    }

    //net64_thread_.connect(ui->tbx_join_ip->text().toStdString(), (std::uint16_t)ui->sbx_port->value());
}

void MainWindow::on_disconnect_btn_pressed()
{
    if(!net64_thread_.is_connected())
        return;

    net64_thread_.disconnect();
}

void MainWindow::on_stop_server_btn_pressed()
{

}

void MainWindow::setup_menus()
{
    join_host_menu_ = ui->menubar->addMenu("Join game");
    auto settings_menu{ui->menubar->addMenu("Settings")};
    auto info_menu{ui->menubar->addMenu("Info")};

    join_host_menu_->addAction("Host game");
    connect(join_host_menu_, &QMenu::triggered, this, &MainWindow::on_join_host_changed);

    connect(settings_menu->addAction("Multiplayer"), &QAction::triggered, this, [this]()
    {
        show_window(multiplayer_cfg_win_, *settings_);
    });
    connect(settings_menu->addAction("Emulator"), &QAction::triggered, this, [this]()
    {
        show_window(m64p_cfg_win_, *settings_);
    });

    connect(info_menu->addAction("Website"), &QAction::triggered, this, []()
    {
        QDesktopServices::openUrl(QUrl("https://net64-mod.github.io"));
    });
    connect(info_menu->addAction("GitHub"), &QAction::triggered, this, []()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/net64-mod/net64-coop"));
    });
    connect(info_menu->addAction("Discord"), &QAction::triggered, this, []()
    {
        QDesktopServices::openUrl(QUrl("https://discord.gg/GgGUKH8"));
    });
    connect(info_menu->addAction("About Net64"), &QAction::triggered, this, []()
    {

    });
    connect(info_menu->addAction("About Qt"), &QAction::triggered, this, &QApplication::aboutQt);
}

void MainWindow::setup_signals()
{
    connect(ui->btn_connect_ip, &QPushButton::clicked, this, &MainWindow::on_connect_btn_pressed);
}

void MainWindow::set_page(int page)
{
    last_page_= ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(page);
}

} // Frontend
