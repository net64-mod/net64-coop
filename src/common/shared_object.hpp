//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <string>

#include "common/resource_handle.hpp"
#include "filesystem.hpp"


#ifdef __linux__
using shared_object_t = void*;
#elif defined _WIN32
    #include <Windows.h>
using shared_object_t = HMODULE;
#endif


shared_object_t load_shared_object(const char* file);

shared_object_t shared_object_from_current_proc();

bool unload_shared_object(shared_object_t object);

void* load_function(shared_object_t object, const char* name);

std::string get_shared_object_error();

template<typename T>
void load_function(shared_object_t object, T& fn_ptr, const char* name)
{
    fn_ptr = reinterpret_cast<T>(load_function(object, name));
}


struct SharedObjectHandle : ResourceHandle<&unload_shared_object>
{
    using Base = ResourceHandle<&unload_shared_object>;

    using Base::Base;

    explicit SharedObjectHandle(const char* file);

    explicit SharedObjectHandle(const std::string& file);

    void* load_function(const char* name);

    void* load_function(const std::string& name);

    template<typename T, typename Str>
    void load_function(T& fn_ptr, const Str& name)
    {
        fn_ptr = reinterpret_cast<T>(load_function(name));
    }
};
