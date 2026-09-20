#pragma once

#include "fixed.h"
#include "player.h"

namespace game
{
    struct checkpoint_info
    {
        int room_index = 0;
        fixed x = 0;
        fixed y = 0;
    };

    // Sets the respawn checkpoint (SPEC.md: "on death, respawn at last save
    // room"). Called once at game start with the initial spawn point, then
    // updated automatically whenever the player enters a save room.
    void set_checkpoint(int room_index, fixed x, fixed y);

    // The checkpoint set_checkpoint last set — what a manual save (SPEC.md
    // section 9) persists as the room/position to resume at.
    checkpoint_info current_checkpoint_info();

    // Cross-cutting world concerns that touch both the player and the
    // active room: door crossing, save-room checkpointing/healing, and
    // death/respawn.
    void update_world(player_state& player);
}
