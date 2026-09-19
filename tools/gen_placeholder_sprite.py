#!/usr/bin/env python3
"""Generates a solid-color placeholder sprite: a Butano-ready BMP + JSON pair.

Butano requires an uncompressed indexed BMP (16 or 256 colors) with no
color-space info in the header, and palette index 0 treated as transparent.
This writes that BMP by hand (stdlib `struct` only) so the tools/ pipeline
has no extra Python dependencies.

Usage:
    gen_placeholder_sprite.py <name> <width> <height> <r> <g> <b> [output_dir]

Example (16x16 red square, matching a valid GBA sprite shape/size):
    gen_placeholder_sprite.py player 16 16 248 40 40 assets/sprites
"""
import json
import os
import struct
import sys


def write_bmp(path, width, height, r, g, b):
    # index 0 = transparent (Butano convention), index 1 = the placeholder color.
    palette = [(0, 0, 0, 0), (b, g, r, 0)]
    while len(palette) < 256:
        palette.append((0, 0, 0, 0))

    row_size = (width + 3) & ~3
    pixel_data_size = row_size * height
    header_size = 14 + 40 + (256 * 4)
    file_size = header_size + pixel_data_size

    file_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, header_size)
    info_header = struct.pack(
        '<IiiHHIIiiII',
        40, width, height, 1, 8, 0, pixel_data_size, 0, 0, 256, 0,
    )
    palette_bytes = b''.join(struct.pack('<4B', *entry) for entry in palette)

    row = bytes([1]) * width + bytes(row_size - width)
    pixel_data = row * height  # bottom-up row order, as BMP requires

    with open(path, 'wb') as f:
        f.write(file_header)
        f.write(info_header)
        f.write(palette_bytes)
        f.write(pixel_data)


def write_json(path):
    with open(path, 'w') as f:
        json.dump({"type": "sprite", "bpp_mode": "bpp_8"}, f, indent=4)
        f.write('\n')


def main():
    if len(sys.argv) < 7:
        print(__doc__)
        sys.exit(1)

    name = sys.argv[1]
    width, height, r, g, b = (int(v) for v in sys.argv[2:7])
    out_dir = sys.argv[7] if len(sys.argv) > 7 else '.'

    os.makedirs(out_dir, exist_ok=True)
    write_bmp(os.path.join(out_dir, f'{name}.bmp'), width, height, r, g, b)
    write_json(os.path.join(out_dir, f'{name}.json'))
    print(f'wrote {name}.bmp + {name}.json ({width}x{height})')


if __name__ == '__main__':
    main()
