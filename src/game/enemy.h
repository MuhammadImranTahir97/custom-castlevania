#pragma once

#include "fixed.h"
#include "player.h"

namespace game
{
    constexpr int skeleton_half_width = 8;
    constexpr int skeleton_half_height = 8;

    struct bone_projectile
    {
        bool active = false;
        fixed x = 0;
        fixed y = 0;
        fixed velocity_x = 0;
        fixed velocity_y = 0;
        int lifetime_frames = 0;
    };

    struct skeleton_state
    {
        fixed x = 0;
        fixed y = 0;
        int facing = -1; // -1 = left, +1 = right
        int hp = 0;
        bool alive = true;
        int throw_timer = 0;
        bool prev_attack_active = false;
        bone_projectile bone;
    };

    void init_skeleton(skeleton_state& skeleton, fixed spawn_x);
    void update_skeleton(skeleton_state& skeleton);

    // Applies the player's whip hitbox to the skeleton, once per swing.
    void apply_attack_to_skeleton(skeleton_state& skeleton, const attack_hitbox& hitbox);
}
