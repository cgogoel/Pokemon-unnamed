#!/usr/bin/env python3
"""Draws the way into the night bloom's glade (Act 2's Grass course) into the Viridian Forest tileset.

The way is the space between the trees, gone dark: no tree in the passage's column, the trees either side as they are,
and its ground fading from the grass into shadow and on into the dark down its middle, its sides blending back into the
plain grass by the trees either side (dithered, in palette 8's greens and darks):

    Viridian Forest's side (FOREST_ROWS, x 39-41, rows 0-4, at its north edge): the entrance from the grass at the
    bottom (row 4), the dark deepening up the passage.
    The glade's side (GLADE_ROWS, its south way out, x 9-11, rows 12-15): the entrance at the top, deepening south.

Palette 8 (the trees') gets DEEP in a free slot. Adds the tiles to tiles.png after the tileset's own (rows from
BASE_TILE_ROWS) and the metatiles after its own 43 (from 0x2AB). Re-running redraws them in place.
tools/hack/compose_glade.py uses them.

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
LAYER_NORMAL, LAYER_COVERED, LAYER_SPLIT = 0, 1, 2
HFLIP, VFLIP = 1 << 10, 1 << 11

FIRST = PRIMARY_TILES + BASE_METATILES  # 0x2AB

# Palette 8: 4-5 dark greens, 8 the darkest, 13-15 grass (light to dark); DEEP new
DARK, DEEP = 8, 9
NEW_COLORS = {DEEP: (32, 28, 24), 10: (255, 0, 255), 11: (255, 0, 255)}
RAMP = [13, 14, 15, 5, 4, DARK, DEEP]   # The grass into the dark
RAMP_PX = 7                             # Pixels a step
FADE = 16                               # The fade from the grass into the dark at the entrance, in pixels
FADE_SIDE = 18                          # and from the far side of the trees beside it
CORNER = 39                             # The entrance's corners' radius, in pixels

# The passage's metatiles (3 a row) and its neighbors' either side (the trees beside it, dark between their crowns
# toward the passage), each row: left, the passage's three, right
FOREST_ROWS = [[FIRST + r * 5 + i for i in range(5)] for r in range(5)]        # Rows 0-4, x 38-42
GLADE_ROWS = [[FIRST + 25 + r * 5 + i for i in range(5)] for r in range(4)]    # Rows 12-15, x 8-12
SIDE = 16                               # The neighbors' cells in the canvas either side of the passage
KEEP = {1, 2, 3, 4, 5, 6, 7}            # Palette 8's leaves and bark: what stays of the neighbors' trees
NEIGHBOR_DARKEST = 4                    # Between the neighbors' crowns it darkens only to RAMP's 4th step (deep green)

# The tree metatiles (compose_glade.py's): the neighbors either side, by row
CANOPY_A = [0x298, 0x299, 0x29A]
CANOPY_B = [0x290, 0x291, 0x292]
TOP = [0x288, 0x289, 0x28A]
TRUNK_1 = [0x2A0, 0x2A1, 0x2A2]
TRUNK_2 = [0x2A3, 0x2A4, 0x2A5]
FOREST_NEIGHBORS = [CANOPY_A, CANOPY_B, CANOPY_A, TRUNK_1, TRUNK_2]    # Columns end at x 38, start at x 42
GLADE_NEIGHBORS = [TOP, CANOPY_B, CANOPY_A, CANOPY_B]                  # Columns end at x 8, start at x 12
BAYER = [[0, 2], [3, 1]]


def noise(x, y, k=0):
    """A fixed pseudo-random 0-99 per pixel (the same every run)."""
    return ((x * 73856093) ^ (y * 19349663) ^ (k * 83492791)) % 100


def canvas(w, h, fill=0):
    return [[fill] * w for _ in range(h)]


def main():
    png = os.path.join(TILESET, 'tiles.png')
    width, rows, plte = read_png(png)
    per_row = width // 8
    rows = rows[:BASE_TILE_ROWS * 8]
    metatiles = open(os.path.join(TILESET, 'metatiles.bin'), 'rb').read()[:BASE_METATILES * 16]
    attrs = open(os.path.join(TILESET, 'metatile_attributes.bin'), 'rb').read()[:BASE_METATILES * 4]

    def tile(t):
        i = t - PRIMARY_TILES
        return [rows[(i // per_row) * 8 + y][(i % per_row) * 8:(i % per_row) * 8 + 8] for y in range(8)]

    def entry_px(e):
        """A metatile entry's 8x8 pixels if it's one of this tileset's palette 8 tiles (else None: the grass)."""
        t = e & 0x3FF
        if t < PRIMARY_TILES or (e >> 12) != PAL:
            return None
        px = tile(t)
        if e & HFLIP:
            px = [list(reversed(r)) for r in px]
        if e & VFLIP:
            px = list(reversed(px))
        return px

    def original(m):
        return list(struct.unpack_from('<8H', metatiles, (m - PRIMARY_TILES) * 16))

    new_tiles, index = [], {}

    def add(px):
        key = tuple(tuple(r) for r in px)
        if key not in index:
            new_tiles.append(px)
            index[key] = PRIMARY_TILES + BASE_TILE_ROWS * per_row + len(new_tiles) - 1
        return index[key] | (PAL << 12)

    def passage(h, depth_at):
        """The darkness, 80 x h (the passage and a cell of its neighbors either side): each pixel's step along RAMP
        from its depth (in pixels from the entrance), over FADE px from the entrance and FADE_SIDE from the
        neighbors' far sides (so the passage is dark to its edges, fading out across the trees beside it), the
        entrance's corners rounded (CORNER), dithered."""
        cv = []
        for y in range(h):
            row = []
            for x in range(48 + 2 * SIDE):
                e, side = depth_at(y) + 0.5, min(x + 0.5, 47.5 + 2 * SIDE - x)   # From the entrance, the sides
                if e < CORNER and side < CORNER:                         # The entrance's corners rounded
                    u, v = (CORNER - e) / CORNER, (CORNER - side) / CORNER
                    fade = (FADE * u + FADE_SIDE * v) / (u + v)          # Between the two fades round the corner
                    dist = (1 - (u * u + v * v) ** 0.5) * CORNER / fade
                else:
                    dist = min(e / FADE, side / FADE_SIDE)
                d = max(0.0, dist) * RAMP_PX * len(RAMP) + (noise(x, y) % 3)
                step, frac = int(d // RAMP_PX), (d % RAMP_PX) / RAMP_PX
                if frac * 4 > BAYER[y % 2][x % 2] + 0.5:
                    step += 1
                row.append(RAMP[max(0, min(step, len(RAMP) - 1))])
            cv.append(row)
        return cv

    forest = passage(80, lambda y: 79 - y)                  # Entrance at the bottom (row 4)
    glade = passage(64, lambda y: y)                        # Entrance at the top (row 12)
    out_meta, out_attr = bytearray(metatiles), bytearray(attrs)
    attrs_of = lambda m: struct.unpack_from('<I', attrs, (m - PRIMARY_TILES) * 4)[0]   # noqa: E731

    def neighbor(m, cv, ox, oy):
        """A tree beside the passage: its leaves and bark kept, what shows between them (the grass, the gaps between
        the crowns) taking the darkness's color there; quarters with no darkness on them stay as they were."""
        e = original(m)
        out = list(e)
        for k in range(4):
            dark = [cv[oy + k // 2 * 8 + y][ox + k % 2 * 8:ox + k % 2 * 8 + 8] for y in range(8)]
            if all(c == RAMP[0] for r in dark for c in r):
                continue
            px = [[None] * 8 for _ in range(8)]
            for layer in (0, 1):
                src = entry_px(e[layer * 4 + k])
                if src:
                    for y in range(8):
                        for x in range(8):
                            if src[y][x] in KEEP:
                                px[y][x] = src[y][x]
            shade = lambda c: RAMP[min(RAMP.index(c), NEIGHBOR_DARKEST)]   # noqa: E731 Shade, not a hole
            out[k] = add([[px[y][x] if px[y][x] is not None else shade(dark[y][x]) for x in range(8)] for y in range(8)])
            out[4 + k] = 0
        return out

    for cv, sources in ((forest, FOREST_NEIGHBORS), (glade, GLADE_NEIGHBORS)):
        for r, trees in enumerate(sources):
            for c in range(5):
                if c in (0, 4):
                    m = trees[2] if c == 0 else trees[0]          # The column ending on the left, starting on the right
                    out_meta += struct.pack('<8H', *neighbor(m, cv, c * 16, r * 16))
                    out_attr += struct.pack('<I', attrs_of(m))
                    continue
                quarters = [add([cv[r * 16 + q // 2 * 8 + y][c * 16 + q % 2 * 8:c * 16 + q % 2 * 8 + 8]
                                 for y in range(8)]) for q in range(4)]
                out_meta += struct.pack('<8H', *(quarters + [0, 0, 0, 0]))
                out_attr += struct.pack('<I', LAYER_COVERED << 29)

    tile_rows = (len(new_tiles) + per_row - 1) // per_row
    rows += [[0] * width for _ in range(tile_rows * 8)]
    for n, px in enumerate(new_tiles):
        for y in range(8):
            for x in range(8):
                rows[BASE_TILE_ROWS * 8 + (n // per_row) * 8 + y][(n % per_row) * 8 + x] = px[y][x]
    write_png(png, width, rows, plte)
    open(os.path.join(TILESET, 'metatiles.bin'), 'wb').write(out_meta)
    open(os.path.join(TILESET, 'metatile_attributes.bin'), 'wb').write(out_attr)
    pal_path = os.path.join(TILESET, 'palettes/%02d.pal' % PAL)
    lines = open(pal_path).read().split('\n')
    for slot, rgb in NEW_COLORS.items():
        lines[3 + slot] = '%d %d %d' % rgb
    open(pal_path, 'w').write('\n'.join(lines))
    print('passage: %d tiles, metatiles 0x%03X-0x%03X' % (len(new_tiles), FIRST, GLADE_ROWS[-1][-1]))


if __name__ == '__main__':
    main()
