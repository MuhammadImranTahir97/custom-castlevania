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

    // No player STR stat exists yet (that's M2) — first-draft effective attack
    // power used against enemies until the real stat block exists.
    constexpr int player_attack_damage = 12;

    // --- M1 step 8: Skeleton enemy (enemies.md) ---
    // Stats locked by enemies.md — do not change without checking there first.
    constexpr int skeleton_max_hp = 18;
    constexpr int skeleton_damage = 10; // not yet consumed — player HP is M2, not M1
    constexpr int skeleton_defense = 4;
    constexpr int skeleton_bone_throw_interval_frames = 90;
    constexpr fixed skeleton_speed = (player_run_speed * 4) / 5; // "Speed 0.8" relative to player_run_speed

    // Bone arc: enemies.md only says "low arc" — first draft, tune by feel,
    // same as the player's own movement/attack values above.
    constexpr fixed skeleton_bone_speed_x = fixed_one + fixed_one / 2;      // 1.5 px/frame
    constexpr fixed skeleton_bone_speed_y = -(fixed_one + fixed_one / 2);   // -1.5 px/frame initial
    constexpr fixed skeleton_bone_gravity = fixed_one / 5;                  // 0.2 px/frame^2
    constexpr int skeleton_bone_lifetime_frames = 50;
}
