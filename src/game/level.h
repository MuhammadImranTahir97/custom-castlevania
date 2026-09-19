#pragma once

#include "fixed.h"

namespace game::level
{
    // Rooms are data (assets/rooms/*.tmj, Tiled JSON exports), converted at
    // build time by tools/convert_rooms.py into generated/include/room_data.h.
    // This
    // module just holds "which room is active" and answers queries
    // against it — it doesn't know the data format itself.

    constexpr int screen_half_width = 120;
    constexpr int screen_half_height = 80;

    struct spawn_point
    {
        fixed x;
        fixed y;
    };

    // Plain-pixel view of one platform in the active room, for the platform
    // layer to turn into placeholder sprites. See src/platform/main.cpp.
    struct platform_view
    {
        int left_x_px;
        int top_y_px;
        int width_px;
    };

    // Makes the given room (by index into the generated room table) the
    // active room and returns its authored spawn point.
    spawn_point load_room(int room_index);

    // Index of the currently active room.
    int current_room_index();

    // Read-only access to the active room's platforms, for rendering.
    int room_platform_count();
    platform_view room_platform(int index);

    // Plain-pixel view of one enemy spawn in the active room.
    // type: 0 = skeleton, 1 = bat, 2 = archer (see tools/convert_rooms.py).
    struct enemy_spawn_view
    {
        int type;
        int x;
        int y;
    };

    // Read-only access to the active room's enemy spawns.
    int room_enemy_spawn_count();
    enemy_spawn_view room_enemy_spawn(int index);

    // Returns the y of the topmost surface under x, in the active room.
    fixed ground_top_y_at(fixed x);

    // If (x, y) overlaps a door in the active room, switches to that
    // door's target room, fills *out_spawn with the target position, and
    // returns true. Returns false (leaving *out_spawn untouched) otherwise.
    bool try_cross_door(fixed x, fixed y, spawn_point& out_spawn);
}
