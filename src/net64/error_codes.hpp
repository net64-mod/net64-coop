//
// Created by henrik on 18.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <system_error>


namespace Net64
{

/// Error codes for namespace Net64
enum struct ErrorCode
{
    INVALID_OFFSET = 1,    ///< Trying to access invalid memory offset
    FAILED_PROC_START,     ///< Failed to start child process
    EMU_NOT_RUNNING        ///< Emulator not running
};

} // Net64

std::error_code make_error_code(Net64::ErrorCode e);

namespace std
{

template<>
struct is_error_code_enum<::Net64::ErrorCode> : std::true_type{};

}
