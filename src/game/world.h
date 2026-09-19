#pragma once

#include "fixed.h"
#include "player.h"

namespace game
{
    // Sets the respawn checkpoint (SPEC.md: "on death, respawn at last save
    // room"). Called once at game start with the initial spawn point, then
    // updated automatically whenever the player enters a save room.
    void set_checkpoint(int room_index, fixed x, fixed y);

    // Cross-cutting world concerns that touch both the player and the
    // active room: door crossing, save-room checkpointing/healing, and
    // death/respawn.
    void update_world(player_state& player);
}
