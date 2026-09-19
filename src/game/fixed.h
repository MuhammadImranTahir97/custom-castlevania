#pragma once

// Minimal Q24.8 fixed-point helpers. The GBA's ARM7TDMI has no FPU, so game
// logic uses integer fixed-point instead of floats.

namespace game
{
    using fixed = int;

    constexpr int fixed_shift = 8;
    constexpr fixed fixed_one = 1 << fixed_shift;

    constexpr fixed to_fixed(int pixels)
    {
        return pixels << fixed_shift;
    }

    constexpr int to_pixels(fixed value)
    {
        return value >> fixed_shift;
    }
}
