//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <cstdint>
#include "common/crtp.hpp"
#include "core/error_codes.hpp"
#include "core/logging.hpp"
#include "types.hpp"


namespace Core
{

/// Base class for OS dependent process classes
struct ProcessBase
{
    /// Memory region in process
    struct MemoryRegion
    {
        std::uintptr_t begin{},
                       end{};
        bool readable{};
    };

    /// Max memory region size to load
    static constexpr std::size_t MAX_REGION_SIZE{50'000'000}; // 50MB
};

}
