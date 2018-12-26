/*
 * Copyright Aleksey Verkholat 2018
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt
*/

#pragma once
#include <cstdint>

namespace uf
{
    namespace short_int
    {
        using u8 = std::uint8_t;
        using i8 = std::int8_t;

        using u16 = std::uint16_t;
        using i16 = std::int16_t;

        using u32 = std::uint32_t;
        using i32 = std::int32_t;

        using u64 = std::uint64_t;
        using i64 = std::int64_t;

        namespace literals
        {
            inline constexpr u8 operator"" _u8(unsigned long long int value) noexcept { return value; }
            inline constexpr i8 operator"" _i8(unsigned long long int value) noexcept { return value; }

            inline constexpr u16 operator"" _u16(unsigned long long int value) noexcept { return value; }
            inline constexpr i16 operator"" _i16(unsigned long long int value) noexcept { return value; }

            inline constexpr u32 operator"" _u32(unsigned long long int value) noexcept { return value; }
            inline constexpr i32 operator"" _i32(unsigned long long int value) noexcept { return value; }

            inline constexpr u64 operator"" _u64(unsigned long long int value) noexcept { return value; }
            inline constexpr i64 operator"" _i64(unsigned long long int value) noexcept { return value; }
        }
        // end namespace literals
    }
    // end namespace short_int
}
// end namespace uf