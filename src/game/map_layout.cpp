#include "map_layout.h"

namespace game
{
    namespace
    {
        // Index order matches room_data.h (catacombs_01 = 0, .., catacombs_18 = 17).
        // Derived from rooms.md: E/W chains follow the corridor, UP/DOWN
        // moves a room; catacombs_11 and catacombs_17 are 2x1 rooms but get
        // one grid cell each here too, same as every other room.
        constexpr map_grid_position positions[map_room_count] = {
            { 0, 0 },  // catacombs_01
            { 1, 0 },  // catacombs_02
            { 2, 0 },  // catacombs_03 (save room)
            { 3, 0 },  // catacombs_04
            { 4, 0 },  // catacombs_05
            { 4, -1 }, // catacombs_06 (rooms.md: DOWN -> catacombs_05)
            { 5, -1 }, // catacombs_07
            { 6, -1 }, // catacombs_08
            { 1, 1 },  // catacombs_09 (rooms.md: UP -> catacombs_02)
            { 5, 0 },  // catacombs_10
            { 7, -1 }, // catacombs_11
            { 5, -2 }, // catacombs_12 (rooms.md: catacombs_07 UP -> catacombs_12)
            { 8, -1 }, // catacombs_13 (save room)
            { 7, 0 },  // catacombs_14 (rooms.md: catacombs_11 DOWN -> catacombs_14)
            { 7, 1 },  // catacombs_15 (rooms.md: catacombs_14 DOWN -> catacombs_15)
            { 9, -1 }, // catacombs_16
            { 10, -1 }, // catacombs_17 (boss arena)
            { 11, -1 }, // catacombs_18
        };
    }

    map_grid_position map_position_for_room(int room_index)
    {
        if(room_index < 0 || room_index >= map_room_count)
        {
            return { 0, 0 };
        }

        return positions[room_index];
    }
}
