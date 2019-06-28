//
// Created by henrik on 07.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <experimental/filesystem>


namespace Frontend
{

namespace fs = std::experimental::filesystem;

struct AppSettings
{
    // Not saved to config file:
    fs::path appdata_path;

    // Load / store functions
    bool load(const fs::path& file);
    bool save(const fs::path& file);


    static const char* MAIN_CONFIG_SUB_DIR,
                     * MAIN_CONFIG_FILENAME;

    fs::path main_config_dir() const;
    fs::path main_config_file_path() const;

    fs::path rom_file_path;


    // Mupen64Plus configuration
    static const char* M64P_DEFAULT_PLUGIN_DIR,
                     * M64P_DEFAULT_ROOT_DIR;

    fs::path m64p_dir() const;

    fs::path m64p_plugin_dir{M64P_DEFAULT_PLUGIN_DIR};
    std::string m64p_video_plugin,
                m64p_audio_plugin,
                m64p_core_plugin,
                m64p_rsp_plugin,
                m64p_input_plugin;
};

} // Frontend
