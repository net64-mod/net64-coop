//
// Created by henrik on 03.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <type_traits>
#include "core/memory/util.hpp"


namespace Core::Memory
{

namespace Impl
{
template<typename T>
using if_no_specialized_conversion_t = std::enable_if_t<!(std::is_integral_v<T>)>;
template<typename T>
constexpr bool is_multibyte_int_v  = (std::is_integral_v<T> && sizeof(T) != 1);
template<typename T>
constexpr bool is_singlebyte_int_v = (std::is_integral_v<T> && sizeof(T) == 1);
} // Impl


// Do not convert anything unless specified

/**
 * Convert n64 value to native representation
 *
 * Non specialized function does nothing
 */
template<typename T, typename = Impl::if_no_specialized_conversion_t<T>>
T to_native(T val)
{
    return val;
}

/**
 * Convert native value to n64 representation
 *
 * Non specialized function does nothing
 */
template<typename T, typename = Impl::if_no_specialized_conversion_t<T>>
T to_n64(T val)
{
    return val;
}

/// Multibyte integers need to be byteswapped
template<typename T, typename std::enable_if_t<Impl::is_multibyte_int_v<T>>* = nullptr>
T to_native(T val)
{
    return from_big(val);
}
template<typename T, typename std::enable_if_t<Impl::is_multibyte_int_v<T>>* = nullptr>
T to_n64(T val)
{
    return to_big(val);
}

} // Core::Memory
