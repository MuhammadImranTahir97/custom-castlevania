#pragma once

namespace game
{
    // Grid positions for the map screen (SPEC.md section 9), one per
    // Catacombs room (room_data.h's index order — alphabetical filename
    // order, catacombs_01 = 0 .. catacombs_18 = 17). Derived from
    // rooms.md's documented exits (W/E/UP/DOWN between rooms), not
    // authored anywhere else, since it's a map-screen rendering concern,
    // not room content itself. Sandbox rooms aren't real content and
    // don't appear on the map.
    constexpr int map_room_count = 18;

    struct map_grid_position
    {
        int x;
        int y;
    };

    // {0, 0} for any index outside [0, map_room_count) — callers only ask
    // for indices below map_room_count in practice, but this stays safe
    // either way.
    map_grid_position map_position_for_room(int room_index);
}
