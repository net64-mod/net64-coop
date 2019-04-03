//
// Created by henrik on 21.02.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once


namespace Common
{

/// Helper base for CRTP classes
template<template<typename...> typename, typename T>
struct Crtp
{
    T& derived()
    {
        return *static_cast<T*>(this);
    }

    const T& derived() const
    {
        return *static_cast<const T*>(this);
    }
};

} // Common
