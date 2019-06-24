//
// Created by henrik on 21.06.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <type_traits>
#include "types.hpp"


namespace Common
{

/// Unsigned integer with at least BITS wide
template<std::size_t BITS>
using Uint = std::conditional_t<BITS == 0, Assert<false>,
             std::conditional_t<BITS <= 8, u8,
             std::conditional_t<BITS <= 16, u16,
             std::conditional_t<BITS <= 32, u32,
             std::conditional_t<BITS <= 64, u64,
             Assert<false>>>>>>;

/// Integer with at least BITS wide
template<std::size_t BITS>
using Int = std::conditional_t<BITS == 0, Assert<false>,
            std::conditional_t<BITS <= 8, i8,
            std::conditional_t<BITS <= 16, i16,
            std::conditional_t<BITS <= 32, i32,
            std::conditional_t<BITS <= 64, i64,
            Assert<false>>>>>>;

/// Get the unsigned version of an integer type
template<typename T>
using unsigned_t = std::conditional<std::is_integral_v<T>,
                       std::conditional_t<std::is_unsigned_v<T>,
                           T,
                       Uint<sizeof(T) * 8>>,
                   Assert<false>>;

/// Get the signed version of an integer type
template<typename T>
using signed_t = std::conditional<std::is_integral_v<T>,
                     std::conditional_t<std::is_signed_v<T>,
                         T,
                     Int<sizeof(T) * 8>>,
                 Assert<false>>;


} // Common
