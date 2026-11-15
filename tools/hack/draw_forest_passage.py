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
KEEP = {1, 2, 3, 4, 5, 6, 7, 8}         # Palette 8's leaves, bark and outline: the neighbors' trees, kept whole
OUTLINE_CLOSE = 5                       # The trees' silhouette closes over pockets up to about twice this, in px

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

    # The colors behind the neighbors' trees (the primary tileset's grass, palette 9's gaps), for the parts of a
    # quarter that keep their look when the quarter is redrawn in palette 8
    prim_dir = os.path.join(ROOT, 'data/tilesets/primary/general_frlg')
    pw, prows, _ = read_png(os.path.join(prim_dir, 'tiles.png'))

    def read_pal(path):
        lines = open(path).read().split('\n')
        return [tuple(map(int, line.split())) for line in lines[3:19]]
    pals = [read_pal(os.path.join(prim_dir if i < 7 else TILESET, 'palettes/%02d.pal' % i)) for i in range(13)]
    pal8 = list(pals[PAL])
    for slot, rgb in NEW_COLORS.items():
        pal8[slot] = rgb
    usable = [i for i in range(1, 16) if pal8[i] != (255, 0, 255)]

    def nearest8(rgb):
        return min(usable, key=lambda i: sum((a - b) ** 2 for a, b in zip(pal8[i], rgb)))

    def any_px(e):
        """(palette, 8x8 color indices) of any metatile entry."""
        t, p = e & 0x3FF, e >> 12
        if t < PRIMARY_TILES:
            n = pw // 8
            px = [prows[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]
        else:
            px = tile(t)
        if e & HFLIP:
            px = [list(reversed(r)) for r in px]
        if e & VFLIP:
            px = list(reversed(px))
        return p, px

    def neighbor_column(trees, cv, ox, depth_at, toward):
        """The trees beside the passage (one column of cells, top to bottom): everything inside the trees'
        silhouette (their leaves, bark and outline, and the pockets between the crowns) kept as it was. The open
        space outside it that runs on from the passage's edge takes the passage's darkness: its own color there near
        the entrance (the rounded arch), the depths further in. Quarters the darkness doesn't touch stay as they were.
        Returns each cell's metatile entries."""
        h = len(trees) * 16
        keep = [[None] * 16 for _ in range(h)]     # A kept palette 8 color, or None for the empty space
        rgb = [[None] * 16 for _ in range(h)]      # What the empty space looks like now
        for r, m in enumerate(trees):
            e = original(m)
            for k in range(4):
                for layer in (0, 1):
                    p, px = any_px(e[layer * 4 + k])
                    for y in range(8):
                        for x in range(8):
                            c = px[y][x]
                            if c == 0 and not (layer == 0 and p != PAL):
                                continue
                            Y, X = r * 16 + k // 2 * 8 + y, k % 2 * 8 + x
                            if p == PAL and c in KEEP:
                                keep[Y][X] = c
                            else:
                                keep[Y][X] = None
                                rgb[Y][X] = pals[p][c]
        # The trees' silhouette: their leaves, bark and outline, closed over the pockets between the crowns (a
        # dilation then an erosion by OUTLINE_CLOSE px; off the column on the passage's side the trees are mirrored,
        # so a pocket opening onto the passage closes too; on its far side, more trees). Everything inside it is protected, kept as it was.
        def solid(x, y, grid):
            if not 0 <= y < h:
                return True
            if not 0 <= x < 16:
                if (x < 0) == (toward == 'right'):
                    return True                                  # The far side: the rest of the trees
                x = 31 - x if x >= 16 else -1 - x                # The passage's side: the trees mirrored
            return grid[y][x]
        tree = [[keep[y][x] is not None for x in range(16)] for y in range(h)]
        rr = OUTLINE_CLOSE
        disk = [(dx, dy) for dy in range(-rr, rr + 1) for dx in range(-rr, rr + 1) if dx * dx + dy * dy <= rr * rr]
        grown = [[any(solid(x + dx, y + dy, tree) for dx, dy in disk) for x in range(16)] for y in range(h)]
        inside = [[all(solid(x + dx, y + dy, grown) for dx, dy in disk) or tree[y][x] for x in range(16)]
                  for y in range(h)]
        # The dark: the open space outside the silhouette, as far as it runs on from the passage's edge
        edge_x = 15 if toward == 'right' else 0
        seen, todo = set(), [(edge_x, y) for y in range(h) if not inside[y][edge_x]]
        while todo:
            x, y = todo.pop()
            if (x, y) in seen or not (0 <= x < 16 and 0 <= y < h) or inside[y][x]:
                continue
            seen.add((x, y))
            todo += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
        deep_x = ox + (16 if toward == 'right' else -16) + (8 if toward == 'right' else 8)
        cells = []
        for r, m in enumerate(trees):
            e = original(m)
            out = list(e)
            for k in range(4):
                px, changed = [], False
                for y in range(8):
                    row = []
                    for x in range(8):
                        Y, X = r * 16 + k // 2 * 8 + y, k % 2 * 8 + x
                        if keep[Y][X] is not None:
                            row.append(keep[Y][X])
                        elif (X, Y) in seen:
                            c = cv[Y][ox + X] if depth_at(Y) < CORNER else cv[Y][deep_x]
                            changed |= c != RAMP[0]
                            row.append(c)
                        else:
                            row.append(nearest8(rgb[Y][X]))
                    px.append(row)
                if changed:
                    out[k], out[4 + k] = add(px), 0
            cells.append(out)
        return cells

    for cv, sources, depth_at in ((forest, FOREST_NEIGHBORS, lambda y: 79 - y), (glade, GLADE_NEIGHBORS, lambda y: y)):
        left = neighbor_column([t[2] for t in sources], cv, 0, depth_at, 'right')      # The column ending at its left
        right = neighbor_column([t[0] for t in sources], cv, 64, depth_at, 'left')     # and the one starting at its right
        for r, trees in enumerate(sources):
            for c in range(5):
                if c in (0, 4):
                    out_meta += struct.pack('<8H', *(left[r] if c == 0 else right[r]))
                    out_attr += struct.pack('<I', attrs_of(trees[2] if c == 0 else trees[0]))
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
