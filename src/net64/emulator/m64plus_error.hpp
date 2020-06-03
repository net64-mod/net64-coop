//
// Created by henrik on 10.02.20.
// Copyright 2020 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <system_error>

#include <mupen64plus/m64p_types.h>


namespace Net64::Emulator::M64PlusHelper
{
/// Mupen64Plus error codes
enum struct Error
{
    // Mupen64Plus internal errors
    SUCCESS = 0,
    NOT_INIT,        ///< Function is disallowed before InitMupen64Plus() is called
    ALREADY_INIT,    ///< InitMupen64Plus() was called twice
    INCOMPATIBLE,    ///< API versions between components are incompatible
    INPUT_ASSERT,    ///< Invalid parameters for function call, such as ParamValue=NULL for GetCoreParameter()
    INPUT_INVALID,   ///< Invalid input data, such as ParamValue="maybe" for SetCoreParameter() to set a BOOL-type value
    INPUT_NOT_FOUND, ///< The input parameter(s) specified a particular item which was not found
    NO_MEMORY,       ///< Memory allocation failed
    FILES,           ///< Error opening, creating, reading, or writing to a file
    INTERNAL,        ///< Internal error (bug)
    INVALID_STATE,   ///< Current program state does not allow operation
    PLUGIN_FAIL,     ///< A plugin function returned a fatal error
    SYSTEM_FAIL,     ///< A system function call, such as an SDL or file operation, failed
    UNSUPPORTED,     ///< Function call is not supported (ie, core not built with debugger)
    WRONG_TYPE,      ///< A given input type parameter cannot be used for desired operation
                     // Interface errors
    LIB_LOAD_FAILED, ///< Failed to load library file
    INVALID_ADDR,    ///< Tried to access out of bounds memory
    SYM_NOT_FOUND    ///< A symbol required by the API could not be located in the specified module
};

/// Overload for Mupen64Plus error codes
std::error_code make_error_code(Error e) noexcept;

} // namespace Net64::Emulator::M64PlusHelper
