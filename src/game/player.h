#pragma once

#include "input.h"

namespace game
{
    struct player_state
    {
        int x = 0;
        int y = 0;
    };

    void update_player(player_state& player, const input_state& input);
}
