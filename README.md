# custom-castlevania

A gothic Metroidvania for **real Game Boy Advance hardware**, written in
C++17 with [Butano](https://github.com/GValiente/butano). It's inspired by
*Castlevania: Circle of the Moon*, but every character, room, sprite and
sound is original or CC0. No Konami code, art, music or level layouts are
in this repo (see the legal section of [SPEC.md](SPEC.md)).

The working title isn't decided yet; the ROM builds as `cot-hack.gba`.
The first area, the Catacombs, is playable with grey-box placeholder art
and no audio.

---

## What's in here

| Path | What it is |
|---|---|
| [SPEC.md](SPEC.md) | The full game design |
| [ROADMAP.md](ROADMAP.md) | Milestones M0–M5, in build order |
| [CLAUDE.md](CLAUDE.md) | Project rules and conventions (also read by Claude Code) |
| [rooms.md](rooms.md), [enemies.md](enemies.md), [arcana.md](arcana.md) | Design data: room specs, enemy stats, the 100 Arcana combos |
| [v2.md](v2.md) | Ideas deliberately deferred until after v1 |
| [troubleshooting/](troubleshooting/) | Problems already solved, and how |
| `src/game/` | Game logic. Pure C++ with no Butano, so it can be ported later |
| `src/platform/` | Everything that talks to the GBA through Butano |
| `assets/rooms/` | Rooms, edited in [Tiled](https://www.mapeditor.org/) (`.tmx`) and exported to `.tmj` |
| `assets/sprites/` | Grey-box placeholder sprites |
| `tools/` | Room converter, placeholder-art generators, door and gate validators |

---

## Setting up a fresh clone

The engine, the toolchain and the ROM are not in this repo, so a fresh
clone needs four things before `make` works.

1. **Python 3**, on your `PATH` as `python`. Every build runs
   `tools/convert_rooms.py` to turn the room files into C++.
2. **Wonderful Toolchain** with the GBA target. Follow Butano's
   [Getting started with Wonderful Toolchain](https://gvaliente.github.io/butano/getting_started_wt.html)
   guide.
   On Windows, read
   [troubleshooting/build-environment.md](troubleshooting/build-environment.md)
   first; it covers every setup problem hit so far.
3. **Butano 21.8.0**, cloned into `tools/butano/` and pinned to the exact
   commit this project was built against. A newer Butano may not compile
   this code.
   ```
   git clone https://github.com/GValiente/butano.git tools/butano
   git -C tools/butano checkout 77dcbcb3d8783596a9f333c64eedbccec77b05dc
   ```
4. **A `.env` file.** Copy `.env.example` to `.env` and set
   `WONDERFUL_TOOLCHAIN` to your install folder (the one that contains
   `bin/`, `toolchain/` and `target/`).

Then run `make` in the repo root. It produces `cot-hack.gba`, which you
can open in [mGBA](https://mgba.io/). Tiled is only needed to edit rooms,
not to build.

After editing rooms, check them with:

```
python tools/validate_doors.py --rooms-dir assets/rooms
python tools/validate_gates.py --rooms-dir assets/rooms
```

---

## Controls

| GBA button | mGBA default key | Action |
|---|---|---|
| D-pad | Arrow keys | Move; in menus, move the cursor |
| A | X | Jump (tap for a short hop, hold for full height) |
| B | Z | Attack |
| Down + B | Down + Z | Cast the equipped Arcana |
| L | A | Swap character, once the Rival has been found |
| R | S | Dodge roll |
| START | Enter | Save, while standing in a save room |
| SELECT | Backspace | Pause menu (L and R switch tabs) |

Debug only, to be removed before release: hold **L + R + START** to warp
to any room, or **L + R + B** to toggle the Rival unlock.

---

## Where things stand (2 Oct 2026)

The vertical slice (M3) is playable, and its checkpoint ("would you play
this if someone else had made it?") passed on 21 Sep 2026. A few items
from M1–M3 are still open (below). After those comes M4, the content
milestone.

| | Built | Needed for v1 |
|---|---|---|
| Rooms | 18 (Catacombs) | 150 in 10 areas |
| Enemy types | 7 | 18 + palette variants |
| Bosses | 1 (Bone Colossus) | 10 |
| Relics | 1 (Double) | 8 |
| Arcana combos | 6 | 100 |
| Story and lore | none | ~50 text boxes, 15 notes |
| Audio | none | 14 tracks, 3 jingles, ~40 sound effects |
| Art | grey boxes | CC0 pack, a tileset per area |

### Bugs and gaps in finished work

- **The Bone Colossus respawns.** `spawn_room_enemies()` in
  `src/game/enemy_spawner.cpp` re-creates it every time you enter
  catacombs_17, and the doors seal again. Backtracking from catacombs_18
  means fighting it again. Its defeat also isn't saved.
- **Hitstop, screen shake and knockback were never built** (M1 items,
  SPEC section 2).
- **Nothing can draw text yet.** The pause menu's Status, Items and
  Options tabs are empty, and story text, lore notes, tutorial prompts
  and the title screen all need text first.
- **Saves leave things out:** explored rooms, bosses beaten and the
  equipped Arcana. There's no playtime counter.
- **catacombs_07's door to catacombs_12** needs `requires: double` so
  `validate_gates.py` checks it. It passes once that's added.

### Rework needed before M4 content

- **Sprites:** 109 of the GBA's 128 sprites are allocated at boot, 4 per
  enemy type whether or not that enemy is in the room, and the sprite
  palettes are nearly full. Allocate sprites per room instead.
- **Map screen:** it uses one sprite per room, with the Catacombs layout
  hard-coded in `src/game/map_layout.cpp` and `src/platform/main.cpp`.
  150 rooms won't fit; move it to a background layer driven by room data.
- **Data over code:** `assets/enemies.json` and `assets/arcana.json`
  (CLAUDE.md rule #4) don't exist yet. Enemies and Arcana combos are
  written by hand in C++.
- **Room format:** add chests, candles, breakable and fake walls, water,
  moving platforms, pushable blocks, warp rooms and lore notes.
- **Difficulty:** all balance values are compile-time constants, so an
  in-game Easy/Normal/Hard option can't change them yet. The multipliers
  SPEC lists don't exist yet either.

### Still to decide (in SPEC, but on no milestone)

- Subweapons: Up+B is reserved for them and hearts are their ammo.
- Area mechanics: swimming, moving platforms, timing hazards, wind.
- Enemy levels, which the EXP soft cap needs.
- Death penalty: SPEC says progress since the last save is lost, but the
  code keeps it.
- Game title, character names, art direction, and which CC0 sprite pack.

### Out-of-date docs

- ROADMAP.md's M1 checkboxes and progress table.
- SPEC.md's "Not yet in development" and "devkitARM".
- SPEC section 4 and arcana.md describe swapping Arcana with the shoulder
  buttons; section 9 moved it to the pause menu.
- SPEC refers to `data/rooms.md` and friends, but those files live at the
  repo root.

### Suggested order

1. Set up the build again (see above).
2. Fix the boss respawn, and save the missing data.
3. Add hitstop, screen shake and knockback, then re-check how combat feels.
4. Add text rendering and finish the Status tab.
5. Rework sprite allocation and the map, and decide on `enemies.json` /
   `arcana.json`.
6. Design and build the Entrance Hall, where the game starts and where
   the Dash Boots are found.
