//
// Created by henrik on 07.10.19.
// Copyright 2019 Net64 Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "net64/client.hpp"
#include "net64/logging.hpp"
#include "net64/server.hpp"
#include "net64/emulator/m64plus.hpp"


namespace Net64
{

/// Initialize global state of some of Net64's dependencies
bool initialize();

void deinitialize();

} // Net64
