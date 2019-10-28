#include "main_window.hpp"
#include "ui_main_window.h"
#include <fstream>
#include <string>
#include <QDesktopServices>
#include <QMessageBox>


namespace Frontend
{

using namespace std::string_literals;

MainWindow::MainWindow(AppSettings& settings, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings_(&settings)
{
    qRegisterMetaType<Net64::Emulator::State>("Net64::Emulator::State");

    ui->setupUi(this);
    setFixedSize(size());
    ui->statusbar->setSizeGripEnabled(false);

    set_page(Page::HOST);
    ui->btn_stop->setDisabled(true);

    setup_menus();
    setup_signals();

    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
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

void MainWindow::on_start_server()
{
    try
    {
        start_emulator();
    }
    catch(...)
    {
        logger()->error("Failed to start emulator uWu");
    }
}

void MainWindow::on_stop_server()
{
    stop_emulator();
}

void MainWindow::on_connect()
{

}

void MainWindow::on_disconnect()
{

}

void MainWindow::on_emulator_state(Net64::Emulator::State state)
{
    using Net64::Emulator::State;

    switch(state)
    {
    case State::STARTING:
        ui->btn_start_server->setDisabled(true);
        break;
    case State::RUNNING:
        set_page(Page::IN_GAME);
        ui->btn_stop->setEnabled(true);
        break;
    case State::STOPPED:
        ui->btn_stop->setDisabled(true);
        ui->btn_start_server->setEnabled(true);
        set_page(last_page_);
        break;
    default: break;
    }
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
    connect(this, &MainWindow::emulator_state, this, &MainWindow::on_emulator_state);
    connect(ui->btn_stop, &QPushButton::clicked, this, &MainWindow::on_stop_server);
    connect(ui->btn_start_server, &QPushButton::clicked, this, &MainWindow::on_start_server);
}

void MainWindow::start_emulator()
{
    assert(!emulator_);
    
    auto emu{std::make_unique<Net64::Emulator::Mupen64Plus>(
        (settings_->m64p_plugin_dir() / settings_->m64p_core_plugin).string(),
        settings_->m64p_dir().string(),
        settings_->m64p_plugin_dir().string()
    )};

    emu->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_video_plugin).string());
    emu->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_audio_plugin).string());
    emu->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_input_plugin).string());
    emu->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_rsp_plugin).string());

    emulator_ = std::move(emu);

    std::ifstream rom_file(settings_->rom_file_path.string(), std::ios::ate | std::ios::binary);
    if(!rom_file)
        throw std::runtime_error("Failed to open ROM file");

    std::vector<std::byte> rom_image(static_cast<std::size_t>(rom_file.tellg()));
    rom_file.seekg(0);
    rom_file.read(reinterpret_cast<char*>(rom_image.data()), static_cast<long>(rom_image.size()));
    rom_file.close();

    emulator_->load_rom(rom_image.data(), rom_image.size());

    emulation_thread_ = std::async([this]()
    {
        emulator_->execute([this](auto state)
        {
            this->emulator_state(state);
        });
        emulator_.reset();
    });
}

void MainWindow::stop_emulator()
{
    assert(emulator_);

    emulator_.reset();
    emulation_thread_.get();
}

void MainWindow::set_page(int page)
{
    last_page_= ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(page);
}

} // Frontend
