#include "main_window.hpp"
#include "ui_main_window.h"
#include <QDesktopServices>


namespace Frontend
{

MainWindow::MainWindow(AppSettings& settings, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings_(&settings)
{
    ui->setupUi(this);
    setFixedSize(size());
    ui->statusbar->setSizeGripEnabled(false);

    ui->stackedWidget->setCurrentIndex(Page::JOIN);

    setup_menus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_join_host_changed(QAction* action)
{
    if(ui->stackedWidget->currentIndex() == Page::JOIN)
    {
        ui->stackedWidget->setCurrentIndex(Page::HOST);
        action->setText("Join game");
        join_host_menu_->setTitle("Host game");
    }
    else if(ui->stackedWidget->currentIndex() == Page::HOST)
    {
        ui->stackedWidget->setCurrentIndex(Page::JOIN);
        action->setText("Host game");
        join_host_menu_->setTitle("Join game");
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

} // Frontend
