#pragma once

// Abstract input, decoupled from any platform's button API.

namespace game
{
    struct input_state
    {
        bool left = false;
        bool right = false;
        bool jump_held = false;
        bool attack_held = false;
        bool dodge_held = false;
        bool swap_held = false;

        // Up/Down alone move nothing (this is a side-scroller) -- they're
        // only meaningful as modifiers held alongside attack_held. See the
        // control map in SPEC.md section 9: up+attack is reserved for a
        // future subweapon system, down+attack is Arcana. update_player
        // (player.cpp) does the actual combining/edge-detection.
        bool up_held = false;
        bool down_held = false;
    };
}
