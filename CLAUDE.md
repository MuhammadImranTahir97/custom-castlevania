# CLAUDE.md — Rules for Claude Code

Read this before writing any code. Read `SPEC.md` for the design.

---

## Project

A Metroidvania for **real Game Boy Advance hardware**.
Engine: **Butano** (C++17) on **Wonderful Toolchain**. Tested in **mGBA**.

This is not a browser game, not a PC game, not an emulator-only target.
It must build to a `.gba` ROM that runs on hardware.

---

## Build environment

The Makefile needs exactly one of these set: `WONDERFUL_TOOLCHAIN` (this
project's toolchain — a Wonderful Toolchain install root, the directory
containing `bin/`, `toolchain/`, `target/`) or `DEVKITARM` (see
`tools/butano/butano/butano.mak`'s autodetection). Without one of these,
`make` fails immediately with `"DEVKITARM and WONDERFUL_TOOLCHAIN not
found"`.

Don't rely on exporting this in your shell profile — it's easy to forget
and the error only shows up the next time you open a new shell. Instead:

1. Copy `.env.example` to `.env` (gitignored, machine-specific).
2. Set `WONDERFUL_TOOLCHAIN=<path-to-your-install>` in it.

The root `Makefile` loads `.env` itself (`-include .env` + `export`), so
plain `make` picks up the toolchain with no shell setup required.

If `make` reports missing DLLs (e.g. `STATUS_DLL_NOT_FOUND` /
`api-ms-win-crt-*.dll`) when actually invoking the compiler, that's a
Windows UCRT/redistributable problem on the machine, not a toolchain or
project config issue — the toolchain path itself was resolving correctly.

---

## 🔴 Hard rules — never break these

### 1. The portability split

```
src/game/      → pure C++. NO Butano headers. Ever.
src/platform/  → ALL Butano code lives here
```

A PSP port is planned. It is only affordable if this line holds.

- Game logic **describes** state: `player.x`, `player.y`, `player.anim_frame`
- The platform layer **draws** it
- Input arrives as an abstract struct, never as `bn::keypad::*` inside `game/`

**Before writing any file, ask: does this belong in `game/` or `platform/`?**

### 2. Never commit the ROM or copyrighted assets

`.gitignore` must contain:

```
*.gba
*.sav
*.zip
build/
```

No Konami sprites, tilesets, music, or room layouts. Ever.
All assets are original or CC0. See the legal section of `SPEC.md`.

### 3. Never read large binaries directly

Do not `cat` or `view` a `.gba`, `.bin`, or any multi-megabyte file.
Write a script that reads byte ranges instead.

### 4. Data over code

Rooms, enemies, and Arcana combos are **data files**, not C++ source.
Adding room #91 must never require writing code.

```
assets/rooms/*.json      → room layouts (exported from Tiled)
assets/enemies.json      → enemy stat blocks
assets/arcana.json       → the 100 combos
```

A converter in `tools/` turns these into GBA-ready binary data at build time.

### 5. One tunable file

Every balance number lives in `src/game/difficulty.h`.
No magic numbers scattered through combat code.

---

## 🟡 Strong preferences

### Build order

Follow `ROADMAP.md`. **Do not skip ahead.**

M1 is one room, one character, one enemy. It must feel good before
anything else is built. If asked to jump to content before M1 feels
right, say so.

### Grey-box first

Placeholder art is correct for the first several months. Coloured
rectangles at the right dimensions. Do not block progress on sprites.

Write a placeholder generator in `tools/` that outputs correctly-sized,
correctly-framed, colour-coded boxes for every sprite the game needs.

### Performance

GBA is ~16.8 MHz. Assume nothing is free.

- No heap allocation in the game loop (Butano's stdlib has no heap by design)
- Fixed-point maths, not floats
- Hot data in IWRAM
- Profile before optimising — Butano ships profiling tools

### Sprite budget

128 OAM entries total. Enemies, projectiles, the player, and effects all
compete. If a room can spawn 12 enemies each firing 3 projectiles, that
is already 48 sprites. Track this.

---

## Code style

- C++17, no exceptions, no RTTI
- `snake_case` for functions and variables, `PascalCase` for types
- Prefer plain structs and free functions over deep class hierarchies
- Comment the *why*, never the *what*
- Keep functions short enough to read without scrolling

---

## Directory layout

```
project/
├── CLAUDE.md              this file
├── SPEC.md                the design
├── ROADMAP.md             milestones
├── v2.md                  ideas deferred past v1
├── data/                  design data (human-readable)
│   ├── enemies.md
│   ├── arcana.md
│   └── rooms.md
├── assets/                game data (machine-readable)
│   ├── rooms/
│   ├── sprites/
│   └── audio/
├── src/
│   ├── game/              pure C++ — portable
│   └── platform/          Butano only
├── tools/                 converters, generators, scripts
└── build/                 gitignored
```

---

## When you are unsure

**Ask. Do not guess.**

Specifically, ask before:

- Adding a feature not in `SPEC.md`
- Changing a balance number (they were deliberated — see `SPEC.md`)
- Introducing a new dependency
- Restructuring the `game/` ↔ `platform/` boundary
- Anything that would take the project past its current milestone

**Scope creep is the number one risk to this project.** If a change makes
the game bigger rather than better, flag it rather than building it.

New ideas that are genuinely good go in `v2.md`. They do not go in the build.

---

## Things that are already decided

Do not re-litigate these. They were worked through carefully:

| Decision | Value |
|---|---|
| Platform | GBA, Butano, devkitARM |
| Characters | 2, instant swap, shared stats |
| Weapon swapping | **Removed** — replaced by character swap |
| Dodge cost | 10 MP, 60-frame regen pause |
| Arcana | 10 × 10 = 100, systematically generated |
| Card acquisition | **Placed in world.** Never RNG |
| Rooms | 150 for v1 |
| Relics | 8, Dash Boots first |
| EXP | `level² × 8`, soft cap on overlevelled kills |
| Save | SRAM, 3 slots, manual, 12 save rooms |
| Audio | Maxmod tracker, last thing built |

---

## Verification before saying something works

- Does it build with `make`?
- Does it run in mGBA without assertion failures?
- Does `src/game/` still contain zero Butano includes?
- Is the frame rate still 60?
- Are sprite counts within budget?

**Do not report a milestone complete without checking all five.**
