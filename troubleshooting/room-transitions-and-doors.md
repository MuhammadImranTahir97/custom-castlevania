# Room transitions: the infinite-loop bug, the fix, and the door validator

## The bug: rooms flickering forever between two doors

**Symptom:** walking through certain doors caused the game to flicker
between two rooms forever instead of settling in the destination room.

**Cause:** `game::level::try_cross_door` checked whether the player's
position overlapped a door, and if so, teleported the player to that
door's authored target position in the target room and switched rooms.
Nothing stopped the *very next frame* from immediately re-checking the
new position against the new room's doors -- and if the target position
itself happened to land inside a door's trigger box (most often the
reciprocal door leading back where the player came from), that door
fired immediately, sending the player right back. Next frame, the same
thing happens in reverse. Forever.

This wasn't a one-off authoring mistake in a single room -- of the 34
doors in the 18 Catacombs rooms (plus 2 in sandbox test rooms, 36 total),
**6** had a target position landing inside another door's trigger box,
because the room-authoring convention of "place the arrival spot just a
few pixels inside the wall, near the door you came from" doesn't leave
enough clearance once the trigger boxes are checked as boxes rather than
points.

## The fix: arrival-door suppression, not a timer

A frame-based cooldown (e.g. "ignore doors for N frames after crossing")
was explicitly rejected -- it doesn't fix the bug, it just changes the
bounce period from 1 frame to N frames, and still bounces forever once
that period elapses if the player hasn't moved.

Instead, `try_cross_door` (`src/game/level.cpp`) now:

1. Takes the caller's actual hitbox (half-width/half-height), not just a
   point, so overlap checks match what a player really occupies.
2. After a crossing, checks whether the landing spot overlaps a door in
   the *new* room. If it does, that door's index is remembered as
   suppressed for the active room.
3. A suppressed door is skipped by the overlap check entirely, every
   frame, until the caller's hitbox no longer overlaps its box -- at
   which point suppression is cleared and it becomes a normal door again.

This handles the bug generically for any door placement, including
legitimate ones (e.g. a door mounted directly above a landing platform,
where the player's standing position is inherently inside the door's
box) without needing every room's data to be perfectly clear of this
overlap.

## The door validator (`tools/validate_doors.py`)

Runtime suppression stops the bounce, but a room that needs it wasn't
placed right, and should still be fixed at the data level. `tools/validate_doors.py`
reuses `convert_rooms.py`'s own room loader (so it checks exactly what
the build will generate) and checks, per door:

- the target position resolves within the target room's bounds
- the target position doesn't land inside another door's trigger box in
  the target room

Run as `python tools/validate_doors.py --rooms-dir assets/rooms`.

## What it caught, and how each was fixed

All fixed by moving the *target* position (not the door's own trigger
box) a comfortable distance away from the reciprocal door's box, clear of
nearby enemies, and confirmed still standing on a real platform:

| Door | Was landing at (game coords) | Problem | Fixed to |
|---|---|---|---|
| `catacombs_01` -> `catacombs_02` | (-104, 40) | inside `catacombs_02`'s door back to `catacombs_01` | (-60, 40) |
| `catacombs_05` -> `catacombs_06` | (-104, 40) | inside `catacombs_06`'s door back to `catacombs_05` | (-40, 40) |
| `catacombs_06` -> `catacombs_05` | (-60, -136) | inside `catacombs_05`'s door back to `catacombs_06` (a ceiling door directly above the only nearby platform) | (-40, -104), on the next platform down instead of the one flush against the door |
| `catacombs_11` -> `catacombs_14` | (0, -136) | inside `catacombs_14`'s door back to `catacombs_11` (same ceiling-door pattern) | (-60, -104), on the next platform down |
| `catacombs_14` -> `catacombs_11` | (-90, 40) | inside `catacombs_11`'s door back to `catacombs_14` | (-52, 40) |
| `sandbox_00` -> `sandbox_01` | (-104, 40) | inside `sandbox_01`'s door back to `sandbox_00` | (-80, 40) |

One additional room-level bug found separately (not a reciprocal-door
overlap, so the validator as described above didn't flag it, but the
same investigation surfaced it): **`catacombs_12`**'s only door had a
trigger box covering almost the entire room, and its target position
(and the room's own authored spawn point) landed exactly on top of the
room's skeleton enemy. Fixed by moving both the door's target and the
room's spawn to a clear spot on the same platform, 30px from the enemy
(matching the ~20-24px spacing convention used for enemy placement
elsewhere in the dungeon).

After all of the above, `validate_doors.py` reports `all doors OK`
across all 36 doors.

## A note on scope

The validator's own strict rule (no overlap with *any* door box) is
stricter than what most doors in the dungeon actually satisfy once
hitbox-width is accounted for -- the standard "land a few pixels inside
the wall from the door you entered" convention used throughout the
Catacombs would flag far more than the 6 above if checked against the
player's actual hitbox rather than a bare point. That's fine: it's
exactly the class of case the runtime suppression fix is meant to make
safe. Only the specific rooms above were fixed at the data level, since
those were the ones an explicit request called out; the validator is
available for a broader sweep later if wanted.
