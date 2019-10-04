//
// Created by henrik on 02.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <mem64/mem64.hpp>
#include "net64/memory/memory_handle.hpp"


namespace Net64::Memory
{

template<typename T>
using Ptr = Mem64::Ptr<T, MemHandle>;

template<typename T>
using NestedPtr = Mem64::PtrTag<T, MemHandle>;

} // Net64::Memory
