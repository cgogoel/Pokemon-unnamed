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


def compose():
    grass(0, 0, W, H)
    trees(0, 0, W, 4)
    trees(0, H - 4, W, H)
    trees(0, 0, 4, H)
    trees(W - 4, 0, W, H)

    # Paths: from the gate up to the central building, across campus, out to each department
    path(30, 24, 34, 50)       # Main avenue from the gate
    path(6, 31, 58, 34)        # Cross avenue
    path(50, 13, 52, 31)       # To the Science Research Institute
    path(40, 11, 42, 31)       # To Literature
    path(9, 25, 11, 31)        # To the hospital
    path(34, 44, 52, 46)       # To Home Economics and the dorms
    path(49, 42, 51, 44)
    path(8, 7, 25, 9)          # Sports field track
    path(8, 15, 25, 17)
    path(8, 9, 10, 15)
    path(23, 9, 25, 15)
    path(15, 17, 17, 20)
    path(14, 34, 16, 40)       # To the ranch
    lay_paths()

    # Sports field (top left) and the research ranch (bottom left), fenced with gaps to their paths
    fence(6, 5, 27, 20, bottom_gap=(15, 17))
    fence(6, 39, 25, 50, top_gap=(14, 16))

    # Central building (Celadon Department Store) with the clock-tower fountain plaza in front
    stamp(CELADON, CEL_W, 9, 5, 9, 10, 27, 14)
    stamp(CELADON, CEL_W, 10, 18, 7, 5, 28, 25)
    # Science Research Institute (Celadon Condominiums), top right
    stamp(CELADON, CEL_W, 27, 4, 7, 8, 48, 5)
    # Literature department (two of Celadon's houses), top middle
    stamp(CELADON, CEL_W, 34, 7, 8, 5, 37, 6)
    # University hospital (Pokemon Center) and pool (Celadon's pond), left
    stamp(CELADON, CEL_W, 46, 7, 5, 5, 8, 20)
    stamp(CELADON, CEL_W, 23, 20, 8, 4, 14, 21)
    # Home economics (Celadon Gym), lower right
    stamp(CELADON, CEL_W, 8, 25, 8, 6, 46, 36)
    # Dorms (two of Celadon's row houses), by the gate
    stamp(CELADON, CEL_W, 32, 25, 8, 5, 37, 39)
    # The gate's back, as Saffron shows its south gate: the path ends at warp tiles in front of its roof
    # (warp tiles at (31,49) and (32,49)), with Celadon's paving instead of Saffron's
    stamp(SAFFRON, SAF_W, 32, 45, 6, 8, 29, 48)
    for x, block in zip(range(30, 34), (0x16D, 0x16E, 0x16E, 0x16F)):
        put(x, 48, block | ELEV)


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
