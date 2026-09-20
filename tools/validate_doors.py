#!/usr/bin/env python3
"""Validates door placement across assets/rooms/*.tmj.

Reuses convert_rooms.py's own room loading, so this checks exactly what
the build will generate. Two checks per door:

  - the door's target position resolves within the target room's bounds
  - the door's target position doesn't land inside another door's trigger
    box in the target room

The second case is the reciprocal-door bug: arriving inside another
door's trigger box fires it again the moment it's checked. src/game/level.cpp
now suppresses the arrival door until the player's hitbox clears it, so this
no longer bounces forever at runtime -- but a room that needs that safety
net wasn't placed right, and should still be fixed at the source.

Usage:
    validate_doors.py [--rooms-dir assets/rooms]
"""
import argparse
import sys

from convert_rooms import build_index, load_rooms, resolve_door_targets


def validate_doors(rooms, index):
    errors = []
    door_count = 0

    for room in rooms:
        for door in room['doors']:
            door_count += 1
            target = rooms[index[door['target_room']]]
            tx, ty = door['target_x'], door['target_y']

            if abs(tx) > target['half_width'] or abs(ty) > target['half_height']:
                errors.append(
                    'room "%s" door -> "%s": target (%d, %d) is out of bounds '
                    '(room half-size is %dx%d)'
                    % (room['id'], door['target_room'], tx, ty,
                       target['half_width'], target['half_height'])
                )

            for other in target['doors']:
                left, right = other['x'], other['x'] + other['width']
                top, bottom = other['y'], other['y'] + other['height']

                if left <= tx <= right and top <= ty <= bottom:
                    errors.append(
                        'room "%s" door -> "%s": target (%d, %d) lands inside '
                        'a door trigger box in "%s" (that door targets "%s")'
                        % (room['id'], door['target_room'], tx, ty,
                           door['target_room'], other['target_room'])
                    )

    return errors, door_count


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--rooms-dir', default='assets/rooms')
    args = parser.parse_args()

    rooms = load_rooms(args.rooms_dir)
    index = build_index(rooms)
    resolve_door_targets(rooms, index)

    errors, door_count = validate_doors(rooms, index)

    print('checked %d door(s) across %d room(s)' % (door_count, len(rooms)))

    for error in errors:
        print('ERROR: %s' % error, file=sys.stderr)

    if errors:
        print('%d door(s) failed validation' % len(errors), file=sys.stderr)
        sys.exit(1)

    print('all doors OK')


if __name__ == '__main__':
    main()
