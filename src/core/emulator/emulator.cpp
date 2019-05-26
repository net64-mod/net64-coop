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

spdlog::logger* EmulatorBase::logger()
{
    static LoggerPtr logger_s{[this]()
    {
        auto logger{get_logger("emulator")->clone(name_)};
        spdlog::register_logger(logger);
        return logger;
    }()};

    return logger_s.get();
}

} // Core::Emulator
