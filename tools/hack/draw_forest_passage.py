#!/usr/bin/env python3
"""Draws the dark passage into the night bloom's glade (Act 2's Grass course) into the Viridian Forest tileset.

The way into the glade is a column of trees at Viridian Forest's north edge (x 39-41). Instead of cutting the trees, their
canopies stay whole and the ground beneath them goes dark: a shadowy hollow under the trees.

    PASSAGE_UNDER   (3, left/middle/right) the trees' bottom row: the canopy's lower edge, with darkness where the
                    trunk and its shadow were (from 0x2A0-0x2A2)
    PASSAGE_MOUTH   (3) the row below: the hollow's mouth, dark fading into the grass (from the shadow row,
                    0x2A3-0x2A5)
    PASSAGE_MOUTH_UP (3) the same mouth upside down, for the glade's side (dark below, grass above)
    PASSAGE_DARK    (1) dark ground

Adds two rows of tiles to tiles.png and the metatiles after the tileset's own 43 (from 0x2AB, in the order above).
Their palette is the trees' (8); the dark is its darkest color, deeper still in the night's tint. Re-running redraws
them in place. tools/hack/compose_glade.py uses them.

    python3 tools/hack/draw_forest_passage.py
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import read_png, write_png  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
TILESET = os.path.join(ROOT, 'data/tilesets/secondary/viridian_forest_frlg')
PRIMARY_TILES = 0x280
BASE_METATILES = 43
BASE_TILE_ROWS = 7
PAL = 8
LAYER_COVERED = 1
HFLIP, VFLIP = 1 << 10, 1 << 11

FIRST = PRIMARY_TILES + BASE_METATILES  # 0x2AB
PASSAGE_UNDER = [FIRST + i for i in range(3)]
PASSAGE_MOUTH = [FIRST + 3 + i for i in range(3)]
PASSAGE_MOUTH_UP = [FIRST + 6 + i for i in range(3)]
PASSAGE_DARK = FIRST + 9

CANOPY = {1, 2, 3, 4}   # The leaves' colors in palette 8
DARK = 8                # (65, 57, 49), the darkest
SHADE = 4               # (57, 82, 0), the hollow's rim


def main():
    png = os.path.join(TILESET, 'tiles.png')
    width, rows, plte = read_png(png)
    per_row = width // 8
    rows = rows[:BASE_TILE_ROWS * 8]

    def tile(t):
        i = t - PRIMARY_TILES
        return [rows[(i // per_row) * 8 + y][(i % per_row) * 8:(i % per_row) * 8 + 8] for y in range(8)]

    def flip(px):
        return [list(reversed(r)) for r in px]

    new_tiles = []

    def add(px):
        new_tiles.append(px)
        return PRIMARY_TILES + BASE_TILE_ROWS * per_row + len(new_tiles) - 1

    # The trees' bottom row: canopy kept, the rest dark
    def darken(px):
        return [[c if c in CANOPY else DARK for c in r] for r in px]
    under_bl, under_br, under_mid = add(darken(tile(0x286))), add(darken(tile(0x287))), add(darken(tile(0x2C4)))

    # The mouth: the shadow row's 48x16 strip, dark under a rounded rim
    strip = [[0] * 48 for _ in range(16)]
    quads = {(0, 0): tile(0x2D2), (8, 0): tile(0x2D3), (0, 8): tile(0x2E2), (8, 8): tile(0x2E3),
             (16, 0): tile(0x2D4), (24, 0): flip(tile(0x2D4)), (16, 8): tile(0x2E4), (24, 8): flip(tile(0x2E4)),
             (40, 0): flip(tile(0x2D2)), (32, 0): flip(tile(0x2D3)), (40, 8): flip(tile(0x2E2)), (32, 8): flip(tile(0x2E3))}
    for (x0, y0), px in quads.items():
        for y in range(8):
            for x in range(8):
                strip[y0 + y][x0 + x] = px[y][x]

    def inside(x, y, grow=0.0):
        dx, dy = (x + 0.5 - 24) / (22 + grow), (y + 0.5 - 4) / (8 + grow)
        return y < 4 and 1 + grow / 4 <= x < 47 - grow / 4 or dx * dx + dy * dy < 1
    for y in range(16):
        for x in range(48):
            if inside(x, y):
                strip[y][x] = DARK
            elif inside(x, y, 1.5):
                strip[y][x] = SHADE
    mouth = []
    for c in range(3):
        mouth.append([add([r[c * 16 + q * 8:c * 16 + q * 8 + 8] for r in strip[h * 8:h * 8 + 8]])
                      for h in range(2) for q in range(2)])
    dark = add([[DARK] * 8 for _ in range(8)])

    assert len(new_tiles) <= 2 * per_row
    rows += [[0] * width for _ in range(16)]
    for n, px in enumerate(new_tiles):
        for y in range(8):
            for x in range(8):
                rows[BASE_TILE_ROWS * 8 + (n // per_row) * 8 + y][(n % per_row) * 8 + x] = px[y][x]
    write_png(png, width, rows, plte)

    # Metatiles: the bottom layer is the original's (the grass under the trees); the top layer the new tiles
    metatiles_path = os.path.join(TILESET, 'metatiles.bin')
    attrs_path = os.path.join(TILESET, 'metatile_attributes.bin')
    metatiles = open(metatiles_path, 'rb').read()[:BASE_METATILES * 16]
    attrs = open(attrs_path, 'rb').read()[:BASE_METATILES * 4]
    attr = LAYER_COVERED << 29

    def original(m):
        return list(struct.unpack_from('<8H', metatiles, (m - PRIMARY_TILES) * 16))

    def put(bottom, top):
        nonlocal metatiles, attrs
        metatiles += struct.pack('<8H', *(bottom + [t | (PAL << 12) for t in top]))
        attrs += struct.pack('<I', attr)

    left_top, mid_top = original(0x2A0)[4:6], original(0x2A1)[4:6]
    right_top = original(0x2A2)[4:6]
    put(original(0x2A0)[:4], [left_top[0] & 0xFFF, left_top[1] & 0xFFF, under_bl, under_br])
    put(original(0x2A1)[:4], [mid_top[0] & 0xFFF, mid_top[1] & 0xFFF, under_mid, under_mid | HFLIP])
    put(original(0x2A2)[:4], [right_top[0] & 0xFFF, right_top[1] & 0xFFF, under_br | HFLIP, under_bl | HFLIP])
    for c in range(3):
        put(original(0x2A3 + c)[:4], mouth[c])
    for c in range(3):
        tl, tr, bl, br = mouth[c]
        put(original(0x2A3 + c)[:4], [bl | VFLIP, br | VFLIP, tl | VFLIP, tr | VFLIP])
    put(original(0x2A3)[:4], [dark] * 4)
    open(metatiles_path, 'wb').write(metatiles)
    open(attrs_path, 'wb').write(attrs)
    assert os.path.getsize(attrs_path) * 4 == os.path.getsize(metatiles_path), 'metatile attributes out of step'
    print('PASSAGE metatiles 0x%03X-0x%03X' % (FIRST, PASSAGE_DARK))


if __name__ == '__main__':
    main()
