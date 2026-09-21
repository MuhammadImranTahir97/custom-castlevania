#!/usr/bin/env python3
"""Generates a placeholder regular_bg tileset: a Butano-ready BMP + JSON
pair, paired with a bg_palette BMP + JSON defining its actual colors.

Room terrain used to be drawn as sprites (one "ground" sprite per ~64px
of platform) -- on real GBA hardware, that's the wrong layer for
non-moving level geometry, and it was eating a large, fixed slice of the
128-sprite hardware budget for something that never moves or animates
(troubleshooting/build-environment.md entry #14). This generates the
tileset src/platform/main.cpp now draws terrain with instead: two 8x8
tiles side by side -- index 0 blank (Butano's transparent-index
convention, also what a freshly-cleared/unused map cell already is,
matching bn::memory::clear semantics), index 1 solid grey (the same
placeholder look "ground"'s sprite had).

Split into two assets (regular_bg_tiles + bg_palette) rather than one
combined "regular_bg" image because the map cells need to be built at
runtime, per room, from room_data's platform list (see
tools/dynamic_regular_bg's own split for the same reason) -- a plain
"regular_bg" bakes both its tiles AND its map at build time from one
static image, which doesn't fit a room whose terrain layout is only
known once actual room data loads.

Usage:
    gen_placeholder_bg_tiles.py <name> <r> <g> <b> [output_dir]

Example (matches the grey "ground" sprite's own color):
    gen_placeholder_bg_tiles.py ground_bg 110 110 110 assets/sprites
"""
import json
import os
import struct
import sys


def _bmp_bytes(width, height, bpp, palette, pixel_indices_by_row):
    pixels_per_byte = 8 // bpp
    bytes_per_row = (width + pixels_per_byte - 1) // pixels_per_byte
    row_size = (bytes_per_row + 3) & ~3
    pixel_data_size = row_size * height
    header_size = 14 + 40 + (len(palette) * 4)
    file_size = header_size + pixel_data_size

    file_header = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, header_size)
    info_header = struct.pack(
        '<IiiHHIIiiII',
        40, width, height, 1, bpp, 0, pixel_data_size, 0, 0, len(palette), 0,
    )
    palette_bytes = b''.join(struct.pack('<4B', b, g, r, a) for (r, g, b, a) in palette)

    rows = bytearray(pixel_data_size)

    # BMP rows are stored bottom-up.
    for y in range(height):
        row = bytearray(row_size)
        indices = pixel_indices_by_row(y)

        for x in range(width):
            index = indices[x]

            if bpp == 4:
                byte_index = x // 2
                shift = 4 if x % 2 == 0 else 0
                row[byte_index] |= index << shift
            else:
                row[x] = index

        rows[(height - 1 - y) * row_size:(height - 1 - y) * row_size + row_size] = row

    return file_header + info_header + palette_bytes + bytes(rows)


def write_tiles_bmp(path, r, g, b):
    # index 0 = blank/transparent (tile 0, an 8x8 all-index-0 block),
    # index 1 = solid grey (tile 1, an 8x8 all-index-1 block) -- two 8x8
    # tiles side by side, 16 wide total.
    palette = [(0, 0, 0, 0), (r, g, b, 0)] + [(0, 0, 0, 0)] * 14

    def indices_by_row(_y):
        return [0] * 8 + [1] * 8

    with open(path, 'wb') as f:
        f.write(_bmp_bytes(16, 8, 4, palette, indices_by_row))


def write_tiles_json(path):
    with open(path, 'w') as f:
        json.dump({"type": "regular_bg_tiles", "bpp_mode": "bpp_4"}, f, indent=4)
        f.write('\n')


def write_palette_bmp(path, r, g, b):
    # Colors only -- pixel content is irrelevant (Butano reads this
    # asset for its palette table, same color order as tiles.bmp's own
    # embedded palette above), so a single flat-index-0 image is enough.
    palette = [(0, 0, 0, 0), (r, g, b, 0)] + [(0, 0, 0, 0)] * 254

    def indices_by_row(_y):
        return [0] * 8

    with open(path, 'wb') as f:
        f.write(_bmp_bytes(8, 8, 8, palette, indices_by_row))


def write_palette_json(path):
    with open(path, 'w') as f:
        json.dump({"type": "bg_palette", "bpp_mode": "bpp_4", "colors_count": 16}, f, indent=4)
        f.write('\n')


def main():
    if len(sys.argv) < 5:
        print(__doc__)
        sys.exit(1)

    name = sys.argv[1]
    r, g, b = (int(v) for v in sys.argv[2:5])
    out_dir = sys.argv[5] if len(sys.argv) > 5 else '.'

    os.makedirs(out_dir, exist_ok=True)
    write_tiles_bmp(os.path.join(out_dir, '%s.bmp' % name), r, g, b)
    write_tiles_json(os.path.join(out_dir, '%s.json' % name))
    write_palette_bmp(os.path.join(out_dir, '%s_palette.bmp' % name), r, g, b)
    write_palette_json(os.path.join(out_dir, '%s_palette.json' % name))
    print('wrote %s.bmp/.json + %s_palette.bmp/.json' % (name, name))


if __name__ == '__main__':
    main()
