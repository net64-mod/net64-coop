//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "shared_library.hpp"

#include <utility>
#ifdef __linux__
#include <dlfcn.h>
#endif


namespace Core::Emulator
{

UniqueLib::UniqueLib(dynlib_t hdl)
:lib{hdl}
{}

UniqueLib::UniqueLib(UniqueLib&& other) noexcept
:lib(other.lib)
{
    other.lib = nullptr;
}

UniqueLib& UniqueLib::operator=(UniqueLib&& other) noexcept
{
    std::swap(lib, other.lib);

    return *this;
}

UniqueLib::~UniqueLib()
{
    if(lib)
        free_library(lib);
}

void UniqueLib::reset(dynlib_t hdl)
{
    if(lib)
        free_library(lib);
    lib = hdl;
}

#ifdef __linux__

dynlib_t load_library(const char* lib_file)
{
    return dlopen(lib_file, RTLD_LAZY);
}

dynlib_t get_current_process()
{
    return dlopen(nullptr, RTLD_LAZY);
}

bool free_library(dynlib_t lib)
{
    return dlclose(lib) == 0;
}

void* get_symbol(dynlib_t lib, const char* symbol_name)
{
    return dlsym(lib, symbol_name);
}

std::string get_lib_error_msg()
{
    auto msg{dlerror()};
    return {msg ? msg : ""};
}


#elif defined _WIN32

dynlib_t load_library(const char* lib_file)
{
    fs::path lib_path{lib_file};

	// Disable the warning popup
	auto old_error_mode{ GetThreadErrorMode() };
	SetThreadErrorMode(SEM_FAILCRITICALERRORS, nullptr);

	// Add the file directory to the search path
	// so that dependencies of the lib get loaded correctly
	SetDllDirectoryA(lib_path.parent_path().string().c_str());

	auto lib{LoadLibraryA(lib_file)};

	// Remove dir again
	SetDllDirectory(nullptr);

	// Restore old warning behaviour
	SetThreadErrorMode(old_error_mode, nullptr);

	return lib;
}

dynlib_t get_current_process()
{
    dynlib_t module;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)get_current_process, &module);
    return module;
}

bool free_library(dynlib_t lib)
{
    return FreeLibrary(lib) != 0;
}

void* get_symbol(dynlib_t lib, const char* symbol_name)
{
    return GetProcAddress(lib, symbol_name);
}

std::string get_lib_error_msg()
{
    return std::to_string(GetLastError());
}

#endif

} // Core::Emulator
