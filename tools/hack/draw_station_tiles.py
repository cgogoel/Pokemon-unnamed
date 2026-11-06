#!/usr/bin/env python3
"""Draws the ranger station's custom metatiles (Act 2 finale, docs/hack/act2_finale.md).

Pokémon Mansion tileset (B1F, the clock), after the furnaces of draw_mansion_furnace.py (which this runs first, so
the tileset is rebuilt from the same base every time). A new row of 16 tiles at the end of tiles.png, palette 09
(the furnaces' iron, brick and flame colors):
    0x384 torch, unlit     0x385 torch, lit       (standing iron braziers on the basement floor)
    0x386 moon emblem      0x387 sun emblem       (painted on the wall, above the furnaces)
    0x388 iron bars        (the cage around B1F's stairs; on the basement floor)
    0x389, 0x38A           the Mansion 1F stair down's right-hand pieces (0x2D2, 0x2DA) on the basement floor

Cerulean Cave tileset (B2F, the flood). A new row of tiles at the end of tiles.png, its palette 08 (cave greys):
    0x306 stepping stone   (a flat rock in the cave's pond water, walkable)

Generic building 2 tileset (1F). No new tiles:
    0x338 a cardboard box  (the building tileset's, on the gate's floor)

Re-running redraws them in place.

    python3 tools/hack/draw_station_tiles.py
"""
import math
import os
import struct

import draw_mansion_furnace as furnace
from draw_mansion_furnace import read_png, write_png

ROOT = furnace.ROOT
PRIMARY_TILES = 640
PRIMARY_METATILES = 640

# --- Pokémon Mansion ------------------------------------------------------------------------------------------------
MANSION = os.path.join(ROOT, 'data/tilesets/secondary/pokemon_mansion_frlg')
MANSION_METATILES = 260   # The tileset's own 258 and the two furnaces
MANSION_TILE_ROWS = 23    # The tileset's own 22 rows and the furnaces' row
MANSION_PAL = 9
KEYS = '.KdilbrpoROYWmax'  # Palette 09 (draw_mansion_furnace.PALETTE), x = white

TORCH_UNLIT = [
    '................',
    '................',
    '................',
    '................',
    '..KKKKKKKKKKKK..',
    '.KllllllllllllK.',
    '.KdmamaaamamadK.',
    '..KiiiiiiiiiiK..',
    '...KddiiiiddK...',
    '....KKdiidKK....',
    '......KilK......',
    '......KidK......',
    '......KidK......',
    '.....KdiidK.....',
    '....KdiiiidK....',
    '....KKKKKKKK....',
]
TORCH_LIT = [
    '.......R........',
    '......RO...R....',
    '...R..ROR.RO....',
    '...RO.OYORYOR...',
    '..ROYOYWYYWYOR..',
    '.KOYWWWWWWWWYOK.',
    '.KYWWWWWWWWWWYK.',
] + TORCH_UNLIT[7:]
BARS = [
    'KKKKKKKKKKKKKKKK',
    'llllllllllllllll',
    'dddddddddddddddd',
    '.Kl..Kl..Kl..Kl.',
    '.Ki..Ki..Ki..Ki.',
    '.Ki..Ki..Ki..Ki.',
    '.Ki..Ki..Ki..Ki.',
    '.Kd..Kd..Kd..Kd.',
    '.Ki..Ki..Ki..Ki.',
    '.Ki..Ki..Ki..Ki.',
    '.Ki..Ki..Ki..Ki.',
    '.Kd..Kd..Kd..Kd.',
    'KKKKKKKKKKKKKKKK',
    'llllllllllllllll',
    'dddddddddddddddd',
    'KKKKKKKKKKKKKKKK',
]


def emblem_moon():
    """A round night-sky plaque with a crescent moon and a few stars."""
    art = [['.'] * 16 for _ in range(16)]
    for y in range(16):
        for x in range(16):
            d = math.hypot(x - 7.5, y - 7.5)
            if d < 6.6:
                art[y][x] = 'd'
            elif d < 7.6:
                art[y][x] = 'K'
            if d < 4.6 and math.hypot(x - 9.6, y - 6.2) > 3.9:
                art[y][x] = 'W' if math.hypot(x - 9.6, y - 6.2) < 4.9 else 'Y'
    for x, y in ((11, 10), (10, 3), (12, 7)):
        art[y][x] = 'x'
    return [''.join(r) for r in art]


def emblem_sun():
    """A sun with eight rays, painted straight on the wall."""
    art = [['.'] * 16 for _ in range(16)]
    for y in range(16):
        for x in range(16):
            dx, dy = x - 7.5, y - 7.5
            d = math.hypot(dx, dy)
            angle = math.degrees(math.atan2(dy, dx)) % 45
            if d < 3.6:
                art[y][x] = 'Y' if d < 2.6 else 'O'
            elif d < 4.4:
                art[y][x] = 'R'
            elif d < 7.6 and (angle < 9 or angle > 36):
                art[y][x] = 'O' if d < 6.2 else 'R'
    art[6][6] = 'W'
    art[6][7] = 'W'
    return [''.join(r) for r in art]


def quads(art, keys):
    """The four 8x8 tiles of a 16x16 picture: top-left, top-right, bottom-left, bottom-right."""
    px = [[keys.index(c) for c in row] for row in art]
    assert len(px) == 16 and all(len(r) == 16 for r in px), art
    return [[r[x0:x0 + 8] for r in px[y0:y0 + 8]] for y0, x0 in ((0, 0), (0, 8), (8, 0), (8, 8))]


def add_tile_row(folder, base_rows, tiles):
    png = os.path.join(folder, 'tiles.png')
    width, rows, plte = read_png(png)
    rows = rows[:base_rows * 8] + [[0] * width for _ in range(8)]
    for n, tile in enumerate(tiles):
        for y in range(8):
            rows[base_rows * 8 + y][n * 8:n * 8 + 8] = tile[y]
    write_png(png, width, rows, plte)
    return PRIMARY_TILES + base_rows * 16


def add_metatiles(folder, base_count, metatiles, attr):
    metatiles_path = os.path.join(folder, 'metatiles.bin')
    attrs_path = os.path.join(folder, 'metatile_attributes.bin')
    data = open(metatiles_path, 'rb').read()[:base_count * 16]
    attrs = open(attrs_path, 'rb').read()[:base_count * 4]
    for layers in metatiles:
        data += struct.pack('<8H', *layers)
        attrs += struct.pack('<I', attr)
    open(metatiles_path, 'wb').write(data)
    open(attrs_path, 'wb').write(attrs)
    assert os.path.getsize(attrs_path) * 4 == os.path.getsize(metatiles_path), 'metatile attributes out of step'


def layers_of(folder, metatile_id):
    data = open(os.path.join(folder, 'metatiles.bin'), 'rb').read()
    i = metatile_id - PRIMARY_METATILES
    return list(struct.unpack('<8H', data[i * 16:i * 16 + 16]))


LAYER_COVERED = 1 << 29  # Both layers under objects: the player walks in front of the torch and the stone
MB_NORMAL = 0x00


def draw_mansion():
    furnace.main()
    unlit, lit, moon, sun, bars = (quads(a, KEYS) for a in (TORCH_UNLIT, TORCH_LIT, emblem_moon(), emblem_sun(), BARS))
    # Tiles: torch top (unlit) x2, torch bottom x2, torch top (lit) x2, moon x4, sun x4, bars top, bars bottom
    tiles = unlit + lit[:2] + moon + sun + [bars[0], bars[2]]
    first = add_tile_row(MANSION, MANSION_TILE_ROWS, tiles)

    def top(*tiles):
        return [(first + t) | (MANSION_PAL << 12) for t in tiles]
    floor = layers_of(MANSION, 0x286)[:4]       # The basement floor
    wall = layers_of(MANSION, 0x2AA)[4:]        # The wall's face (drawn on its top layer)
    add_metatiles(MANSION, MANSION_METATILES, [
        floor + top(0, 1, 2, 3),                 # 0x384 torch, unlit
        floor + top(4, 5, 2, 3),                 # 0x385 torch, lit
        wall + top(6, 7, 8, 9),                  # 0x386 moon
        wall + top(10, 11, 12, 13),              # 0x387 sun
        floor + top(14, 14, 15, 15),             # 0x388 bars
    ], MB_NORMAL | LAYER_COVERED)
    # The stair down's pieces that show the floor, on the basement's floor instead of 1F's (same attributes)
    attrs = open(os.path.join(MANSION, 'metatile_attributes.bin'), 'rb').read()
    for n, piece in enumerate((0x2D2, 0x2DA)):
        i = piece - PRIMARY_METATILES
        add_metatiles(MANSION, MANSION_METATILES + 5 + n, [floor + layers_of(MANSION, piece)[4:]],
                      struct.unpack('<I', attrs[i * 4:i * 4 + 4])[0])
    print('Mansion: torch 0x384/0x385, moon 0x386, sun 0x387, bars 0x388, stair pieces 0x389/0x38A')


# --- Cerulean Cave --------------------------------------------------------------------------------------------------
CAVE = os.path.join(ROOT, 'data/tilesets/secondary/cerulean_cave_frlg')
CAVE_METATILES = 134
CAVE_TILE_ROWS = 16
CAVE_PAL = 8
POND_WATER = (0x1A0, 0x1A1, 0x1A2, 0x1A3)  # The primary's pond water (metatile 0x12B's bottom layer), palette 4


def stepping_stone():
    """A flat stone slab seen from above: a light top face over a darker rim. Keys are palette 08's indices."""
    art = [['0'] * 16 for _ in range(16)]
    face = lambda x, y: ((x - 7.5) / 7.2) ** 2 + ((y - 6.6) / 5.6) ** 2 < 1
    body = lambda x, y: face(x, y) or ((x - 7.5) / 7.2) ** 2 + ((y - 8.6) / 5.6) ** 2 < 1
    for y in range(16):
        for x in range(16):
            if not body(x, y):
                continue
            edge = any(not body(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if edge:
                art[y][x] = 'f'
            elif not face(x, y):
                art[y][x] = '4'
            else:
                shade = (x - 7.5) * 0.5 + (y - 6.6)
                art[y][x] = '1' if shade < -3.5 else '2' if shade < 1.5 else '3'
    for x, y in ((5, 6), (10, 4), (9, 8), (4, 9)):
        art[y][x] = '5'
    return [''.join(r) for r in art]


def draw_cave():
    stone = quads(stepping_stone(), '012345' + '6789abcde' + 'f')
    first = add_tile_row(CAVE, CAVE_TILE_ROWS, stone)
    water = [t | (4 << 12) for t in POND_WATER]
    add_metatiles(CAVE, CAVE_METATILES, [
        water + [(first + t) | (CAVE_PAL << 12) for t in range(4)],   # 0x306 stepping stone
    ], MB_NORMAL | LAYER_COVERED)
    print('Cerulean Cave: stepping stone 0x306')


# --- Generic building 2 (1F) ----------------------------------------------------------------------------------------
GATE = os.path.join(ROOT, 'data/tilesets/secondary/generic_building_2_frlg')
GATE_METATILES = 184
BOX = (0x0DD, 0x0DE, 0x0ED, 0x0EE)  # The building tileset's cardboard box (metatile 0x181), palette 1


def draw_gate():
    """No new tiles: the building tileset's box, standing on the gate's floor (0x281)."""
    floor = layers_of(GATE, 0x281)[:4]
    add_metatiles(GATE, GATE_METATILES, [floor + [t | (1 << 12) for t in BOX]], MB_NORMAL | LAYER_COVERED)
    print('Generic building 2: box on the floor 0x338')


if __name__ == '__main__':
    draw_mansion()
    draw_cave()
    draw_gate()
