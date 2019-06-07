//
// Created by henrik on 02.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

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

bool create_app_dirs()
{
    if(!QDir{}.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)))
    {
        std::clog << "Failed to create AppLocalDataLocation: ";
        std::clog << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString() << '\n';
        return false;
    }

    return true;
}

void setup_logging()
{
    auto global_log_file{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString() +
            "/log.txt", true)
    };

    global_log_file->set_level(spdlog::level::debug);

    // Console logging for Frontend
    auto stdout_sink{std::make_shared<spdlog::sinks::stderr_color_sink_mt>()};
    stdout_sink->set_pattern("[%^%l%$] [%n] %v");
    stdout_sink->set_level(spdlog::level::info);
    LoggerPtr frontend_logger{new spdlog::logger("frontend", {global_log_file, stdout_sink})};
    spdlog::register_logger(frontend_logger);

    // Setup logging of Core
    Core::init_logging_sinks([global_log_file]{
        std::vector<spdlog::sink_ptr> sinks{
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>(), global_log_file
        };

        sinks[0]->set_pattern("[%^%l%$] [core::%n] %v");
        sinks[0]->set_level(spdlog::level::info);

        return sinks;
    });
}

} // Frontend

int main(int argc, char* argv[])
{
    using namespace Frontend;

    AppSettings settings;
    int ret{};
    {
        QCoreApplication::setApplicationName("Net64-Coop");
        QCoreApplication::setOrganizationName("Net64 Project");
        QCoreApplication::setApplicationVersion({BuildInfo::GIT_DESC});

        if(!create_app_dirs())
            return 1;
        setup_logging();

        // Load config file
        settings.appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
        settings.load(settings.main_config_file_path());


        QApplication app{argc, argv};

        // Log system information
        auto logger{spdlog::get("frontend")};
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


