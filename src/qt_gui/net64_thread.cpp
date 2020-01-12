//
// Created by henrik on 10.01.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "qt_gui/net64_thread.hpp"


static std::string error_msg(std::error_code ec)
{
    return "[" + std::string(ec.category().name()) + ':' + std::to_string(ec.value()) + ']' + ec.message();
}

static std::string error_msg(const std::system_error& e)
{
    return e.what() + std::string(" (") + error_msg(e.code()) + ")";
}

namespace Frontend
{

Net64Obj::Net64Obj(AppSettings& config):
    config_{&config}
{
    timer_->setTimerType(Qt::PreciseTimer);
    timer_->setInterval(CLIENT_INTERV.count());
    QObject::connect(timer_, &QTimer::timeout, this, &Net64Obj::tick);
    QObject::connect(this, &Net64Obj::emulation_stopped, this, &Net64Obj::on_emulator_stopped);
    timer_->start();
}

Net64Obj::~Net64Obj()
{
    if(client_.has_value())
    {
        if(client_->connected())
            disconnect();
        destroy_net64();
    }
    if(emulator_)
    {
        if(emulator_->running())
            stop_emulation();
        destroy_emulator();
    }
}

void Net64Obj::set_config(AppSettings* config)
{
    config_ = config;
}

void Net64Obj::initialize_emulator()
{
    assert(!emulator_);

    try
    {
        auto emu{std::make_unique<Net64::Emulator::Mupen64Plus>(
            (config_->m64p_plugin_dir() / config_->m64p_core_plugin).string(),
            config_->m64p_dir().string(),
            config_->m64p_plugin_dir().string()
        )};

        emu->add_plugin(config_->m64p_plugin_dir() / config_->m64p_audio_plugin);
        emu->add_plugin(config_->m64p_plugin_dir() / config_->m64p_rsp_plugin);
        emu->add_plugin(config_->m64p_plugin_dir() / config_->m64p_input_plugin);
        emu->add_plugin(config_->m64p_plugin_dir() / config_->m64p_video_plugin);

        emulator_ = std::move(emu);
    }
    catch(const std::system_error& e)
    {
        logger()->error("Failed to initialize emulator: {}", error_msg(e));
        emulator_initialized(e.code());
        return;
    }

    emulator_initialized({});
}

void Net64Obj::start_emulation(std::vector<std::byte> rom_image)
{
    assert(!emulator_->running());

    try{emulator_->load_rom(rom_image.data(), rom_image.size());}
    catch(const std::system_error& e)
    {
        logger()->error("Failed to start emulation: {}", error_msg(e));
        emulation_started(e.code());
        return;
    }

    emulation_thread_ = std::async([this]()
    {
        emulator_->execute([this](auto state)
        {
            using Net64::Emulator::State;

            switch(state)
            {
            case State::STARTING:
                break;
            case State::RUNNING:
                emulation_started({});
                break;
            case State::PAUSED:
                break;
            case State::STOPPED:
            {
                emulation_stopped({});
                break;
            }
            }
        });
    });

    emulation_started({});
}

void Net64Obj::initialize_net64()
{
    if(!client_.has_value())
        initializing_net64_ = true;
}

void Net64Obj::connect(std::string ip, std::uint16_t port)
{
    if(!client_)
    {
        logger()->debug("Can't connect without client object");
        connected(make_error_code(Net64::ErrorCode::UNKNOWN));
        return;
    }

    auto ec{client_->connect(ip.c_str(), port)};
    if(ec)
    {
        logger()->error("Failed to connect to server: {}", error_msg(ec));
    }

    connected(ec);
}

void Net64Obj::disconnect()
{
    if(!client_)
    {
        logger()->debug("Can't disconnect without client object");
        disconnected();
        return;
    }

    client_->disconnect();

    disconnected();
}

void Net64Obj::destroy_net64()
{
    client_ = {};

    if(initializing_net64_)
    {
        initializing_net64_ = false;
        net64_initialized(make_error_code(Net64::ErrorCode::UNKNOWN));
    }

    net64_destroyed();
}

void Net64Obj::stop_emulation()
{
    assert(emulator_);

    emulator_->stop();

    emulation_stopped({});
}

void Net64Obj::destroy_emulator()
{
    emulator_.reset();

    emulator_destroyed();
}

void Net64Obj::on_emulator_stopped()
{//@todo: call the public signal in this slot
    if(emulation_thread_.valid())
    {
        std::error_code ec;
        try{emulation_thread_.get();}
        catch(const std::system_error& e)
        {
            logger()->error("Emulator stopped with error: {}", error_msg(e));
        }
        catch(const std::exception& e)
        {
            logger()->error("Emulator stopped with error: {}", e.what());
        }

        emulator_->unload_rom();
    }

    destroy_net64();
}

void Net64Obj::tick()
{
    if(initializing_net64_)
    {
        Net64::Memory::MemHandle mem_hdl(*emulator_);
        if(Net64::Client::game_initialized(mem_hdl))
        {
            std::error_code ec;
            try{client_ = Net64::Client(mem_hdl);}
            catch(const std::system_error& e)
            {
                ec = e.code();
            }
            catch(const std::exception& e)
            {
                logger()->error("Error while starting Net64 client: {}", e.what());
                ec = make_error_code(Net64::ErrorCode::UNKNOWN);
            }
            net64_initialized(ec);
            initializing_net64_ = false;
        }
    }
    if(client_.has_value())
        client_->tick();
}


Net64Thread::Net64Thread(AppSettings& config)
{
    qRegisterMetaType<std::error_code>("std::error_code");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::uint16_t>("std::uint16_t");
    qRegisterMetaType<std::vector<std::byte>>("std::vector<std::byte>");

    auto* obj{new Net64Obj(config)};

    obj->moveToThread(&thread_);

    QObject::connect(&thread_, &QThread::finished, obj, &QObject::deleteLater);
    QObject::connect(this, &Net64Thread::s_set_config, obj, &Net64Obj::set_config);
    QObject::connect(this, &Net64Thread::s_initialize_emulator, obj, &Net64Obj::initialize_emulator);
    QObject::connect(this, &Net64Thread::s_start_emulation, obj, &Net64Obj::start_emulation);
    QObject::connect(this, &Net64Thread::s_initialize_net64, obj, &Net64Obj::initialize_net64);
    QObject::connect(this, &Net64Thread::s_connect, obj, &Net64Obj::connect);
    QObject::connect(this, &Net64Thread::s_disconnect, obj, &Net64Obj::disconnect);
    QObject::connect(this, &Net64Thread::s_destroy_net64, obj, &Net64Obj::destroy_net64);
    QObject::connect(this, &Net64Thread::s_stop_emulation, obj, &Net64Obj::stop_emulation);
    QObject::connect(this, &Net64Thread::s_destroy_emulator, obj, &Net64Obj::destroy_emulator);

    QObject::connect(obj, &Net64Obj::emulator_initialized, this, &Net64Thread::o_emulator_initialized);
    QObject::connect(obj, &Net64Obj::emulation_started, this, &Net64Thread::o_emulation_started);
    QObject::connect(obj, &Net64Obj::net64_initialized, this, &Net64Thread::o_net64_initialized);
    QObject::connect(obj, &Net64Obj::connected, this, &Net64Thread::o_connected);
    QObject::connect(obj, &Net64Obj::disconnected, this, &Net64Thread::o_disconnected);
    QObject::connect(obj, &Net64Obj::net64_destroyed, this, &Net64Thread::o_net64_destroyed);
    QObject::connect(obj, &Net64Obj::emulation_stopped, this, &Net64Thread::o_emulation_stopped);
    QObject::connect(obj, &Net64Obj::emulator_destroyed, this, &Net64Thread::o_emulator_destroyed);

    thread_.start();
}

Net64Thread::~Net64Thread()
{
    thread_.quit();
    thread_.wait();
}

void Net64Thread::set_config(AppSettings& config)
{
    s_set_config(&config);
}

bool Net64Thread::is_emulator_initialized() const
{
    return is_emulator_initialized_;
}

bool Net64Thread::is_emulation_running() const
{
    return is_emulation_running_;
}

bool Net64Thread::is_net64_initialized() const
{
    return is_net64_initialized_;
}

bool Net64Thread::is_connected() const
{
    return is_connected_;
}

void Net64Thread::initialize_emulator()
{
    s_initialize_emulator();
}

void Net64Thread::start_emulation(std::vector<std::byte> rom_image)
{
    s_start_emulation(std::move(rom_image));
}

void Net64Thread::initialize_net64()
{
    s_initialize_net64();
}

void Net64Thread::connect(std::string ip, std::uint16_t port)
{
    s_connect(std::move(ip), port);
}

void Net64Thread::disconnect()
{
    s_disconnect();
}

void Net64Thread::destroy_net64()
{
    s_destroy_net64();
}

void Net64Thread::stop_emulation()
{
    s_stop_emulation();
}

void Net64Thread::destroy_emulator()
{
    s_destroy_emulator();
}

void Net64Thread::o_emulator_initialized(std::error_code ec)
{
    if(!ec)
        is_emulator_initialized_ = true;

    emulator_initialized(ec);
}

void Net64Thread::o_emulation_started(std::error_code ec)
{
    if(!ec)
        is_emulation_running_ = true;

    emulation_started(ec);
}

void Net64Thread::o_net64_initialized(std::error_code ec)
{
    if(!ec)
        is_net64_initialized_ = true;

    net64_initialized(ec);
}

void Net64Thread::o_connected(std::error_code ec)
{
    if(!ec)
        is_connected_ = true;

    connected(ec);
}

void Net64Thread::o_disconnected()
{
    is_connected_ = false;

    disconnected();
}

void Net64Thread::o_net64_destroyed()
{
    is_net64_initialized_ = false;

    net64_destroyed();
}

void Net64Thread::o_emulation_stopped(std::error_code ec)
{
    is_emulation_running_ = false;

    emulation_stopped(ec);
}

void Net64Thread::o_emulator_destroyed()
{
    is_emulator_initialized_ = false;

    emulator_destroyed();
}

} // Frontend
