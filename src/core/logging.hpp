//
// Created by henrik on 10.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <system_error>
#include <string_view>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/null_sink.h>


namespace Core
{

using LoggerPtr = std::shared_ptr<spdlog::logger>;
using SinkFactory = std::function<std::vector<spdlog::sink_ptr>()>;

void init_logging_sinks(const SinkFactory& fn);
std::vector<spdlog::sink_ptr> get_logging_sinks();
std::shared_ptr<spdlog::logger> get_logger(std::string_view name);

#define CLASS_LOGGER_N_(name, fn_name) static spdlog::logger* fn_name() \
                                       { \
                                           static ::Core::LoggerPtr logger_s{::Core::get_logger(name)}; \
                                           return logger_s.get(); \
                                       }

#define CLASS_LOGGER_(name) CLASS_LOGGER_N_(name, logger)

} // Core
