#pragma once

#include "enemy.h"
#include "fixed.h"
#include "player.h"

namespace game
{
    // Up to this many of EACH enemy type can be active in a room at once.
    // catacombs_02 needs 3 Zombies at once — raise it further if a future
    // room needs more.
    constexpr int max_enemies_per_type = 4;

    // Spawns the active room's enemies (from its room data), replacing
    // whatever was previously active. Enemies aren't part of a room-state/
    // save system yet — leaving a room and coming back respawns it fresh.
    void spawn_room_enemies();

    void update_enemies(fixed player_x, fixed player_y);

    // Applies the player's whip to every active enemy and grants EXP for
    // any it kills.
    void apply_attacks_to_enemies(const attack_hitbox& hitbox, player_state& player);
    void apply_enemy_contact_to_player(player_state& player);

    // Read-only access for rendering.
    int active_skeleton_count();
    const skeleton_state& active_skeleton(int index);

    int active_bat_count();
    const bat_state& active_bat(int index);

    int active_archer_count();
    const archer_state& active_archer(int index);

    int active_zombie_count();
    const zombie_state& active_zombie(int index);

    int active_bone_pillar_count();
    const bone_pillar_state& active_bone_pillar(int index);

    int active_fleaman_count();
    const fleaman_state& active_fleaman(int index);

    int active_medusa_head_count();
    const medusa_head_state& active_medusa_head(int index);
}
