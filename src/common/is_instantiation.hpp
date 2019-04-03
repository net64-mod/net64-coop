//
// Created by henrik on 24.01.19.
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <type_traits>


namespace Common
{

template<template<typename...>typename TTemplate, typename T>
struct IsInstantiationOf : std::false_type{};

template<template<typename...>typename TTemplate, typename... TArgs>
struct IsInstantiationOf<TTemplate, TTemplate<TArgs...>> : std::true_type{};


/// Check if TType is an instantiation of TTemplate
template<template<typename...>typename TTemplate, typename TType>
constexpr bool is_instantiation_of_v{IsInstantiationOf<TTemplate, TType>::value};

} // Common
