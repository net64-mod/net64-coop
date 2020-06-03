//
// Created by henrik on 30.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <memory>


/// Allows using standalone functions as deleter for unique_ptr
template<auto* FN>
struct Deleter;

template<typename Res, typename Ret, Ret (*FN)(Res)>
struct Deleter<FN>
{
    using pointer = Res;
    using ElementType = std::conditional_t<std::is_pointer_v<Res>, std::remove_pointer_t<Res>, void>;

    void operator()(Res res) { FN(res); }
};

/**
 * RAII wrapper for resources with custom deleter
 * @tparam free_fn Function used to free the resource
 */
template<auto* free_fn>
using ResourceHandle = std::unique_ptr<typename Deleter<free_fn>::ElementType, Deleter<free_fn>>;
