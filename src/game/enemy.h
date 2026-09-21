#pragma once

#include "arcana_types.h"
#include "difficulty.h"
#include "fixed.h"
#include "player.h"
#include "projectile.h"

namespace game
{
    // Applies the player's whip hitbox to any enemy with .x, .y, .hp, .alive
    // and .prev_attack_active fields, once per swing. Returns true if this
    // call is what killed it (so the caller can grant EXP exactly once).
    // out_damage_dealt/out_hit are optional -- arcana.h's Mercury on-hit
    // effect (burn/slow/lifesteal) needs to know the damage of a hit that
    // didn't kill, which the bool return alone can't express.
    template<typename EnemyState>
    bool apply_whip_to_enemy(EnemyState& enemy, int half_width, int half_height, int defense,
            int attack_power, const attack_hitbox& hitbox, int* out_damage_dealt = nullptr,
            bool* out_hit = nullptr)
    {
        bool attack_just_started = hitbox.active && ! enemy.prev_attack_active;
        enemy.prev_attack_active = hitbox.active;

        if(! enemy.alive || ! attack_just_started)
        {
            return false;
        }

        fixed half_w = to_fixed(half_width);
        fixed half_h = to_fixed(half_height);

        bool overlap_x = (hitbox.x - hitbox.half_width) < (enemy.x + half_w)
                && (hitbox.x + hitbox.half_width) > (enemy.x - half_w);
        bool overlap_y = (hitbox.y - hitbox.half_height) < (enemy.y + half_h)
                && (hitbox.y + hitbox.half_height) > (enemy.y - half_h);

        if(overlap_x && overlap_y)
        {
            int damage = apply_defense(attack_power, defense);

            if(out_damage_dealt)
            {
                *out_damage_dealt = damage;
            }

            if(out_hit)
            {
                *out_hit = true;
            }

            enemy.hp -= damage;

            if(enemy.hp <= 0)
            {
                enemy.hp = 0;
                enemy.alive = false;
                return true;
            }
        }

        return false;
    }

    // Damages the player on contact with any alive enemy with .x, .y fields.
    template<typename EnemyState>
    void apply_enemy_contact(const EnemyState& enemy, int half_width, int half_height, int damage,
            player_state& player)
    {
        if(! enemy.alive)
        {
            return;
        }

        fixed half_w = to_fixed(half_width);
        fixed half_h = to_fixed(half_height);
        fixed player_half_w = to_fixed(player_half_width);
        fixed player_half_h = to_fixed(player_half_height);

        bool overlap_x = (enemy.x - half_w) < (player.x + player_half_w)
                && (enemy.x + half_w) > (player.x - player_half_w);
        bool overlap_y = (enemy.y - half_h) < (player.y + player_half_h)
                && (enemy.y + half_h) > (player.y - player_half_h);

        if(overlap_x && overlap_y)
        {
            damage_player(player, damage);
        }
    }

    // Damages the player on contact with an active projectile, consuming it.
    void apply_projectile_contact(arc_projectile& proj, int half_width, int half_height, int damage,
            player_state& player);

    // --- Skeleton — walker (enemies.md) ---

    constexpr int skeleton_half_width = 8;
    constexpr int skeleton_half_height = 8;

    struct skeleton_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1; // -1 = left, +1 = right
        int hp = 0;
        bool alive = true;
        int throw_timer = 0;
        bool prev_attack_active = false;
        arc_projectile bone;
        arcana_status status; // arcana.h -- burn/slow from the player's Arcana
    };

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x, fixed spawn_y);
    void update_skeleton(skeleton_state& skeleton);

    // --- Bat — flyer (enemies.md) ---

    constexpr int bat_half_width = 8;
    constexpr int bat_half_height = 8;

    struct bat_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        bool triggered = false; // idles until the player is in range, then swoops permanently
        int swoop_timer = 0;    // drives the erratic wiggle
        arcana_status status;   // arcana.h -- burn/slow from the player's Arcana
    };

    void init_bat(bat_state& bat, fixed spawn_x, fixed spawn_y);
    void update_bat(bat_state& bat, fixed player_x, fixed player_y);

    // --- Skeleton Archer — shooter (enemies.md) ---

    constexpr int archer_half_width = 8;
    constexpr int archer_half_height = 8;

    struct archer_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        int shoot_timer = 0;
        arc_projectile arrow;
        arcana_status status; // arcana.h -- burn/slow from the player's Arcana
    };

    void init_archer(archer_state& archer, fixed spawn_x, fixed spawn_y);
    void update_archer(archer_state& archer, fixed player_x);

    // --- Zombie — walker, doesn't turn at ledges (enemies.md) ---

    constexpr int zombie_half_width = 8;
    constexpr int zombie_half_height = 8;

    struct zombie_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed velocity_y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        bool falling = false; // walked off a ledge — unlike the Skeleton, it doesn't turn around
        arcana_status status; // arcana.h -- burn/slow from the player's Arcana
    };

    void init_zombie(zombie_state& zombie, fixed spawn_x, fixed spawn_y);
    void update_zombie(zombie_state& zombie);

    // --- Bone Pillar — stationary shooter (enemies.md) ---
    // Can't decide its own facing (it never moves) — the room authors it,
    // via level::enemy_spawn_view::facing.

    constexpr int bone_pillar_half_width = 8;
    constexpr int bone_pillar_half_height = 8;

    struct bone_pillar_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        int fire_timer = 0;
        arc_projectile fireball;
        arcana_status status; // arcana.h -- burn/slow from the player's Arcana
    };

    void init_bone_pillar(bone_pillar_state& pillar, fixed spawn_x, fixed spawn_y, int facing);
    void update_bone_pillar(bone_pillar_state& pillar);

    // --- Fleaman — jumper (enemies.md) ---

    constexpr int fleaman_half_width = 8;
    constexpr int fleaman_half_height = 8;

    struct fleaman_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed velocity_y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        bool grounded = true;
        int hop_timer = 0; // frames left grounded before the next random hop
        arcana_status status; // arcana.h -- burn/slow from the player's Arcana
    };

    void init_fleaman(fleaman_state& fleaman, fixed spawn_x, fixed spawn_y);
    void update_fleaman(fleaman_state& fleaman);

    // --- Medusa Head — continuous-spawn flyer (enemies.md) ---
    // Unlike other enemies, killing (or losing track of) one doesn't remove
    // it permanently: it resets to its spawn edge after a cooldown and
    // flies again, like the source game's endless Medusa Head volleys.
    // Spawn edge/direction is authored, via level::enemy_spawn_view::facing.

    constexpr int medusa_head_half_width = 8;
    constexpr int medusa_head_half_height = 8;

    struct medusa_head_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed origin_x = 0;
        fixed origin_y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false;
        int wave_timer = 0;
        int respawn_timer = 0; // >0 while waiting to respawn after death/off-screen
        arcana_status status;  // arcana.h -- burn/slow from the player's Arcana
    };

    void init_medusa_head(medusa_head_state& medusa, fixed spawn_x, fixed spawn_y, int facing);
    void update_medusa_head(medusa_head_state& medusa);

    // --- Bone Colossus — first boss (rooms.md: catacombs_17) ---
    // Three phases by HP fraction, each changing its attack pattern
    // (enemies.md boss rule: "phases change the pattern, not just the
    // numbers"), not just three copies of the same fight with more damage.
    // See update_bone_colossus for the phase thresholds and attack timings
    // (difficulty.h has the frame counts).

    constexpr int bone_colossus_half_width = 32;
    constexpr int bone_colossus_half_height = 32;
    constexpr int bone_colossus_rib_count = 5;

    enum class bone_colossus_attack
    {
        none,
        slam,        // overhead slam — rollable
        slam_second, // phase 2's second combo hit — its own fresh tell, so the first roll's i-frames have ended by the time this goes active
        sweep,       // low bone sweep — unrollable, only jumping clears it
        rib_spread,  // phase 3's 5-bone arc — unrollable, only standing in a gap avoids it
    };

    // A melee attack's hitbox — the boss equivalent of player::attack_hitbox.
    // unrollable means dodge i-frames don't stop it (enemies.md boss rule:
    // "at least one attack cannot be rolled") — see
    // player::damage_player_unrollable.
    struct boss_attack_hitbox
    {
        bool active = false;
        bool unrollable = false;
        fixed x = 0;
        fixed y = 0;
        fixed half_width = 0;
        fixed half_height = 0;
        int damage = 0;
    };

    struct bone_colossus_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1;
        int hp = 0;
        bool alive = true;
        bool prev_attack_active = false; // for apply_whip_to_enemy

        // No arcana_status here, deliberately -- arcana.md's balance rule
        // "no combo may trivialise a boss" means Mercury/Diana's burn/slow
        // don't touch the Colossus; its already-tuned fight stays untouched.

        bone_colossus_attack attack = bone_colossus_attack::none;
        int attack_timer = 0;   // frames since the current attack began
        int decision_timer = 0; // frames until the next attack while idle (attack == none)
        bool attack_hit_applied = false; // one hit per attack's active window, not one per frame

        int summon_timer = 0; // phase 2+: frames until the next Skeleton pair (enemy_spawner.cpp acts on it)

        arc_projectile ribs[bone_colossus_rib_count]; // phase 3's rib-cage spread
    };

    void init_bone_colossus(bone_colossus_state& boss, fixed spawn_x, fixed spawn_y);

    // Returns true on the exact frame phase 2+ wants a pair of Skeletons
    // summoned — enemy_spawner.cpp owns the Skeleton array, so it's the one
    // that actually spawns them; this only requests it.
    bool update_bone_colossus(bone_colossus_state& boss, fixed player_x);

    boss_attack_hitbox get_bone_colossus_attack_hitbox(const bone_colossus_state& boss);

    // Applies get_bone_colossus_attack_hitbox's current hitbox to the
    // player, once per attack (not once per frame it's active), respecting
    // unrollable per boss_attack_hitbox.
    void apply_bone_colossus_attack_to_player(bone_colossus_state& boss, player_state& player);
}
