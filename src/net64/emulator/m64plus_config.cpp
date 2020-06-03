//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "net64/emulator/m64plus_config.hpp"


namespace Net64::Emulator::M64PlusHelper
{
Config::Config(dynlib_t core_lib): core_(core_lib)
{
    load_symbols();
}

ConfigSection Config::open_section(const char* name)
{
    ConfigSection::cfg_section_hdl_t hdl{};

    auto ret = fn_.config_open_section(name, &hdl);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return ConfigSection(core_, hdl);
}

void Config::save()
{
    auto ret = fn_.config_save_file();

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void Config::save(const char* section)
{
    auto ret = fn_.config_save_section(section);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void Config::delete_section(const char* section)
{
    auto ret = fn_.config_delete_section(section);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

bool Config::has_unsaved_changes(const char* section) const
{
    return static_cast<bool>(fn_.config_has_unsaved_changes(section));
}

void Config::revert_changes(const char* section)
{
    auto ret = fn_.config_revert_changes(section);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void Config::load_symbols()
{
    auto resolve{[this](auto& ptr, const char* name) {
        ptr = reinterpret_cast<std::remove_reference_t<decltype(ptr)>>(get_symbol(core_, name));
        if(!ptr)
            throw std::system_error(make_error_code(Error::SYM_NOT_FOUND));
    }};

    resolve(fn_.config_delete_section, "ConfigDeleteSection");
    resolve(fn_.config_save_file, "ConfigSaveFile");
    resolve(fn_.config_has_unsaved_changes, "ConfigHasUnsavedChanges");
    resolve(fn_.config_open_section, "ConfigOpenSection");
    resolve(fn_.config_list_sections, "ConfigListSections");
    resolve(fn_.config_revert_changes, "ConfigRevertChanges");
    resolve(fn_.config_save_section, "ConfigSaveSection");
}


ConfigSection::ConfigSection(dynlib_t core_lib, cfg_section_hdl_t section_hdl):
    core_(core_lib), section_hdl_(section_hdl)
{
    load_symbols();
}

void ConfigSection::set_default(const char* param, int value, const char* help_str)
{
    auto ret = fn_.config_set_default_int(section_hdl_, param, value, help_str);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set_default(const char* param, float value, const char* help_str)
{
    auto ret = fn_.config_set_default_float(section_hdl_, param, value, help_str);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set_default(const char* param, bool value, const char* help_str)
{
    auto ret = fn_.config_set_default_bool(section_hdl_, param, value, help_str);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set_default(const char* param, const char* value, const char* help_str)
{
    auto ret = fn_.config_set_default_string(section_hdl_, param, value, help_str);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set(const char* param, int value)
{
    auto ret = fn_.config_set_parameter(section_hdl_, param, param_type_t::M64TYPE_INT, &value);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set(const char* param, float value)
{
    auto ret = fn_.config_set_parameter(section_hdl_, param, param_type_t::M64TYPE_FLOAT, &value);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set(const char* param, bool value)
{
    auto int_val{static_cast<int>(value)};
    auto ret = fn_.config_set_parameter(section_hdl_, param, param_type_t::M64TYPE_BOOL, &int_val);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

void ConfigSection::set(const char* param, const char* value)
{
    auto ret = fn_.config_set_parameter(section_hdl_, param, param_type_t::M64TYPE_STRING, value);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));
}

const char* ConfigSection::help(const char* param)
{
    return fn_.config_get_parameter_help(section_hdl_, param);
}

ConfigSection::param_type_t ConfigSection::type(const char* param)
{
    param_type_t type;

    auto ret = fn_.config_get_parameter_type(section_hdl_, param, &type);

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return type;
}

int ConfigSection::get_int(const char* param)
{
    int value;
    auto ret = fn_.config_get_parameter(section_hdl_, param, param_type_t::M64TYPE_INT, &value, sizeof(value));

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return value;
}

float ConfigSection::get_float(const char* param)
{
    float value;
    auto ret = fn_.config_get_parameter(section_hdl_, param, param_type_t::M64TYPE_FLOAT, &value, sizeof(value));

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return value;
}

bool ConfigSection::get_bool(const char* param)
{
    int value;
    auto ret = fn_.config_get_parameter(section_hdl_, param, param_type_t::M64TYPE_BOOL, &value, sizeof(value));

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return static_cast<bool>(value);
}

std::string ConfigSection::get_string(const char* param)
{
    std::string value(STRING_BUF_LEN, '\0');
    auto ret =
        fn_.config_get_parameter(section_hdl_, param, param_type_t::M64TYPE_STRING, value.data(), (int)value.size());

    if(ret != Error::SUCCESS)
        throw std::system_error(make_error_code(ret));

    return value;
}

void ConfigSection::load_symbols()
{
    auto resolve{[this](auto& ptr, const char* name) {
        ptr = reinterpret_cast<std::remove_reference_t<decltype(ptr)>>(get_symbol(core_, name));
        if(!ptr)
            throw std::system_error(make_error_code(Error::SYM_NOT_FOUND));
    }};

    resolve(fn_.config_list_parameters, "ConfigListParameters");
    resolve(fn_.config_get_parameter, "ConfigGetParameter");
    resolve(fn_.config_get_parameter_help, "ConfigGetParameterHelp");
    resolve(fn_.config_get_parameter_type, "ConfigGetParameterType");
    resolve(fn_.config_set_default_bool, "ConfigSetDefaultBool");
    resolve(fn_.config_set_default_float, "ConfigSetDefaultFloat");
    resolve(fn_.config_set_default_int, "ConfigSetDefaultInt");
    resolve(fn_.config_set_default_string, "ConfigSetDefaultString");
    resolve(fn_.config_set_parameter, "ConfigSetParameter");
}

} // namespace Net64::Emulator::M64PlusHelper
