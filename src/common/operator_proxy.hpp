//
// Created by henrik on 11.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <utility>


namespace Common
{

/**
 * Extends the lifetime of the contained value to
 * allow operator "->" to return temporaries.
 */
template<typename T>
struct OperatorProxy
{
    OperatorProxy(T val)
    :val_{std::move(val)}
    {
    }

    const T* operator->()
    {
        return &val_;
    }

private:
    T val_;
};

} // Common
