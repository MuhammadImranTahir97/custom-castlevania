#pragma once

// Abstract input, decoupled from any platform's button API.

namespace game
{
    struct input_state
    {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    };
}
