#!/usr/bin/env python3
"""Lays out a base design for Celadon University (Act 2) by stamping pieces of existing FireRed maps.

Writes data/layouts/CeladonUniversity/map.bin (+ border.bin) and stamps the university gate's front onto Celadon City's
north edge. Every piece is copied with its collision and elevation, so it plays like the original. This is a starting
point to refine in Porymap; re-running it overwrites the campus (not Celadon, which it only stamps once).

    python3 tools/hack/compose_university.py [--celadon]
"""
import json
import os
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
W, H = 64, 56
ELEV = 3 << 12
SOLID = 1 << 10


def load(layout_id):
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    l = layouts[layout_id]
    w, h = l['width'], l['height']
    path = os.path.join(ROOT, l['blockdata_filepath'])
    return w, h, list(struct.unpack('<%dH' % (w * h), open(path, 'rb').read())), path


CEL_W, CEL_H, CELADON, CELADON_PATH = load('LAYOUT_CELADON_CITY')
SAF_W, SAF_H, SAFFRON, _ = load('LAYOUT_SAFFRON_CITY')
R6_W, R6_H, ROUTE6, _ = load('LAYOUT_ROUTE6')

grid = [0] * (W * H)


def put(x, y, block):
    if 0 <= x < W and 0 <= y < H:
        grid[y * W + x] = block


def stamp(src, src_w, sx, sy, w, h, dx, dy):
    for y in range(h):
        for x in range(w):
            put(dx + x, dy + y, src[(sy + y) * src_w + sx + x])


def grass(x0, y0, x1, y1):
    pattern = ((0x009, 0x008), (0x011, 0x010))
    for y in range(y0, y1):
        for x in range(x0, x1):
            put(x, y, pattern[y % 2][x % 2] | ELEV)


def trees(x0, y0, x1, y1):
    for y in range(y0, y1):
        for x in range(x0, x1):
            put(x, y, ((0x01C, 0x01D), (0x014, 0x015))[y % 2][x % 2] | SOLID)


def fence(x0, y0, x1, y1, bottom_gap=(0, 0), top_gap=(0, 0)):
    # Celadon's pond fence: posts 0EC/0ED on top, sides 0F4/0F5, bottom corners 353/354, rails 0E7. Gaps are (x0, x1)
    for x in range(x0 + 1, x1 - 1):
        if not top_gap[0] <= x < top_gap[1]:
            put(x, y0, 0x0E7 | SOLID)
        if not bottom_gap[0] <= x < bottom_gap[1]:
            put(x, y1 - 1, 0x0E7 | SOLID)
    for y in range(y0 + 1, y1 - 1):
        put(x0, y, 0x0F4 | SOLID)
        put(x1 - 1, y, 0x0F5 | SOLID)
    put(x0, y0, 0x0EC | SOLID)
    put(x1 - 1, y0, 0x0ED | SOLID)
    put(x0, y1 - 1, 0x353 | SOLID)
    put(x1 - 1, y1 - 1, 0x354 | SOLID)


PAVED = set()


def path(x0, y0, x1, y1):
    for y in range(y0, y1):
        for x in range(x0, x1):
            PAVED.add((x, y))


def lay_paths():
    # Celadon's paving with its edges: top 166, bottom 176, left 16D, right 16F, corners 165/167/175/177
    edges = {
        (False, True, False, True): 0x165, (False, True, True, True): 0x166, (False, True, True, False): 0x167,
        (True, True, False, True): 0x16D, (True, True, True, True): 0x16E, (True, True, True, False): 0x16F,
        (True, False, False, True): 0x175, (True, False, True, True): 0x176, (True, False, True, False): 0x177,
    }
    for (x, y) in PAVED:
        key = ((x, y - 1) in PAVED, (x, y + 1) in PAVED, (x - 1, y) in PAVED, (x + 1, y) in PAVED)
        put(x, y, edges.get(key, 0x16E) | ELEV)


def tree(x, y):
    # A lone tree (Celadon's): canopy over the grass above, then trunk and shadow
    put(x, y, 0x00E | ELEV)
    put(x + 1, y, 0x00F | ELEV)
    put(x, y + 1, 0x01E | SOLID)
    put(x + 1, y + 1, 0x01F | SOLID)
    put(x, y + 2, 0x026 | SOLID)
    put(x + 1, y + 2, 0x027 | SOLID)


def bushes(x0, y0, x1, y1, block=0x005):
    for y in range(y0, y1):
        for x in range(x0, x1):
            put(x, y, block | (SOLID if block == 0x005 else ELEV))


def hedge(x0, y, n):
    # Three-tile hedge pieces: ends 13D/13F, middle 13E
    for i in range(n):
        put(x0 + i, y, (0x13D if i == 0 else 0x13F if i == n - 1 else 0x13E) | SOLID)


def pond(x0, y0, x1, y1, fenced=False):
    # Celadon's pond water: top 1A5-1A7, middle 1AD-1AF, bottom 2CB/2DD/2CC
    if fenced:
        fence(x0 - 1, y0 - 1, x1 + 1, y1)
    for y in range(y0, y1):
        row = (0x1A5, 0x1A6, 0x1A7) if y == y0 else (0x2CB, 0x2DD, 0x2CC) if y == y1 - 1 else (0x1AD, 0x1AE, 0x1AF)
        for x in range(x0, x1):
            put(x, y, (row[0] if x == x0 else row[2] if x == x1 - 1 else row[1]) | (ELEV if y == y0 else SOLID | ELEV))


def compose():
    grass(0, 0, W, H)
    trees(0, 0, W, 4)
    trees(0, H - 4, W, H)
    trees(0, 0, 4, H)
    trees(W - 4, 0, W, H)

    # Paths
    path(4, 29, 60, 31)        # The street in front of the main hall
    path(26, 29, 38, 37)       # Fountain square
    path(30, 37, 34, 50)       # Avenue from the gate
    path(15, 17, 17, 29)       # To the stadium
    path(8, 27, 10, 29)        # To the ranch
    path(46, 20, 48, 29)       # Beside the east wing, up to Literature
    path(36, 20, 48, 22)
    path(52, 17, 55, 29)       # To the Science Research Institute
    path(18, 47, 30, 49)       # To the hospital and the pool
    path(19, 43, 22, 47)
    path(8, 45, 10, 47)
    path(8, 47, 18, 49)
    path(34, 46, 46, 48)       # To the dorms
    path(48, 31, 50, 41)       # To Home Economics
    path(48, 39, 56, 41)
    path(8, 7, 24, 9)          # The stadium's track
    path(8, 14, 24, 16)
    path(8, 9, 10, 14)
    path(22, 9, 24, 14)
    lay_paths()

    # Stadium (top left), fenced
    fence(5, 5, 27, 18, bottom_gap=(15, 17))
    # Research ranch (left)
    fence(4, 19, 14, 28, bottom_gap=(8, 10))
    bushes(6, 21, 7, 22)
    bushes(11, 23, 12, 24)
    bushes(7, 24, 9, 25, 0x004)
    # Main hall: Department Store in the middle (the clock tower), Game Corner wings, glass greenhouse behind
    stamp(CELADON, CEL_W, 9, 5, 9, 11, 27, 18)
    stamp(CELADON, CEL_W, 38, 17, 3, 5, 17, 23)
    stamp(CELADON, CEL_W, 31, 17, 7, 6, 20, 23)
    stamp(CELADON, CEL_W, 31, 17, 7, 6, 36, 23)
    stamp(CELADON, CEL_W, 38, 17, 3, 5, 43, 23)
    stamp(CELADON, CEL_W, 24, 16, 6, 3, 20, 19)
    # Gyarados fountain in the square
    stamp(CELADON, CEL_W, 10, 18, 7, 5, 28, 31)
    # Literature (two houses) behind the east wing
    stamp(CELADON, CEL_W, 36, 25, 8, 6, 37, 14)
    # Science Research Institute (Condominiums), top right
    stamp(CELADON, CEL_W, 27, 4, 7, 9, 50, 8)
    # Hospital (Pokemon Center) and the pool, bottom left
    stamp(CELADON, CEL_W, 46, 7, 5, 5, 18, 38)
    pond(7, 38, 15, 44, fenced=True)
    # Reflecting pool along the avenue
    pond(25, 38, 29, 46)
    # Dorms (two houses) by the gate
    stamp(CELADON, CEL_W, 36, 25, 8, 6, 37, 40)
    # Home Economics (Gym) with its garden
    stamp(CELADON, CEL_W, 8, 25, 8, 6, 50, 33)
    pond(53, 42, 58, 46)
    bushes(51, 42, 52, 46, 0x004)
    bushes(50, 46, 59, 47)
    # The gate's back, as Saffron shows its south gate: the path ends at warp tiles in front of its roof
    # (warp tiles at (31,49) and (32,49)), with Celadon's paving instead of Saffron's
    stamp(SAFFRON, SAF_W, 32, 45, 6, 8, 29, 48)
    for x, block in zip(range(30, 34), (0x16D, 0x16E, 0x16E, 0x16F)):
        put(x, 48, block | ELEV)

    # Trees: lining the avenue, and groves filling the open grass
    for y in (37, 40, 43):
        tree(35, y)
    fill_with_trees()


def is_grass(x, y):
    return 0 <= x < W and 0 <= y < H and grid[y * W + x] & 0x3FF in (0x008, 0x009, 0x010, 0x011)


# Open lawns kept clear of trees: the stadium's field and the ranch
LAWNS = ((5, 5, 27, 18), (4, 19, 14, 28))


TREE_BLOCKS = (0x014, 0x015, 0x01C, 0x01D, 0x024, 0x025, 0x00E, 0x00F, 0x01E, 0x01F, 0x026, 0x027)


def fill_with_trees():
    # Groves of lone trees wherever they fit on open grass, keeping a one-tile walkway along paths, buildings and
    # water (trees may touch other trees), like the art's wooded campus
    def lawn(x, y):
        return any(a <= x < c and b <= y < d for (a, b, c, d) in LAWNS)

    def ok(x, y, dx, dy):
        if not (0 <= x < W and 0 <= y < H):
            return True
        if 0 <= dx < 2 and 0 <= dy < 3:
            return is_grass(x, y) and not lawn(x, y)
        return is_grass(x, y) or grid[y * W + x] & 0x3FF in TREE_BLOCKS

    for y in range(4, H - 6):
        for x in range(4, W - 5):
            if all(ok(x + dx, y + dy, dx, dy) for dx in range(-1, 3) for dy in range(-1, 4)):
                tree(x, y)


def write(path, blocks):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(struct.pack('<%dH' % len(blocks), *blocks))


def stamp_celadon_gate():
    # Route 6's gate front (roof, double door) over Celadon's north tree line and unused top walkway, doors at (38,5)
    for y in range(6):
        for x in range(6):
            CELADON[y * CEL_W + 36 + x] = ROUTE6[y * R6_W + 10 + x]
    write(CELADON_PATH, CELADON)


def main():
    compose()
    out = os.path.join(ROOT, 'data/layouts/CeladonUniversity')
    write(os.path.join(out, 'map.bin'), grid)
    tree_border = [0x01C | SOLID, 0x01D | SOLID, 0x014 | SOLID, 0x015 | SOLID]
    write(os.path.join(out, 'border.bin'), tree_border)
    if '--celadon' in sys.argv:
        stamp_celadon_gate()


if __name__ == '__main__':
    main()
