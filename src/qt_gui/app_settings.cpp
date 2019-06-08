//
// Created by henrik on 07.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "app_settings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/logging.hpp"


namespace Frontend
{

const char* AppSettings::MAIN_CONFIG_SUB_DIR{"config/"};
const char* AppSettings::MAIN_CONFIG_FILENAME{"config.json"};

const char* AppSettings::M64P_DEFAULT_PLUGIN_DIR{"../emulator/mupen64plus/"};
const char* AppSettings::M64P_DEFAULT_CONFIG_SUB_DIR{"config/mupen64plus/config/"};
const char* AppSettings::M64P_DEFAULT_DATA_SUB_DIR{"config/mupen64plus/data/"};


bool AppSettings::load(const fs::path& file)
{
    try
    {
        nlohmann::json json;

        // load file
        std::ifstream config_file(file.string());
        if(!config_file)
            throw std::runtime_error(fmt::format("Failed to open config file: {}", file.string()));

        config_file >> json;

        config_file.close();


        auto& m64p_obj{json["mupen64plus"]};

        // Helper function
        auto load_string = [](auto& value, const auto& json_obj)
        {
            value = json_obj.get<std::string>();
        };

        // Load last selected rom file
        load_string(rom_file_path, json["rom_file"]);

        // Load mupen64plus plugin directory
        load_string(m64p_plugin_dir, m64p_obj["plugin_dir"]);

        // Load last selected mupen64plus plugins
        load_string(m64p_core_plugin, m64p_obj["core_plugin"]);
        load_string(m64p_video_plugin, m64p_obj["video_plugin"]);
        load_string(m64p_audio_plugin, m64p_obj["audio_plugin"]);
        load_string(m64p_input_plugin, m64p_obj["input_plugin"]);
        load_string(m64p_rsp_plugin, m64p_obj["rsp_plugin"]);

    }
    catch(const std::exception& e)
    {
        Core::get_logger("frontend")->warn("Failed to load config file: {}", e.what());
        return false;
    }

    return true;
}

bool AppSettings::save(const fs::path& file)
{
    try
    {
        nlohmann::json json;

        // Store currently selected rom file
        json["rom_file"] = rom_file_path.string();

        auto& m64p_obj{json["mupen64plus"]};

        // Store mupen64plus plugin directory
        m64p_obj["plugin_dir"] = m64p_plugin_dir.string();

        // Store currently selected mupen64plus plugins
        m64p_obj["video_plugin"] = m64p_video_plugin;
        m64p_obj["audio_plugin"] = m64p_audio_plugin;
        m64p_obj["input_plugin"] = m64p_input_plugin;
        m64p_obj["rsp_plugin"] = m64p_rsp_plugin;
        m64p_obj["core_plugin"] = m64p_core_plugin;


        // Write to file
        std::ofstream config_file(file.string());
        if(!config_file)
            throw std::runtime_error(fmt::format("Failed to open config file: {}", file.string()));

        config_file << json.dump(4);
    }
    catch(const std::exception& e)
    {
        Core::get_logger("frontend")->warn("Failed to save config file: {}", e.what());
        return false;
    }

    return true;
}

fs::path AppSettings::main_config_dir() const
{
    return appdata_path / MAIN_CONFIG_SUB_DIR;
}

fs::path AppSettings::main_config_file_path() const
{
    return main_config_dir() / MAIN_CONFIG_FILENAME;
}

fs::path AppSettings::m64p_config_dir() const
{
    return appdata_path / M64P_DEFAULT_CONFIG_SUB_DIR;
}

fs::path AppSettings::m64p_data_dir() const
{
    return appdata_path / M64P_DEFAULT_DATA_SUB_DIR;
}

}
