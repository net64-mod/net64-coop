//
// Created by henrik on 10.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "logging.hpp"


namespace Net64
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
    static SinkFactory sink_fn_s{};

    return sink_fn_s;
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

std::shared_ptr<spdlog::logger> get_logger(const std::string& name)
{
    [[maybe_unused]]
    static LoggerPtr root_logger_s = []{
        auto sinks{get_logging_sinks()};

        auto root_logger{std::make_shared<spdlog::logger>("root", sinks.begin(), sinks.end())};
        root_logger->set_level(spdlog::level::trace);

        spdlog::register_logger(root_logger);

        return root_logger;
    }();

    auto logger{spdlog::get(name)};

    if(logger)
        return logger;

    logger = root_logger_s->clone(name);
    spdlog::register_logger(logger);

    return logger;
}

} // Net64
