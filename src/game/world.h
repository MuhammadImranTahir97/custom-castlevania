#pragma once

#include "player.h"

namespace game
{
    // Cross-cutting world concerns that touch both the player and the
    // active room — currently just stepping through a door into a new one.
    void update_world(player_state& player);
}
