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
    return "[" + QString::fromStdString(ec.category().name()) + ":" + QString::fromStdString(std::to_string(ec.value())) + "] " + QString::fromStdString(ec.message());
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
    settings_(&settings),
    net64_thread_(*settings_)
{
    qRegisterMetaType<Net64::Emulator::State>("Net64::Emulator::State");

    ui->setupUi(this);
    setWindowIcon(QIcon{":/icons/net64-icon128.png"});
    setFixedSize(size());
    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->addWidget(statustext_ = new QLabel);
    statustext_->setText("Ready");

    set_page(Page::SETUP);

    setup_menus();
    setup_signals();

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());

    // temp @todo
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

void MainWindow::on_emulator_settings()
{
}

void MainWindow::on_start_server_btn_pressed()
{
    on_connect_btn_pressed();
}

void MainWindow::on_connect_btn_pressed()
{
    ui->btn_connect_ip->setDisabled(true);
    ui->btn_start_server->setDisabled(true);

    connect_net64();
}

void MainWindow::on_disconnect_btn_pressed()
{

}

void MainWindow::on_stop_server_btn_pressed()
{
    stop_emulation();
}

void MainWindow::on_emulator_state(Net64::Emulator::State state)
{
    auto old_state{emu_state_.load()};
    emu_state_ = state;

    switch(state)
    {
    case Net64::Emulator::State::RUNNING:
        if(old_state == Net64::Emulator::State::PAUSED)
            emulator_unpaused();
        else
            emulator_started();
        break;
    case Net64::Emulator::State::PAUSED:
        emulator_paused();
        break;
    case Net64::Emulator::State::JOINABLE:
        emulator_joinable();
        break;
    }
}

void MainWindow::on_emulator_started()
{
    set_page(Page::IN_GAME);
    ui->btn_connect_ip->setDisabled(false);
    ui->btn_start_server->setDisabled(false);
}

void MainWindow::on_emulator_paused()
{

}

void MainWindow::on_emulator_unpaused()
{

}

void MainWindow::on_emulator_joinable()
{
    if(net64_thread_.is_initializing() || net64_thread_.is_initialized())
    {
        connect_once(&net64_thread_, &Net64Thread::net64_destroyed, [this]{on_emulator_joinable();});
        net64_thread_.destroy_net64();
        return;
    }

    std::error_code ec;
    emulator_->join(ec);
    emulator_.reset();

    set_page(Page::SETUP);

    if(ec)
        error_popup("Emulation halted due to an error", format_error_msg(ec));
}

void MainWindow::setup_menus()
{
    auto settings_menu{ui->menubar->addMenu("Settings")};
    auto info_menu{ui->menubar->addMenu("Info")};

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
        QDesktopServices::openUrl(QUrl("https://discord.gg/aUmWKaw"));
    });
    connect(info_menu->addAction("About Net64"), &QAction::triggered, this, []()
    {

    });
    connect(info_menu->addAction("About Qt"), &QAction::triggered, this, &QApplication::aboutQt);
}

void MainWindow::setup_signals()
{
    connect(this, &MainWindow::emulator_started, this, &MainWindow::on_emulator_started);
    connect(this, &MainWindow::emulator_paused, this, &MainWindow::on_emulator_paused);
    connect(this, &MainWindow::emulator_unpaused, this, &MainWindow::on_emulator_unpaused);
    connect(this, &MainWindow::emulator_joinable, this, &MainWindow::on_emulator_joinable);

    connect(ui->btn_connect_ip, &QPushButton::clicked, this, &MainWindow::on_connect_btn_pressed);
    connect(ui->btn_start_server, &QPushButton::clicked, this, &MainWindow::on_start_server_btn_pressed);
    connect(ui->btn_stop, &QPushButton::clicked, this, &MainWindow::on_stop_server_btn_pressed);
}

void MainWindow::set_page(Page page)
{
    last_page_= static_cast<Page>(ui->stackedWidget->currentIndex());
    ui->stackedWidget->setCurrentIndex(static_cast<int>(page));
}

void MainWindow::start_emulation()
{
    if(emulator_)
        return;

    try
    {
        auto emu{std::make_unique<Net64::Emulator::Mupen64Plus>(
            (settings_->m64p_plugin_dir() / settings_->m64p_core_plugin).string(),
            settings_->m64p_dir().string(),
            settings_->m64p_plugin_dir().string()
        )};

        auto add_plugin{[&emu, this](const std::string& str){emu->add_plugin((settings_->m64p_plugin_dir() / str).string());}};

        add_plugin(settings_->m64p_video_plugin);
        add_plugin(settings_->m64p_audio_plugin);
        add_plugin(settings_->m64p_rsp_plugin);
        add_plugin(settings_->m64p_input_plugin);

        emulator_ = std::move(emu);

        emulator_->load_rom(reinterpret_cast<void*>(rom_image_.data()), rom_image_.size());

        emulator_->start([this](auto state){on_emulator_state(state);});
    }
    catch(const std::system_error& e)
    {
        error_popup("Failed to start emulation", format_error_msg(e.code()));
        emulator_.reset();
    }
}

void MainWindow::stop_emulation()
{
    emulator_->stop();
}

void MainWindow::connect_net64()
{
    if(!emulator_)
    {
        // Emulator not running, start it
        connect_once(this, &MainWindow::emulator_started, [this]{connect_net64();});
        start_emulation();
        return;
    }
    if(!net64_thread_.is_initialized())
    {
        // Net64 not yet initialized
        connect_once(&net64_thread_, &Net64Thread::net64_initialized, [this](auto ec)
        {
            if(ec)
            {
                if(ec != make_error_code(std::errc::timed_out))
                    error_popup("Failed to initialize Net64", format_error_msg(ec));
            }
            else
            {
                connect_net64();
            }
        });
        net64_thread_.initialize_net64(emulator_.get());
        return;
    }

    // Connect to server
    //net64_thread_.connect();
}

} // Frontend
