//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include <cassert>

#include "shared_object.hpp"

#ifdef __linux__
    #include <dlfcn.h>
#endif


SharedObjectHandle::SharedObjectHandle(const char* file): Base(load_shared_object(file))
{
}

SharedObjectHandle::SharedObjectHandle(const std::string& file): SharedObjectHandle(file.c_str())
{
}

void* SharedObjectHandle::load_function(const char* name)
{
    assert(get());

    return ::load_function(get(), name);
}

void* SharedObjectHandle::load_function(const std::string& name)
{
    return load_function(name.c_str());
}


#ifdef __linux__

shared_object_t load_shared_object(const char* file)
{
    return dlopen(file, RTLD_LAZY);
}

shared_object_t shared_object_from_current_proc()
{
    return dlopen(nullptr, RTLD_LAZY);
}

bool unload_shared_object(shared_object_t object)
{
    return dlclose(object) == 0;
}

void* load_function(shared_object_t object, const char* name)
{
    return dlsym(object, name);
}

std::string get_shared_object_error()
{
    auto msg{dlerror()};
    return {msg ? msg : ""};
}

#elif defined _WIN32

shared_object_t load_shared_object(const char* file)
{
    fs::path lib_path{file};

    // Disable the warning popup
    auto old_error_mode{GetThreadErrorMode()};
    SetThreadErrorMode(SEM_FAILCRITICALERRORS, nullptr);

    // Add the file directory to the search path
    // so that dependencies of the lib get loaded correctly
    SetDllDirectoryA(lib_path.parent_path().string().c_str());

    auto obj{LoadLibraryA(file)};

    // Remove dir again
    SetDllDirectory(nullptr);

    // Restore old warning behaviour
    SetThreadErrorMode(old_error_mode, nullptr);

    return obj;
}

shared_object_t shared_object_from_current_proc()
{
    shared_object_t module;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)shared_object_from_current_proc, &module);
    return module;
}

bool unload_shared_object(shared_object_t object)
{
    return FreeLibrary(object) != 0;
}

void* load_function(shared_object_t object, const char* name)
{
    return GetProcAddress(object, name);
}

std::string get_shared_object_error()
{
    return std::to_string(GetLastError());
}

#endif
