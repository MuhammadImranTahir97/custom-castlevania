#pragma once

#include "fixed.h"

// A generic straight/arcing shot, shared by every enemy projectile
// (skeleton's bone, archer's arrow, bone pillar's fireball, the Bone
// Colossus's ribs) and the player's own Diana Arcana cast (arcana.h) --
// split out of enemy.h so player.h can hold one without including enemy.h
// (which itself includes player.h for player_state -- a straight include
// would cycle).

namespace game
{
    // Both the bone and the arrow render as 8x8 placeholder sprites.
    constexpr int projectile_half_width = 4;
    constexpr int projectile_half_height = 4;

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
}
