//
// Created by henrik on 02.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include <experimental/filesystem>
#include <iostream>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "build_info.hpp"
#include "core/logging.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/mainframe.hpp"


namespace Frontend
{

using Core::LoggerPtr;

namespace fs = std::experimental::filesystem;

static void create_app_dirs(const AppSettings& settings)
{
    try
    {
        fs::create_directories(settings.main_config_dir());
        fs::create_directories(settings.m64p_data_dir());
        fs::create_directories(settings.m64p_config_dir());
    }
    catch(const std::exception& e)
    {
        std::cout << "Failed to create application directories: " << e.what() << '\n';
    }
}

static void setup_logging()
{
    auto global_log_file{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString() +
            "/log.txt", true)
    };
    global_log_file->set_level(spdlog::level::debug);

    // Terminal logging
    auto stdout_sink{std::make_shared<spdlog::sinks::stderr_color_sink_mt>()};
    stdout_sink->set_pattern("[%^%l%$] [%n] %v");
    stdout_sink->set_level(spdlog::level::info);

    // Pass logging configuration to core
    Core::init_logging_sinks([global_log_file, stdout_sink]{
        std::vector<spdlog::sink_ptr> sinks{
            stdout_sink, global_log_file
        };

        return sinks;
    });
}

} // Frontend

int main(int argc, char* argv[])
{
    using namespace Frontend;

    QCoreApplication::setApplicationName("Net64-Coop");
    QCoreApplication::setOrganizationName("Net64 Project");
    QCoreApplication::setApplicationVersion({BuildInfo::GIT_DESC});

    AppSettings settings;
    settings.appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
    create_app_dirs(settings);

    setup_logging();

    // Load config file
    settings.load(settings.main_config_file_path());

    int ret{};
    {
        QApplication app{argc, argv};

        // Log system information
        auto logger{Core::get_logger("frontend")};
        logger->info("Starting {} {}", QCoreApplication::applicationName().toStdString(),
                     QCoreApplication::applicationVersion().toStdString());
        logger->info("Operating system: {} {}", QSysInfo::productType().toStdString(),
                     QSysInfo::productVersion().toStdString());
        logger->info("Kernel: {} {}", QSysInfo::kernelType().toStdString(), QSysInfo::kernelVersion().toStdString());
        logger->info("CPU architecture: buildtime={} runtime={}", QSysInfo::buildCpuArchitecture().toStdString(),
                     QSysInfo::currentCpuArchitecture().toStdString());
        logger->info("Byte order: {}", QSysInfo::ByteOrder == QSysInfo::LittleEndian ? "Little" : "Big");
        logger->info("Configuration folder: {}", settings.appdata_path);

        MainFrame win(nullptr, settings);

        win.show();

        ret = QApplication::exec();
    }

    // Store settings
    settings.save(settings.main_config_file_path());

    return ret;
}


