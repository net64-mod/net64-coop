//
// Created by henrik on 30.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

/// Allows using standalone functions as deleter for unique_ptr
template<auto* FN>
struct Deleter;

template<typename Ptr, void(*FN)(Ptr*)>
struct Deleter<FN>
{
    void operator()(Ptr* ptr)
    {
        FN(ptr);
    }
};
