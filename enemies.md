# data/enemies.md — Enemy Specifications

18 base types · 7 AI behaviours · ~35 encounters with palette variants

All numbers are tunable via `difficulty.h` multipliers.

---

## The 7 AI behaviours

| Type | Behaviour |
|---|---|
| **Walker** | Patrols ground, turns at edges and walls |
| **Flyer** | Ignores gravity, moves on a fixed pattern |
| **Jumper** | Hops in arcs, semi-random timing |
| **Shooter** | Maintains distance, fires projectiles |
| **Stationary** | Does not move, attacks in range |
| **Charger** | Detects player, winds up, rushes |
| **Teleporter** | Blinks to a new position near the player |

Every enemy is one of these + stats + sprite. New enemies cost data, not code.

---

## 🛡️ The anti-roll toolkit

The dodge is cheap (10 MP) with generous i-frames. **7 of 18 enemies
(~40%) are designed to punish roll-spam**, each using a *different* trick
so no single counter works everywhere.

| Trick | Enemy | What it defeats |
|---|---|---|
| Arcing attack | Axe Armor | Rolling underneath |
| Delayed hit | Harpy | Rolling too early |
| Homing projectile | Fire Witch | Rolling away |
| Lingering hitbox | Evil Pillar | Short i-frames (11 frames) |
| Multi-hit combo | Were-panther | One roll, three hits |
| Long charge distance | Armor Knight | Out-distancing the attack |
| Unrollable grab | Mimic | I-frames entirely |

**The lesson each teaches:** rolling is a tool with a correct moment, not
a button to mash.

These are also where **character choice starts mattering** — spear-reach
logic favours the Hunter against the Armor Knight; the Rival's speed
escapes the Were-panther's combo.

---

# Stat blocks

Format:
```
HP / STR(damage) / DEF / EXP / Speed / First area
```

---

## Walkers

### Zombie
```
HP 20 · DMG 8 · DEF 2 · EXP 6 · Speed 0.4 · Catacombs
AI: walker, does not turn at ledges (walks off)
Spawns in groups of 3–5
Drops: hearts (common)
```
The series opener. Slow, tanky, arrives in numbers. Teaches crowd control.

### Skeleton
```
HP 18 · DMG 10 · DEF 4 · EXP 8 · Speed 0.8 · Catacombs
AI: walker + throws bone in a low arc every 90 frames
Drops: hearts, Potion (rare)
```
The default Castlevania enemy. **Use this one for M1** — simple, readable.

### 🛡️ Axe Armor
```
HP 40 · DMG 16 · DEF 10 · EXP 22 · Speed 0.5 · Machine Tower
AI: walker + throws axe in a HIGH ARC every 120 frames
Anti-roll: the arc passes over a rolling player and lands behind them
Drops: hearts, armor piece (rare)
```
Teaches that not all projectiles can be rolled under.

### Were-wolf
```
HP 55 · DMG 20 · DEF 8 · EXP 35 · Speed 1.6 · Gallery
AI: fast walker, lunges 40px when within 60px (20-frame windup)
Drops: hearts, Potion
```

---

## Flyers

### Medusa Head
```
HP 8 · DMG 6 · DEF 0 · EXP 3 · Speed 1.0 · Catacombs
AI: flyer, sine wave, spawns continuously from screen edge
Drops: nothing
```
Iconic, hated, perfectly designed. Use over pits. Use sparingly — they
are a *hazard*, not an enemy.

### Bat
```
HP 10 · DMG 7 · DEF 1 · EXP 4 · Speed 1.2 · Entrance Hall
AI: flyer, idles until player within 80px, then erratic swoop
Drops: hearts (common)
```

### 🛡️ Harpy
```
HP 35 · DMG 18 · DEF 6 · EXP 28 · Speed 1.4 · Observatory
AI: flyer, hovers above player, screeches, DIVES AFTER 40-FRAME DELAY
Anti-roll: the delay outlasts a panic-roll's i-frames (frames 4–14)
Drops: hearts, MP Vessel (rare)
```
Punishes rolling on reaction. Must be rolled on the *dive*, not the screech.

---

## Jumpers

### Fleaman
```
HP 12 · DMG 9 · DEF 2 · EXP 7 · Speed 1.5 · Catacombs
AI: jumper, random hop height and direction, 30–60 frame intervals
Drops: hearts
```
Chaotic in the good way. Hard to hit, low threat.

### Frog
```
HP 16 · DMG 8 · DEF 3 · EXP 9 · Speed 0.9 · Waterway
AI: jumper, predictable fixed arc toward player, 70-frame interval
Drops: hearts
```
The readable jumper — counterweight to the Fleaman.

---

## Shooters

### Bone Pillar
```
HP 30 · DMG 12 · DEF 14 · EXP 15 · Speed 0 · Catacombs
AI: stationary head, fires horizontal fireball every 100 frames
Drops: hearts
```
Lane control. Forces jumping or timing.

### 🛡️ Fire Witch
```
HP 28 · DMG 15 · DEF 5 · EXP 26 · Speed 0.7 · Machine Tower
AI: floats, retreats from player, fires HOMING fireballs every 80 frames
Anti-roll: homing means rolling away does not work — break line of sight
           or destroy the projectile
Drops: hearts, Arcana card location marker
```

### Skeleton Archer
```
HP 22 · DMG 14 · DEF 6 · EXP 18 · Speed 0.6 · Chapel
AI: shooter, arcing arrows, backs away if player closes to 50px
Drops: hearts
```

---

## Stationary

### 🛡️ Evil Pillar
```
HP 50 · DMG 22 · DEF 18 · EXP 30 · Speed 0 · Inner Quarters
AI: stationary, emits AOE pulse every 150 frames
    Pulse hitbox LINGERS 25 frames — longer than i-frames (11)
Anti-roll: you cannot roll through the pulse; you must be outside it
Drops: hearts, Potion
```

### 🛡️ Mimic
```
HP 45 · DMG 25 · DEF 12 · EXP 40 · Speed 0.3 · Gallery
AI: disguised as a chest. Opens into a GRAB attack when approached
    Grab IGNORES i-frames entirely
Anti-roll: the only counter is not being in range
Drops: guaranteed good item (the joke pays off)
```
Teaches that some things cannot be dodged — only avoided.

---

## Chargers

### 🛡️ Armor Knight
```
HP 70 · DMG 24 · DEF 22 · EXP 50 · Speed 2.2 (charging) · Chapel
AI: charger, 50-frame windup with visible tell, then charges 200px
Anti-roll: the charge covers 200px; a roll covers 40–50px
           You must roll THROUGH it, not away from it
Favours: Hunter (whip reach lets you punish from outside charge range)
Drops: hearts, armor (rare)
```

### 🛡️ Were-panther
```
HP 60 · DMG 14 ×3 · DEF 14 · EXP 45 · Speed 1.8 · Observatory
AI: charger, closes distance, then a 3-HIT CLAW COMBO
    Hits at frames 0, 20, 40 of the attack
Anti-roll: one roll (11 i-frames) dodges hit 1; hits 2 and 3 land
Favours: Rival (speed to escape between hits)
Drops: hearts, accessory (rare)
```

---

## Teleporters

### Ghost
```
HP 25 · DMG 13 · DEF 0 · EXP 20 · Speed 0.5 · Clock Tower
AI: teleporter, drifts through walls, blinks when hit
Immune to: physical (needs Arcana or subweapon)
Drops: MP restore
```
Nowhere is safe. Teaches Arcana use.

### Marionette
```
HP 38 · DMG 19 · DEF 9 · EXP 33 · Speed 1.0 · Inner Quarters
AI: teleporter, blinks BEHIND the player, 30-frame pause, then strikes
Drops: hearts, Potion
```

---

# Palette variants

Same sprite, different palette + stats. **Near-zero VRAM cost.**
18 sprites → ~35 distinct encounters.

| Base | Variants |
|---|---|
| Skeleton | Bone · Frozen (ice shots, Waterway) · Cursed (dark, +50% stats, Inner Quarters) |
| Bat | Cave · Blood (lifesteal) · Thunder (faster, Clock Tower) |
| Armor Knight | Iron · Flame (burn on contact) · Ice (freezes player briefly) |
| Zombie | Rotting · Drowned (Waterway) |
| Fleaman | Common · Crimson (faster, Observatory) |
| Bone Pillar | Bone · Obsidian (3-shot spread) |
| Fire Witch | Fire · Frost (freezing homing shots) |
| Ghost | Pale · Wraith (2× HP, Ceremonial) |

**Rule:** a variant must change *behaviour*, not just colour and numbers.
A palette swap that is only "same thing but tougher" is filler.

---

# The 10 bosses

Full attack patterns to be specified during M3–M4. Structure:

| # | Boss | Area | Reward | Notes |
|---|---|---|---|---|
| 1 | Bone Colossus | Catacombs | **Double** | Simple, teaches phase reading |
| 2 | The Countess *(1st)* | Machine Tower | **Kick Boots** | Taunts, flees at 50% HP |
| 3 | The Warden | Gallery | **Heavy Ring** | Grab-heavy, anti-roll |
| 4 | Choir of Ash | Chapel | **Roc Wing** | Multi-target, crowd pressure |
| 5 | TBD | Waterway | **Cleansing** | TBD |
| 6 | Tidewyrm | Clock Tower | **Tackle** | Arena hazard fight |
| 7 | The Countess *(2nd)* | Observatory | **Final Key** | Full fight, no escape |
| 8 | TBD | Inner Quarters | TBD | TBD |
| 9 | The Mentor *(corrupted)* | Ceremonial | *story* | Found corrupted, fought, freed — the corruption arc (SPEC.md section 7) |
| 10 | **The Count** | Ceremonial | *ending* | 3 phases |

**Slots #5 and #8 were both "The Rival"** (a two-part corrupted-then-freed
arc). SPEC.md moved that unlock to right after boss #1 (Catacombs) —
found trapped, freed on the spot, no fight — and gave the corruption arc
to the Mentor (#9) instead. #5 keeps Waterway's Cleansing reward; #8 has
none yet. Both need a real boss designed before M4b.

### Boss design rules

1. **Every attack has a tell.** Minimum 20 frames of visible windup.
2. **At least one attack cannot be rolled** — forces positioning.
3. **Phases change the pattern**, not just the numbers.
4. **Both characters must be viable.** No boss requires a specific one.
5. **No unavoidable damage.** Ever.
