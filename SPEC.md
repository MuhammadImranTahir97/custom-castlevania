# SPEC.md — Game Design Specification

**Working title:** *(TBD)*
**Platform:** Game Boy Advance
**Engine:** Butano (C++) on devkitARM
**Genre:** Metroidvania / action-exploration
**Target length:** ~5 hours
**Status:** Design complete. Not yet in development.

---

## 0. What this game is

A gothic vampire-hunting Metroidvania for real GBA hardware, inspired by
*Castlevania: Circle of the Moon* but built from original characters and
original content.

**Design goals, in priority order:**

1. **Feel** — movement and combat must feel good within the first 10 seconds
2. **No grinding** — progression comes from exploration, never from RNG
3. **Two playstyles** — the character swap is the core hook
4. **Finishable** — scope is deliberately constrained

### Legal boundary

This game uses **no Konami intellectual property**.

| Free to use | Not used |
|---|---|
| Dracula (Bram Stoker, public domain) | Nathan Graves, Belmonts, any CotM character |
| Vampires, whips, gothic castles | Vampire Killer, named Konami weapons |
| Roman gods, mythological creatures | "DSS" as a branded system name |
| The Metroidvania genre entirely | Konami sprites, music, tilesets, room layouts |

The magic system is renamed the **Arcana System** for this reason.
All sprites, tilesets and music must be original or CC0-licensed.

---

## 1. Architecture

### The portability rule

This is the single most important structural decision. A PSP port is
planned for after v1 ships, and it is only cheap if the split below is
maintained from day one.

```
src/
├── game/          ← NO Butano. Pure, portable C++.
│   ├── player.cpp       movement, stats, jump physics
│   ├── enemy.cpp        AI, HP, damage
│   ├── arcana.cpp       card combos
│   ├── level.cpp        room data, collision
│   └── save.cpp         save data structure
│
└── platform/      ← ALL Butano lives here
    ├── gba_render.cpp
    ├── gba_input.cpp
    ├── gba_audio.cpp
    └── gba_save.cpp
```

**Hard rule:** no file under `src/game/` may include a Butano header.

Game logic describes state (*"player at x=40, y=88, frame 3, facing left"*).
The platform layer turns that into pixels. Input arrives as an abstract
button struct, not as Butano key reads.

**Cost of doing this now:** almost nothing.
**Cost of retrofitting later:** a full rewrite.

### Hardware budget

| Resource | Limit | Notes |
|---|---|---|
| Screen | 240 × 160 | Every HUD pixel costs game visibility |
| CPU | ~16.8 MHz | Keep per-frame work small |
| IWRAM | 32 KB | Hot data only |
| EWRAM | 256 KB | Everything else |
| VRAM | 96 KB | Sprite and tile budget — the real constraint |
| Sprites on screen | 128 OAM entries | Watch enemy + projectile counts |
| Cartridge | 8 MB target | Comfortable; CotM was 8 MB |

---

## 2. Movement & combat feel

This is the foundation. Everything else sits on it. All values below live
in `difficulty.h` and are tunable.

### Movement

| Feature | Spec |
|---|---|
| Base run speed | Fast from the first frame — **no dash unlock required** |
| Variable jump height | Tap = short hop, hold = full arc |
| Coyote time | 5 frames of grace after leaving a ledge |
| Input buffering | 6 frames — a jump pressed early still fires on landing |
| Air control | Full horizontal steering mid-jump |

### Combat

| Feature | Spec |
|---|---|
| Whip/blade cancel | Can move or jump out of attack recovery frames |
| Hitstop | 3-frame freeze on impact |
| Screen shake | On heavy hits and boss attacks |
| Attack while airborne | Allowed |
| Knockback | Short, and **never into a pit** |
| Invulnerability frames | 40 frames after taking damage |

### The dodge roll

| Property | Value |
|---|---|
| Distance | ~40 px (Hunter) / ~50 px dash (Rival) |
| Duration | 18 frames |
| **I-frames** | Frames 4–14 |
| **MP cost** | **10 MP** |
| **Regen pause after use** | 60 frames |
| Cancels into attack | After frame 14 |
| In the air | ❌ Not allowed |
| Through enemies | ❌ Not allowed |
| Under projectiles | ✅ Some |

**Low-MP behaviour:** below 10 MP the roll still executes as pure movement
with **no i-frames**. The player is never helpless — they only lose the
safety net. This teaches the resource without a tutorial.

**Why the regen pause matters:** at 10 MP the cost barely limits anything.
The 60-frame regen freeze is what actually prevents roll-spam. If
playtesting shows spam, **raise the pause, not the cost.**

### Difficulty

Moderate by default. Every tunable lives in one file:

```c
// difficulty.h
enemy_damage_multiplier
enemy_hp_multiplier
player_iframe_duration
dodge_mp_cost
dodge_regen_pause
exp_multiplier
heart_drop_rate
```

Changing four numbers produces Easy / Normal / Hard.

---

## 3. The two characters

Character swap replaces weapon swapping entirely. **Do not implement both.**

| | **The Hunter** | **The Rival** |
|---|---|---|
| Weapon | Whip | Twin blades |
| Reach | Long | Short |
| Attack speed | Medium | Fast (3-hit combo) |
| Damage per hit | Medium | Low |
| Run speed | Standard | Faster |
| Jump | Standard arc | Higher, floatier |
| Dodge | 40 px roll | 50 px dash, shorter i-frames |
| Strong against | Groups, ranged enemies | Single tough enemies |
| Weak against | Fast swarms | Anything needing distance |

**Shared between them:** HP, MP, hearts, all 7 stats, all Arcana cards,
all relics, all subweapons.

**Separate:** moveset, jump arc, attack hitbox, dodge style, sprite set.

**Swapping:** instant, shoulder button, allowed mid-air, zero cost.

Shared stats are deliberate — swapping is about *situation*, never about
managing two separate characters.

### Balance safeguards

1. **No dominant pick.** Hunter's reach is worthless in tight corridors.
   Rival's speed is worthless against a shooter across a gap.
2. **Anti-roll enemies split the difference** — Armor Knight's long charge
   favours Hunter's reach; Were-panther's 3-hit combo favours Rival's
   escape speed.
3. **Some Arcana favour one** — Mercury (whip enhancer) is Hunter-oriented;
   Diana (projectiles) scales with Rival's faster attack rate.

### Character gates

~8 rooms in the whole castle require a specific character:

| Gate | Requires |
|---|---|
| High ledge, no platform | Rival's higher jump |
| Switch across a wide gap | Hunter's whip reach |
| Narrow shaft, tight timing | Rival's speed |
| Cracked wall above a pit | Hunter's reach from distance |

Keep this light. Most gating stays with relics — this is seasoning.

---

## 4. The Arcana System

Two card types. Equip one of each. The pair produces an effect.

- **Action card (10)** — determines the **type of effect** and the sprite
- **Attribute card (10)** — determines the **element**, the palette, and the
  rule applied to the effect

**10 × 10 = 100 combinations.**

### Why this is affordable on GBA

The Attribute card is essentially a **palette swap plus a status effect**.
One Action sprite rendered in ten different palettes produces ten visually
distinct spells at almost zero VRAM cost. This is the single most
GBA-appropriate trick available and it is what makes 100 combos viable for
a solo developer.

**You design 20 things. The 100 combos generate themselves.**

### Action cards — effect types

| Card | Effect type | Cost model |
|---|---|---|
| Mercury | Weapon enhancer | Per second |
| Venus | Stat modifier | Per second |
| Jupiter | Protective shield | Per second |
| Mars | Weapon transform | Per use |
| Diana | Weapon projectiles | Per use |
| Apollo | Explosives / AOE | Per use |
| Neptune | Recovery | Per second |
| Saturn | Familiar | Per second |
| Uranus | Summon | Per use |
| Pluto | Special | Per use |

### Attribute cards — elements

| Card | Element | Rule applied |
|---|---|---|
| Salamander | Fire | Burn damage over time |
| Serpent | Ice | Freeze / slow |
| Mandragora | Plant | Lifesteal |
| Golem | Earth | Higher damage, slower |
| Cockatrice | Stone | Petrify chance |
| Manticore | Poison | Poison damage over time |
| Griffin | Wind | Faster, homing |
| Thunderbird | Lightning | Chains between enemies |
| Unicorn | Holy | Bonus vs undead, minor heal |
| Black Dog | Dark | Highest damage, self-cost |

Full 100-combo table: see `data/arcana.md`

### Fixes to CotM's version

| CotM problem | This game's fix |
|---|---|
| Cards dropped at ~1% from specific enemies | **Cards are placed in the world.** Chests, secret rooms, boss rewards. Never RNG |
| Effects hidden until first use | **All 100 effects visible in the menu from the start**, greyed out until owned |
| Menu pause to swap | **Instant swap**, shoulder buttons, mid-combat |
| Motion inputs on 3 cards | **Single button for everything** |
| Most combos were filler | Systematic generation means every combo is *understandable*, even when weak |

### Making big spells feel big without motion inputs

- Longer cast animation (character braces, then releases) — free, just frames
- Screen flash on activation — ~10 lines
- Larger hitstop on impact — already built
- Higher MP cost as the real limiter — just a number

### MP economy

| | Value |
|---|---|
| Starting MP | 100 |
| Regen | 1 MP per 6 frames (~10/sec) |
| Dodge cost | 10 MP |
| Sustained Arcana drain | 2–8 MP/sec |
| Per-use Arcana cost | 15–40 MP |

**Rule:** no sustained effect may ever drain faster than 10 MP/sec. The
player should never be *forced* to switch off a buff — but strong buffs
should mean rarely sitting on a full bar.

**The core tension:** MP fuels both the dodge and the Arcana. Keeping a
buff running means possibly not having 10 MP when a dodge is needed. That
decision, made constantly, is the heart of the combat.

---

## 5. Stats, EXP & items

### The seven stats

| Stat | Effect | Start | Per level |
|---|---|---|---|
| HP | Health | 100 | +8 |
| MP | Arcana + dodge fuel | 100 | +5 |
| HEARTS | Subweapon fuel | 50 | — |
| STR | Damage dealt | 10 | +2 |
| DEF | Damage reduction | 10 | +2 |
| INT | Arcana spell power | 10 | +2 |
| LCK | Drop rate + crit chance | 10 | +2 |

**100 MP = exactly 10 dodges.** Deliberate — easy for players to reason about.

**LCK is honest.** In CotM it secretly gated card drops, which felt terrible.
Here it only improves loot and crit, and it is displayed plainly.

### LCK curve

The stat grows at full rate; the **effect** is capped, not the growth.

| LCK | Crit chance | Drop rate bonus |
|---|---|---|
| 10 (start) | 5% | +0% |
| 30 | 15% | +30% |
| 50 | 22% | +50% |
| 70 | 27% | +65% |
| **Cap** | **30%** | **+75%** |

### EXP curve

```
EXP required for next level = level² × 8
```

| Level | Needed | Running total |
|---|---|---|
| 1→2 | 8 | 8 |
| 5→6 | 200 | ~500 |
| 10→11 | 800 | ~3,500 |
| 20→21 | 3,200 | ~25,000 |

Early levels arrive fast (progress felt in the first 10 minutes); later
levels slow down (exploration beats grinding). **Level ~25 is endgame.**

### Anti-grind soft cap

| Enemy level vs player | EXP awarded |
|---|---|
| Equal or higher | 100% |
| 1–4 below | 100% → 50% (sliding) |
| 5+ below | 25% |
| — | **Never zero** |

Grinding remains *possible* for players who are stuck. It simply stops
being *efficient*.

### Items

| Category | Contents |
|---|---|
| **Consumables** | Potion, High Potion, Antidote, Mind Restore |
| **Equipment** | Armor + 2 accessory slots |
| **Permanent upgrades** | Heart Container, MP Vessel, HP Vessel |
| **Key items** | Relics, Arcana cards, final key |

Only 3 equipment slots. More slots means more menu, and menus are not fun.

### Drop rules

| Source | Drops |
|---|---|
| Candles / lamps | Hearts, small HP |
| Enemies | Hearts, occasional consumable |
| Chests | **Guaranteed** — equipment, vessels, cards |
| Bosses | Relic + guaranteed reward |

**The rule that matters:** anything progression-relevant is in a chest,
never a random drop. Cards, relics and vessels are always placed.

---

## 6. The castle

**150 rooms, 10 areas.** Full room specs: see `data/rooms.md`

| # | Area | Rooms | Theme |
|---|---|---|---|
| 1 | Entrance Hall | 12 | Tutorial, no gates |
| 2 | Catacombs | 18 | Dark, cramped, undead |
| 3 | Machine Tower | 17 | Vertical, moving platforms |
| 4 | Underground Gallery | 15 | Wide, statues, art |
| 5 | Chapel | 17 | Tall, holy, stained glass |
| 6 | Waterway | 18 | Water physics, swimming |
| 7 | Clock Tower | 15 | Gears, timing hazards |
| 8 | Observatory | 17 | Wind, highest point |
| 9 | Inner Quarters | 12 | The Count's halls |
| 10 | Ceremonial Room | 9 | Final gauntlet |

Each area has **its own tileset and palette**. On a 240×160 screen, that
is what makes areas feel genuinely different.

### Room budget

| Purpose | Rooms |
|---|---|
| Critical path | 75 |
| Optional / backtrack rewards | 45 |
| Save & warp rooms | 12 |
| Boss arenas | 10 |
| Secret rooms | 8 |

**30% optional** — enough to reward exploration without doubling the work.

### The eight relics

| # | Relic | Ability | Found |
|---|---|---|---|
| 1 | Dash Boots | Dash across wide gaps | Entrance Hall |
| 2 | Double | Double jump | Catacombs boss |
| 3 | Kick Boots | Wall-jump narrow shafts | Machine Tower boss |
| 4 | Heavy Ring | Push heavy blocks | Gallery boss |
| 5 | Roc Wing | High jump | Chapel boss |
| 6 | Cleansing | Survive poisoned water | Waterway boss |
| 7 | Tackle | Break cracked walls | Clock Tower boss |
| 8 | Final Key | Opens the Ceremonial Room | Observatory boss |

**Dash Boots come first, not late.** Movement must feel good from minute one.

### Critical path

```
Entrance ─▶ Catacombs ─▶ Machine Tower ─▶ Gallery ─▶ Chapel
                                                        │
Ceremonial ◀─ Inner Quarters ◀─ Observatory ◀─ Clock ◀─ Waterway
```

Linear spine, with locked side-rooms in every area to return to.

**3 warp rooms.** Too many and the castle stops feeling like a place; too
few and backtracking becomes a chore.

### The rule that keeps it honest

> **Never gate the critical path behind a secret.**

If a player must find a hidden wall to progress, they get stuck and quit.
Secrets reward — vessels, cards, good equipment. Secrets never block.

---

## 7. Story & characters

### Cast

| Role | Character | Name |
|---|---|---|
| Player 1 | A young hunter, first real mission | *TBD* |
| Player 2 | Fellow apprentice, corrupted then freed | *TBD* |
| Goal | The mentor, captured in the opening | *TBD* |
| Mid-boss | The Countess — taunts, vanishes, returns | *TBD* |
| Final boss | The Count | *TBD* |

Five characters. That is all a GBA game needs.
**Names are deliberately left blank — they should be the developer's choice.**

### Shape

1. Two apprentices and their mentor confront the Count
2. The ritual succeeds; the castle rises
3. The mentor is captured; the apprentices are separated and fall
4. The Hunter climbs back up alone
5. The Rival is found corrupted, fought, then freed — **and becomes playable**
6. Final confrontation; the castle falls

The Rival's arc is a *gameplay* unlock, not just a cutscene. That is the
strongest story beat in the design.

### Story budget

| Moment | Text boxes |
|---|---|
| Opening | 8 |
| Rival encounters (×3) | 15 |
| Countess encounters (×2) | 8 |
| Pre-final-boss | 6 |
| Ending | 10 |
| **Total** | **~50** |

That is the whole game. CotM was roughly this sparse and nobody complained.

**Why so little:** Metroidvania story lives in the environment — a ruined
chapel, a flooded waterway, a room of coffins. Text boxes interrupt play;
rooms do not.

### Lore notes

**~15 readable items** scattered across the castle — a diary page, a
servant's note, a carved inscription.

Costs almost nothing (text only), fully optional, rewards explorers, and
allows worldbuilding without cutscenes.

---

## 8. Audio

GBA music is **tracker music**, not audio files. Samples plus patterns.
A 3-minute song is ~40 KB instead of 4 MB. Handled by **Maxmod** via Butano.
Formats: `.mod`, `.xm`, `.it`

### Channel budget

GBA provides 8 usable channels.

| Channels | Use |
|---|---|
| 5–6 | Music |
| 2–3 | **Reserved for SFX** |

If music consumes all 8, the whip crack cuts off the bassline. Music must
be written with headroom.

### Soundtrack — 14 tracks

| Track | Where | Feel |
|---|---|---|
| Title | Menu | Restrained, gothic |
| Prologue | Opening | Ominous |
| Entrance Hall | Area 1 | The main theme |
| Catacombs | Area 2 | Low, sparse, dread |
| Machine Tower | Area 3 | Mechanical, driving |
| Gallery | Area 4 | Eerie, elegant |
| Chapel | Area 5 | Organ, choral |
| Waterway | Area 6 | Echoing, liquid |
| Clock Tower | Area 7 | Ticking rhythm |
| Observatory | Area 8 | Vast, windswept |
| Boss | All bosses | Urgent |
| Rival battle | Rival fights | Emotional — the story track |
| Final boss | The Count | Everything at once |
| Ending | Credits | Resolution |

Plus 3 jingles (1–3 sec): item get, relic get, game over.

### Budget

| Asset | Size |
|---|---|
| 14 tracks + 3 jingles | ~600 KB |
| ~40 sound effects | ~80 KB |
| **Total** | **~680 KB** |

### Sound effects (~40)

Hunter: whip crack, whip hit, roll ·
Rival: blade slash ×3, blade hit, dash ·
Shared: jump, land, hurt, death, heal ·
Subweapons: dagger, axe, cross, holy water ·
Arcana: cast, buff on, buff off, ×8 elements ·
World: chest, door, candle break, save, switch, secret found ·
Enemies: hit, death, alert · UI: cursor, confirm, cancel

### Three cheap tricks worth doing

| Trick | Effect |
|---|---|
| **Music ducking** | Drop music volume 30% for 10 frames on a big hit — makes impacts feel enormous |
| **Boss music fade-in** | Music shifts *before* the boss appears — dread, for free |
| **Ambient silence** | One or two areas with no music, just dripping water — contrast makes everything else louder |

### Sourcing

CC0 tracker module packs first. Custom composition (OpenMPT) later.
**Never rip CotM's tracks — Konami owns them.**

**Audio is the last thing built.** Silence or a single looping placeholder
track is fine for the first eight months.

---

## 9. Saves & UI

### Save system

| | |
|---|---|
| Storage | **SRAM** (Butano-supported, universal emulator + real cart support) |
| Slots | 3 |
| Size per slot | ~2 KB |
| Save points | 12 rooms — one per area plus extras |
| Auto-save | **No.** Manual only |
| Saved data | Stats, inventory, cards, relics, map explored, room ID, playtime |

**Save rooms restore HP and MP fully.** They are checkpoints and rest stops.

### Death

| | |
|---|---|
| On death | Respawn at last save room |
| Lost | Progress since last save |
| **Kept** | **All permanent items** — cards, relics, vessels, map |

No permadeath, no lost items. Harsh deaths make people quit.

### Pause menu — 5 tabs, shoulder buttons cycle

| Tab | Contents |
|---|---|
| Status | 7 stats, level, EXP bar, playtime |
| Arcana | The 10×10 grid — all effects visible from the start |
| Items | Consumables, equip armor + 2 accessories |
| Map | Explored rooms, save points, warps, sealed doors |
| Options | Difficulty, sound, button config |

### HUD — minimal

```
┌────────────────────────────┐
│ HP ████████░░   ♥ 34       │
│ MP ██████░░░░   [◆]        │
│                            │
│           game             │
│                            │
└────────────────────────────┘
```

- HP bar, MP bar, heart count
- Active Arcana icon — only when an effect is running
- Active character indicator — small, corner
- **Nothing else.** No on-screen minimap, no EXP bar, no timer

On a 240×160 screen, every HUD pixel is a pixel of game the player cannot see.

### Map screen

In a 150-room castle the map **is** the navigation.

- Explored rooms filled in
- **Sealed doors marked** — so the player can see where they could not go
- Save points and warps flagged
- Current room blinking
- Room completion %

**"Sealed doors marked" is the single best quality-of-life feature in a
Metroidvania.** CotM did it poorly. This game should do it well.

---

## 10. Scope

### In v1

150 rooms across 10 areas · 2 playable characters · 100 Arcana combos ·
18 enemy types plus palette variants · 10 bosses · 8 relics · full story ·
15 lore notes · CC0 art and music

### Not in v1

Battle Arena · extra modes (Magician, Fighter, Shooter, Thief) ·
New Game+ · boss rush · PSP port · original art · original soundtrack

**Everything in the second list is easier to add after shipping than before.**
New ideas during development go into `v2.md` — **never into the build.**

### Timeline

**~11 months** at steady part-time pace. Honest, not padded.

Full milestone breakdown: see `ROADMAP.md`

### The three things most likely to kill this project

| Risk | Mitigation |
|---|---|
| **Scope creep** | The "not in v1" list is locked. New ideas go to `v2.md` |
| **Art paralysis** | Grey boxes are fine for 8 months. Do not stop to draw |
| **Skipping milestones** | M1–M3 exist so the project can be abandoned cheaply. Do not skip to M4 |

---

## Open decisions

These are deliberately unresolved and belong to the developer:

- [ ] Game title
- [ ] All five character names
- [ ] Art direction (once past grey-box)
- [ ] Which CC0 sprite pack
- [ ] Whether to keep 150 rooms or expand toward 250 after v1
