#pragma once

#include "enemy.h"
#include "fixed.h"
#include "player.h"

namespace game
{
    // Up to this many of EACH enemy type can be active in a room at once.
    // Small on purpose — raise it if a room actually needs more.
    constexpr int max_enemies_per_type = 2;

    // Spawns the active room's enemies (from its room data), replacing
    // whatever was previously active. Enemies aren't part of a room-state/
    // save system yet — leaving a room and coming back respawns it fresh.
    void spawn_room_enemies();

    void update_enemies(fixed player_x, fixed player_y);
    void apply_attacks_to_enemies(const attack_hitbox& hitbox);
    void apply_enemy_contact_to_player(player_state& player);

    // Read-only access for rendering.
    int active_skeleton_count();
    const skeleton_state& active_skeleton(int index);

    int active_bat_count();
    const bat_state& active_bat(int index);

    int active_archer_count();
    const archer_state& active_archer(int index);
}
