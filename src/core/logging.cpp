//
// Created by henrik on 10.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "logging.hpp"


namespace Core
{

struct LoggingSinks
{
    friend void (init_logging_sinks)(const SinkFactory&);
    friend std::vector<spdlog::sink_ptr> (get_logging_sinks)();

private:
    static SinkFactory& get_sink_fn();
};

SinkFactory& LoggingSinks::get_sink_fn()
{
    static SinkFactory s_sink_fn{};

    return s_sink_fn;
}

void init_logging_sinks(const SinkFactory& fn)
{
    LoggingSinks::get_sink_fn() = fn;
}

std::vector<spdlog::sink_ptr> get_logging_sinks()
{
    if(!LoggingSinks::get_sink_fn())
        return std::vector<spdlog::sink_ptr>{std::make_shared<spdlog::sinks::null_sink_mt>()};
    else
        return LoggingSinks::get_sink_fn()();
}

std::shared_ptr<spdlog::logger> get_logger(std::string_view name)
{
    [[maybe_unused]]
    static bool s_initialized = []{
        auto sinks{get_logging_sinks()};

        auto emu_logger{std::make_shared<spdlog::logger>("emulator", sinks.begin(), sinks.end())};
        spdlog::register_logger(emu_logger);
        spdlog::register_logger(emu_logger->clone("process"));
        spdlog::register_logger(emu_logger->clone("memory"));

        return true;
    }();

    return spdlog::get(std::string(name));
}

} // Core
