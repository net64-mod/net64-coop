//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <experimental/filesystem>
#include <string>

#ifdef __linux__
    using dynlib_t = void*;
#elif defined _WIN32
    #include <Windows.h>
    #undef max
    using dynlib_t = HMODULE;
#endif


namespace Core::Emulator
{

namespace fs = std::experimental::filesystem;

struct UniqueLib
{
    UniqueLib() = default;

    explicit UniqueLib(dynlib_t hdl);

    UniqueLib(const UniqueLib&) = delete;

    UniqueLib(UniqueLib&& other) noexcept;

    UniqueLib& operator=(UniqueLib&& other) noexcept;

    ~UniqueLib();

    void reset(dynlib_t hdl = nullptr);


    dynlib_t lib{nullptr};
};

dynlib_t load_library(const fs::directory_entry& lib_file);

dynlib_t get_current_process();

bool free_library(dynlib_t lib);

void* get_symbol(dynlib_t lib, const char* symbol_name);

std::string get_lib_error_msg();

} // Core::Emulator
