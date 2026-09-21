# data/rooms.md — Room Specifications

**150 rooms · 10 areas**

This file defines the **room format**, the **area-level plan**, and one
**fully specified area** (Catacombs) as the worked example for the rest.

Remaining areas are specified at M4a, one at a time, using this format.

---

## The room format

```
ROOM: <area>_<nn>
Size: <W>x<H> screens          (1 screen = 240x160 px = 30x20 tiles)
Exits:
  <DIR> → <room_id> [gate: <requirement>]
Layout:
  <plain-English description of geometry>
Enemies:
  <count>x <Enemy Name> (<placement / behaviour note>)
Items:
  <item> (<location note>)
Notes:
  <anything special>
```

### Worked example

```
ROOM: catacombs_07
Size: 1x1
Exits:
  W → catacombs_06
  E → catacombs_08
  UP → catacombs_12 [gate: Double]
Layout:
  Ground floor across full width. Ceiling gap top-right leading up.
  Platform at x=120 y=80, 3 tiles wide.
  Cracked wall on the east side at ground level.
Enemies:
  2x Zombie (ground, patrol, will walk off ledges)
  1x Medusa Head (spawns from right edge, sine loop)
Items:
  3x Candle (hearts)
  Hidden: HP Vessel behind cracked wall [gate: Tackle]
Notes:
  First room teaching that walls can be broken. Crack is visually obvious.
```

---

## Room budget across the castle

| Purpose | Rooms |
|---|---|
| Critical path | 75 |
| Optional / backtrack rewards | 45 |
| Save & warp rooms | 12 |
| Boss arenas | 10 |
| Secret rooms | 8 |
| **Total** | **150** |

---

## Area plan

| # | Area | Rooms | Tileset | Gate to enter | Boss reward |
|---|---|---|---|---|---|
| 1 | Entrance Hall | 12 | Stone, torchlit | — | Dash Boots *(chest)* |
| 2 | Catacombs | 18 | Dark stone, bones | — | **Double** |
| 3 | Machine Tower | 17 | Iron, gears | Double | **Kick Boots** |
| 4 | Underground Gallery | 15 | Marble, statues | Kick Boots | **Heavy Ring** |
| 5 | Chapel | 17 | Stained glass | Heavy Ring | **Roc Wing** |
| 6 | Waterway | 18 | Wet stone, water | Roc Wing | **Cleansing** |
| 7 | Clock Tower | 15 | Brass, gears | Cleansing | **Tackle** |
| 8 | Observatory | 17 | Sky, brass, wind | Tackle | **Final Key** |
| 9 | Inner Quarters | 12 | Red velvet, dark | Final Key | *story* |
| 10 | Ceremonial Room | 9 | Ritual stone | — | *ending* |

**Warp rooms (3):** Entrance Hall, Chapel, Observatory.
**Save rooms (12):** one per area, plus extras in Catacombs and Waterway.

---

## Critical path

```
Entrance ─▶ Catacombs ─▶ Machine Tower ─▶ Gallery ─▶ Chapel
                                                        │
Ceremonial ◀─ Inner Quarters ◀─ Observatory ◀─ Clock ◀─ Waterway
```

Linear spine with locked side-rooms in every area to return to later.

---

## 🔴 Design rules — non-negotiable

1. **Never gate the critical path behind a secret.**
   If the player must find a hidden wall to progress, they get stuck and
   quit. Secrets reward. Secrets never block.

2. **Every room needs a reason to exist** — a fight, a secret, a view, or
   a fun jump. Empty connecting corridors are padding, and players feel it.

3. **Show the lock before you give the key.** The player should walk past
   every gate at least once before they can open it. That memory is the
   whole Metroidvania loop.

4. **Save rooms are visible on the map from the moment the area is entered.**

5. **No blind drops.** Never require a leap of faith into unseen danger.

6. **Character gates are marked.** A room needing a specific character
   shows a visual hint. ~8 rooms total.

7. **Rival-only vertical gaps are a design tool for after the Rival joins,
   never before.** The Rival's higher/floatier jump (SPEC.md section 3)
   covers more horizontal distance per unit of climb than the Hunter's —
   useful for deliberately gating a gap behind the Rival specifically once
   she's playable (area 9 onward — see the boss list's "becomes playable").
   Before that point, every vertical gap in every room must be climbable
   on the Hunter's jump alone, full stop, because there is no other option
   yet. This bit Catacombs once already (area 2's climbing shaft in
   catacombs_05 was originally spaced for a jump distance only the Rival
   could make, which is exactly backwards this early) — check new gap
   spacing against the Hunter's actual jump arc, not just "does it look
   climbable," before treating a room as done.

---

# AREA 2 — CATACOMBS *(18 rooms)*

**Theme:** dark stone, bone piles, dripping water, coffins
**Palette:** desaturated blue-greys, single warm torch accent
**Music:** low, sparse, dread
**Entered from:** Entrance Hall (east)
**Boss reward:** Double *(double jump)*
**This is the M3 vertical slice — build this area completely first.**

---

```
ROOM: catacombs_01
Size: 1x1
Exits:
  W → entrance_12
  E → catacombs_02
Layout:
  Flat corridor. Low ceiling. Bone piles as foreground decoration.
Enemies:
  2x Zombie (slow patrol)
Items:
  2x Candle (hearts)
Notes:
  Tone-setter. Deliberately easy. Palette shift from Entrance Hall
  should be immediately obvious.
```

```
ROOM: catacombs_02
Size: 1x1
Exits:
  W → catacombs_01
  E → catacombs_03
  DOWN → catacombs_09
Layout:
  Ground floor with a pit at centre-right. Floor gap leads down.
  Ledge on the far side of the pit, reachable with a normal jump.
Enemies:
  3x Zombie (two will walk into the pit — teaches their AI)
  1x Skeleton
Items:
  3x Candle
Notes:
  First branching room. Down path is optional.
```

```
ROOM: catacombs_03  ⭐ SAVE ROOM
Size: 1x1
Exits:
  W → catacombs_02
  E → catacombs_04
Layout:
  Small chamber. Save point in the centre, lit.
Enemies:
  none
Items:
  Save point (restores HP + MP fully)
Notes:
  Deliberately early. Players should find their first save quickly.
```

```
ROOM: catacombs_04
Size: 2x1
Exits:
  W → catacombs_03
  E → catacombs_05
Layout:
  Wide horizontal hall, two screens across.
  Three platforms at varying heights. Pit spanning the lower middle.
Enemies:
  4x Skeleton (spread across platforms, throwing bones)
  2x Medusa Head (spawn from east edge, sine loop over the pit)
Items:
  4x Candle
  Chest: Potion
Notes:
  First real fight. Medusa Heads over a pit — the classic Castlevania
  pressure test. Tune carefully; this is where difficulty is first felt.
```

```
ROOM: catacombs_05
Size: 1x2 (vertical)
Exits:
  W → catacombs_04
  UP → catacombs_06
  E → catacombs_10 [gate: Double]
Layout:
  Vertical shaft. Staggered platforms climbing the left wall.
  High doorway in the east wall, 68px above the floor -- UNREACHABLE
  without Double (Rival's single-jump apex is ~65px; Hunter's double
  jump reaches ~74-77px with a comfortably-timed second press).
Enemies:
  3x Bat (erratic, swoop on approach)
  2x Fleaman (chaotic hopping between platforms)
Items:
  2x Candle
Notes:
  🔑 FIRST VISIBLE GATE. Standing under the east doorway and jumping
  straight up is the obvious first thing to try; it doesn't reach. The
  player must see it, fail, and remember it. No platform sits in front
  of the doorway (grey-box: a floating platform there blocks ground-
  level approach entirely under this engine's wall-collision rule --
  see src/game/player.cpp's move_and_collide comment -- so the door
  itself is the only marker for now; a real sprite/ledge lip is later
  visual polish, not a functional requirement).
  This room teaches the entire Metroidvania loop.
```

```
ROOM: catacombs_06
Size: 1x1
Exits:
  DOWN → catacombs_05
  E → catacombs_07
Layout:
  Narrow corridor, low ceiling. Bone Pillar embedded in the far wall
  firing down the length of the corridor.
Enemies:
  1x Bone Pillar (horizontal fireballs every 100 frames)
  2x Zombie (walking into the fire line)
Items:
  2x Candle
Notes:
  Teaches lane control and timing. The Zombies demonstrate the fireball
  is dangerous before the player walks into it.
```

```
ROOM: catacombs_07
Size: 1x1
Exits:
  W → catacombs_06
  E → catacombs_08
  UP → catacombs_12 [gate: Double]
Layout:
  Ground floor full width. Ceiling gap top-right.
  Platform at x=120 y=80, 3 tiles wide.
  Visibly cracked wall on the east side at ground level.
Enemies:
  2x Zombie
  1x Medusa Head
Items:
  3x Candle
  Hidden: HP Vessel behind cracked wall [gate: Tackle]
Notes:
  🔑 First cracked wall in the game. Make the crack visually unmistakable.
  Tackle is found much later (Clock Tower) — this is a long-memory gate.
```

```
ROOM: catacombs_08
Size: 1x1
Exits:
  W → catacombs_07
  E → catacombs_11
Layout:
  Open chamber, two side ledges. Coffins along the back wall.
Enemies:
  3x Skeleton
  2x Fleaman
Items:
  Chest: ARCANA CARD — Venus (Action)
Notes:
  🃏 FIRST ARCANA CARD. Placed, not dropped. Paired with Salamander
  (found in Entrance Hall) this gives the player their first working
  combo: STR ×1.3.
  Trigger a tutorial prompt explaining the Arcana menu here.
```

```
ROOM: catacombs_09
Size: 1x1  [OPTIONAL]
Exits:
  UP → catacombs_02
Layout:
  Dead-end chamber below the main path. Water on the floor.
Enemies:
  4x Zombie (Drowned variant)
Items:
  Chest: Heart Container (+10 max hearts)
Notes:
  First optional reward. Teaches that exploring off-path pays.
```

```
ROOM: catacombs_10  [OPTIONAL — gate: Double]
Size: 1x1
Exits:
  W → catacombs_05
Layout:
  Small high chamber. Nothing but a chest and a view down the shaft.
Enemies:
  2x Bat
Items:
  Chest: MP Vessel (+20 max MP)
Notes:
  🔑 The payoff for catacombs_05's gate. The player will return here
  minutes after beating the boss. That moment — remembering the ledge,
  coming back, getting the reward — is the core loop working.
```

```
ROOM: catacombs_11
Size: 2x1
Exits:
  W → catacombs_08
  E → catacombs_13
  DOWN → catacombs_14
Layout:
  Wide hall. Broken floor sections creating three pits.
  Platforms between them at jump distance.
Enemies:
  3x Skeleton
  4x Medusa Head (continuous spawn from both edges)
Items:
  4x Candle
Notes:
  Difficulty step up. Medusa Heads over pits, from both sides.
  This is the room that makes the player respect the dodge roll.
```

```
ROOM: catacombs_12  [OPTIONAL — gate: Double]
Size: 1x1
Exits:
  DOWN → catacombs_07
Layout:
  Narrow high alcove.
Enemies:
  1x Skeleton (Cursed variant — tougher, dark palette)
Items:
  Chest: ARCANA CARD — Mandragora (Attribute)
Notes:
  Second backtrack reward. Mandragora (lifesteal) is genuinely useful,
  making the return trip feel worthwhile.
```

```
ROOM: catacombs_13  ⭐ SAVE ROOM
Size: 1x1
Exits:
  W → catacombs_11
  E → catacombs_16
Layout:
  Small lit chamber. Save point centre.
Enemies:
  none
Items:
  Save point
Notes:
  Pre-boss save. Two rooms before the boss, not one — the player should
  have to commit a little.
```

```
ROOM: catacombs_14  [OPTIONAL]
Size: 1x2 (vertical)
Exits:
  UP → catacombs_11
  DOWN → catacombs_15
Layout:
  Descending shaft. Narrow. Tight platform spacing.
Enemies:
  3x Bat
  2x Fleaman
Items:
  3x Candle
Notes:
  Connector to the secret room below.
```

```
ROOM: catacombs_15  [SECRET]
Size: 1x1
Exits:
  UP → catacombs_14
Layout:
  Hidden chamber. Entrance concealed behind a false wall at the bottom
  of catacombs_14 — passable by walking into it, no relic needed.
Enemies:
  none
Items:
  Chest: Bone Charm (accessory — DEF +5, undead resistance)
  Lore note #3
Notes:
  🔍 FIRST SECRET. No gate, just observation. Teaches the player that
  false walls exist. Reward is good but not required.
```

```
ROOM: catacombs_16
Size: 1x1
Exits:
  W → catacombs_13
  E → catacombs_17
Layout:
  Long approach corridor. Descending slope. Torches lining the walls,
  growing dimmer toward the east.
Enemies:
  2x Skeleton
Items:
  2x Candle
Notes:
  🎵 Boss music begins fading in here, before the boss is visible.
  Light level drops progressively. Pure atmosphere — the fight is next.
```

```
ROOM: catacombs_17  💀 BOSS ARENA
Size: 2x1
Exits:
  W → catacombs_16 (sealed during fight)
  E → catacombs_18 (opens on victory)
Layout:
  Wide flat arena, two screens. No platforms, no pits.
  Bone piles as breakable scenery at the edges.
Enemies:
  BOSS: Bone Colossus
Items:
  none
Notes:
  💀 FIRST BOSS. Flat arena is deliberate — this fight tests the combat
  fundamentals, not platforming.

  BONE COLOSSUS
  HP 400 · DEF 12 · EXP 300

  Phase 1 (100–60% HP):
    - Overhead slam (30-frame windup, heavy telegraph, rollable)
    - Bone sweep (20-frame windup, low, must JUMP not roll)

  Phase 2 (60–25% HP):
    - Adds: summons 2 Skeletons every 15 seconds
    - Slam becomes a 2-hit combo (one roll dodges only the first)

  Phase 3 (below 25%):
    - Adds: rib-cage projectile spread (5 bones in an arc, unrollable —
      must be positioned between them)
    - Attack speed +25%

  Teaches: read the tell, not all attacks are rollable, phases change
  patterns. Both characters are viable — Hunter pokes from range,
  Rival punishes recovery frames.
```

```
ROOM: catacombs_18
Size: 1x1
Exits:
  W → catacombs_17
  E → machine_tower_01
Layout:
  Small chamber past the arena. A pedestal in the centre, lit from above.
Enemies:
  none
Items:
  RELIC: Double (double jump)
  Lore note #4
Notes:
  🔑 RELIC ROOM. On pickup, show a brief prompt explaining double jump.

  IMPORTANT: the player should immediately want to backtrack to
  catacombs_05 and catacombs_07. Consider a subtle map hint marking
  rooms with unreachable areas once the relic is collected.
```

---

## Catacombs summary

| Metric | Value |
|---|---|
| Rooms | 18 |
| Critical path | 11 |
| Optional | 4 |
| Secret | 1 |
| Save rooms | 2 |
| Boss arena | 1 |
| Arcana cards | 2 (Venus, Mandragora) |
| Relics | 1 (Double) |
| Vessels | 2 (HP, MP) |
| Heart Containers | 1 |
| Lore notes | 2 |
| Enemy types used | Zombie, Skeleton, Bat, Fleaman, Medusa Head, Bone Pillar |

**Estimated playtime:** 25–35 minutes first visit.

---

## Remaining areas

Specify one area at a time during **M4a**, using the format above.
Do not write all nine at once — layout decisions in later areas depend on
how the earlier ones actually play.

| Area | Status |
|---|---|
| 1 — Entrance Hall | ⬜ specify before M3 (tutorial rooms) |
| 2 — Catacombs | ✅ **complete** |
| 3 — Machine Tower | ⬜ M4a |
| 4 — Underground Gallery | ⬜ M4a |
| 5 — Chapel | ⬜ M4a |
| 6 — Waterway | ⬜ M4a |
| 7 — Clock Tower | ⬜ M4a |
| 8 — Observatory | ⬜ M4a |
| 9 — Inner Quarters | ⬜ M4a |
| 10 — Ceremonial Room | ⬜ M4a |
