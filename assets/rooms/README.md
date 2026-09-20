# Rooms: Tiled maps

Rooms are authored in [Tiled](https://www.mapeditor.org/) as `.tmx` files
and exported to `.tmj` (Tiled's JSON format). `tools/convert_rooms.py`
reads the committed `.tmj` files and generates the C++ room data — it
does not invoke Tiled itself, so building the ROM never requires Tiled
to be installed. Tiled is only needed for editing rooms.

## Workflow

1. Open the room's `.tmx` in Tiled and edit it.
2. Export it: **File > Export As...** and save over the matching `.tmj`
   (same base name, next to the `.tmx`), or from the command line:
   ```
   "C:\Program Files\Tiled\tiled.exe" --export-map assets/rooms/<room>.tmx assets/rooms/<room>.tmj
   ```
3. Commit both the `.tmx` and the re-exported `.tmj`.
4. `make` picks up the change automatically (the converter runs on every
   build via the Makefile's `EXTTOOL` hook).

A room's id is its filename without the extension, e.g. `catacombs_00.tmj`
is room `catacombs_00`. Ids must be unique — they're how doors reference
their target room.

## Room contents

Each room is a single object layer named `objects`, sized in whole
screens (240x160 each) — 1x1, 2x1 wide, or 1x2 tall, per `rooms.md`'s
`Size:` field — containing:

- **Platforms** — rectangles with a custom property `kind` = `platform`.
  Flat-topped; the rectangle's top edge is the walkable surface.
- **Doors** — rectangles with `kind` = `door`, plus custom properties
  `target_room` (string, the room id to switch to), `target_x` and
  `target_y` (int, where to place the player in that room). Touching a
  door instantly switches rooms — it's a hard cut, like classic
  Zelda/Metroid screen transitions, even between two multi-screen rooms.
- **Spawn** — exactly one point object with `kind` = `spawn`: where the
  player starts if this is the first room loaded (or the checkpoint, if
  this is a save room).
- **Enemies** — point objects with `kind` = `enemy` and an `enemy_type`
  property (one of: `skeleton`, `bat`, `archer`, `zombie`, `bone_pillar`,
  `fleaman`, `medusa_head` — see `enemies.md`) and an optional int
  `facing` property (-1 or +1, default -1), for enemies that can't decide
  their own facing dynamically — a stationary Bone Pillar's aim, or which
  edge a Medusa Head spawner flies in from. Entering a room spawns fresh
  copies of everything in its enemy list; enemy state (HP, position)
  isn't preserved when you leave and come back — that's a later,
  save-system-adjacent concern.

Custom properties are added in Tiled via the Properties panel (the `+`
button) on a selected object.

Coordinates in Tiled are pixels from the map's top-left corner; the
converter translates them into the game's screen-centered convention
using the map's own width/height, so a room can be bigger than one
screen — `src/game/level.cpp`'s camera scrolls within whatever size the
room turns out to be, clamped so it never shows past the room's edges.

Each enemy type currently allows at most `game::max_enemies_per_type`
(4) active instances per room — see `src/game/enemy_spawner.h`.
