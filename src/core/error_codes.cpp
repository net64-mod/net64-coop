//
// Created by henrik on 18.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "error_codes.hpp"


namespace
{

const struct CoreErrorCategory : std::error_category
{
    const char* name() const noexcept override;
    std::string message(int ev) const override;
}g_core_error_category;

const char* CoreErrorCategory::name() const noexcept
{
    return "net64core";
}

std::string CoreErrorCategory::message(int ev) const
{
    using namespace Core;

    switch(static_cast<ErrorCode>(ev))
    {
        case ErrorCode::INVALID_OFFSET:
            return "Trying to access memory out-of-bounds";
        case ErrorCode::FAILED_PROC_START:
            return "Failed to start child process";
        case ErrorCode::EMU_NOT_RUNNING:
            return "Emulator not running";
        default:
            return "Unkown error";
    }
}

} // anonymous

std::error_code make_error_code(Core::ErrorCode e)
{
    return {static_cast<int>(e), g_core_error_category};
}
