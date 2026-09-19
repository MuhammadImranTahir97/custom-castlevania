#pragma once

#include "fixed.h"

// Every tunable balance number lives here. No magic numbers in game code.

namespace game::difficulty
{
    // --- M1 movement ---
    // SPEC.md only specifies these qualitatively ("fast", "tap vs hold").
    // Tune these by feel — nothing here is a locked design decision yet.
    constexpr fixed player_run_speed = to_fixed(1) + fixed_one / 2;          // 1.5 px/frame
    constexpr fixed player_gravity = fixed_one / 4;                         // 0.25 px/frame^2
    constexpr fixed player_jump_velocity = -(to_fixed(4) + fixed_one / 2);  // -4.5 px/frame at takeoff
    constexpr fixed player_jump_cut_velocity = -(to_fixed(1) + fixed_one / 2); // early-release clamp
    constexpr fixed player_max_fall_speed = to_fixed(4);                   // terminal velocity

    // Locked by SPEC.md — do not change without checking there first.
    constexpr int player_coyote_frames = 5;
    constexpr int player_input_buffer_frames = 6;
}
