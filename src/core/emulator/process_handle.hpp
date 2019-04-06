//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include "core/emulator/handle_config.hpp"


#ifdef USE_PROCESS_HANDLE_LINUX_
    #include "core/emulator/process_linux.hpp"
    namespace Core{using Process = ProcessLinux;}
#elif defined USE_PROCESS_HANDLE_WINDOWS_
    #include "core/emulator/process_windows.hpp"
    namespace Core{using Process = ProcessWindows;};
#endif
