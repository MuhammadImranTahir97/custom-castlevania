#pragma once

#include "arcana_types.h"
#include "difficulty.h"
#include "fixed.h"
#include "input.h"
#include "projectile.h"

namespace game
{
    constexpr int player_half_width = 8;
    constexpr int player_half_height = 8;

    enum class action_kind
    {
        none,
        attack,
        dodge
    };

    // The two playable characters (SPEC.md section 3). Shared: HP, MP,
    // hearts, all seven stats, position — same player_state either way.
    // Separate: moveset, jump arc, attack hitbox, dodge style.
    enum class character_kind
    {
        hunter,
        rival
    };

    struct player_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed velocity_x = 0;
        fixed velocity_y = 0;
        bool grounded = false;
        int frames_since_grounded = 0;
        int jump_buffer_frames = 0;
        bool prev_jump_held = false;

        // Double (SPEC.md's relic table: "Double jump" — Catacombs boss
        // reward, rooms.md's catacombs_18). Relics are shared between
        // characters (SPEC.md section 3), so this isn't per-character.
        // used_double_jump resets in move_and_collide the moment the
        // player lands — see try_jump (player.cpp).
        bool has_double_jump = false;
        bool used_double_jump = false;

        int facing = 1; // +1 = right, -1 = left

        // The seven stats (SPEC.md section 5). max_hp/max_mp grow with level;
        // hearts/int/lck are tracked but inert until subweapons/Arcana/loot exist.
        int level = 1;
        int exp = 0;
        int max_hp = difficulty::player_base_hp;
        int max_mp = difficulty::player_base_mp;
        int hearts = difficulty::player_base_hearts;
        int str = difficulty::player_base_str;
        int def = difficulty::player_base_def;
        int intelligence = difficulty::player_base_int;
        int lck = difficulty::player_base_lck;

        int mp = max_mp;
        int mp_regen_counter = 0;
        int mp_regen_pause_frames = 0;

        int hp = max_hp;
        int invuln_frames = 0;

        action_kind action = action_kind::none;
        int action_timer = 0;
        bool dodge_has_iframes = false;

        character_kind character = character_kind::hunter;
        bool prev_swap_held = false;

        // The Rival's 3-hit combo. Unused by the Hunter.
        int combo_step = 0;        // 0, 1 or 2 — which of the 3 hits comes next
        int combo_reset_timer = 0; // frames spent idle since the last hit ended

        bool prev_attack_held = false;
        bool prev_dodge_held = false;

        // Arcana (SPEC.md section 4 / arcana.md) -- M3: 2 Action x 3
        // Attribute (6 combos). See arcana.h for the actual effects. Loadout
        // is picked from the pause menu's Arcana tab (SPEC.md's control
        // map), not in gameplay -- see main.cpp's render_pause_menu.
        arcana_loadout arcana;
        bool arcana_active = false;      // Mercury: toggled on/off by down+attack, drains MP/s while true
        int arcana_mp_drain_counter = 0; // sub-frame accumulator for Mercury's per-second MP drain
        arc_projectile arcana_projectile; // Diana's in-flight shot
    };

    struct attack_hitbox
    {
        bool active = false;
        fixed x = 0;
        fixed y = 0;
        fixed half_width = 0;
        fixed half_height = 0;
    };

    void init_player(player_state& player, fixed spawn_x, fixed spawn_y);
    void update_player(player_state& player, const input_state& input);

    attack_hitbox get_attack_hitbox(const player_state& player);

    // Effective attack power for the character's current hit (Hunter uses
    // STR directly; the Rival's combo hits for rival_damage_percent of it).
    int current_attack_power(const player_state& player);

    // The one damage formula used everywhere damage is dealt (SPEC.md
    // section 5): damage = raw_damage * 100 / (100 + defense), floored at 1.
    // Percentage-based so defense never fully cancels an attacker's damage,
    // unlike flat subtraction.
    int apply_defense(int raw_damage, int defense);

    // True while dodge i-frames or post-hit i-frames are active.
    bool player_is_invulnerable(const player_state& player);

    // Reduces HP by apply_defense(raw_damage, player.def) and starts
    // i-frames. No-op if already invulnerable. Does not handle death —
    // that's M3 (Death and respawn).
    void damage_player(player_state& player, int raw_damage);

    // Same as damage_player, but ignores dodge i-frames — only the brief
    // post-hit invulnerability still blocks it. For "unrollable" attacks
    // (enemies.md's boss design rule: "at least one attack cannot be
    // rolled — forces positioning"), where dodging through the hitbox
    // shouldn't work and only actually moving out of it (e.g. jumping
    // above a low sweep) should.
    void damage_player_unrollable(player_state& player, int raw_damage);

    // Adds EXP and applies any level-ups (SPEC.md's level^2 * 8 curve),
    // raising max_hp/max_mp/str/def/int/lck per level. Multiple level-ups
    // from one grant are handled (rare, but possible at low levels).
    void grant_exp(player_state& player, int amount);

    // Fully restores HP and MP (SPEC.md: what a save room does on entry,
    // and what a respawn does after death — see src/game/world.cpp).
    void restore_full(player_state& player);
}
