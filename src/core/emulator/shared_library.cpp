//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "shared_library.hpp"


#ifdef __linux__
#include <dlfcn.h>

dynlib_t Core::Emulator::load_library(const char* lib_path)
{
    return dlopen(lib_path, RTLD_NOW);
}

dynlib_t Core::Emulator::get_current_library()
{
    return dlopen(nullptr, RTLD_NOW);
}

bool Core::Emulator::free_library(dynlib_t lib)
{
    return dlclose(lib) == 0;
}

void* Core::Emulator::get_symbol(dynlib_t lib, const char* symbol_name)
{
    return dlsym(lib, symbol_name);
}

#elif defined _WIN32

dynlib_t Core::Emulator::load_library(const char* lib_path)
{
    return LoadLibraryA(lib_path);
}

dynlib_t Core::Emulator::get_current_library()
{
    dynlib_t module;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)GetCurrentModule(), &module);
    return module;
}

bool Core::Emulator::free_library(dynlib_t lib)
{
    return FreeLibrary(lib) != 0;
}

bool Core::Emulator::get_symbol(dynlib_t lib, const char* symbol_name)
{
    return GetProcAddress(lib, symbol_name);
}

#endif
