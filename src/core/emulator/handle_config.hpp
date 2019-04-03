//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#ifdef __linux__
    #define USE_PROCESS_HANDLE_LINUX_
#elif defined _WIN64
    #define USE_PROCESS_HANDLE_WINDOWS_
#endif

