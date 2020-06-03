//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <string>
#include <mupen64plus/m64p_types.h>
#include "net64/emulator/m64plus_error.hpp"
#include "net64/emulator/shared_library.hpp"


namespace Net64::Emulator::M64PlusHelper
{

struct ConfigSection;

struct Config
{
    explicit Config(dynlib_t core_lib);

    ConfigSection open_section(const char* name);

    template<typename Fn>
    void list_sections(const Fn& fn)
    {
        auto ret = fn_.config_list_sections(const_cast<Fn*>(&fn), [](void* context, const char* section)
        {
            (*reinterpret_cast<const Fn*>(context))(section);
        });

        if(ret != Error::SUCCESS)
            throw std::system_error(make_error_code(ret));
    }

    void save();

    void save(const char* section);

    void delete_section(const char* section);

    bool has_unsaved_changes(const char* section) const;

    void revert_changes(const char* section);

private:
    void load_symbols();

    dynlib_t core_;

    struct
    {
        Error(*config_list_sections)(void*, void(*)(void*, const char*));
        Error(*config_open_section)(const char*, m64p_handle*);
        int(*config_has_unsaved_changes)(const char*);
        Error(*config_delete_section)(const char*);
        Error(*config_save_file)();
        Error(*config_save_section)(const char*);
        Error(*config_revert_changes)(const char*);
    }fn_{};
};

struct ConfigSection
{
    using cfg_section_hdl_t = m64p_handle;
    using param_type_t = m64p_type;

    static constexpr std::size_t STRING_BUF_LEN{128};

    ConfigSection(dynlib_t core_lib, cfg_section_hdl_t section_hdl);

    template<typename Fn>
    void list_parameters(const Fn& fn)
    {
        auto ret = fn_.config_list_parameters(const_cast<Fn*>(&fn), [](void* context, const char* name, param_type_t type)
        {
            (*reinterpret_cast<const Fn*>(context))(name, type);
        });

        if(ret != Error::SUCCESS)
            throw std::system_error(make_error_code(ret));
    }

    void set_default(const char* param, int value, const char* help_str);
    void set_default(const char* param, float value, const char* help_str);
    void set_default(const char* param, bool value, const char* help_str);
    void set_default(const char* param, const char* value, const char* help_str);

    void set(const char* param, int value);
    void set(const char* param, float value);
    void set(const char* param, bool value);
    void set(const char* param, const char* value);

    const char* help(const char* param);
    param_type_t type(const char* param);

    int get_int(const char* param);
    float get_float(const char* param);
    bool get_bool(const char* param);
    std::string get_string(const char* param);

private:
    void load_symbols();

    dynlib_t core_;
    cfg_section_hdl_t section_hdl_;

    struct
    {
        Error(*config_list_parameters)(cfg_section_hdl_t, void*, void(*)(void*, const char*, param_type_t));
        Error(*config_set_parameter)(m64p_handle, const char*, m64p_type, const void*);
        Error(*config_get_parameter)(m64p_handle, const char*, m64p_type, void*, int);
        Error(*config_get_parameter_type)(m64p_handle, const char*, m64p_type*);
        const char*(*config_get_parameter_help)(m64p_handle, const char*);
        Error(*config_set_default_int)(m64p_handle, const char*, int, const char*);
        Error(*config_set_default_float)(m64p_handle, const char*, float, const char*);
        Error(*config_set_default_bool)(m64p_handle, const char*, int, const char*);
        Error(*config_set_default_string)(m64p_handle, const char*, const char*, const char*);
    }fn_{};
};

}
