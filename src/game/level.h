#pragma once

#include "fixed.h"

namespace game::level
{
    // Rooms are data (assets/rooms/*.tmj, Tiled JSON exports), converted at
    // build time by tools/convert_rooms.py into generated/include/room_data.h.
    // This module just holds "which room is active" and answers queries
    // against it — it doesn't know the data format itself.

    // The GBA's fixed visible viewport. Rooms can be bigger than this (see
    // room_half_width/height below) — the camera scrolls within them.
    constexpr int screen_half_width = 120;
    constexpr int screen_half_height = 80;

    // Returned by ground_top_y_at when x has no platform under it at all
    // (as opposed to a lower one) — e.g. a walker mid-pit-fall.
    constexpr fixed no_ground_y = to_fixed(10000);

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
    // active room and returns its authored spawn point. Also resets the
    // camera to center on that spawn point.
    spawn_point load_room(int room_index);

    // Index of the currently active room.
    int current_room_index();

    // The active room's authored spawn point, without switching rooms
    // (unlike load_room). Used to checkpoint at a save room's own spawn.
    spawn_point current_room_spawn_point();

    // True if the active room is a save room (SPEC.md: restores HP/MP fully
    // and becomes the respawn checkpoint on entry — see src/game/world.cpp).
    bool current_room_is_save_room();

    // The active room's total world size in pixels, divided by two. A
    // single-screen room has room_half_width() == screen_half_width; a
    // "2x1" room (rooms.md) has room_half_width() == screen_half_width * 2.
    // Movement/AI code clamps to these, not the fixed screen size, so
    // walkers and the player can use the room's full width, not just one
    // screen of it.
    int room_half_width();
    int room_half_height();

    // Moves the camera to follow (focus_x, focus_y), clamped so the visible
    // screen never shows past the active room's edges. For a room no
    // bigger than one screen this always resolves to (0, 0) — the old,
    // effectively-fixed camera behavior before rooms could scroll.
    void update_camera(fixed focus_x, fixed focus_y);
    fixed camera_x();
    fixed camera_y();

    // Read-only access to the active room's platforms, for rendering.
    int room_platform_count();
    platform_view room_platform(int index);

    // Plain-pixel view of one enemy spawn in the active room.
    // type: see tools/convert_rooms.py's ENEMY_TYPE_IDS.
    // facing: -1 or +1; only meaningful for enemies that don't decide their
    // own facing dynamically (e.g. a stationary Bone Pillar, or which edge
    // a Medusa Head spawner flies in from). Defaults to -1 if unauthored.
    struct enemy_spawn_view
    {
        int type;
        int x;
        int y;
        int facing;
    };

    // Read-only access to the active room's enemy spawns.
    int room_enemy_spawn_count();
    enemy_spawn_view room_enemy_spawn(int index);

    // Returns the y of the topmost surface under x, in the active room.
    fixed ground_top_y_at(fixed x);

    // If the (half_width, half_height) box centered on (x, y) overlaps a
    // door in the active room, switches to that door's target room, fills
    // *out_spawn with the target position, and returns true. Returns false
    // (leaving *out_spawn untouched) otherwise.
    //
    // The door landed on in the target room (if the target position itself
    // overlaps one — a reciprocal door placed flush against the arrival
    // spot, intentionally or not) is suppressed: it's ignored by this
    // function until the caller's box no longer overlaps it. Without this,
    // arriving inside a door's own trigger box re-fires it next frame and
    // sends the caller right back, forever. No frame timer is used — a
    // timer just changes the bounce period, it doesn't stop the bounce.
    bool try_cross_door(fixed x, fixed y, fixed half_width, fixed half_height, spawn_point& out_spawn);
}
