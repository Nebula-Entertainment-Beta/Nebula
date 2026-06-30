#pragma once

#include <cstddef>
#include <cstdint>

namespace Nebula
{

    enum class Tasto : std::uint8_t
    {
        Unknown,
        num1,
        num2,
        num3,
        num4,
        num5,
        num6,
        num7,
        num8,
        num9,
        num0,
        tab,
        q,
        w,
        e,
        r,
        t,
        y,
        u,
        i,
        o,
        p,
        a,
        s,
        d,
        f,
        g,
        h,
        j,
        k,
        l,
        z,
        x,
        c,
        v,
        b,
        n,
        m,
        space,
        left_shift,
        Count,
    };

    enum class TastoDelMouse : std::uint8_t
    {
        left,
        right,
        middle,
        tastoMouse1,
        tastoMouse2,
        Count,
    };

    inline constexpr std::size_t tastoSlotCount()
    {
        return static_cast<std::size_t>(Tasto::Count);
    }

    inline constexpr std::size_t tastoDelMouseSlotCount()
    {
        return static_cast<std::size_t>(TastoDelMouse::Count);
    }

} // namespace Nebula
