#pragma once

#include "fixed.h"
#include "input.h"

namespace game
{
    constexpr int player_half_width = 8;
    constexpr int player_half_height = 8;

    struct player_state
    {
        fixed x = 0;
        fixed y = 0;
        fixed velocity_x = 0;
        fixed velocity_y = 0;
        bool grounded = false;
        int frames_since_grounded = 0;
        int jump_buffer_frames = 0;
        bool prev_jump_held = false;
    };

    void init_player(player_state& player);
    void update_player(player_state& player, const input_state& input);
}
