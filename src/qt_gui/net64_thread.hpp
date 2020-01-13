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

    void initialize_emulator();
    void start_emulation(std::vector<std::byte> rom_image);
    void initialize_net64();
    void connect(std::string ip, std::uint16_t port);
    void disconnect();
    void destroy_net64();
    void stop_emulation();
    void destroy_emulator();

signals:
    void emulator_initialized(std::error_code ec);
    void emulation_started(std::error_code ec);
    void net64_initialized(std::error_code ec);
    void connected(std::error_code ec);
    void disconnected();
    void net64_destroyed();
    void emulation_stopped(std::error_code ec);
    void emulator_destroyed();

private slots:
    void on_emulator_stopped();

private:
    void tick();

    QTimer* timer_{new QTimer(this)};
    std::unique_ptr<Net64::Emulator::IEmulator> emulator_;
    std::optional<Net64::Client> client_;
    AppSettings* config_;

    bool initializing_net64_{false};

    std::future<void> emulation_thread_;

    CLASS_LOGGER_("net64-thread")
};

struct Net64Thread : QObject
{
    Q_OBJECT

public:
    explicit Net64Thread(AppSettings& config);
    ~Net64Thread() override;

    void set_config(AppSettings& config);

    bool is_emulator_initialized() const;
    bool is_emulation_running() const;
    bool is_net64_initialized() const;
    bool is_connected() const;

public slots:
    void initialize_emulator();
    void start_emulation(std::vector<std::byte> rom_image);
    void initialize_net64();
    void connect(std::string ip, std::uint16_t port);
    void disconnect();
    void destroy_net64();
    void stop_emulation();
    void destroy_emulator();

signals:
    void emulator_initialized(std::error_code ec);
    void emulation_started(std::error_code ec);
    void net64_initialized(std::error_code ec);
    void connected(std::error_code ec);
    void disconnected();
    void net64_destroyed();
    void emulation_stopped(std::error_code ec);
    void emulator_destroyed();

private slots:
    void o_emulator_initialized(std::error_code ec);
    void o_emulation_started(std::error_code ec);
    void o_net64_initialized(std::error_code ec);
    void o_connected(std::error_code ec);
    void o_disconnected();
    void o_net64_destroyed();
    void o_emulation_stopped(std::error_code ec);
    void o_emulator_destroyed();

signals:
    void s_set_config(AppSettings*);
    void s_initialize_emulator();
    void s_start_emulation(std::vector<std::byte>);
    void s_initialize_net64();
    void s_connect(std::string, std::uint16_t);
    void s_disconnect();
    void s_destroy_net64();
    void s_stop_emulation();
    void s_destroy_emulator();

private:
    QThread thread_;

    bool is_emulator_initialized_{};
    bool is_emulation_running_{};
    bool is_net64_initialized_{};
    bool is_connected_{};

};

} // Frontend
