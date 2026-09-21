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

        // Arcana (arcana.h). Every other GBA button is already spoken for
        // (A jump, B attack, L swap, R dodge, START save, SELECT pause) --
        // UP/DOWN were the only ones free.
        bool arcana_cast_held = false;  // UP -- Mercury: held to sustain. Diana: edge-press to fire.
        bool arcana_cycle_held = false; // DOWN -- edge-press cycles the 6 combos, one flat list
    };
}
