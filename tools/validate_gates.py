#!/usr/bin/env python3
"""Validates every declared gate in assets/rooms/*.tmj against the real
jump physics, for every character x relic combination.

A "gate" is a door (see convert_rooms.py's room schema) with a "requires"
property -- a '+'-separated list of relic and/or character names (e.g.
"double", "rival") declaring what the room author *intended* the gate to
need. This script doesn't trust that intent; it independently simulates
whether it actually holds, the same way a human would have to reason
through it by hand (which is exactly how catacombs_05's gate was designed
and re-designed twice this project already -- see rooms.md and
troubleshooting/build-environment.md). Two characters and (eventually) 8
relics is 2 x 2^8 = 512 combinations once every relic exists; checking
that by hand every time a gate's geometry changes is exactly the kind of
mistake this project has already made more than once.

What it checks, per gate: find the platform directly beneath the door
(the room's actual launch point for it), compute the vertical rise
required to reach the door from there, then for every (character, owned
relics) combination compare its best-case achievable rise against that
requirement:
  - A combination the "requires" declaration says SHOULD pass, but whose
    best case still falls short, means the gate is impossible even for
    the intended combination -- reported as FAIL.
  - A combination "requires" says should NOT pass, but whose best case
    clears it anyway, means the gate can be bypassed -- reported as FAIL.
"Best case" for a double-jump-eligible combination means a skilled,
well-timed second press; there's no meaningful "worst-case timing" to
also check here, because a badly-timed second press just degrades
toward a plain single jump (always available anyway, timing or not) --
there's no floor below that worth calling a distinct case.

Jump-height-relevant relics are tracked in JUMP_RELICS below. Currently
just Double (the only one implemented in src/game/player.cpp) -- extend
that list, not this script's logic, as more are built (SPEC.md's Roc
Wing is "High jump" and will need an entry here once it exists in code).
Non-jump relics (Dash Boots' horizontal dash, Kick Boots' wall-jump, ...)
aren't modeled -- this script is jump-height gates only, matching what
currently exists in the actual game.

The Rival isn't a character that exists yet, from the player's own
perspective, before she's found (SPEC.md section 7's Shape step 4,
rooms.md's gate-design rule #7) -- PRE_RIVAL_UNLOCK_ROOMS below excludes
Rival-character combinations for gates in those rooms, so a gate that
would only be bypassable by a character the player can't have yet
correctly doesn't get flagged.

Usage:
    validate_gates.py --rooms-dir assets/rooms
"""
import argparse
import glob
import itertools
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import convert_rooms  # reuses its Tiled-parsing (load_room) -- see its own docstring

# --- Physics constants, mirrored exactly from src/game/difficulty.h's
# Q24.8 fixed-point values (src/game/fixed.h) so the simulation below
# takes the identical path the real game code does, not an approximation. ---
FIXED_ONE = 256


def to_fixed(pixels):
    return int(round(pixels * FIXED_ONE))


PLAYER_HALF_HEIGHT = 8  # src/game/player.h's player_half_height, same for both characters
MAX_FALL_SPEED = to_fixed(4)  # player_max_fall_speed

HUNTER_JUMP_VELOCITY = -(to_fixed(4) + FIXED_ONE // 2)
HUNTER_GRAVITY = FIXED_ONE // 4
HUNTER_JUMP_CUT_VELOCITY = -(to_fixed(1) + FIXED_ONE // 2)

# rival_* constants in difficulty.h are derived from the Hunter's own via
# integer fixed-point math (`(player_x * 115) / 100` etc.) -- replicated
# here the same way, not as separately-rounded pixel values, so this
# matches the real per-frame arithmetic bit-for-bit.
RIVAL_JUMP_VELOCITY = (HUNTER_JUMP_VELOCITY * 115) // 100
RIVAL_GRAVITY = (HUNTER_GRAVITY * 8) // 10
RIVAL_JUMP_CUT_VELOCITY = (HUNTER_JUMP_CUT_VELOCITY * 115) // 100

CHARACTER_PHYSICS = {
    'hunter': (HUNTER_JUMP_VELOCITY, HUNTER_GRAVITY, HUNTER_JUMP_CUT_VELOCITY),
    'rival': (RIVAL_JUMP_VELOCITY, RIVAL_GRAVITY, RIVAL_JUMP_CUT_VELOCITY),
}

# Relics that change achievable jump height, and therefore matter to this
# script. Extend as more are implemented in src/game/player.cpp -- e.g.
# Roc Wing ("High jump", SPEC.md's relic table) once it exists in code.
JUMP_RELICS = ['double']

# Rooms before the Rival's unlock point (rooms.md rule #7 / SPEC.md
# section 7's Shape step 4: she's found in catacombs_18, right after the
# Catacombs boss). Catacombs_18 itself is already "after" -- she's found
# there, not blocked by anything in it -- so only 01 through the boss
# arena (17) are excluded.
PRE_RIVAL_UNLOCK_ROOMS = {'catacombs_%02d' % n for n in range(1, 18)}

# How exhaustively to sweep double-jump timings when looking for the
# true best case (see double_jump_max_rise). Frame 1 is the earliest a
# second press can register at all (see player.cpp's jump_buffer/
# edge-detection); frames/gaps beyond these horizons only get further
# from the best case, not closer to a new one (a double jump left this
# late has always already started losing height to the fall, in every
# case checked while writing this).
DOUBLE_JUMP_RELEASE_FRAME_HORIZON = 60
DOUBLE_JUMP_REPRESS_GAP_HORIZON = 30


def single_jump_max_rise(jump_velocity, gravity):
    """The deterministic max apex (full hold, no jump-cut) -- matches
    player.cpp's try_jump + apply_gravity with the button held the whole
    ascent."""
    velocity_y = jump_velocity
    y = 0
    min_y = 0

    while True:
        velocity_y += gravity

        if velocity_y > MAX_FALL_SPEED:
            velocity_y = MAX_FALL_SPEED

        y += velocity_y

        if y < min_y:
            min_y = y

        if velocity_y >= 0:
            break

    return -min_y / FIXED_ONE


def double_jump_rise(jump_velocity, gravity, jump_cut_velocity, release_frame, repress_gap):
    """Total rise for one specific press/release/re-press timing.

    Order within a frame matters and must match update_normal's real
    call order exactly (apply_gravity, try_jump, apply_jump_cut,
    move_and_collide): velocity changes (gravity, then a cut or a fresh
    jump2 overwrite) are resolved BEFORE that frame's position update,
    not after -- getting this backwards makes a same-frame "land or
    jump2" race resolve as landing every time, when the real game's
    order lets a buffered jump2 press win instead (this was a real bug
    caught writing this script: an earlier version updated position
    first, and it manufactured physically-impossible trajectories that
    fell back through the original ground level while still "airborne").

    Returns whatever height was actually reached even if the player
    lands before repress_gap elapses (jump2 never fires) -- the caller
    doesn't need to distinguish that from a deliberate single jump held
    a while then released, since the same is possible in a normal.
    """
    velocity_y = jump_velocity
    y = 0
    min_y = 0
    frame = 0
    jump2_started = False

    while frame < 200:
        frame += 1
        velocity_y += gravity

        if velocity_y > MAX_FALL_SPEED:
            velocity_y = MAX_FALL_SPEED

        if frame == release_frame and velocity_y < jump_cut_velocity:
            velocity_y = jump_cut_velocity

        if frame == release_frame + repress_gap and not jump2_started:
            velocity_y = jump_velocity
            jump2_started = True

        y += velocity_y

        if y >= 0 and not jump2_started:
            # Landed (recharging the double jump, per move_and_collide)
            # before the scheduled repress ever fired -- whatever height
            # was reached is what this timing actually gets, and jump2
            # never happened.
            return -min_y / FIXED_ONE

        if y < min_y:
            min_y = y

        if jump2_started and velocity_y >= 0 and frame > release_frame + repress_gap:
            break

    return -min_y / FIXED_ONE


def double_jump_max_rise(jump_velocity, gravity, jump_cut_velocity):
    """The true best case across every timing a player could execute a
    double jump with -- not a handful of sampled points. Always >= the
    plain single-jump apex (a well-timed second press strictly adds
    height; see the module docstring for why there's no meaningful
    worst-case floor to compute alongside this)."""
    best = None

    for release_frame in range(1, DOUBLE_JUMP_RELEASE_FRAME_HORIZON + 1):
        for gap in range(1, DOUBLE_JUMP_REPRESS_GAP_HORIZON + 1):
            rise = double_jump_rise(jump_velocity, gravity, jump_cut_velocity, release_frame, gap)

            if best is None or rise > best:
                best = rise

    return best


def combo_max_rise(character, owned_relics):
    """Best-case rise this (character, owned relics) combination can
    achieve, across every jump technique available to it."""
    jump_velocity, gravity, jump_cut_velocity = CHARACTER_PHYSICS[character]
    single_max = single_jump_max_rise(jump_velocity, gravity)

    if 'double' in owned_relics:
        return max(single_max, double_jump_max_rise(jump_velocity, gravity, jump_cut_velocity))

    return single_max


def all_combos(rival_available):
    """Every (character, owned_relics frozenset) combination -- 2 x
    2^len(JUMP_RELICS) total (scaling automatically as JUMP_RELICS
    grows), or just the Hunter's half of that if the Rival isn't a
    character the player could have yet for this gate."""
    characters = list(CHARACTER_PHYSICS) if rival_available else ['hunter']

    for character in characters:
        for count in range(len(JUMP_RELICS) + 1):
            for owned in itertools.combinations(JUMP_RELICS, count):
                yield character, frozenset(owned)


def combo_label(character, owned_relics):
    if not owned_relics:
        return character

    return character + '+' + '+'.join(sorted(owned_relics))


def combo_satisfies(character, owned_relics, required_tokens):
    for token in required_tokens:
        if token in JUMP_RELICS:
            if token not in owned_relics:
                return False
        elif token in CHARACTER_PHYSICS:
            if token != character:
                return False
        else:
            raise ValueError('unknown "requires" token "%s" (expected one of: %s)'
                    % (token, ', '.join(list(CHARACTER_PHYSICS) + JUMP_RELICS)))

    return True


def find_launch_platform(door, platforms):
    """The platform this gate is actually jumped from: among platforms
    overlapping the door's x-range and lying at or below it, the closest
    one underneath -- the same "what would you actually be standing on"
    reasoning used by hand for catacombs_05 (rooms.md)."""
    door_bottom = door['y'] + door['height']
    candidates = [
        p for p in platforms
        if p['x'] < door['x'] + door['width'] and p['x'] + p['width'] > door['x']
        and p['y'] >= door_bottom
    ]

    if not candidates:
        return None

    return min(candidates, key=lambda p: p['y'])


def required_rise_for_gate(door, platforms):
    """The vertical center-displacement a jump needs to reach this gate
    from its launch platform -- None if no platform underneath it was
    found (the room needs a look, not something this script can measure)."""
    platform = find_launch_platform(door, platforms)

    if platform is None:
        return None

    door_bottom = door['y'] + door['height']
    return platform['y'] - door_bottom - (2 * PLAYER_HALF_HEIGHT)


def validate_gate(required_rise, requires, rival_available):
    required_tokens = set(requires.split('+'))
    findings = []

    for character, owned_relics in all_combos(rival_available):
        max_rise = combo_max_rise(character, owned_relics)
        label = combo_label(character, owned_relics)
        satisfies = combo_satisfies(character, owned_relics, required_tokens)
        can_pass = max_rise >= required_rise

        if satisfies and not can_pass:
            findings.append((
                label,
                'intended to pass, but even its best-case rise %.2fpx < required %dpx (gate is impossible)'
                % (max_rise, required_rise)
            ))
        elif not satisfies and can_pass:
            findings.append((
                label,
                'NOT intended to pass, but best-case rise %.2fpx >= required %dpx (gate can be bypassed)'
                % (max_rise, required_rise)
            ))

    return findings


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--rooms-dir', default='assets/rooms')
    args = parser.parse_args()

    paths = sorted(glob.glob(os.path.join(args.rooms_dir, '*.tmj')))
    gate_count = 0
    problem_count = 0

    for path in paths:
        room = convert_rooms.load_room(path)
        rival_available = room['id'] not in PRE_RIVAL_UNLOCK_ROOMS

        for door in room['doors']:
            requires = door.get('requires')

            if not requires:
                continue

            gate_count += 1
            required_rise = required_rise_for_gate(door, room['platforms'])

            if required_rise is None:
                problem_count += 1
                print('%s -> %s (requires %s): no platform found beneath this gate -- '
                        'can\'t measure it, check the room by hand'
                        % (room['id'], door['target_room'], requires))
                continue

            findings = validate_gate(required_rise, requires, rival_available)

            if findings:
                problem_count += 1
                print('%s -> %s (requires %s, needs %dpx rise):'
                        % (room['id'], door['target_room'], requires, required_rise))

                for label, message in findings:
                    print('  [FAIL] %s: %s' % (label, message))

    print()
    print('checked %d declared gate(s) across %d room(s)' % (gate_count, len(paths)))

    if problem_count:
        print('%d gate(s) have issues' % problem_count)
        sys.exit(1)

    print('all gates OK')


if __name__ == '__main__':
    main()
