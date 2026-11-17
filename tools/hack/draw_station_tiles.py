#!/usr/bin/env python3
"""Draws the ranger station's custom metatiles (Act 2 finale, docs/hack/act2_finale.md).

Pokémon Mansion tileset (B1F, the clock), after the furnaces of draw_mansion_furnace.py (which this runs first, so
the tileset is rebuilt from the same base every time). A new row of 16 tiles at the end of tiles.png, palette 09
(the furnaces' iron, brick and flame colors):
    0x384 torch, unlit     0x385 torch, lit       (standing iron braziers on the basement floor; the lit flames
                                                   flicker: frames in anim/torch/)
    0x386 moon emblem      0x387 sun emblem       (painted on the wall, above the furnaces)
    0x388 railing, back    (the cage around B1F's stairs, an iron railing on the basement floor; railing_pieces)
    0x389, 0x38A           the Mansion 1F stair down's right-hand pieces (0x2D2, 0x2DA) on the basement floor
    0x38B-0x392            the railing's sides, corners and gate (tiles in the free slots RAILING_SLOTS)

Cerulean Cave tileset (B2F, the flood). A new row of tiles at the end of tiles.png, its palette 08 (cave greys):
    0x306 stepping stone   (a flat rock in the cave's pond water, walkable)

    0x307 the ladder up's top half on the chamber floor (no new tiles)

Mt. Ember tileset (B4F). No new tiles:
    0x362 the ladder up's top half on the altar room's floor

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
# The cage around B1F's stair down: an iron railing, light from the upper left like the rest of FireRed. Each piece
# is a 16x16 picture on the basement floor; the pictures are cut into 8x8 tiles and repeats (and mirror images) are
# shared, so the whole railing fits in the 12 tiles the tileset has left.
#   rail     the back of the cage, seen face on: a top rail, bars, a bottom rail on the floor
#   side     a side of the cage seen from above: the top rail running away from the viewer, the left half of the
#            square (mirrored for the right side)
#   corner   the back corners: a post, the rail running off to the right (mirrored for the right)
#   front    the front corners: the side's rail coming down onto a post, the rail running off to the right
#   gate     the front's three squares that drop into the floor: spear-tipped bars through the rails, and a padlock
#            on the middle one
POST_X = range(3, 8)    # The posts and the sides' top rail: columns 3-7, in the left half of the square
POST_COLORS = 'KlidK'


def _canvas():
    return [['.'] * 16 for _ in range(16)]


def _rail(art, x0, x1, gate=False):
    for x in range(x0, x1 + 1):
        bar = x % 4 in (1, 2)
        shade = 'l' if x % 4 == 1 else 'd'
        if gate:
            # Spear tips above the top rail; the bars run through both rails
            if bar:
                art[1][x] = 'K'
                art[2][x] = shade
            for y, c in ((3, 'K'), (4, 'i'), (5, 'K'), (12, 'K'), (13, 'i'), (14, 'd'), (15, 'K')):
                art[y][x] = c
            for y in range(6, 12):
                art[y][x] = shade if bar else '.'
            if bar:
                art[3][x], art[4][x], art[5][x] = shade, shade, shade
        else:
            for y, c in ((3, 'K'), (4, 'l'), (5, 'i'), (6, 'K'), (12, 'K'), (13, 'i'), (14, 'd'), (15, 'K')):
                art[y][x] = c
            for y in range(7, 12):
                art[y][x] = shade if bar else '.'


def _post(art, top):
    for i, x in enumerate(POST_X):
        for y in range(top + 1, 15):
            art[y][x] = POST_COLORS[i]
        art[15][x] = 'K'
        art[top][x] = 'K' if i in (0, 4) else 'x' if i == 1 else 'l'   # A cap, lit from the upper left
    art[top - 1][4], art[top - 1][5], art[top - 1][6] = 'K', 'K', 'K'


def _side(art, y0, y1):
    for y in range(y0, y1 + 1):
        for i, x in enumerate(POST_X):
            art[y][x] = POST_COLORS[i]
        if y % 4 == 3:  # The bars' tops, seen from above
            art[y][5] = 'K'


def railing_pieces():
    rail, side, corner, front, gate, lock = (_canvas() for _ in range(6))
    _rail(rail, 0, 15)
    _side(side, 0, 15)
    _rail(corner, 8, 15)
    _post(corner, 2)
    _side(front, 0, 2)
    _rail(front, 8, 15)
    _post(front, 3)
    _rail(gate, 0, 15, gate=True)
    _rail(lock, 0, 15, gate=True)
    # The padlock, hanging off the top rail in the middle of the gate (brass: Y lit, O shaded)
    for y, row in enumerate(['...KKKK...', '..K....K..', '..K....K..', '.KKKKKKKK.', '.KYYYYOOK.', '.KYWYKYOK.',
                             '.KYYYKOOK.', '.KOOOOOOK.', '..KKKKKK..']):
        for x, c in enumerate(row):
            if c != '.':
                lock[5 + y][3 + x] = c
    return [[''.join(r) for r in a] for a in (rail, side, corner, front, gate, lock)]


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


HFLIP = 1 << 10
RAILING_SLOTS = list(range(358, 368)) + [382, 383]  # Free tiles: the furnaces' row after its six, this row's last two
BLANK_TILE = 0                                       # The primary tileset's first tile is empty


def cut_tiles(pictures, slots):
    """Cuts 16x16 pictures into 8x8 tiles, sharing repeats and mirror images. Returns the new tiles (one per slot
    used) and, for each picture, its four tile entries (slot index, hflip) or None where the quarter is empty."""
    tiles, entries = [], []
    for art in pictures:
        quarters = []
        for q in quads(art, KEYS):
            if not any(any(row) for row in q):
                quarters.append(None)
                continue
            mirror = [row[::-1] for row in q]
            for n, t in enumerate(tiles):
                if t == q:
                    quarters.append((n, False))
                    break
                if t == mirror:
                    quarters.append((n, True))
                    break
            else:
                tiles.append(q)
                quarters.append((len(tiles) - 1, False))
        entries.append(quarters)
    assert len(tiles) <= len(slots), 'railing: %d tiles for %d free slots' % (len(tiles), len(slots))
    return tiles, entries


def put_tiles(folder, slots, tiles):
    png = os.path.join(folder, 'tiles.png')
    width, rows, plte = read_png(png)
    per_row = width // 8
    for slot, tile in zip(slots, tiles):
        for y in range(8):
            rows[(slot // per_row) * 8 + y][(slot % per_row) * 8:(slot % per_row) * 8 + 8] = tile[y]
    write_png(png, width, rows, plte)


def draw_mansion():
    furnace.main()
    unlit, lit, moon, sun = (quads(a, KEYS) for a in (TORCH_UNLIT, TORCH_LIT, emblem_moon(), emblem_sun()))
    # Tiles: torch top (unlit) x2, torch bottom x2, torch top (lit) x2, moon x4, sun x4, then two railing tiles
    first = add_tile_row(MANSION, MANSION_TILE_ROWS, unlit + lit[:2] + moon + sun)
    railing_tiles, railing = cut_tiles(railing_pieces(), RAILING_SLOTS)
    put_tiles(MANSION, RAILING_SLOTS, railing_tiles)
    # The lit torch's flames flicker (its top two tiles, animated by InitTilesetAnim_PokemonMansion)
    flames = furnace.flicker_frames(TORCH_LIT, 'ROYW', '.', (1, 0, 14, 6))
    furnace.write_anim_frames(os.path.join(MANSION, 'anim/torch'), [furnace.tiles_of(f, 0, 1) for f in flames],
                              KEYS, furnace.PALETTE)

    def top(*tiles):
        return [(first + t) | (MANSION_PAL << 12) for t in tiles]

    def piece(quarters, mirror=False):
        if mirror:
            quarters = [quarters[1], quarters[0], quarters[3], quarters[2]]
        out = []
        for q in quarters:
            if q is None:
                out.append(BLANK_TILE)
            else:
                n, flip = q
                out.append((PRIMARY_TILES + RAILING_SLOTS[n]) | (MANSION_PAL << 12) | (HFLIP if flip != mirror else 0))
        return out
    floor = layers_of(MANSION, 0x286)[:4]       # The basement floor
    wall = layers_of(MANSION, 0x2AA)[4:]        # The wall's face (drawn on its top layer)
    rail, side, corner, front, gate, lock = railing
    add_metatiles(MANSION, MANSION_METATILES, [
        floor + top(0, 1, 2, 3),                 # 0x384 torch, unlit
        floor + top(4, 5, 2, 3),                 # 0x385 torch, lit
        wall + top(6, 7, 8, 9),                  # 0x386 moon
        wall + top(10, 11, 12, 13),              # 0x387 sun
        floor + piece(rail),                     # 0x388 railing, the back
    ], MB_NORMAL | LAYER_COVERED)
    # The stair down's pieces that show the floor, on the basement's floor instead of 1F's (same attributes)
    attrs = open(os.path.join(MANSION, 'metatile_attributes.bin'), 'rb').read()
    for n, stair_piece in enumerate((0x2D2, 0x2DA)):
        i = stair_piece - PRIMARY_METATILES
        add_metatiles(MANSION, MANSION_METATILES + 5 + n, [floor + layers_of(MANSION, stair_piece)[4:]],
                      struct.unpack('<I', attrs[i * 4:i * 4 + 4])[0])
    add_metatiles(MANSION, MANSION_METATILES + 7, [
        floor + piece(side),                     # 0x38B railing, left side
        floor + piece(side, True),               # 0x38C railing, right side
        floor + piece(corner),                   # 0x38D railing, back left corner
        floor + piece(corner, True),             # 0x38E railing, back right corner
        floor + piece(front),                    # 0x38F railing, front left corner
        floor + piece(front, True),              # 0x390 railing, front right corner
        floor + piece(gate),                     # 0x391 the gate
        floor + piece(lock),                     # 0x392 the gate, padlocked
    ], MB_NORMAL | LAYER_COVERED)
    print('Mansion: torch 0x384/0x385, moon 0x386, sun 0x387, stair pieces 0x389/0x38A, railing 0x388 and '
          '0x38B-0x392 (%d tiles)' % len(railing_tiles))


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
    # The ladder up's top half standing on the chamber's floor, not on a scrap of the rock it leans on elsewhere
    add_metatiles(CAVE, CAVE_METATILES + 1, [layers_of(CAVE, 0x281)[:4] + layers_of(CAVE, 0x28E)[4:]],
                  attr_of(CAVE, 0x28E))
    print('Cerulean Cave: stepping stone 0x306, ladder top on the floor 0x307')


def attr_of(folder, metatile_id):
    i = metatile_id - PRIMARY_METATILES
    return struct.unpack('<I', open(os.path.join(folder, 'metatile_attributes.bin'), 'rb').read()[i * 4:i * 4 + 4])[0]


# --- Mt. Ember (B4F) ------------------------------------------------------------------------------------------------
EMBER = os.path.join(ROOT, 'data/tilesets/secondary/mt_ember_frlg')
EMBER_METATILES = 226


def draw_ember():
    """No new tiles: the ladder up's top half on the altar room's floor (0x281) instead of a scrap of rock."""
    add_metatiles(EMBER, EMBER_METATILES, [layers_of(EMBER, 0x281)[:4] + layers_of(EMBER, 0x28E)[4:]],
                  attr_of(EMBER, 0x28E))
    print('Mt. Ember: ladder top on the floor 0x362')


# --- Generic building 2 (1F) ----------------------------------------------------------------------------------------
GATE = os.path.join(ROOT, 'data/tilesets/secondary/generic_building_2_frlg')
GATE_METATILES = 184
BOX = (0x0DD, 0x0DE, 0x0ED, 0x0EE)  # The building tileset's cardboard box (metatile 0x181), palette 1


SWITCH_SLOTS = range(152, 160)  # The building tileset's last eight tiles, empty and unused
GATE_WALL = 0x28F               # The wall above the dado rail, where the switch is mounted
SWITCH_KEYS = '.Kgsw' + 'xxxxxxxx' + 'Rr' + 'h'  # The primary's palette 0: outline, greys, white, red, wall shade


def wall_switch(pulled):
    """A small breaker box mounted on the wall above the dado rail, its red lever up (or pulled down), with a drop
    shadow on the wall to the right and below. All inside the one square, nothing over the next."""
    art = [['.'] * 16 for _ in range(16)]
    for y in range(2, 13):
        for x in range(4, 14):
            art[y][x] = 'h' if (x == 13 or y == 12) else '.'
    for y in range(1, 12):
        for x in range(3, 13):
            edge = x in (3, 12) or y in (1, 11)
            art[y][x] = 'K' if edge else 'w' if (x == 4 or y == 2) else 'h' if (x == 11 or y == 10) else 's'
    for x, y in ((5, 3), (10, 3), (5, 9), (10, 9)):
        art[y][x] = 'g'                                            # Screws
    for y in range(4, 9):
        art[y][7], art[y][8] = 'K', 'K'                           # The lever's slot
    arm, knob = ((range(3, 6), 1) if not pulled else (range(7, 10), 9))
    for y in arm:
        art[y][7], art[y][8] = 'g', 'g'
    for dy, row in enumerate(('.KK.', 'KrRK', 'KRRK') if not pulled else ('KrRK', 'KRRK', '.KK.')):
        for dx, c in enumerate(row):
            if c != '.':
                art[knob + dy][6 + dx] = c
    return [''.join(r) for r in art]


def draw_gate():
    """The building tileset's box, standing on the gate's floor (0x281); the wall switch, up and pulled."""
    floor = layers_of(GATE, 0x281)[:4]
    add_metatiles(GATE, GATE_METATILES, [floor + [t | (1 << 12) for t in BOX]], MB_NORMAL | LAYER_COVERED)
    up, down = (quads(wall_switch(p), SWITCH_KEYS) for p in (False, True))
    put_tiles(GATE, list(SWITCH_SLOTS), up + down)
    wall = layers_of(GATE, GATE_WALL)[:4]
    switch = lambda first: wall + [(PRIMARY_TILES + SWITCH_SLOTS[first + n]) for n in range(4)]
    add_metatiles(GATE, GATE_METATILES + 1, [switch(0), switch(4)], attr_of(GATE, GATE_WALL))
    print('Generic building 2: box on the floor 0x338, wall switch 0x339 (up) / 0x33A (pulled)')


if __name__ == '__main__':
    draw_mansion()
    draw_cave()
    draw_gate()
    draw_ember()
