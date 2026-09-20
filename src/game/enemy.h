#pragma once

#include "difficulty.h"
#include "fixed.h"
#include "player.h"

namespace game
{
    // Both the bone and the arrow render as 8x8 placeholder sprites.
    constexpr int projectile_half_width = 4;
    constexpr int projectile_half_height = 4;

    // A thrown/arcing projectile. Shared by the skeleton's bone and the
    // archer's arrow — same physics (gravity, lifetime), different tuning.
    struct arc_projectile
    {
        bool active = false;
        fixed x = 0;
        fixed y = 0;
        fixed velocity_x = 0;
        fixed velocity_y = 0;
        int lifetime_frames = 0;
    };

    void launch_arc_projectile(arc_projectile& proj, fixed x, fixed y, fixed velocity_x, fixed velocity_y);
    void update_arc_projectile(arc_projectile& proj, fixed gravity, int lifetime_limit);

    // Applies the player's whip hitbox to any enemy with .x, .y, .hp, .alive
    // and .prev_attack_active fields, once per swing. Returns true if this
    // call is what killed it (so the caller can grant EXP exactly once).
    template<typename EnemyState>
    bool apply_whip_to_enemy(EnemyState& enemy, int half_width, int half_height, int defense,
            int attack_power, const attack_hitbox& hitbox)
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
    };

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x);
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
    };

    void init_archer(archer_state& archer, fixed spawn_x);
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
    };

    void init_zombie(zombie_state& zombie, fixed spawn_x);
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
    };

    void init_fleaman(fleaman_state& fleaman, fixed spawn_x);
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
    };

    void init_medusa_head(medusa_head_state& medusa, fixed spawn_x, fixed spawn_y, int facing);
    void update_medusa_head(medusa_head_state& medusa);
}
