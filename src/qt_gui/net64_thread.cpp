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
    settings_{&config}
{
    timer_->setTimerType(Qt::PreciseTimer);
    timer_->setInterval(CLIENT_INTERV.count());
    QObject::connect(timer_, &QTimer::timeout, this, &Net64Obj::tick);
    timer_->start();
}

Net64Obj::~Net64Obj()
{/*
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
    }*/
}

void Net64Obj::set_config(AppSettings* config)
{
    settings_ = config;
}

void Net64Obj::initialize_net64(Net64::Emulator::IEmulator* emu)
{
    memory_hdl_ = Net64::Memory::MemHandle(*emu);
    initializing_net64_ = true;
}

void Net64Obj::connect(std::string ip, std::uint16_t port)
{

}

void Net64Obj::disconnect()
{

}

void Net64Obj::destroy_net64()
{
    client_ = {};
    memory_hdl_ = {};
    if(initializing_net64_)
    {
        initializing_net64_ = false;
        net64_initialized(make_error_code(std::errc::timed_out));
    }
    net64_destroyed();
}

void Net64Obj::tick()
{
    if(initializing_net64_)
    {
        if(Net64::Client::game_initialized(*memory_hdl_))
        {
            std::error_code ec;
            try{client_ = Net64::Client(*memory_hdl_);}
            catch(const std::system_error& e)
            {
                ec = e.code();
            }
            catch(const std::exception& e)
            {
                logger()->error("Error while starting Net64 client: {}", e.what());
                ec = make_error_code(Net64::ErrorCode::UNKNOWN);
            }
            initializing_net64_ = false;
            net64_initialized(ec);
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
    //qRegisterMetaType<Net64::>("Net64::Memory::MemHandle");

    auto* obj{new Net64Obj(config)};

    obj->moveToThread(&thread_);

    QObject::connect(&thread_, &QThread::finished, obj, &QObject::deleteLater);
    QObject::connect(this, &Net64Thread::s_set_config, obj, &Net64Obj::set_config);
    QObject::connect(this, &Net64Thread::s_initialize_net64, obj, &Net64Obj::initialize_net64);
    QObject::connect(this, &Net64Thread::s_connect, obj, &Net64Obj::connect);
    QObject::connect(this, &Net64Thread::s_disconnect, obj, &Net64Obj::disconnect);
    QObject::connect(this, &Net64Thread::s_destroy_net64, obj, &Net64Obj::destroy_net64);

    QObject::connect(obj, &Net64Obj::net64_initialized, this, &Net64Thread::o_net64_initialized);
    QObject::connect(obj, &Net64Obj::connected, this, &Net64Thread::o_connected);
    QObject::connect(obj, &Net64Obj::disconnected, this, &Net64Thread::o_disconnected);
    QObject::connect(obj, &Net64Obj::net64_destroyed, this, &Net64Thread::o_net64_destroyed);

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

bool Net64Thread::is_initializing() const
{
    return is_initializing_;
}

bool Net64Thread::is_initialized() const
{
    return is_initialized_;
}

bool Net64Thread::is_connected() const
{
    return is_connected_;
}

void Net64Thread::initialize_net64(Net64::Emulator::IEmulator* emu)
{
    assert(emu);

    is_initializing_ = true;
    s_initialize_net64(emu);
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

void Net64Thread::o_net64_initialized(std::error_code ec)
{
    if(!ec)
        is_initialized_ = true;

    is_initializing_ = false;
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
    is_initialized_ = false;
    is_initializing_ = false;

    net64_destroyed();
}

} // Frontend
