//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>


// Unsigned integer types
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Signed integer types
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Floating point types
using f32 = float;
using f64 = double;

// N64 types
using n64_usize_t = u32;
using n64_ssize_t = i32;
using n64_addr_t = u32;
using n64_saddr_t = i32;


#ifdef NDEBUG
    #define NOT_IMPLEMENTED_ std::fprintf(stderr, "[UNIMPLEMENTED FUNCTION] %s\n", __func__);
#else
    #define NOT_IMPLEMENTED_ std::fprintf(stderr, "%s, line %d: function \"%s\" not implemented\n", \
                                          __FILE__, __LINE__, __func__);
#endif


struct NonCopyable
{
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(const NonCopyable&) = delete;

protected:
    NonCopyable() = default;
};

struct NonMoveable : NonCopyable
{
    NonMoveable(NonMoveable&&) = delete;
    NonMoveable& operator=(NonMoveable&&) = delete;

protected:
    NonMoveable() = default;
};
