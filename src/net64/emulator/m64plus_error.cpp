//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#include "net64/emulator/m64plus_error.hpp"

namespace
{
const struct M64PlusErrorCategory : std::error_category
{
    const char* name() const noexcept override { return "mupen64plus"; }
    std::string message(int ev) const override
    {
        using Net64::Emulator::M64PlusHelper::Error;

        switch(static_cast<Error>(ev))
        {
        case Error::NOT_INIT:
            return "Function is disallowed before InitMupen64Plus() is called";
        case Error::ALREADY_INIT:
            return "InitMupen64Plus() was called twice";
        case Error::INCOMPATIBLE:
            return "API versions between components are incompatible";
        case Error::INPUT_ASSERT:
            return "Invalid parameters for function call";
        case Error::INPUT_INVALID:
            return "Invalid input data";
        case Error::INPUT_NOT_FOUND:
            return "The input parameter(s) specified a particular item which was not found";
        case Error::NO_MEMORY:
            return "Memory allocation failed";
        case Error::FILES:
            return "Error opening, creating, reading, or writing to a file";
        case Error::INTERNAL:
            return "Internal error (bug)";
        case Error::INVALID_STATE:
            return "Current program state does not allow operation";
        case Error::PLUGIN_FAIL:
            return "A plugin function returned a fatal error";
        case Error::SYSTEM_FAIL:
            return "A system function call, such as an SDL or file operation, failed";
        case Error::UNSUPPORTED:
            return "Function call is not supported";
        case Error::WRONG_TYPE:
            return "A given input type parameter cannot be used for desired operation";

        case Error::LIB_LOAD_FAILED:
            return "Failed to load library file";
        case Error::INVALID_ADDR:
            return "Tried to access out of bounds memory";
        case Error::SYM_NOT_FOUND:
            return "A symbol required by the API could not be located in the specified module";
        default:
            return "[Unknown Error]";
        }
    }
} m64p_error_category_g;

} // namespace

namespace Net64::Emulator::M64PlusHelper
{
std::error_code make_error_code(Error e) noexcept
{
    return {static_cast<int>(e), m64p_error_category_g};
}

} // namespace Net64::Emulator::M64PlusHelper
