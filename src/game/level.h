#pragma once

#include "fixed.h"

namespace game::level
{
    // M1's single hand-built room: a floor plus one raised platform with a
    // ledge, so coyote time has an edge to actually walk off. Real room data
    // and collision arrive in M2 via the Tiled -> GBA converter.

    constexpr int screen_half_width = 120;
    constexpr int screen_half_height = 80;

    // Returns the y of the topmost surface under the given x position (the
    // highest platform whose horizontal span contains x).
    fixed ground_top_y_at(fixed x);
}
