//
// Created by henrik on 02.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "filesystem.hpp"
#include <iostream>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <SDL.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "build_info.hpp"
#include "net64/net64.hpp"
#include "qt_gui/app_settings.hpp"
#include "qt_gui/main_window.hpp"


namespace Frontend
{

using Net64::LoggerPtr;

static void set_default_mupen_plugins(AppSettings& settings)
{
    using Net64::Emulator::M64PlusHelper::Plugin;

    // Helper function
    auto set_plugin_if_empty{[](auto& config_string, const auto& file_path, auto plugin_type)
    {
        if(config_string.empty())
        {
            if(file_path.string().find(AppSettings::M64P_DEFAULT_PLUGINS[plugin_type]) != std::string::npos &&
               fs::is_regular_file(file_path) && Plugin::get_plugin_info(file_path.string()).type == plugin_type)
            {
                config_string = file_path.filename().string();
            }
            else
            {
                Net64::get_logger("frontend")->warn("Did not find default {} plugin",
                                  Plugin::type_str(M64PLUGIN_CORE));
            }
        }
    }};

    for(auto dir_entry : fs::directory_iterator(settings.m64p_plugin_dir()))
    {
        set_plugin_if_empty(settings.m64p_core_plugin, dir_entry.path(), M64PLUGIN_CORE);
        set_plugin_if_empty(settings.m64p_video_plugin, dir_entry.path(), M64PLUGIN_GFX);
        set_plugin_if_empty(settings.m64p_audio_plugin, dir_entry.path(), M64PLUGIN_AUDIO);
        set_plugin_if_empty(settings.m64p_input_plugin, dir_entry.path(), M64PLUGIN_INPUT);
        set_plugin_if_empty(settings.m64p_rsp_plugin, dir_entry.path(), M64PLUGIN_RSP);
    }
}

static void install_routine(AppSettings& settings)
{
    // Copy mupen64plus binaries
    if(!fs::exists(settings.m64p_default_plugin_dir()) || fs::is_empty(settings.m64p_default_plugin_dir()))
    {
        try
        {
            fs::create_directories(settings.m64p_default_plugin_dir());
            fs::copy(settings.shipped_m64p_binaries_dir(), settings.m64p_default_plugin_dir(), fs::copy_options::recursive);
            std::cout << "Mupen64Plus bin dir missing. Copied default binaries\n";
        }
        catch(const std::exception& e)
        {
            std::cout << "Failed to copy Mupen64Plus binaries: " << e.what() << '\n';
        }
    }

    // Create config dir
    try
    {
        fs::create_directories(settings.main_config_dir());
    }
    catch(const std::exception& e)
    {
        std::cout << "Failed to create config dir: " << e.what() << '\n';
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
    Net64::init_logging_sinks([global_log_file, stdout_sink]{
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

    QCoreApplication::setApplicationName("Net64");
    QCoreApplication::setOrganizationName("Net64 Project");
    QCoreApplication::setApplicationVersion({BuildInfo::GIT_DESC});

    AppSettings settings;
    settings.appdata_path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation).toStdString();
    install_routine(settings);

    setup_logging();
    auto logger{Net64::get_logger("frontend")};

    // Load config file
    settings.load(settings.main_config_file_path());
    set_default_mupen_plugins(settings);

    // Initialize ENet
    if(!Net64::initialize())
    {
        logger->critical("Failed to Net64 library");
        return EXIT_FAILURE;
    }

    if(SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) != 0)
    {
        logger->critical("Failed to initialize SDL: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    int ret{};
    {
        QApplication app{argc, argv};

        // Log system information
        logger->info("Starting {} {}", QCoreApplication::applicationName().toStdString(),
                     QCoreApplication::applicationVersion().toStdString());
        logger->info("Operating system: {} {}", QSysInfo::productType().toStdString(),
                     QSysInfo::productVersion().toStdString());
        logger->info("Kernel: {} {}", QSysInfo::kernelType().toStdString(), QSysInfo::kernelVersion().toStdString());
        logger->info("CPU architecture: buildtime={} runtime={}", QSysInfo::buildCpuArchitecture().toStdString(),
                     QSysInfo::currentCpuArchitecture().toStdString());
        logger->info("Byte order: {}", QSysInfo::ByteOrder == QSysInfo::LittleEndian ? "Little" : "Big");
        logger->info("Configuration folder: {}", settings.appdata_path);

        MainWindow win(settings);

        win.show();

        ret = QApplication::exec();
    }

    // Store settings
    settings.save(settings.main_config_file_path());

    Net64::deinitialize();

    SDL_Quit();

    return ret;
}


