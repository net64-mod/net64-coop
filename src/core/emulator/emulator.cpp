//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "emulator.hpp"


namespace Core::Emulator
{

EmulatorBase::EmulatorBase(std::string emu_name)
:name_{std::move(emu_name)}
{}

std::string_view EmulatorBase::name() const
{
    return name_;
}

namespace
{

const struct EmulatorErrorCondCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "emulator_error_condition";
    }
    std::string message(int ev) const override
    {
        switch(static_cast<EmulatorErrorCond>(ev))
        {
        case EmulatorErrorCond::MEM_ACCESS_VIOLATION:
            return "";
        case EmulatorErrorCond::NOT_RUNNING:
            return "";
        case EmulatorErrorCond::ROM_EXEC_FAIL:
            return "";
        case EmulatorErrorCond::ROM_LOAD_FAIL:
            return "";
        default:
            return "[Unknown Condition]";
        }
    }
    bool equivalent(const std::error_code& code, int cond) const noexcept override
    {
        switch(static_cast<EmulatorErrorCond>(cond))
        {

        }
    }
};

} // anonymous

} // Core::Emulator
