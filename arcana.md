# data/arcana.md — The Arcana System

**10 Action cards × 10 Attribute cards = 100 combinations**

*(Renamed from CotM's "DSS" — the system name is Konami's. The card names
themselves are Roman gods and mythological creatures, all public domain.)*

---

## How it works

- **Action card** → the **effect type** and the **sprite**
- **Attribute card** → the **element**, the **palette**, and the **rule**

**You design 20 things. The 100 combos generate themselves.**

### Why this is affordable on GBA

The Attribute card is a palette swap plus a status rule. One Action sprite
rendered in ten palettes gives ten visually distinct spells at near-zero
VRAM cost. This is what makes 100 combos viable for a solo developer.

---

## The 10 Action cards

| Card | Effect type | Cost model | Base cost |
|---|---|---|---|
| Mercury | Weapon enhancer | per second | 4 MP/s |
| Venus | Stat modifier | per second | 3 MP/s |
| Jupiter | Protective shield | per second | 5 MP/s |
| Mars | Weapon transform | per use | 25 MP |
| Diana | Weapon projectile | per use | 15 MP |
| Apollo | Explosive / AOE | per use | 30 MP |
| Neptune | Recovery | per second | 6 MP/s |
| Saturn | Familiar | per second | 5 MP/s |
| Uranus | Summon | per use | 40 MP |
| Pluto | Special | per use | 20 MP |

**Hard rule:** no sustained effect may drain faster than **10 MP/sec**
(the regen rate). The player is never *forced* to switch off a buff.

## The 10 Attribute cards

| Card | Element | Rule applied |
|---|---|---|
| Salamander | Fire | Burn — damage over time |
| Serpent | Ice | Freeze / slow |
| Mandragora | Plant | Lifesteal |
| Golem | Earth | Higher damage, slower |
| Cockatrice | Stone | Petrify chance |
| Manticore | Poison | Poison — damage over time |
| Griffin | Wind | Faster, homing |
| Thunderbird | Lightning | Chains between enemies |
| Unicorn | Holy | Bonus vs undead, minor self-heal |
| Black Dog | Dark | Highest damage, small self-cost |

---

# The 100 combinations

---

## ☿ MERCURY — Weapon enhancer *(4 MP/s)*

Coats the active weapon in an element. Works on whip and twin blades alike.

| + Attribute | Effect |
|---|---|
| Salamander | **Flame weapon** — +20% dmg, burns for 3s |
| Serpent | **Frost weapon** — +10% dmg, slows enemies 50% for 2s |
| Mandragora | **Thorn weapon** — +10% dmg, heals 10% of damage dealt |
| Golem | **Stone weapon** — +40% dmg, −25% attack speed |
| Cockatrice | **Petrify weapon** — +5% dmg, 15% chance to petrify 2s |
| Manticore | **Venom weapon** — +10% dmg, poisons for 5s |
| Griffin | **Gale weapon** — +10% dmg, +30% attack speed |
| Thunderbird | **Storm weapon** — +15% dmg, arcs to 1 nearby enemy |
| Unicorn | **Sacred weapon** — +15% dmg, ×2 vs undead, heals 2 HP/hit |
| Black Dog | **Void weapon** — +50% dmg, costs 2 HP per hit |

---

## ♀ VENUS — Stat modifier *(3 MP/s)*

| + Attribute | Effect |
|---|---|
| Salamander | STR ×1.3 |
| Serpent | DEF ×1.3 |
| Mandragora | LCK ×1.4 |
| Golem | DEF ×1.5, movement speed −20% |
| Cockatrice | Immune to petrify · DEF ×1.2 |
| Manticore | Immune to poison · hearts gained ×2 |
| Griffin | INT ×1.3 · movement speed +15% |
| Thunderbird | STR ×1.2 · attack speed +20% |
| Unicorn | DEF ×2.0, STR ×0.5 — *the turtle build* |
| Black Dog | STR ×2.0, DEF ×0.5 — *the glass cannon* |

---

## ♃ JUPITER — Protective shield *(5 MP/s)*

Orbiting shields that block projectiles and damage on contact.

| + Attribute | Effect |
|---|---|
| Salamander | 2 fireballs orbit · burn on contact |
| Serpent | 4 ice shards orbit · freeze on contact |
| Mandragora | Heal 3 HP/s while standing still |
| Golem | I-frames ×2 after taking damage (80 frames) |
| Cockatrice | Reflects projectiles back at the shooter |
| Manticore | Poison cloud around player, 4 dmg/s to enemies in it |
| Griffin | Gust shield — deflects projectiles, +10% move speed |
| Thunderbird | **All damage taken −50%** — expensive, 9 MP/s |
| Unicorn | Blocks the next 3 hits entirely, then expires |
| Black Dog | Reflects 50% of damage taken back to the attacker |

---

## ♂ MARS — Weapon transform *(25 MP per use, 20s duration)*

Replaces the active weapon entirely for a fixed time.

| + Attribute | Effect |
|---|---|
| Salamander | **Flame sword** — wide arc, burns |
| Serpent | **Ice spear** — long reach, pierces, slows |
| Mandragora | **Vine flail** — hits 3 times, lifesteal |
| Golem | **Stone hammer** — huge dmg, very slow, breaks cracked walls |
| Cockatrice | **Gorgon claws** — fast triple hit, petrify chance |
| Manticore | **Venom scythe** — wide arc, poisons |
| Griffin | **Wind glaive** — fastest attack in the game, low dmg |
| Thunderbird | **Storm lance** — pierces, chains to 2 enemies |
| Unicorn | **Sacred blade** — ×3 vs undead, heals on kill |
| Black Dog | **Void edge** — highest dmg in game, drains 2 MP/s extra |

---

## ⚸ DIANA — Weapon projectile *(15 MP per use)*

Fires a projectile along the weapon's attack path.

| + Attribute | Effect |
|---|---|
| Salamander | Fireball — travels straight, explodes small |
| Serpent | Ice shard ×3 — spread, slows |
| Mandragora | Seed shot — sprouts a damaging vine where it lands |
| Golem | Boulder — slow, heavy, breaks cracked walls |
| Cockatrice | Stone gaze — narrow beam, high petrify chance |
| Manticore | Venom spit ×2 — leaves poison pools |
| Griffin | Wind blade — **homing**, pierces |
| Thunderbird | Lightning bolt — instant travel, chains to 3 |
| Unicorn | Light lance — pierces everything in a line |
| Black Dog | Shadow bolt — highest dmg, costs 5 HP |

---

## ☀ APOLLO — Explosive / AOE *(30 MP per use)*

Screen-affecting or area attacks. The "panic button" category.

| + Attribute | Effect |
|---|---|
| Salamander | Fire nova — radius 80px, burns all |
| Serpent | Frost burst — radius 100px, freezes all 3s |
| Mandragora | Life bloom — radius 60px, heals 25% of dmg dealt |
| Golem | Earthquake — screen-wide, ground enemies only, heavy dmg |
| Cockatrice | Gaze pulse — radius 70px, petrifies all 2s |
| Manticore | Toxic cloud — radius 90px, lingers 5s |
| Griffin | Cyclone — pulls enemies toward player, then damages |
| Thunderbird | Chain storm — hits every enemy on screen once |
| Unicorn | Holy light — screen-wide, ×3 vs undead, heals 20 HP |
| Black Dog | Void collapse — highest AOE dmg, costs 15 HP |

---

## ♆ NEPTUNE — Recovery *(6 MP/s)*

| + Attribute | Effect |
|---|---|
| Salamander | Cure burn · fire resistance +50% |
| Serpent | Cure freeze · ice resistance +50% |
| Mandragora | **Regenerate 4 HP/s** |
| Golem | Slowly restores hearts (1/s) |
| Cockatrice | Cure petrify · petrify immunity |
| Manticore | Cure poison · poison immunity |
| Griffin | Restores 2 MP/s extra *(net −4 MP/s)* |
| Thunderbird | Cure all status ailments instantly, then expires |
| Unicorn | **Regenerate 8 HP/s** — expensive, 10 MP/s |
| Black Dog | Converts hearts into HP, 1 heart = 3 HP |

---

## ♄ SATURN — Familiar *(5 MP/s)*

A companion that follows the player and attacks independently.

| + Attribute | Effect |
|---|---|
| Salamander | Fire imp — melee, burns |
| Serpent | Ice sprite — fires slowing shots |
| Mandragora | Sprout — heals player 2 HP/s instead of attacking |
| Golem | Stone guardian — slow, blocks projectiles with its body |
| Cockatrice | Basilisk chick — petrifying gaze on nearest enemy |
| Manticore | Wasp — fast, poisons |
| Griffin | Hawk — fastest familiar, dives at enemies |
| Thunderbird | Storm wisp — chains lightning between enemies |
| Unicorn | Foal — ×2 dmg vs undead, heals 1 HP/s |
| Black Dog | Shade — highest dmg familiar, drains 1 HP/s |

---

## ♅ URANUS — Summon *(40 MP per use)*

A single powerful one-shot summon. Boss-fight tools.

| + Attribute | Effect |
|---|---|
| Salamander | Salamander erupts — column of fire, heavy dmg |
| Serpent | Leviathan — sweeps the screen horizontally, freezes |
| Mandragora | Great root — erupts under all enemies, lifesteal |
| Golem | Colossus — slams down, screen-wide, breaks walls |
| Cockatrice | Basilisk — petrifies every enemy on screen 4s |
| Manticore | Manticore — sweeping poison breath |
| Griffin | Griffin — dives across screen, pulls and damages |
| Thunderbird | **Thunderbird — strongest single summon in the game** |
| Unicorn | Unicorn — heals to full, damages all undead heavily |
| Black Dog | Cerberus — 3 bites, highest raw dmg, costs 20 HP |

---

## ♇ PLUTO — Special *(20 MP per use)*

The rule-breakers. Deliberately strange.

| + Attribute | Effect |
|---|---|
| Salamander | **Item crash** — consumes all hearts for damage scaled to the amount |
| Serpent | Freezes *time* for 3 seconds — enemies and projectiles halt |
| Mandragora | Converts all MP into HP at 1:1 |
| Golem | +100% DEF, but movement locked for 5s |
| Cockatrice | Petrify **self** — immune to all damage 4s, cannot act |
| Manticore | Poisons every enemy on screen |
| Griffin | **Dodge costs 0 MP for 15s** |
| Thunderbird | Doubles Arcana effects for 10s, doubles MP drain |
| Unicorn | **Revive** — survive the next fatal hit at 1 HP |
| Black Dog | **Subweapons cost MP instead of hearts for 30s** |

---

# Balance rules

1. **No combo may trivialise a boss.** If one does, fix the combo, not the boss.
2. **Weak combos are acceptable.** Systematically generated means weak
   combos are *understandable*, not random filler.
3. **Sustained drain never exceeds 10 MP/s.**
4. **Every Attribute must have at least 3 genuinely useful combos**, or its
   rule is too narrow.
5. **No combo may fully replace dodging.** Pluto+Cockatrice and
   Pluto+Griffin come close — watch them in testing.

---

# Card placement

Cards are **placed in the world**, never dropped. This is the single
biggest fix to CotM's design.

| Cards | Where |
|---|---|
| 8 | Chests along the critical path |
| 6 | Boss rewards |
| 4 | Secret rooms (Tackle / Heavy Ring gated) |
| 2 | Character-gated rooms |

**Rules:**
- Cards ascend roughly in power by area — no Uranus in the Entrance Hall
- **Every card is reachable before the final boss**
- **No card gates the critical path** — Arcana is power, never a key
- The map marks rooms containing uncollected cards once the area is explored

---

# Menu presentation

The 10×10 grid is **fully visible from the start** — all 100 effects
described, greyed out until both cards are owned.

**Why:** it turns hunting a card into a *goal* instead of a slot machine.
The player can plan a build from hour one. This is the second biggest fix
to CotM's version.

**Swapping is instant** — shoulder buttons, no pause, mid-combat.
Menu-swapping means nobody swaps.
