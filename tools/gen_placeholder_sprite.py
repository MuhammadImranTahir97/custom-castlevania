#!/usr/bin/env python3
"""Generates a solid-color placeholder sprite: a Butano-ready BMP + JSON pair.

Butano requires an uncompressed indexed BMP (16 or 256 colors) with no
color-space info in the header, and palette index 0 treated as transparent.
This writes that BMP by hand (stdlib `struct` only) so the tools/ pipeline
has no extra Python dependencies.

Uses 4bpp (16-color) rather than 8bpp: on real GBA hardware, 8bpp sprites
all share one global 256-color palette, so two differently-colored 8bpp
placeholder sprites would collide and render as the same color. 4bpp
sprites each get their own 16-color palette bank instead.

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
    while len(palette) < 16:
        palette.append((0, 0, 0, 0))

    bytes_per_row = (width + 1) // 2  # 2 pixels per byte (4 bits each)
    row_size = (bytes_per_row + 3) & ~3
    pixel_data_size = row_size * height
    header_size = 14 + 40 + (16 * 4)
    file_size = header_size + pixel_data_size

    file_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, header_size)
    info_header = struct.pack(
        '<IiiHHIIiiII',
        40, width, height, 1, 4, 0, pixel_data_size, 0, 0, 16, 0,
    )
    palette_bytes = b''.join(struct.pack('<4B', *entry) for entry in palette)

    row = bytearray(row_size)
    for x in range(width):
        byte_index = x // 2
        if x % 2 == 0:
            row[byte_index] |= 1 << 4
        else:
            row[byte_index] |= 1
    pixel_data = bytes(row) * height  # bottom-up row order, as BMP requires

    with open(path, 'wb') as f:
        f.write(file_header)
        f.write(info_header)
        f.write(palette_bytes)
        f.write(pixel_data)


def write_json(path):
    with open(path, 'w') as f:
        json.dump({"type": "sprite", "bpp_mode": "bpp_4"}, f, indent=4)
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
