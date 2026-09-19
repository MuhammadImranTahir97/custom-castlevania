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

    // --- M1 combat (steps 5-7) ---
    // Whip attack timing. First draft, like movement above — tune by feel.
    constexpr int player_attack_startup_frames = 4;
    constexpr int player_attack_active_frames = 6;
    constexpr int player_attack_recovery_frames = 10;
    constexpr fixed player_attack_range = to_fixed(20);          // hitbox offset from player center
    constexpr fixed player_attack_hitbox_half_width = to_fixed(8);
    constexpr fixed player_attack_hitbox_half_height = to_fixed(4);

    // Dodge roll — locked by SPEC.md.
    constexpr int player_dodge_duration_frames = 18;
    constexpr int player_dodge_iframe_start = 4;
    constexpr int player_dodge_iframe_end = 14;
    constexpr int player_dodge_mp_cost = 10;
    constexpr int player_dodge_regen_pause_frames = 60;
    constexpr int player_dodge_distance = 40;
    constexpr fixed player_dodge_velocity = to_fixed(player_dodge_distance) / player_dodge_duration_frames;

    // MP economy — locked by SPEC.md.
    constexpr int player_max_mp = 100;
    constexpr int player_mp_regen_frames = 6; // 1 MP every N frames
}
