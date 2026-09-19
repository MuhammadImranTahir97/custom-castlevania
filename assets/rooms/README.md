# Room JSON schema (interim, pre-Tiled)

Tiled isn't installed yet (see `ROADMAP.md`'s M0 checklist), so these files
are hand-authored JSON rather than real Tiled exports. The shape is kept
close to a Tiled object layer (rectangle objects with x/y/width/height) on
purpose: once Tiled is installed, only `tools/convert_rooms.py` should need
to change, not the room loader or the game logic that uses it.

Coordinates are in the same space as everywhere else in the game: pixels,
Butano's screen-centered convention (x: -120..120, y: -80..80 for one
240x160 screen). Rooms are not scrolling — each room is exactly one screen,
and doors are instant transitions to another room, like classic Zelda/
Metroid screen transitions.

```json
{
    "id": "unique_room_id",
    "spawn": { "x": 0, "y": 40 },
    "platforms": [
        { "x": -120, "y": 48, "width": 240 }
    ],
    "doors": [
        {
            "x": 104, "y": -80, "width": 16, "height": 160,
            "target_room": "other_room_id",
            "target_x": -104, "target_y": 40
        }
    ]
}
```

- `id`: unique string, referenced by other rooms' `doors[].target_room`.
- `spawn`: where the player starts if this is the first room loaded.
- `platforms`: flat-topped rectangles. `y` is the top surface.
- `doors`: trigger rectangles. Touching one switches the active room to
  `target_room` and places the player at `(target_x, target_y)` in it.

Run `tools/convert_rooms.py` to regenerate the C++ data (also runs
automatically as part of `make`, via the Makefile's `EXTTOOL` hook).
