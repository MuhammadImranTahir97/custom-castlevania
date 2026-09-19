# ROADMAP.md — Build Order

**Total estimate: ~11 months, part-time.** Honest, not padded.

---

## The principle

> **Build in the order that lets you quit early with no regret.**

Each milestone answers one question. If the answer is bad, you stop having
lost weeks instead of a year.

**Do not skip milestones to get to content faster.** The checkpoints are
the whole point.

---

## M0 — Setup *(~3 days)*

- [x] Install Wonderful Toolchain (GBA target) — used instead of devkitPro/devkitARM
- [x] Install Butano, build one of its example projects
- [x] Install mGBA
- [ ] Install Tiled (map editor)
- [x] Create repo, add `.gitignore` (`*.gba`, `*.sav`, `build/`)
- [x] Create `src/game/` and `src/platform/` folders
- [x] Get a red square moving on screen with the d-pad

**Done when:** you can change a line of C++, run `make`, and see the
result in mGBA within 30 seconds.

---

## M1 — Does it feel good? *(~2 weeks)*

The most important milestone in the project.

- [ ] 1 room, hand-built, no room loader yet
- [ ] Hunter only — run, jump, land
- [ ] Variable jump height (tap vs hold)
- [ ] Coyote time (5 frames)
- [ ] Input buffering (6 frames)
- [ ] Full air control
- [ ] Whip attack with hitbox
- [ ] Attack cancel out of recovery
- [ ] Dodge roll — 18 frames, i-frames 4–14
- [ ] 1 enemy: Skeleton (walker AI)
- [ ] Hitstop (3 frames on impact)
- [ ] Screen shake
- [ ] Grey boxes for every sprite

### 🔴 CHECKPOINT

**Is moving and hitting things fun, with no art, no music, no progression?**

If no → **fix the feel.** Do not proceed. Nothing built later fixes a
game that feels bad to control.

Expect to spend a week just tuning numbers here. That is correct.

---

## M2 — Is it a game? *(~1 month)*

- [x] Room loader — rooms as data, not code
- [ ] 10 rooms, connected, with transitions (2 so far, proving the loader)
- [x] Tiled → GBA converter in `tools/` (reads an interim JSON schema until Tiled is installed — see `assets/rooms/README.md`)
- [ ] 3 enemy types (walker, flyer, shooter)
- [ ] Enemy HP, damage, death
- [ ] Player HP, damage, i-frames
- [ ] EXP and level-up
- [ ] Basic HUD (HP, MP bars)
- [ ] **The Rival** — second character, instant swap
- [ ] Both movesets distinct and working

### 🔴 CHECKPOINT

**Does exploring and fighting hold up for 10 straight minutes?**

Also: **does the character swap actually matter**, or is one obviously
better? If one dominates, fix the balance now — it only gets harder later.

---

## M3 — Is it worth finishing? *(~2 months)*

- [ ] One complete area: **Catacombs**, all 18 rooms
- [ ] 6 enemy types placed and balanced
- [ ] **First boss** with real attack patterns and phases
- [ ] Save system (SRAM, 3 slots)
- [ ] Save rooms restoring HP/MP
- [ ] Pause menu — all 5 tabs
- [ ] Map screen with sealed-door markers
- [ ] **Arcana working** — 2 Action × 3 Attribute cards (6 combos)
- [ ] First relic (Dash Boots) and one real gate it opens
- [ ] Death and respawn

### 🔴 CHECKPOINT — the honest one

**Would you play this if someone else had made it?**

This is the last cheap exit. If M3 is not fun, **more content will not
save it.** Stop, or redesign, or restart. Do not push to M4 hoping volume
fixes it.

If M3 *is* fun — you have a real game and the rest is execution.

---

## M4 — Content *(~6 months)*

The long stretch. Mostly repetition of things already proven to work.

### M4a — Areas *(~3 months)*
- [ ] Areas 1, 3, 4, 5 built (Entrance, Machine Tower, Gallery, Chapel)
- [ ] Areas 6, 7, 8 built (Waterway, Clock Tower, Observatory)
- [ ] Areas 9, 10 built (Inner Quarters, Ceremonial Room)
- [ ] All 150 rooms placed and connected
- [ ] 3 warp rooms

### M4b — Systems *(~2 months)*
- [ ] All 18 enemy types
- [ ] Palette variants (~35 total encounters)
- [ ] All 10 bosses
- [ ] All 8 relics and their gates
- [ ] All 100 Arcana combos
- [ ] ~8 character-gated rooms
- [ ] All items, equipment, vessels

### M4c — Content *(~1 month)*
- [ ] Story — all ~50 text boxes
- [ ] 15 lore notes placed
- [ ] CC0 sprite pack swapped in for grey boxes
- [ ] CC0 / custom music — 14 tracks
- [ ] ~40 sound effects
- [ ] Title screen, ending

---

## M5 — Ship *(~2 months)*

- [ ] Full balance pass — every area, every boss
- [ ] Bug fixing
- [ ] **Test on real GBA hardware** (flashcart)
- [ ] Test on GBA SP, and on a non-backlit GBA — check the palettes
- [ ] Difficulty options (Easy / Normal / Hard via `difficulty.h`)
- [ ] Music ducking, boss fade-in, ambient silence passes
- [ ] Playtesting with people who are not you
- [ ] Release

---

## After v1

Only after shipping:

- Battle Arena (20 rooms)
- Extra modes (Magician, Fighter, Shooter, Thief)
- New Game+
- Boss rush
- **PSP port** — this is where the `game/` ↔ `platform/` split pays off
- Original art replacing CC0
- Original soundtrack
- Expansion toward 250 rooms

---

## Progress tracking

| Milestone | Target | Status |
|---|---|---|
| M0 — Setup | 3 days | 🟨 (Tiled install left) |
| M1 — Feel | 2 weeks | ⬜ |
| M2 — Game | 1 month | ⬜ |
| M3 — Vertical slice | 2 months | ⬜ |
| M4a — Areas | 3 months | ⬜ |
| M4b — Systems | 2 months | ⬜ |
| M4c — Content | 1 month | ⬜ |
| M5 — Ship | 2 months | ⬜ |

---

## If you fall behind

**Cut content, never cut polish.**

A 100-room game that feels great beats a 150-room game that feels rough.
Areas are the first thing to cut — the design tolerates it. Feel is the
last thing to cut, because it cannot be added back later.
