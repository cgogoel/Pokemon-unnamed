#!/usr/bin/env python3
"""Lays out the abandoned ranger station (Act 2 finale, docs/hack/act2_finale.md): 1F and four basements.

    1F   the gate building (Saffron's north/south gate, LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE), widened on the left
         for a stair down behind the left counter (Underground Path entrance's), its north door walled up.
    B1F  the clock: a Pokémon Mansion basement room; 12 torches at the hours of a clock face around a caged stair
         down; two furnaces under a moon and a sun.
    B2F  the flood: a Cerulean Cave chamber under water; stepping stones one jump apart, five dead ends.
    B3F  the overgrowth: a pattern on the floor, then a field of tall grass where only that pattern's path is safe.
    B4F  the chamber: Mt. Ember's Ruby Path altar room (B5F), stretched.

Uses the custom metatiles of tools/hack/draw_station_tiles.py (run that first). Also writes
src/data/act2_station_layout.h, the positions the station's C code needs (src/act2_station.c), so the two can't
drift apart. Re-running overwrites all five layouts: fine-tune in Porymap afterwards only if you won't re-run it.

    python3 tools/hack/compose_ranger_station.py
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10


def blk(metatile, solid=False, elevation=3):
    return metatile | (SOLID if solid else 0) | (elevation << 12)


def load(layout_id):
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    l = layouts[layout_id]
    w, h = l['width'], l['height']
    return w, h, list(struct.unpack('<%dH' % (w * h), open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()))


class Grid:
    def __init__(self, w, h, fill):
        self.w, self.h = w, h
        self.cells = [fill] * (w * h)

    def __getitem__(self, xy):
        return self.cells[xy[1] * self.w + xy[0]]

    def __setitem__(self, xy, block):
        x, y = xy
        assert 0 <= x < self.w and 0 <= y < self.h, xy
        self.cells[y * self.w + x] = block

    def copy_from(self, src, sx, sy, w, h, dx, dy):
        sw, _, blocks = src
        for y in range(h):
            for x in range(w):
                self[dx + x, dy + y] = blocks[(sy + y) * sw + sx + x]


# ---------------------------------------------------------------------------------------------------------------------
# 1F: gTileset_BuildingFrlg + gTileset_GenericBuilding2
F1_STAIRS = (2, 3)        # The stair down's warp tile (MB_DOWN_LEFT_STAIR_WARP: stand on it, press left)
F1_DOOR = (5, 9)          # The exit mats, (5..7, 9)
F1_COUNTER = (4, 8)       # The left counter's bottom end; with (4, 9) below it, slides away (ACT2_FINALE_BASEMENT_OPEN)
F1_SWITCH = (6, 1)        # The switch panel on the wall (a bg_event, faced from (6, 2))
GB2_BOX = 0x338           # A box on the floor (draw_station_tiles.py adds it to the generic building tileset)


def compose_1f():
    src = load('LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE')
    sw, _, s = src
    old = lambda x, y: s[y * sw + x]
    g = Grid(11, 11, blk(0x008, True, 0))
    # Columns: old 0-1 (left aisle) -> 0-3, old 2-6 (counter, middle, counter) -> 4-8, old 7-8 (right aisle) -> 9-10
    columns = [0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8]
    for x, ox in enumerate(columns):
        for y in range(11):
            g[x, y] = old(ox, y)
    # The left aisle: floor (the plants are gone), the window at its top
    for y in range(2, 10):
        for x in range(0, 4):
            g[x, y] = blk(0x281 if x else 0x282)
    for x in range(2, 4):
        g[x, 0], g[x, 1] = blk(0x287, True, 0), blk(0x28F, True, 0)
    # The right aisle: one plant left, the rest bare
    for y in range(2, 10):
        g[9, y] = old(7, y)
        g[10, y] = blk(0x282) if y not in (2, 3) else old(8, y)
    # North door walled up
    for x in range(5, 8):
        g[x, 0], g[x, 1] = blk(0x287, True, 0), blk(0x28F, True, 0)
    # The stair down (Underground Path entrance's), at the top of the left aisle
    stairs = load('LAYOUT_UNDERGROUND_PATH_ENTRANCE')
    g.copy_from(stairs, 5, 3, 3, 3, 0, 2)
    # Abandoned: boxes left behind
    for xy in ((0, 8), (0, 9), (1, 9), (10, 8), (10, 9), (7, 2)):
        g[xy] = blk(GB2_BOX, True)
    # The switch panel on the wall above the middle
    g[F1_SWITCH] = blk(0x2DC, True, 0)
    return g


def open_1f(g):
    """What the switch does (Act2Station_DrawFloor draws the same): the counter's bottom end slides away."""
    g[F1_COUNTER] = blk(0x295, True, 0)
    g[F1_COUNTER[0], F1_COUNTER[1] + 1] = blk(0x283)


# ---------------------------------------------------------------------------------------------------------------------
# B1F: gTileset_BuildingFrlg + gTileset_PokemonMansion
B1_W, B1_H = 21, 23
B1_CENTER = (10, 10)
B1_RADIUS = {12: (0, -6), 1: (3, -5), 2: (5, -3), 3: (6, 0), 4: (5, 3), 5: (3, 5),
             6: (0, 6), 7: (-3, 5), 8: (-5, 3), 9: (-6, 0), 10: (-5, -3), 11: (-3, -5)}
B1_TORCHES = [(B1_CENTER[0] + B1_RADIUS[h][0], B1_CENTER[1] + B1_RADIUS[h][1]) for h in range(1, 13)]
B1_FURNACE_MOON = (6, 2)
B1_FURNACE_SUN = (14, 2)
B1_GATE = [(9, 12), (10, 12), (11, 12)]
B1_STAIRS_DOWN = (11, 9)  # MB_DOWN_LEFT_STAIR_WARP
B1_STAIRS_UP = (17, 17)   # MB_UP_RIGHT_STAIR_WARP
TORCH_UNLIT, TORCH_LIT, MOON, SUN, BARS = 0x384, 0x385, 0x386, 0x387, 0x388
FURNACE_COLD, FURNACE_LIT = 0x382, 0x383
BASEMENT_FLOOR = 0x286
STAIR_WARP_BASEMENT, STAIR_SIDE_BASEMENT = 0x389, 0x38A  # Mansion 1F's 0x2D2 and 0x2DA on the basement floor


def compose_b1f():
    g = Grid(B1_W, B1_H, blk(0x286))
    W, H = B1_W, B1_H
    for x in range(W):
        g[x, 0], g[x, 1], g[x, 2] = blk(0x2A2, True, 0), blk(0x2AA, True, 0), blk(0x2B2, True, 0)
        g[x, H - 3], g[x, H - 2], g[x, H - 1] = blk(0x2A2, True, 0), blk(0x2AA, True, 0), blk(0x2B2, True, 0)
        g[x, 3] = blk(0x287)
        g[x, H - 4] = blk(0x29F)
    for y in range(1, H):
        g[0, y], g[W - 1, y] = blk(0x2A8, True, 0), blk(0x2AC, True, 0)
        if 3 <= y < H - 4:
            g[1, y] = blk(0x287)
    g[0, 0], g[1, 0], g[1, 1], g[1, 2], g[1, 3] = (blk(m, True, 0) for m in (0x2A0, 0x2A1, 0x2A9, 0x2B1, 0x290))
    g[1, 3] = blk(0x290)
    g[W - 2, 0], g[W - 1, 0], g[W - 2, 1], g[W - 2, 2] = (blk(m, True, 0) for m in (0x2A3, 0x2A4, 0x2AB, 0x2B3))
    g[W - 2, 3] = blk(0x291)
    g[0, H - 4], g[0, H - 3], g[0, H - 2], g[0, H - 1] = (blk(m, True, 0) for m in (0x32D, 0x331, 0x339, 0x341))
    g[1, H - 4] = blk(0x352)
    g[W - 1, H - 4], g[W - 1, H - 3], g[W - 1, H - 2], g[W - 1, H - 1] = (blk(m, True, 0) for m in (0x32E, 0x332, 0x33A, 0x342))
    # The stair up, in an alcove in the bottom right corner (Pokémon Mansion B1F's, x 33-37, y 25-31)
    g.copy_from(load('LAYOUT_POKEMON_MANSION_B1F'), 33, 25, 5, 7, 16, 13)
    # Furnaces in the wainscot of the top wall, a moon and a sun painted above them
    for (x, y), emblem in ((B1_FURNACE_MOON, MOON), (B1_FURNACE_SUN, SUN)):
        g[x, y] = blk(FURNACE_COLD, True, 0)
        g[x, y - 1] = blk(emblem, True, 0)
    for xy in B1_TORCHES:
        g[xy] = blk(TORCH_UNLIT, True)
    # The caged stair down in the middle (Pokémon Mansion 1F's stair down)
    cx, cy = B1_CENTER
    for x in range(cx - 2, cx + 3):
        for y in range(cy - 2, cy + 3):
            g[x, y] = blk(BARS, True)
    mansion_1f = load('LAYOUT_POKEMON_MANSION_1F')
    g.copy_from(mansion_1f, 23, 27, 3, 2, cx - 1, cy - 1)
    g[cx + 1, cy - 1], g[cx + 1, cy] = blk(STAIR_WARP_BASEMENT), blk(STAIR_SIDE_BASEMENT)
    for x in range(cx - 1, cx + 2):
        g[x, cy + 1] = blk(BASEMENT_FLOOR)
    return g


def open_b1f_gate(g):
    for xy in B1_GATE:
        g[xy] = blk(BASEMENT_FLOOR)


# ---------------------------------------------------------------------------------------------------------------------
# B2F and B3F: gTileset_General_Frlg + gTileset_CeruleanCave
CAVE_FLOOR = 0x281
WATER = 0x12B
WATER_EDGES = {  # (wall/floor above, left, right) -> the pond water piece with banks on those sides
    (False, False, False): 0x12B, (True, False, False): 0x123, (False, True, False): 0x12A,
    (False, False, True): 0x12C, (True, True, False): 0x120, (True, False, True): 0x121,
    (False, True, True): 0x12B, (True, True, True): 0x123,
}
STONE = 0x306
LADDER_UP_TOP, LADDER_UP, LADDER_DOWN = 0x28E, 0x296, 0x285


def cave_room(w, h):
    """A cave chamber: rock walls around the edge (the cave's raised rock, seen from inside)."""
    g = Grid(w, h, blk(CAVE_FLOOR))
    for x in range(w):
        g[x, 0] = blk(0x299, True, 0)
        g[x, h - 1] = blk(0x289, True, 0)
    for y in range(h):
        g[0, y] = blk(0x292, True, 0)
        g[w - 1, y] = blk(0x290, True, 0)
    g[0, 0], g[w - 1, 0] = blk(0x29B, True, 0), blk(0x29C, True, 0)
    g[0, h - 1], g[w - 1, h - 1] = blk(0x2A3, True, 0), blk(0x2A4, True, 0)
    return g


def is_water(g, x, y):
    return (g[x, y] & 0x3FF) in set(WATER_EDGES.values()) | {STONE}


def edge_water(g):
    """Banks on the pond water where it meets rock or floor (above, left or right of it)."""
    for y in range(g.h):
        for x in range(g.w):
            if (g[x, y] & 0x3FF) != WATER:
                continue
            key = tuple(not is_water(g, x + dx, y + dy) for dx, dy in ((0, -1), (-1, 0), (1, 0)))
            g[x, y] = blk(WATER_EDGES[key], True, 1)


# B2F: stones on a lattice two squares apart (a jump over one square of water). Rows of the lattice, top to bottom:
# x a stone, S where the start platform reaches it, E the end platform. Exactly five dead ends.
B2_LATTICE = [
    'xxxx.xxE',
    'x.x..x..',
    'x.xxxx.x',
    '....x..x',
    'xxxxxxxx',
    'S..x..x.',
]
B2_W, B2_H = 21, 17
B2_ORIGIN = (3, 3)         # Map position of the lattice's (0, 0)
B2_LADDER_UP = (1, 15)     # Arrival from B1F
B2_LADDER_DOWN = (19, 1)   # To B3F
B2_MEDKIT = (19, 5)        # An object
B2_START = [(x, y) for x in range(1, 5) for y in range(14, 16)]
B2_END = [(x, y) for x in range(18, 20) for y in range(1, 6)]


def compose_b2f():
    g = cave_room(B2_W, B2_H)
    for y in range(1, B2_H - 1):
        for x in range(1, B2_W - 1):
            g[x, y] = blk(WATER, True, 1)
    for xy in B2_START + B2_END:
        g[xy] = blk(CAVE_FLOOR)
    ox, oy = B2_ORIGIN
    for j, row in enumerate(B2_LATTICE):
        for i, c in enumerate(row):
            if c != '.':
                g[ox + 2 * i, oy + 2 * j] = blk(STONE)
    g[B2_LADDER_UP[0], B2_LADDER_UP[1] - 1] = blk(LADDER_UP_TOP, True)
    g[B2_LADDER_UP] = blk(LADDER_UP)
    g[B2_LADDER_DOWN] = blk(LADDER_DOWN)
    # The bottom wall's top edge reads as a bank where water meets it
    for x in range(1, B2_W - 1):
        if is_water(g, x, B2_H - 2):
            g[x, B2_H - 1] = blk(0x2FD, True, 0)
    edge_water(g)
    return g


def b2f_dead_ends(g):
    """Walk the chamber the way the player can (a step, or a jump over one square of water onto a stone or floor):
    every square reachable from the ladder, and the stones with only one way on (the dead ends), each with the
    square of water beyond it where its Pokémon waits."""
    def walkable(x, y):
        return 0 <= x < g.w and 0 <= y < g.h and not (g[x, y] & SOLID)

    def moves(x, y):
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            if walkable(x + dx, y + dy):
                yield (x + dx, y + dy), (dx, dy)
            elif 0 <= x + dx < g.w and 0 <= y + dy < g.h and (g[x + dx, y + dy] & 0x3FF) in WATER_EDGES.values() \
                    and walkable(x + 2 * dx, y + 2 * dy):
                yield (x + 2 * dx, y + 2 * dy), (dx, dy)
    seen, todo = {B2_LADDER_UP}, [B2_LADDER_UP]
    while todo:
        p = todo.pop()
        for q, _ in moves(*p):
            if q not in seen:
                seen.add(q)
                todo.append(q)
    assert B2_LADDER_DOWN in seen, 'B2F: the far end is out of reach'
    ends = []
    for p in sorted(seen, key=lambda p: (p[1], p[0])):
        if (g[p] & 0x3FF) != STONE:
            continue
        ways = list(moves(*p))
        if len(ways) == 1:
            (dx, dy) = ways[0][1]
            ends.append((p, (p[0] - dx, p[1] - dy)))
    assert len(ends) == 5, 'B2F: %d dead ends' % len(ends)
    return ends


# B3F: the floor pattern (bottom) and the field of tall grass (top) are the same 9x9 grid. Rows top to bottom:
# X the path. It starts at the bottom left (entered from below) and leaves at the top.
B3_PATH = [
    '......X..',
    '.XXXX.X..',
    '.X..X.X..',
    '.X..X.XXX',
    '.X..X...X',
    '.XX.XXX.X',
    '..X...X.X',
    'XXX...XXX',
    'X........',
]
B3_W, B3_H = 15, 28
B3_FIELD = (3, 3)          # The grass field's top left
B3_PATTERN = (3, 16)       # The pattern's top left
B3_SIZE = 9
B3_LADDER_UP = (13, 26)    # Arrival from B2F
B3_LADDER_DOWN = (9, 1)    # To B4F (above the path's exit)
B3_SIGN = (2, 25)
TALL_GRASS, GRASS = 0x00D, 0x008
PATTERN_FLOOR, PATTERN_PATH = 0x2D3, 0x0BC


def b3f_path():
    cells = [(x, y) for y, row in enumerate(B3_PATH) for x, c in enumerate(row) if c == 'X']
    start = [c for c in cells if c[1] == B3_SIZE - 1]
    end = [c for c in cells if c[1] == 0]
    assert len(start) == 1 and len(end) == 1
    # A simple path: from the start, each cell has exactly one way on
    order, prev = [start[0]], None
    while order[-1] != end[0]:
        x, y = order[-1]
        nxt = [c for c in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)) if c in cells and c != prev]
        assert len(nxt) == 1, 'B3F: the path branches or touches itself at %s' % ((x, y),)
        prev = order[-1]
        order.append(nxt[0])
    assert len(order) == len(cells), 'B3F: stray path cells'
    return order


def compose_b3f():
    g = cave_room(B3_W, B3_H)
    fx, fy = B3_FIELD
    px, py = B3_PATTERN
    path = b3f_path()
    # Overgrown: trees down both sides, grass everywhere
    for y in range(1, B3_H - 1):
        for x in range(1, B3_W - 1):
            g[x, y] = blk(GRASS)
    for y in range(1, fy + B3_SIZE):
        for x in (1, 2, B3_W - 3, B3_W - 2):
            left = x in (1, B3_W - 3)
            g[x, y] = blk((0x01E if left else 0x01F) if y % 2 else (0x016 if left else 0x017), True)
    # The field of tall grass, the strips above (the exit) and below (the start) it
    for y in range(B3_SIZE):
        for x in range(B3_SIZE):
            g[fx + x, fy + y] = blk(TALL_GRASS)
    # A row of trees between the field's start and the pattern, open at the field's left end
    for x in range(1, B3_W - 1):
        g[x, fy + B3_SIZE + 1] = blk(0x01C if x % 2 else 0x01D, True)
        g[x, fy + B3_SIZE + 2] = blk(0x024 if x % 2 else 0x025, True)
    for y in (fy + B3_SIZE + 1, fy + B3_SIZE + 2):
        g[B3_W - 2, y] = blk(GRASS)
    # The pattern, carved into a stone floor
    for y in range(-1, B3_SIZE + 1):
        for x in range(-1, B3_SIZE + 1):
            g[px + x, py + y] = blk(PATTERN_FLOOR)
    for x, y in path:
        g[px + x, py + y] = blk(PATTERN_PATH)
    g[B3_LADDER_UP[0], B3_LADDER_UP[1] - 1] = blk(LADDER_UP_TOP, True)
    g[B3_LADDER_UP] = blk(LADDER_UP)
    g[B3_LADDER_DOWN] = blk(LADDER_DOWN)
    g[B3_SIGN] = blk(0x2CD, True)
    return g


# ---------------------------------------------------------------------------------------------------------------------
# B4F: gTileset_General_Frlg + gTileset_MtEmber
B4_EXTRA_ROWS = 5
B4_LADDER_UP = (11, 13)    # Arrival from B3F
B4_TRIO = {'HEATRAN': (5, 6), 'SUICUNE': (7, 6), 'VIRIZION': (9, 6)}
B4_SCENE = {'MEW': (7, 9), 'DAD': (6, 10), 'MOM': (8, 10)}
B4_TRIGGER_ROW = 12        # x 3-10
B4_TRIGGER_X = range(3, 11)


def compose_b4f():
    src = load('LAYOUT_MT_EMBER_RUBY_PATH_B5F')
    sw, sh, s = src
    old = lambda x, y: s[y * sw + x]
    g = Grid(sw, sh + B4_EXTRA_ROWS, 0)
    # Rows 0-8 as they are (the altar, the floor's diamond), then the diamond's bottom (old rows 9-10) inside the
    # straight walls of old row 8, the extra rows, and the room's narrowing bottom (old rows 9-14) with a bare floor
    for y in range(9):
        for x in range(sw):
            g[x, y] = old(x, y)
    g[7, 7] = old(7, 6)    # No pedestal: the trio stands on the diamond
    for k, oy in enumerate((9, 10)):
        for x in range(sw):
            inner = 4 <= x <= 10 and (old(x, oy) & SOLID) == 0
            g[x, 9 + k] = old(x, oy) if inner else blk(0x281) if 3 <= x <= 11 else old(x, 8)
    g[11, 6], g[11, 7] = blk(0x281), blk(0x281)  # Its ladder (to B4F) moves to the bottom
    for k in range(B4_EXTRA_ROWS - 2):
        for x in range(sw):
            g[x, 11 + k] = old(x, 8) if not 3 <= x <= 11 else blk(0x281)
    for k, oy in enumerate(range(9, sh)):
        for x in range(sw):
            b = old(x, oy)
            g[x, 9 + B4_EXTRA_ROWS + k] = blk(0x281) if (b & 0x3FF) in (0x303, 0x300, 0x2FC, 0x301, 0x305, 0x304) else b
    g[B4_LADDER_UP[0], B4_LADDER_UP[1] - 1] = blk(0x28E, True)
    g[B4_LADDER_UP] = blk(0x297)
    return g


# ---------------------------------------------------------------------------------------------------------------------
LAYOUTS = [
    ('LAYOUT_RANGER_STATION_1F', 'RangerStation_1F', 'gTileset_BuildingFrlg', 'gTileset_GenericBuilding2', 0x008, True),
    ('LAYOUT_RANGER_STATION_B1F', 'RangerStation_B1F', 'gTileset_BuildingFrlg', 'gTileset_PokemonMansion', 0x008, True),
    ('LAYOUT_RANGER_STATION_B2F', 'RangerStation_B2F', 'gTileset_General_Frlg', 'gTileset_CeruleanCave', 0x291, True),
    ('LAYOUT_RANGER_STATION_B3F', 'RangerStation_B3F', 'gTileset_General_Frlg', 'gTileset_CeruleanCave', 0x291, True),
    ('LAYOUT_RANGER_STATION_B4F', 'RangerStation_B4F', 'gTileset_General_Frlg', 'gTileset_MtEmber', 0x291, True),
]


def write(layout_id, folder, primary, secondary, border, grid):
    path = os.path.join(ROOT, 'data/layouts/layouts.json')
    data = json.load(open(path))
    entry = {
        'id': layout_id, 'name': folder + '_Layout', 'width': grid.w, 'height': grid.h,
        'border_width': 2, 'border_height': 2, 'primary_tileset': primary, 'secondary_tileset': secondary,
        'border_filepath': 'data/layouts/%s/border.bin' % folder,
        'blockdata_filepath': 'data/layouts/%s/map.bin' % folder, 'layout_version': 'frlg',
    }
    for i, l in enumerate(data['layouts']):
        if l.get('id') == layout_id:
            data['layouts'][i] = entry
            break
    else:
        data['layouts'].append(entry)
    open(path, 'w').write(json.dumps(data, indent=2) + '\n')
    os.makedirs(os.path.join(ROOT, 'data/layouts', folder), exist_ok=True)
    open(os.path.join(ROOT, entry['blockdata_filepath']), 'wb').write(struct.pack('<%dH' % len(grid.cells), *grid.cells))
    open(os.path.join(ROOT, entry['border_filepath']), 'wb').write(struct.pack('<4H', *[blk(border, True, 0)] * 4))


def write_header(b2_ends):
    def pt(p):
        return '{%d, %d}' % p
    path = b3f_path()
    lines = [
        '// Generated by tools/hack/compose_ranger_station.py: positions on the ranger station\'s floors (docs/hack/act2_finale.md)',
        '',
        '// 1F: the counter that slides away from the stair down',
        '#define STATION_1F_COUNTER_X %d' % F1_COUNTER[0],
        '#define STATION_1F_COUNTER_Y %d' % F1_COUNTER[1],
        '',
        '// B1F: the torches at 1 to 12 o\'clock, then the moon and sun furnaces (VAR_ACT2_CLOCK\'s bits)',
        'static const s8 sClockPieces[][2] = {',
    ] + ['    %s,' % pt(p) for p in B1_TORCHES + [B1_FURNACE_MOON, B1_FURNACE_SUN]] + [
        '};',
        'static const s8 sClockGate[][2] = {%s};' % ', '.join(pt(p) for p in B1_GATE),
        '',
        '// B2F: the stones at the dead ends, and the square of water beyond each where its Pokémon waits',
        'static const s8 sDeadEnds[][2][2] = {',
    ] + ['    {%s, %s},' % (pt(a), pt(b)) for a, b in b2_ends] + [
        '};',
        '',
        '// B3F: the field of tall grass and the pattern on the floor (the same grid); the path starts from below the',
        '// field\'s bottom row and leaves above its top row',
        '#define STATION_FIELD_X %d' % B3_FIELD[0],
        '#define STATION_FIELD_Y %d' % B3_FIELD[1],
        '#define STATION_PATTERN_X %d' % B3_PATTERN[0],
        '#define STATION_PATTERN_Y %d' % B3_PATTERN[1],
        '#define STATION_FIELD_SIZE %d' % B3_SIZE,
        '#define STATION_PATTERN_PATH 0x%03X // The pattern\'s path squares' % PATTERN_PATH,
        '#define STATION_PATH_START_X %d' % (B3_FIELD[0] + path[0][0]),
        '#define STATION_PATH_START_Y %d // Below the field' % (B3_FIELD[1] + B3_SIZE),
        '',
    ]
    open(os.path.join(ROOT, 'src/data/act2_station_layout.h'), 'w').write('\n'.join(lines))


def main():
    grids = [compose_1f(), compose_b1f(), compose_b2f(), compose_b3f(), compose_b4f()]
    ends = b2f_dead_ends(grids[2])
    for (layout_id, folder, primary, secondary, border, _), grid in zip(LAYOUTS, grids):
        write(layout_id, folder, primary, secondary, border, grid)
    write_header(ends)
    print('B2F dead ends (stone, Pokemon):', ends)
    print('B3F path: %d steps' % len(b3f_path()))


if __name__ == '__main__':
    main()
