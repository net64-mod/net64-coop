//
// Created by henrik on 31.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//


#pragma once

#include <type_traits>
#include "types.hpp"


namespace Net64::Memory
{

inline void bswap32(void* data, std::size_t n)
{
    n -= (n % 4);

    for(auto ptr{reinterpret_cast<u8*>(data)}; ptr < reinterpret_cast<u8*>(data) + n; ptr += 4)
    {
        auto tmp{ptr[0]};
        ptr[0] = ptr[3];
        ptr[3] = tmp;
        tmp = ptr[1];
        ptr[1] = ptr[2];
        ptr[2] = tmp;
    }
}

} // Net64::Memory
