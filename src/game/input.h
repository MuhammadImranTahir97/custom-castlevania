#pragma once

// Abstract input, decoupled from any platform's button API.

namespace game
{
    struct input_state
    {
        bool left = false;
        bool right = false;
        bool jump_held = false;
    };
}
