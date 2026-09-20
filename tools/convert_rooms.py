#!/usr/bin/env python3
"""Converts assets/rooms/*.tmj (real Tiled JSON exports) into GBA-ready C++
room data.

Rooms are data, not code (see CLAUDE.md's data-over-code rule) — this is
the build-time converter that rule calls for. It is wired into the root
Makefile's EXTTOOL hook, so it runs automatically before compilation.

Workflow: edit a room's *.tmx in Tiled, then export it to a *.tmj next to
it (Tiled's File > Export As, or `tiled --export-map room.tmx room.tmj`).
This script only reads the committed *.tmj files — it does not invoke
Tiled itself, so the ROM build doesn't require Tiled to be installed.

Room schema (one "objects" object layer per map; see assets/rooms/README.md):
  - a "kind"=platform rectangle per flat-topped platform
  - a "kind"=door rectangle with target_room/target_x/target_y properties
  - a "kind"=spawn point, the room's default spawn position
  - a "kind"=enemy point per enemy, with an "enemy_type" property (one
    of: skeleton, bat, archer, zombie, bone_pillar, fleaman, medusa_head)
    and an optional int "facing" (-1 or +1, default -1) for enemies that
    don't decide their own facing dynamically

A room can also have a map-level (not object) custom property
"is_save_room" (bool): SPEC.md's save rooms, which restore HP/MP fully
and become the respawn checkpoint on entry (src/game/world.cpp).

Tiled's coordinates are pixels from the map's top-left corner; the game
uses Butano's screen-centered convention. This script converts between
them using the map's own width/height, so a room can be more than one
screen (rooms.md's "2x1"/"1x2" rooms) — src/game/level.cpp's camera
scrolls within whatever size the room turns out to be.

Usage:
    convert_rooms.py --rooms-dir assets/rooms --build generated
"""
import argparse
import glob
import json
import os
import sys

# Must match the type-index convention documented in src/game/level.h and
# used by src/game/enemy_spawner.cpp.
ENEMY_TYPE_IDS = {
    'skeleton': 0,
    'bat': 1,
    'archer': 2,
    'zombie': 3,
    'bone_pillar': 4,
    'fleaman': 5,
    'medusa_head': 6,
}


def load_room(path):
    with open(path, 'r') as f:
        data = json.load(f)

    half_width = (data['width'] * data['tilewidth']) // 2
    half_height = (data['height'] * data['tileheight']) // 2

    def to_game_x(tiled_x):
        return tiled_x - half_width

    def to_game_y(tiled_y):
        return tiled_y - half_height

    platforms = []
    doors = []
    enemies = []
    spawn = None

    for layer in data.get('layers', []):
        if layer.get('type') != 'objectgroup':
            continue

        for obj in layer.get('objects', []):
            props = {p['name']: p['value'] for p in obj.get('properties', [])}
            kind = props.get('kind')

            if kind == 'platform':
                platforms.append({
                    'x': to_game_x(obj['x']),
                    'y': to_game_y(obj['y']),
                    'width': obj['width'],
                })
            elif kind == 'door':
                doors.append({
                    'x': to_game_x(obj['x']),
                    'y': to_game_y(obj['y']),
                    'width': obj['width'],
                    'height': obj['height'],
                    'target_room': props['target_room'],
                    # target_x/target_y are authored in the TARGET room's own
                    # Tiled pixel space, not this room's — they can't be
                    # converted here, since that needs the target room's own
                    # half_width/half_height, which may differ from this
                    # room's (e.g. a door from a 1x1 room into a 2x1 one).
                    # resolve_door_targets() converts these once every room's
                    # dimensions are known.
                    'target_x_raw': props['target_x'],
                    'target_y_raw': props['target_y'],
                })
            elif kind == 'spawn':
                spawn = {'x': to_game_x(obj['x']), 'y': to_game_y(obj['y'])}
            elif kind == 'enemy':
                enemy_type = props['enemy_type']

                if enemy_type not in ENEMY_TYPE_IDS:
                    raise ValueError(
                        '%s has an enemy with unknown enemy_type "%s" (expected one of: %s)'
                        % (path, enemy_type, ', '.join(ENEMY_TYPE_IDS))
                    )

                enemies.append({
                    'type': ENEMY_TYPE_IDS[enemy_type],
                    'x': to_game_x(obj['x']),
                    'y': to_game_y(obj['y']),
                    'facing': int(props.get('facing', -1)),
                })

    if spawn is None:
        raise ValueError('%s has no "spawn" point object' % path)

    map_props = {p['name']: p['value'] for p in data.get('properties', [])}
    is_save_room = bool(map_props.get('is_save_room', False))

    room_id = os.path.splitext(os.path.basename(path))[0]
    return {
        'id': room_id, 'spawn': spawn, 'platforms': platforms, 'doors': doors,
        'enemies': enemies, 'is_save_room': is_save_room,
        'half_width': half_width, 'half_height': half_height,
    }


def load_rooms(rooms_dir):
    paths = sorted(glob.glob(os.path.join(rooms_dir, '*.tmj')))
    return [load_room(path) for path in paths]


def resolve_door_targets(rooms, index):
    """Converts each door's target_x_raw/target_y_raw (authored in the
    target room's own Tiled pixel space) into that target room's
    screen-centered game coordinates, now that every room's own
    half_width/half_height is known."""
    for room in rooms:
        for door in room['doors']:
            if door['target_room'] not in index:
                raise ValueError(
                    'room "%s" has a door targeting unknown room "%s"'
                    % (room['id'], door['target_room'])
                )

            target = rooms[index[door['target_room']]]
            door['target_x'] = door['target_x_raw'] - target['half_width']
            door['target_y'] = door['target_y_raw'] - target['half_height']


def build_index(rooms):
    index = {}

    for i, room in enumerate(rooms):
        room_id = room['id']

        if room_id in index:
            raise ValueError('duplicate room id "%s"' % room_id)

        index[room_id] = i

    return index


def write_header(path):
    with open(path, 'w') as f:
        f.write('#pragma once\n\n')
        f.write('// Generated by tools/convert_rooms.py from assets/rooms/*.tmj.\n')
        f.write('// Do not edit by hand.\n\n')
        f.write('namespace game::room_data\n{\n')
        f.write('    struct platform_def\n    {\n')
        f.write('        int x;\n        int y;\n        int width;\n')
        f.write('    };\n\n')
        f.write('    struct door_def\n    {\n')
        f.write('        int x;\n        int y;\n        int width;\n        int height;\n')
        f.write('        int target_room;\n        int target_x;\n        int target_y;\n')
        f.write('    };\n\n')
        f.write('    struct enemy_spawn_def\n    {\n')
        f.write('        int type;\n        int x;\n        int y;\n        int facing;\n')
        f.write('    };\n\n')
        f.write('    struct room_def\n    {\n')
        f.write('        const char* id;\n')
        f.write('        int spawn_x;\n        int spawn_y;\n')
        f.write('        const platform_def* platforms;\n        int platform_count;\n')
        f.write('        const door_def* doors;\n        int door_count;\n')
        f.write('        const enemy_spawn_def* enemy_spawns;\n        int enemy_spawn_count;\n')
        f.write('        int is_save_room;\n')
        f.write('        int half_width;\n        int half_height;\n')
        f.write('    };\n\n')
        f.write('    extern const room_def rooms[];\n')
        f.write('    extern const int room_count;\n')
        f.write('}\n')


def cpp_string(value):
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'


def write_source(path, rooms, index):
    with open(path, 'w') as f:
        f.write('#include "room_data.h"\n\n')
        f.write('// Generated by tools/convert_rooms.py from assets/rooms/*.tmj.\n')
        f.write('// Do not edit by hand.\n\n')
        f.write('namespace game::room_data\n{\n')

        for i, room in enumerate(rooms):
            platforms = room['platforms']
            doors = room['doors']

            f.write('    static const platform_def platforms_%d[] = {\n' % i)

            for p in platforms:
                f.write('        { %d, %d, %d },\n' % (p['x'], p['y'], p['width']))

            if not platforms:
                f.write('        { 0, 0, 0 },\n')

            f.write('    };\n\n')

            f.write('    static const door_def doors_%d[] = {\n' % i)

            for d in doors:
                if d['target_room'] not in index:
                    raise ValueError(
                        'room "%s" has a door targeting unknown room "%s"'
                        % (room['id'], d['target_room'])
                    )

                target = index[d['target_room']]
                f.write(
                    '        { %d, %d, %d, %d, %d, %d, %d },\n'
                    % (d['x'], d['y'], d['width'], d['height'], target, d['target_x'], d['target_y'])
                )

            if not doors:
                f.write('        { 0, 0, 0, 0, 0, 0, 0 },\n')

            f.write('    };\n\n')

            enemies = room['enemies']

            f.write('    static const enemy_spawn_def enemies_%d[] = {\n' % i)

            for e in enemies:
                f.write('        { %d, %d, %d, %d },\n' % (e['type'], e['x'], e['y'], e['facing']))

            if not enemies:
                f.write('        { 0, 0, 0, -1 },\n')

            f.write('    };\n\n')

        f.write('    const room_def rooms[] = {\n')

        for i, room in enumerate(rooms):
            spawn = room['spawn']
            f.write(
                '        { %s, %d, %d, platforms_%d, %d, doors_%d, %d, enemies_%d, %d, %d, %d, %d },\n'
                % (
                    cpp_string(room['id']), spawn['x'], spawn['y'],
                    i, len(room['platforms']),
                    i, len(room['doors']),
                    i, len(room['enemies']),
                    1 if room['is_save_room'] else 0,
                    room['half_width'], room['half_height'],
                )
            )

        f.write('    };\n\n')
        f.write('    const int room_count = %d;\n' % len(rooms))
        f.write('}\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--rooms-dir', default='assets/rooms')
    parser.add_argument('--build', default='generated')
    args = parser.parse_args()

    rooms = load_rooms(args.rooms_dir)

    if not rooms:
        print('no room files found in %s' % args.rooms_dir, file=sys.stderr)
        sys.exit(1)

    index = build_index(rooms)
    resolve_door_targets(rooms, index)

    include_dir = os.path.join(args.build, 'include')
    src_dir = os.path.join(args.build, 'src')
    os.makedirs(include_dir, exist_ok=True)
    os.makedirs(src_dir, exist_ok=True)

    write_header(os.path.join(include_dir, 'room_data.h'))
    write_source(os.path.join(src_dir, 'room_data.cpp'), rooms, index)

    print('converted %d room(s) from %s' % (len(rooms), args.rooms_dir))


if __name__ == '__main__':
    main()
