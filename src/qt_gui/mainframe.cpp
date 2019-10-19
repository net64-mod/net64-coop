#include "mainframe.hpp"
#include "ui_mainframe.h"
#include <fstream>
#include <QMessageBox>
#include <filesystem>
#include <QDirIterator>
#include <net64/memory/util.hpp>

namespace Frontend
{

using namespace std::string_literals;

MainFrame::MainFrame(QWidget* parent, AppSettings& settings)
    : QMainWindow(parent)
    , ui(new Ui::MainFrame)
    , settings_{&settings}
    , client_{new Mario20::Client(QUrl("ws://127.0.0.1:3678"), this)}
    , server_{new Mario20::Server(this)}
{
    ui->setupUi(this);

    setWindowIcon(QIcon{":/icons/net64-icon128.png"});

    ui->lineEdit->setText(QString::fromStdString(settings_->rom_file_path.string()));

    qRegisterMetaType<Net64::Emulator::State>();
    connect(this, &MainFrame::emulator_state, this, &MainFrame::on_emulator_state);
    connect(ui->pushButtonConnect, &QPushButton::clicked, this, &MainFrame::on_pushButton_connect);
    connect(ui->pushButtonDebug, &QPushButton::clicked, this, &MainFrame::on_pushButton_debug);
    connect(ui->pushButtonHost, &QPushButton::clicked, this, &MainFrame::on_pushButton_host);
    connect(client_, &Mario20::Client::state_changed, this, &MainFrame::on_client_state_changed);
}

MainFrame::~MainFrame()
{
    settings_->rom_file_path = ui->lineEdit->text().toStdString();

    delete ui;
}

void MainFrame::closeEvent(QCloseEvent* event)
{
    if(emulator_)
        stop_emulator_threads();
    QMainWindow::closeEvent(event);
}

void MainFrame::on_action_emulator_settings_triggered()
{
    show_window(m64p_settings_win_, *settings_);
}

void MainFrame::on_start_emulator()
{
    if(emulator_)
    {
        stop_emulator_threads();
        return;
    }
    try
    {
        emulator_ = std::make_unique<Net64::Emulator::Mupen64Plus>(
            (settings_->m64p_plugin_dir() / settings_->m64p_core_plugin).string(),
            settings_->m64p_dir().string(),
            settings_->m64p_plugin_dir().string()
        );

        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_video_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_audio_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_input_plugin).string());
        emulator_->add_plugin((settings_->m64p_plugin_dir() / settings_->m64p_rsp_plugin).string());

        std::ifstream rom_file(ui->lineEdit->text().toStdString(), std::ios::ate | std::ios::binary);
        if(!rom_file)
            throw std::runtime_error("Failed to open ROM file");

        std::vector<std::uint8_t> rom_image(rom_file.tellg());
        rom_file.seekg(0);
        rom_file.read(reinterpret_cast<char*>(rom_image.data()), rom_image.size());
        rom_file.close();

        emulator_->load_rom(rom_image.data(), rom_image.size());
        load_patches();

        assert(!patch_thread_active_);
        patch_thread_active_ = true;
        patch_thread_ = std::async([this]()
        {
            this->patch_loop();
        });

        emulation_thread_ = std::async([this]()
        {
            emulator_->execute([this](auto state)
            {
                this->emulator_state(state);
            });
            patch_thread_active_ = false;
            patch_thread_.get();
            if (client_->is_connected())
            {
                client_->close();
                while (client_->is_connected())
                    std::this_thread::yield();
            }
            emulator_.reset();
        });
    }
    catch(const std::system_error& e)
    {
        QMessageBox box;
        box.setWindowTitle("Error in " + QString::fromStdString(e.code().category().name()));
        box.setText(QString::fromStdString("An error has occurred: "s + e.what() + "\nError code: " +
                                           e.code().category().name() + ":"s + std::to_string(e.code().value())));
        box.exec();
    }
}

void MainFrame::on_emulator_state(Net64::Emulator::State state)
{
    using Net64::Emulator::State;

    switch(state)
    {
    case State::STARTING:
        ui->pushButton->setDisabled(true);
        break;
    case State::RUNNING:
        ui->pushButton->setEnabled(true);
        ui->pushButton->setText("Stop");
        break;
    case State::STOPPED:
        ui->pushButton->setText("Start");
        break;
    default: break;
    }
}

void MainFrame::on_pushButton_connect()
{
    if (!emulator_)
        return;

    auto ip = ui->lineEditIp->text();
    auto port = ui->spinBoxPort->value();
    client_->set_url(QUrl(QString("ws://%1:%2").arg(ip).arg(port)));

    auto read = [this](uint32_t addr, void* data, size_t size)
    {
        try
        {
            auto mem = emulator_->mem_ptr();
            memcpy(data, mem + addr, size);
        }
        catch (const std::runtime_error&)
        {
        }
    };
    auto write = [this](uint32_t addr, const void* data, size_t size)
    {
        try
        {
            auto mem = emulator_->mem_ptr();
            memcpy(mem + addr, data, size);
        }
        catch (const std::runtime_error&)
        {
        }
    };
    client_->open(read, write);
}

void MainFrame::on_pushButton_debug()
{
    if (!emulator_)
    {
        std::vector<char> test = { 9,8,7,6,5,4,3 };
        QByteArray data = QByteArray(test.data(), (int)test.size());
        printf("data: %s\n", Mario20::tohex(data).c_str());
        auto compressed = Mario20::compress(test.data(), test.size());
        printf("compressed: %s\n", Mario20::tohex(compressed).c_str());
        auto decompressed = Mario20::decompress(compressed.data(), compressed.size());
        printf("decompressed: %s\n", Mario20::tohex(decompressed).c_str());
        return;
    }
    auto mem = emulator_->mem_ptr();

    auto writeMem = [mem](uint32_t addr, const void* data, size_t size)
    {
        memcpy(mem + addr, data, size);
    };

    /*std::array<uint8_t, 4> buffer;

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x04;
    buffer[3] = 0x00;

    writeMem(0x33b238, buffer.data(), buffer.size());

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x01;
    buffer[3] = 0x01;

    writeMem(0x33b248, buffer.data(), buffer.size());

    buffer[0] = 0x00;
    buffer[1] = 0x00;
    buffer[2] = 0x00;
    buffer[3] = 0x00;

    writeMem(0x38eee0, buffer.data(), buffer.size());*/

    //mem[0x367703] = 1;
    //mem[0x365FFC] = 1;
    //emulator_->write(0x367703, u32(1));
    //emulator_->write(0x365FFC, u32(1));


    // toggle debug mode
    auto debugaddr = 0x32D598;
    static constexpr size_t BSWAP_SIZE{ 4 };
    auto real_addr{ debugaddr - (2 * (debugaddr % BSWAP_SIZE)) + (BSWAP_SIZE - 1) };
    mem[real_addr] = mem[real_addr] ? 0 : 0x59;

    // set luigi
    // mem[0x365FF3] = 2;
}

void MainFrame::on_client_state_changed(Mario20::Client::State state)
{
    switch (state)
    {
    case Mario20::Client::State::Connecting:
        ui->pushButtonConnect->setEnabled(false);
        ui->pushButtonConnect->setText("Connecting...");
        break;
    case Mario20::Client::State::Connected:
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setText("Disconnect");
        break;
    case Mario20::Client::State::Disconnected:
        ui->pushButtonConnect->setEnabled(true);
        ui->pushButtonConnect->setText("Connect");
        break;
    }
}

void MainFrame::on_pushButton_host()
{
    ui->pushButtonHost->setEnabled(false);
    server_->listen(ui->spinBoxPort->value());
}

void MainFrame::load_patches()
{
    patches_.clear();

    QDirIterator it(":/patches");
    while (it.hasNext())
    {
        auto patch_path = it.next();
        QFile patch_file(patch_path);
        if (!patch_file.open(QFile::ReadOnly))
            throw std::runtime_error("Failed to open patch file");
        
        auto patch_name = QFileInfo(patch_file).fileName();
        char* end = nullptr;
        auto patch_addr = strtoul(patch_name.toUtf8().constData(), &end, 16);
        
        std::vector<uint8_t> patch_data;
        patch_data.resize(patch_file.size());
        patch_file.read(reinterpret_cast<char*>(patch_data.data()), patch_data.size());

        printf("loaded patch 0x%lx[0x%zu]\n", patch_addr, patch_data.size());
        patches_[patch_addr] = patch_data;
    }

    /*namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(R"(c:\CodeBlocks\sm64o\SM64O\bin\Release\Ressources)"))
    {
        const auto filenameStr = entry.path().filename().string();
        const auto pathStr = entry.path().string();
        if (entry.is_regular_file())
        {
            char* end = nullptr;
            auto patch_addr = strtoul(filenameStr.c_str(), &end, 16);

            std::ifstream patch_file(pathStr, std::ios::ate | std::ios::binary);
            if (!patch_file)
                throw std::runtime_error("Failed to open patch file");

            std::vector<std::uint8_t> patch_data(patch_file.tellg());
            patch_file.seekg(0);
            patch_file.read(reinterpret_cast<char*>(patch_data.data()), patch_data.size());
            patch_file.close();

            printf("loaded patch 0x%x[0x%zx]\n", patch_addr, patch_data.size());
            patches_[patch_addr] = patch_data;
        }
    }*/
}

void MainFrame::apply_patches()
{
    auto mem = emulator_->mem_ptr();
    auto v1 = *(uint32_t*)(mem + 0);
    auto v2 = *(uint32_t*)(mem + 4);
    printf("v1: 0x%08X, v2: 0x%08X\n", v1, v2);
    assert(v1 == 0x3C1A8032 && v2 == 0x275A7650);

    auto m1 = *(uint32_t*)(mem + 0x246000);
    auto m2 = *(uint32_t*)(mem + 0x33A57C);
    printf("m1: 0x%08X, m2: 0x%08X\n", m1, m2);
    assert(m1 == 0x3C088034 && m2 == 0x66AD0C39);

    {
        auto f = fopen("mem.dump", "wb");
        fwrite(mem, Net64::Emulator::IEmulator::RAM_SIZE, 1, f);
        fclose(f);
    }

    for (const auto& [patch_addr, patch_data] : patches_)
    {
        auto status = "empty";
        for (size_t i = 0; i < patch_data.size(); i++)
        {
            if (mem[patch_addr + i])
            {
                status = "patch";
                break;
            }
        }
        printf("patch 0x%x[0x%zx]: %s\n", patch_addr, patch_data.size(), status);
    }
    for (const auto& [patch_addr, patch_data] : patches_)
        memcpy(mem + patch_addr, patch_data.data(), patch_data.size());

    mem[0x365FFC] = 1; // let client think that he is host
    mem[0x367703] = 1; // let client think that he has player ID 1
    mem[0x365FF3] = 2; // character luigi
}

void MainFrame::check_patches()
{
    auto mem = emulator_->mem_ptr();
    for (const auto& [patch_addr, patch_data] : patches_)
    {
        std::vector<uint8_t> actual_data(patch_data.size());
        memcpy(actual_data.data(), mem + patch_addr, actual_data.size());
        printf("check 0x%x[0x%zx]: %s\n", patch_addr, actual_data.size(), patch_data == actual_data ? "ok" : "fail");
    }
}

void MainFrame::patch_loop()
{
    puts("waiting for memory to be initialized...");
    while (patch_thread_active_)
    {
        std::this_thread::yield();
        try
        {
            auto mem = emulator_->mem_ptr();
            auto v1 = *(uint32_t*)(mem + 0);
            auto v2 = *(uint32_t*)(mem + 4);
            auto m1 = *(uint32_t*)(mem + 0x246000);
            auto m2 = *(uint32_t*)(mem + 0x33A57C);
            
            if (v1 == 0x3C1A8032 && v2 == 0x275A7650 && m1 == 0x3C088034 && m2 == 0x66AD0C39)
            {
                puts("applying patches!");
                apply_patches();
                patch_thread_active_ = false;
            }
        }
        catch(const std::system_error&)
        {
        }
    }
    puts("finished patching");
}

void MainFrame::stop_emulator_threads()
{
    emulator_->stop();
    std::future_status status;
    do
    {
        status = emulation_thread_.wait_for(std::chrono::microseconds(15));
        QCoreApplication::processEvents();
    } while (status != std::future_status::ready);
    emulation_thread_.get();
    assert(!emulator_);
}

} // Frontend
