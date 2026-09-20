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
    constexpr int player_base_mp = 100;
    constexpr int player_mp_regen_frames = 6; // 1 MP every N frames

    // Player HP / i-frames — locked by SPEC.md.
    constexpr int player_base_hp = 100;
    constexpr int player_invuln_frames = 40;

    // --- The seven stats & leveling (SPEC.md section 5) — locked values ---
    constexpr int player_hp_per_level = 8;
    constexpr int player_mp_per_level = 5;
    constexpr int player_base_hearts = 50; // no per-level change (subweapon fuel — not built yet)
    constexpr int player_base_str = 10;
    constexpr int player_str_per_level = 2;
    constexpr int player_base_def = 10;
    constexpr int player_def_per_level = 2;
    constexpr int player_base_int = 10;    // Arcana spell power — not built yet, tracked but inert
    constexpr int player_int_per_level = 2;
    constexpr int player_base_lck = 10;    // drop rate + crit — not built yet, tracked but inert
    constexpr int player_lck_per_level = 2;

    // EXP required to go from `level` to `level + 1`.
    constexpr int exp_for_next_level(int level)
    {
        return level * level * 8;
    }

    // --- Skeleton enemy (enemies.md) — walker ---
    // Stats locked by enemies.md — do not change without checking there first.
    constexpr int skeleton_max_hp = 18;
    constexpr int skeleton_damage = 10;
    constexpr int skeleton_defense = 4;
    constexpr int skeleton_exp_reward = 8;
    constexpr int skeleton_bone_throw_interval_frames = 90;
    constexpr fixed skeleton_speed = (player_run_speed * 4) / 5; // "Speed 0.8" relative to player_run_speed

    // Bone arc: enemies.md only says "low arc" — first draft, tune by feel,
    // same as the player's own movement/attack values above.
    constexpr fixed skeleton_bone_speed_x = fixed_one + fixed_one / 2;      // 1.5 px/frame
    constexpr fixed skeleton_bone_speed_y = -(fixed_one + fixed_one / 2);   // -1.5 px/frame initial
    constexpr fixed skeleton_bone_gravity = fixed_one / 5;                  // 0.2 px/frame^2
    constexpr int skeleton_bone_lifetime_frames = 50;

    // --- Zombie enemy (enemies.md) — walker, doesn't turn at ledges ---
    // Stats locked by enemies.md — do not change without checking there first.
    constexpr int zombie_max_hp = 20;
    constexpr int zombie_damage = 8;
    constexpr int zombie_defense = 2;
    constexpr int zombie_exp_reward = 6;
    constexpr fixed zombie_speed = (player_run_speed * 4) / 10; // "Speed 0.4"
    constexpr fixed zombie_fall_gravity = player_gravity;       // reuses the player's fall physics

    // --- M2 enemies (enemies.md) ---

    // Bat — flyer. Stats locked by enemies.md.
    constexpr int bat_max_hp = 10;
    constexpr int bat_damage = 7;
    constexpr int bat_defense = 1;
    constexpr int bat_exp_reward = 4;
    constexpr fixed bat_trigger_range = to_fixed(80);        // "idles until player within 80px"
    constexpr fixed bat_speed = (player_run_speed * 12) / 10; // "Speed 1.2"
    // "Erratic swoop" has no exact spec — first draft: a simple zigzag wiggle.
    constexpr fixed bat_wiggle_speed = fixed_one / 2;
    constexpr int bat_wiggle_period_frames = 16;

    // Skeleton Archer — shooter. Stats locked by enemies.md.
    constexpr int archer_max_hp = 22;
    constexpr int archer_damage = 14;
    constexpr int archer_defense = 6;
    constexpr int archer_exp_reward = 18;
    constexpr fixed archer_retreat_range = to_fixed(50); // "backs away if player closes to 50px"
    constexpr fixed archer_speed = (player_run_speed * 6) / 10; // "Speed 0.6"

    // Arrow arc: enemies.md says "arcing arrows" but gives no interval or
    // shape — first draft, tune by feel, same pattern as the skeleton's bone.
    constexpr int archer_shoot_interval_frames = 80;
    constexpr fixed archer_arrow_speed_x = fixed_one + fixed_one / 2;
    constexpr fixed archer_arrow_speed_y = -(fixed_one + fixed_one / 2);
    constexpr fixed archer_arrow_gravity = fixed_one / 5;
    constexpr int archer_arrow_lifetime_frames = 60;

    // --- The Rival — second character (SPEC.md section 3) ---
    // Exact multipliers/frame counts aren't given beyond qualitative
    // descriptions ("faster", "higher, floatier", "shorter i-frames") —
    // first draft, tune by feel, same as the Hunter's own values above.
    constexpr fixed rival_run_speed = (player_run_speed * 12) / 10;           // "faster"
    constexpr fixed rival_gravity = (player_gravity * 8) / 10;                // "floatier"
    constexpr fixed rival_jump_velocity = (player_jump_velocity * 115) / 100; // "higher"
    constexpr fixed rival_jump_cut_velocity = (player_jump_cut_velocity * 115) / 100;

    // Dash — distance is locked by SPEC.md (50px); duration is kept at the
    // Hunter's 18f (not specified otherwise) with a shorter i-frame window.
    constexpr int rival_dodge_distance = 50; // locked by SPEC.md
    constexpr int rival_dodge_duration_frames = player_dodge_duration_frames;
    constexpr fixed rival_dodge_velocity = to_fixed(rival_dodge_distance) / rival_dodge_duration_frames;
    constexpr int rival_dodge_iframe_start = player_dodge_iframe_start;
    constexpr int rival_dodge_iframe_end = 8; // "shorter i-frames" than the Hunter's 4-14

    // Twin blades — short reach, fast, 3-hit combo, low damage per hit.
    constexpr int rival_attack_startup_frames = 2;
    constexpr int rival_attack_active_frames = 3;
    constexpr int rival_attack_recovery_frames = 5;
    constexpr fixed rival_attack_range = to_fixed(10);
    constexpr fixed rival_attack_hitbox_half_width = to_fixed(6);
    constexpr fixed rival_attack_hitbox_half_height = to_fixed(4);
    constexpr int rival_combo_window_frames = 20; // time after a hit to chain the next one
    constexpr int rival_damage_percent = 60;      // "Damage per hit: Low" vs the Hunter's 100%

    // --- M3 enemies (enemies.md) ---

    // Bone Pillar — stationary shooter. Stats locked by enemies.md.
    constexpr int bone_pillar_max_hp = 30;
    constexpr int bone_pillar_damage = 12;
    constexpr int bone_pillar_defense = 14;
    constexpr int bone_pillar_exp_reward = 15;
    constexpr int bone_pillar_fire_interval_frames = 100; // "fires... every 100 frames"

    // Fireball: enemies.md gives no speed/lifetime — first draft, tune by
    // feel, a straight (zero-gravity) shot rather than an arc.
    constexpr fixed bone_pillar_fireball_speed = to_fixed(1) + fixed_one / 2; // 1.5 px/frame
    constexpr int bone_pillar_fireball_lifetime_frames = 90;

    // Fleaman — jumper. Stats locked by enemies.md.
    constexpr int fleaman_max_hp = 12;
    constexpr int fleaman_damage = 9;
    constexpr int fleaman_defense = 2;
    constexpr int fleaman_exp_reward = 7;
    constexpr fixed fleaman_speed = (player_run_speed * 15) / 10; // "Speed 1.5"
    constexpr int fleaman_hop_interval_min_frames = 30;
    constexpr int fleaman_hop_interval_max_frames = 60;

    // Hop height: enemies.md gives no exact value — first draft, tune by
    // feel, reusing the player's own gravity so hops land cleanly.
    constexpr fixed fleaman_gravity = player_gravity;
    constexpr fixed fleaman_hop_velocity_min = -(to_fixed(2));
    constexpr fixed fleaman_hop_velocity_max = -(to_fixed(4));

    // Medusa Head — continuous-spawn flyer. Stats locked by enemies.md.
    constexpr int medusa_head_max_hp = 8;
    constexpr int medusa_head_damage = 6;
    constexpr int medusa_head_defense = 0;
    constexpr int medusa_head_exp_reward = 3;
    constexpr fixed medusa_head_speed = to_fixed(1); // "Speed 1.0"

    // Sine wave + respawn loop: enemies.md says "spawns continuously from
    // screen edge" but gives no wave shape or cooldown — first draft.
    constexpr fixed medusa_head_wave_amplitude = to_fixed(24);
    constexpr int medusa_head_wave_period_frames = 60;
    constexpr int medusa_head_respawn_cooldown_frames = 45;

    // --- Bone Colossus — first boss (rooms.md: catacombs_17) ---
    // HP/defense/EXP and the phase HP thresholds are locked by rooms.md.
    constexpr int bone_colossus_max_hp = 400;
    constexpr int bone_colossus_defense = 12;
    constexpr int bone_colossus_exp_reward = 300;
    constexpr int bone_colossus_phase2_hp_percent = 60; // phase 2 starts below this
    constexpr int bone_colossus_phase3_hp_percent = 25; // phase 3 starts below this

    // Windup frame counts are locked by rooms.md ("30-frame windup",
    // "20-frame windup") and are enemies.md's boss design rule #1's
    // 20-frame minimum tell — phase 3's "+25% attack speed" (below) is
    // deliberately not applied to these, only to recovery/cooldown, so the
    // tell never shrinks below what the player was taught to read.
    constexpr int bone_colossus_slam_windup_frames = 30;    // locked
    constexpr int bone_colossus_slam_active_frames = 10;    // first draft
    constexpr int bone_colossus_slam_recovery_frames = 20;  // first draft
    constexpr int bone_colossus_slam_damage = 22;           // first draft
    constexpr fixed bone_colossus_slam_half_width = to_fixed(28);
    constexpr fixed bone_colossus_slam_half_height = to_fixed(16);

    // Phase 2's slam becomes a 2-hit combo (rooms.md: "one roll dodges
    // only the first"). The second hit gets its own full 20-frame windup
    // — starting only after the first hit's active frames end — rather
    // than a short follow-up, specifically so a roll timed to the first
    // hit's tell (18-frame dodge duration) has completely finished by the
    // time the second hit goes active, forcing a second, separately-timed
    // dodge rather than one roll covering both.
    constexpr int bone_colossus_slam2_windup_frames = 20;   // locked (the 20f minimum)
    constexpr int bone_colossus_slam2_active_frames = 10;   // first draft
    constexpr int bone_colossus_slam2_recovery_frames = 20; // first draft

    constexpr int bone_colossus_sweep_windup_frames = 20;   // locked
    constexpr int bone_colossus_sweep_active_frames = 15;   // first draft
    constexpr int bone_colossus_sweep_recovery_frames = 25; // first draft
    constexpr int bone_colossus_sweep_damage = 18;          // first draft
    // Low and wide — "must JUMP not roll" (rooms.md) means jumping has to
    // physically clear this band; a jump apex clears well above it (see
    // player_jump_velocity/gravity), while a grounded dodge's height
    // doesn't change at all, so it can't.
    constexpr fixed bone_colossus_sweep_half_width = to_fixed(140);
    constexpr fixed bone_colossus_sweep_half_height = to_fixed(10);

    // Rooms.md's "no unavoidable damage" attacks that also aren't rollable
    // (sweep and, in phase 3, the rib spread) skip the player's dodge
    // i-frames entirely rather than the doing the same overlap check other
    // enemies use — see player::damage_player_unrollable.

    // Phase 3's rib-cage spread: 5 bones fired in a fan with gaps between
    // them (rooms.md: "must be positioned between them"). Reuses the
    // Skeleton's arc_projectile/bone visual — same "bone" theming, no new
    // asset needed.
    constexpr int bone_colossus_ribs_windup_frames = 20;    // locked
    constexpr int bone_colossus_ribs_damage = 16;           // first draft
    constexpr fixed bone_colossus_rib_speed = to_fixed(2);        // first draft
    constexpr fixed bone_colossus_rib_spread_step = to_fixed(1) / 2; // vertical fan spacing, first draft
    constexpr fixed bone_colossus_rib_gravity = fixed_one / 6;    // first draft
    constexpr int bone_colossus_rib_lifetime_frames = 70;         // first draft

    // Idle time between attack decisions (phase 1/2) — not specified,
    // first draft, tune by feel.
    constexpr int bone_colossus_decision_cooldown_frames = 50;

    // "Summons 2 Skeletons every 15 seconds" (rooms.md) — locked interval,
    // GBA runs at 60fps.
    constexpr int bone_colossus_summon_interval_frames = 15 * 60;

    // Phase 3: "attack speed +25%" (rooms.md) — applied only to the
    // recovery/cooldown timers above (via *100/125), never to the locked
    // windup frames, per the comment on those.
    constexpr int bone_colossus_phase3_speed_percent = 125;
}
