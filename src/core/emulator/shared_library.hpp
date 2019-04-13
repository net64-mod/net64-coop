//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#ifdef __linux__
    using dynlib_t = void*;
#elif defined _WIN32
    #include <Windows.h>
    #undef max
    using dynlib_t = HMODULE;
#endif


namespace Core::Emulator
{

dynlib_t load_library(const char* lib_path);

dynlib_t get_current_process();

bool free_library(dynlib_t lib);

void* get_symbol(dynlib_t lib, const char* symbol_name);

} // Core::Emulator
