//
// Created by henrik on 31.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//


#pragma once

#include <type_traits>
#include "types.hpp"


namespace Core::Memory
{

/// Return offset of member-pointer
template<typename T, typename U, typename V>
T offset_of(V U::*const ptr)
{
    const U val{};
    return static_cast<T>(reinterpret_cast<std::uintptr_t>(&(val.*ptr)) - reinterpret_cast<std::uintptr_t>(&val));
}


// Recursive swap helpers
namespace Impl
{
template<typename T, std::size_t BYTE = 0>
void swap_to_little(T in, u8 out[]);

template<typename T, std::size_t BYTE = 0>
void swap_to_big(T in, u8 out[]);

template<typename T, std::size_t BYTE = 0>
void swap_from_little(T& out, const u8 in[]);

template<typename T, std::size_t BYTE = 0>
void swap_from_big(T& out, const u8 in[]);
} // Impl


/// Convert native endian integer to little endian
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T to_little(T val)
{
    T ret{};
    auto raw{reinterpret_cast<u8*>(&ret)};
    Impl::swap_to_little(val, raw);
    return ret;
}

/// Convert native endian integer to big endian
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T to_big(T val)
{
    T ret{};
    auto raw{reinterpret_cast<u8*>(&ret)};
    Impl::swap_to_big(val, raw);
    return ret;
}

/// Convert big endian integer to native endian
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T from_big(T val)
{
    T ret{};
    auto raw{reinterpret_cast<const u8*>(&val)};
    Impl::swap_from_big(ret, raw);
    return ret;
}

/// Convert little endian integer to native endian
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T from_little(T val)
{
    T ret{};
    auto raw{reinterpret_cast<const u8*>(&val)};
    Impl::swap_from_little(ret, raw);
    return ret;
}


namespace Impl
{

template<typename T, std::size_t BYTE>
void swap_to_little(T in, u8 out[])
{
    (void)in;
    if constexpr(BYTE != sizeof(T))
    {
        out[BYTE] = (u8)((in & (T{0xFF} << (8 * BYTE))) >> (8 * BYTE));
        swap_to_little<T, BYTE + 1>(in, out);
    }
    else
    {
        return;
    }
}

template<typename T, std::size_t BYTE>
void swap_to_big(T in, u8 out[])
{
    (void)in;
    if constexpr(BYTE != sizeof(T))
    {
        out[sizeof(T) - (BYTE + 1)] = (u8)((in & (T{0xFF} << (8 * BYTE))) >> (8 * BYTE));
        swap_to_big<T, BYTE + 1>(in, out);
    }
    else
    {
        return;
    }
}

template<typename T, std::size_t BYTE>
void swap_from_little(T& out, const u8 in[])
{
    (void)out;
    if constexpr(BYTE != sizeof(T))
    {
        out |= T((T)in[BYTE] << (BYTE * 8));
        swap_from_little<T, BYTE + 1>(out, in);
    }
    else
    {
        return;
    }
}

template<typename T, std::size_t BYTE>
void swap_from_big(T& out, const u8 in[])
{
    (void)out;
    if constexpr(BYTE != sizeof(T))
    {
        out |= T((T)in[sizeof(T) - (BYTE + 1)] << (BYTE * 8));
        swap_from_big<T, BYTE + 1>(out, in);
    }
    else
    {
        return;
    }
}

} // Impl

} // Core::Memory
