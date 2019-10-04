//
// Created by henrik on 03.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <type_traits>
#include "core/memory/util.hpp"


namespace Net64::Memory
{

constexpr n64_addr_t LOGICAL_BASE{0x80000000};


inline n64_addr_t addr_physical_to_logical(n64_addr_t addr)
{
    return addr + LOGICAL_BASE;
}

inline n64_addr_t addr_logical_to_physical(n64_addr_t addr)
{
    assert(0x80000000 <= addr);

    return addr - LOGICAL_BASE;
}

namespace Impl
{
template<typename T>
constexpr bool is_multibyte_int_v  = (std::is_integral_v<T> && sizeof(T) != 1);
template<typename T>
using if_no_specialized_conversion_t = std::enable_if_t<!(is_multibyte_int_v<T>)>;
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

} // Net64::Memory
