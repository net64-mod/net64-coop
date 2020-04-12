//
// Created by henrik on 10.01.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <QThread>
#include <QTimer>
#ifndef Q_MOC_RUN
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#endif


namespace Frontend
{

struct Net64Obj : QObject
{
    Q_OBJECT

public:
    static constexpr std::chrono::milliseconds CLIENT_INTERV{25};

    explicit Net64Obj(AppSettings& config);
    ~Net64Obj() override;

public slots:
    void set_config(AppSettings* config);

    void initialize_net64(Net64::Emulator::IEmulator* emu);
    void connect(std::string ip, std::uint16_t port);
    void disconnect();
    void destroy_net64();

signals:
    void net64_initialized(std::error_code ec);
    void connected(std::error_code ec);
    void disconnected();
    void net64_destroyed();

private:
    void tick();

    QTimer* timer_{new QTimer(this)};
    std::optional<Net64::Client> client_;
    std::optional<Net64::Memory::MemHandle> memory_hdl_;
    AppSettings* settings_;

    bool initializing_net64_{false};

    CLASS_LOGGER_("net64-thread")
};

struct Net64Thread : QObject
{
    Q_OBJECT

public:
    explicit Net64Thread(AppSettings& config);
    ~Net64Thread() override;

    void set_config(AppSettings& config);

    bool is_initializing() const;
    bool is_initialized() const;
    bool is_connected() const;

public slots:
    void initialize_net64(Net64::Emulator::IEmulator* emu);
    void connect(std::string ip, std::uint16_t port);
    void disconnect();
    void destroy_net64();

signals:
    void net64_initialized(std::error_code ec);
    void connected(std::error_code ec);
    void disconnected();
    void net64_destroyed();

private slots:
    void o_net64_initialized(std::error_code ec);
    void o_connected(std::error_code ec);
    void o_disconnected();
    void o_net64_destroyed();

signals:
    void s_set_config(AppSettings*);
    void s_initialize_net64(Net64::Emulator::IEmulator*);
    void s_connect(std::string, std::uint16_t);
    void s_disconnect();
    void s_destroy_net64();

private:
    QThread thread_;

    bool is_initializing_{};
    bool is_initialized_{};
    bool is_connected_{};

};

} // Frontend
