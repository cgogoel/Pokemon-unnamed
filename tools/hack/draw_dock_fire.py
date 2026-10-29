#!/usr/bin/env python3
"""Draws the burning dock planks of Vermilion's harbor (Act 2's Water puzzle) into the Vermilion tileset.

A 16x16 flame over each kind of pier plank the fires sit on: the plank's own tiles underneath, the flame on top, in
palette 11 (unused before). Two looks per plank, the flame and its mirror image, so neighbouring fire squares don't
repeat. Adds 4 tiles (a new row at the end of tiles.png) and two metatiles per plank after the tileset's own,
starting at METATILE_VermilionCity_DockFire_First, in the order of PLANKS (plank, then mirrored). The C side
(src/act2_story.c, sDockFirePlanks) uses the same order. Re-running redraws them in place.

    python3 tools/hack/draw_dock_fire.py
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import read_png, write_png  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
TILESET = os.path.join(ROOT, 'data/tilesets/secondary/vermilion_city_frlg')
PRIMARY = os.path.join(ROOT, 'data/tilesets/primary/general_frlg')
PRIMARY_TILES = 640
PRIMARY_METATILES = 640
BASE_METATILES = 168      # The tileset's own metatiles, before the fires
BASE_TILE_ROWS = 9        # tiles.png's rows of 16 tiles, before the fires
PAL = 11
LAYER_COVERED = 1         # Drawn under objects, like the plank
PLANKS = [0x1BE, 0x1C6, 0x1C5, 0x1C7, 0x1CE]  # Pier top edge, middle, left edge, right edge, bottom edge

PALETTE = [
    (115, 205, 164),  # 0 transparent
    (106, 16, 8),     # D dark red
    (197, 32, 16),    # R red
    (255, 123, 16),   # O orange
    (255, 213, 49),   # Y yellow
    (255, 246, 197),  # W white-hot
    (139, 32, 8),     # E embers
    (74, 65, 65),     # K soot
] + [(0, 0, 0)] * 8
KEYS = '.DROYWEK'

FLAME = [
    '......R.........',
    '.....RO.....R...',
    '....ROO....RO...',
    '...ROOR...ROO...',
    '..ROYOR..ROYOR..',
    '..ROYYOR.ROYOR..',
    '.ROYYYORROYYOR..',
    '.ROYWYYOROYYYOR.',
    'ROYYWWYOOYYWYOR.',
    'ROYWWWYYOYWWYYOR',
    'ROYWWWWYYYWWWYOR',
    'ROYYWWWWWWWWYYOR',
    'DROYYWWWWWWYYORD',
    '.DROYYYYYYYYORD.',
    'KKEDROOOOOORDEKK',
    'KEEKKDRRRRDKKEEK',
]


def main():
    assert all(len(row) == 16 for row in FLAME) and len(FLAME) == 16
    open(os.path.join(TILESET, 'palettes/%02d.pal' % PAL), 'w').write(
        'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in PALETTE))

    # Tiles: the flame's top-left, top-right, bottom-left, bottom-right
    art = [[KEYS.index(c) for c in row] for row in FLAME]
    png = os.path.join(TILESET, 'tiles.png')
    width, rows, plte = read_png(png)
    rows = rows[:BASE_TILE_ROWS * 8] + [[0] * width for _ in range(8)]
    for n, (x0, y0) in enumerate(((0, 0), (8, 0), (0, 8), (8, 8))):
        for y in range(8):
            for x in range(8):
                rows[BASE_TILE_ROWS * 8 + y][n * 8 + x] = art[y0 + y][x0 + x]
    write_png(png, width, rows, plte)
    first_tile = PRIMARY_TILES + BASE_TILE_ROWS * 16

    # Metatiles: the plank's own layer underneath, the flame (or its mirror image) on top
    primary = open(os.path.join(PRIMARY, 'metatiles.bin'), 'rb').read()
    primary_attrs = open(os.path.join(PRIMARY, 'metatile_attributes.bin'), 'rb').read()
    flame = [(first_tile + t) | (PAL << 12) for t in range(4)]
    hflip = 1 << 10
    mirrored = [flame[1] | hflip, flame[0] | hflip, flame[3] | hflip, flame[2] | hflip]

    metatiles_path = os.path.join(TILESET, 'metatiles.bin')
    attrs_path = os.path.join(TILESET, 'metatile_attributes.bin')
    metatiles = open(metatiles_path, 'rb').read()[:BASE_METATILES * 16]
    attrs = open(attrs_path, 'rb').read()[:BASE_METATILES * 4]  # Read before reopening to write (which empties it)
    for plank in PLANKS:
        bottom = list(struct.unpack_from('<4H', primary, plank * 16))
        attr = struct.unpack_from('<I', primary_attrs, plank * 4)[0]
        attr = (attr & ~(3 << 29)) | (LAYER_COVERED << 29)
        for top in (flame, mirrored):
            metatiles += struct.pack('<8H', *(bottom + top))
            attrs += struct.pack('<I', attr)
    open(metatiles_path, 'wb').write(metatiles)
    open(attrs_path, 'wb').write(attrs)
    assert os.path.getsize(attrs_path) * 4 == os.path.getsize(metatiles_path), 'metatile attributes out of step'
    print('METATILE_VermilionCity_DockFire_First 0x%03X (%d metatiles)'
          % (PRIMARY_METATILES + BASE_METATILES, 2 * len(PLANKS)))


if __name__ == '__main__':
    main()
