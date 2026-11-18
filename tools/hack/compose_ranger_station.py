#!/usr/bin/env python3
"""Lays out the abandoned ranger station (Act 2 finale, docs/hack/act2_finale.md): 1F and four basements.

    1F   the gate building (Saffron's north/south gate, LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE), widened on the left
         for a stair down behind the left counter (Underground Path entrance's), its north door walled up.
    B1F  the clock: a Pokémon Mansion basement room; 12 torches at the hours of a clock face round a stone mosaic
         whose middle slab slides away (a ladder down); two furnaces under a moon and a sun. A ring of corridor and
         rooms round it, a lone ranger's post: their things, cobwebs, and their notes.
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

import draw_station_grove
import draw_station_tiles as tiles
from draw_station_tiles import furniture_id, mosaic_id

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
GB2_SWITCH = 0x339        # The wall switch, its lever up (0x33A pulled: Act2Station_DrawFloor), draw_station_tiles.py


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
    g[F1_SWITCH] = blk(GB2_SWITCH, True, 0)
    return g


def open_1f(g):
    """What the switch does (Act2Station_DrawFloor draws the same): the counter's bottom end slides away."""
    g[F1_COUNTER] = blk(0x295, True, 0)
    g[F1_COUNTER[0], F1_COUNTER[1] + 1] = blk(0x283)


# ---------------------------------------------------------------------------------------------------------------------
# B1F: gTileset_BuildingFrlg + gTileset_PokemonMansion. The clock room (the old floor, 21x23) stands in the middle,
# a ring of corridor and rooms round it: an office, a hall with statues, a lab, a greenhouse, a study, and the stair
# hall in the bottom right corner where the stair from 1F comes down. Walls are Pokemon Mansion's pieces.
B1_W, B1_H = 33, 54
B1_CLOCK = (6, 15)          # The clock room's top left corner (its wall's cap), 21x23
CLOCK_W, CLOCK_H = 21, 23


def clock_xy(x, y):
    return B1_CLOCK[0] + x, B1_CLOCK[1] + y


B1_CENTER = clock_xy(10, 10)
B1_RADIUS = {12: (0, -6), 1: (3, -5), 2: (5, -3), 3: (6, 0), 4: (5, 3), 5: (3, 5),
             6: (0, 6), 7: (-3, 5), 8: (-5, 3), 9: (-6, 0), 10: (-5, -3), 11: (-3, -5)}
B1_TORCHES = [(B1_CENTER[0] + B1_RADIUS[h][0], B1_CENTER[1] + B1_RADIUS[h][1]) for h in range(1, 13)]
B1_FURNACE_MOON = clock_xy(6, 2)
B1_FURNACE_SUN = clock_xy(14, 2)
B1_LADDER = B1_CENTER             # The mosaic's middle: the slab, then the ladder down (MB_LADDER)
B1_STAIRS_UP = (29, 48)           # MB_UP_RIGHT_STAIR_WARP, in the stair hall
B1_CLOCK_DOOR = [clock_xy(x, 0) for x in (9, 10, 11)]  # The gap in the clock room's top wall
B1_TERMINAL = clock_xy(3, 2)      # The terminal on the clock room's top wall, where the notes found are typed up
B1_LAB_DOOR = (26, 10)            # The rooms the guards stand in the doors of (one square wide)
B1_STUDY_DOOR = (16, 42)
TORCH_UNLIT, TORCH_LIT, MOON, SUN = 0x384, 0x385, 0x386, 0x387
FURNACE_COLD, FURNACE_LIT = 0x382, 0x383
BASEMENT_FLOOR = 0x286

# Pokemon Mansion's wall pieces (as its B1F uses them). A wall running across is three rows: its cap, its face, its
# base (wainscot); the floor's row above the cap is the floor-edge row (the top of the wall seen from above).
M_FLOOR, M_SHADE = 0x286, 0x287
M_EDGE, M_EDGE_LEFT_END, M_EDGE_RIGHT_END, M_EDGE_BY_WALL = 0x29F, 0x351, 0x353, 0x352
M_WALL = (0x2A2, 0x2AA, 0x2B2)          # Cap, face, base
M_WALL_LEFT_END = (0x2F3, 0x2FB, 0x303)  # A wall's free end, the floor (a door) to its left
M_WALL_RIGHT_END = (0x2F4, 0x2FC, 0x304)
M_POST = 0x2B4                            # A wall running down (seen from above)
M_POST_THROUGH = 0x2EF                    # In the floor-edge row: a wall running down meets (or crosses) one across
M_POST_INTO_LEFT, M_POST_INTO_RIGHT = 0x2EE, 0x2ED  # ...one across that runs off to its left / its right
M_CAP_TEE = 0x2F7                         # The cap where a wall running down leaves one across
M_CAP_TOP_LEFT, M_CAP_TOP_RIGHT = 0x2F5, 0x2F6  # A box's top corners, its walls running down from them
M_OUTER_LEFT_TEE = (0x32D, 0x335, 0x2A8, 0x2A8)  # The outer left wall, a wall across leaving it (edge row to base)
M_OUTER_RIGHT_TEE = (0x32E, 0x336, 0x2AC, 0x2AC)
M_CAPS = {0x2A2, 0x2F3, 0x2F4, 0x2F5, 0x2F6, 0x2F7, 0x335, 0x336, 0x331, 0x332}
M_BASES = {0x2B2, 0x303, 0x304, 0x2B1, 0x2B3}


class Walls:
    """Pokemon Mansion walls on a grid of floor; shade() then gives the floor its shading and edges."""

    def __init__(self, g):
        self.g, self.cells = g, set()

    def put(self, x, y, metatile):
        self.g[x, y] = blk(metatile, True, 0)
        self.cells.add((x, y))

    def across(self, y, x0, x1, doors=()):
        """A wall across, its cap in row y, from x0 to x1; doors: the columns left open."""
        for x in range(x0, x1 + 1):
            if x in doors:
                continue
            pieces = M_WALL_RIGHT_END if x + 1 in doors else M_WALL_LEFT_END if x - 1 in doors else M_WALL
            for dy, m in enumerate(pieces):
                self.put(x, y + dy, m)

    def down(self, x, y0, y1):
        for y in range(y0, y1 + 1):
            self.put(x, y, M_POST)

    def shade(self, skip=()):
        g, cells = self.g, self.cells
        mt = lambda x, y: g[x, y] & 0x3FF
        floor = lambda x, y: (x, y) not in cells and (x, y) not in skip
        for y in range(1, g.h - 1):
            for x in range(1, g.w - 1):
                if not floor(x, y):
                    continue
                if mt(x, y + 1) in M_CAPS and (x, y + 1) in cells:
                    left_open = floor(x - 1, y) and mt(x - 1, y + 1) not in M_CAPS
                    right_open = floor(x + 1, y) and mt(x + 1, y + 1) not in M_CAPS
                    m = (M_EDGE_BY_WALL if (x - 1, y) in cells else M_EDGE_LEFT_END if left_open
                         else M_EDGE_RIGHT_END if right_open else M_EDGE)
                elif (x - 1, y) in cells or ((x, y - 1) in cells and mt(x, y - 1) in M_BASES):
                    m = M_SHADE
                else:
                    m = M_FLOOR
                g[x, y] = blk(m)


def mansion_frame(g, walls):
    """The outer wall round the whole floor (as Pokemon Mansion B1F's)."""
    W, H = g.w, g.h
    for x in range(W):
        for y, m in ((0, 0x2A2), (1, 0x2AA), (2, 0x2B2), (H - 3, 0x2A2), (H - 2, 0x2AA), (H - 1, 0x2B2)):
            walls.put(x, y, m)
    for y in range(1, H):
        walls.put(0, y, 0x2A8)
        walls.put(W - 1, y, 0x2AC)
    for (x, y), m in zip(((0, 0), (1, 0), (1, 1), (1, 2)), (0x2A0, 0x2A1, 0x2A9, 0x2B1)):
        walls.put(x, y, m)
    for (x, y), m in zip(((W - 2, 0), (W - 1, 0), (W - 2, 1), (W - 2, 2)), (0x2A3, 0x2A4, 0x2AB, 0x2B3)):
        walls.put(x, y, m)
    for y, m in zip(range(H - 4, H), (0x32D, 0x331, 0x339, 0x341)):
        walls.put(0, y, m)
    for y, m in zip(range(H - 4, H), (0x32E, 0x332, 0x33A, 0x342)):
        walls.put(W - 1, y, m)


def tee_outer(walls, y, left=True):
    """A wall across (its cap in row y) leaving the outer left (or right) wall."""
    x = 0 if left else walls.g.w - 1
    for dy, m in enumerate(M_OUTER_LEFT_TEE if left else M_OUTER_RIGHT_TEE):
        walls.put(x, y - 1 + dy, m)


def compose_clock_room():
    """The clock room as it was (its own walls drawn plainly; compose_b1f() gives it its box of walls)."""
    g = Grid(CLOCK_W, CLOCK_H, blk(BASEMENT_FLOOR))
    W, H = CLOCK_W, CLOCK_H
    for x in range(W):
        g[x, 3] = blk(M_SHADE)
        g[x, H - 4] = blk(M_EDGE)
    for y in range(3, H - 4):
        g[1, y] = blk(M_SHADE)
    g[1, H - 4] = blk(M_EDGE_BY_WALL)
    # Furnaces in the wainscot of the top wall, a moon and a sun painted above them (positions in the room's own
    # squares here: compose_b1f() moves them with the room)
    for x in range(W):
        g[x, 0], g[x, 1], g[x, 2] = (blk(m, True, 0) for m in M_WALL)
    for (x, y), emblem in (((6, 2), MOON), ((14, 2), SUN)):
        g[x, y] = blk(FURNACE_COLD, True, 0)
        g[x, y - 1] = blk(emblem, True, 0)
    # The terminal where the notes are typed up (Pokemon Mansion B1F's machine, its foot on the floor below)
    tx, ty = B1_TERMINAL[0] - B1_CLOCK[0], B1_TERMINAL[1] - B1_CLOCK[1]
    g[tx, ty], g[tx, ty + 1] = blk(0x31D, True, 0), blk(0x36C, True)
    cx, cy = B1_CENTER[0] - B1_CLOCK[0], B1_CENTER[1] - B1_CLOCK[1]
    for h in range(1, 13):
        g[cx + B1_RADIUS[h][0], cy + B1_RADIUS[h][1]] = blk(TORCH_UNLIT, True)
    # The stone mosaic in the middle of the torches (draw_station_mosaic.py), its slab right in the middle
    for dy in range(5):
        for dx in range(5):
            g[cx - 2 + dx, cy - 2 + dy] = blk(mosaic_id(dx, dy))
    return g


def compose_b1f(webs=True):
    g = Grid(B1_W, B1_H, blk(M_FLOOR))
    W, H = B1_W, B1_H
    walls = Walls(g)
    mansion_frame(g, walls)
    # Top band (rows 3-11): the office (x 1-10), the statue hall (x 12-20, open to the corridor), the lab (x 22-31)
    walls.put(11, 0, M_CAP_TEE)
    walls.down(11, 1, 7)
    walls.put(11, 8, M_POST_INTO_LEFT)
    walls.across(9, 1, 11, doors=(5, 6))
    for dy, m in enumerate(M_WALL_RIGHT_END):
        walls.put(11, 9 + dy, m)
    tee_outer(walls, 9)
    walls.put(21, 0, M_CAP_TEE)
    walls.down(21, 1, 7)
    walls.put(21, 8, M_POST_INTO_RIGHT)
    walls.across(9, 21, W - 2, doors=(B1_LAB_DOOR[0],))
    for dy, m in enumerate(M_WALL_LEFT_END):
        walls.put(21, 9 + dy, m)
    tee_outer(walls, 9, left=False)
    # The clock room: a box of walls, its door in the top wall between the furnaces
    ox, oy = B1_CLOCK
    x0, x1, y0, y1 = ox, ox + CLOCK_W - 1, oy, oy + CLOCK_H - 1
    walls.across(y0, x0 + 1, x1 - 1, doors=[x for x, _ in B1_CLOCK_DOOR])
    walls.put(x0, y0, M_CAP_TOP_LEFT)
    walls.put(x1, y0, M_CAP_TOP_RIGHT)
    walls.down(x0, y0 + 1, y1 - 4)
    walls.down(x1, y0 + 1, y1 - 4)
    walls.put(x0, y1 - 3, M_POST_INTO_RIGHT)
    walls.put(x1, y1 - 3, M_POST_INTO_LEFT)
    walls.across(y1 - 2, x0, x1)
    for dy in range(3):
        walls.put(x0, y1 - 2 + dy, M_WALL_LEFT_END[dy])
        walls.put(x1, y1 - 2 + dy, M_WALL_RIGHT_END[dy])
    # Bottom band (rows 41-50): the greenhouse (x 1-10), the study (x 12-20), the stair hall (x 22-31, open)
    walls.across(41, 1, 20, doors=(5, 6, B1_STUDY_DOOR[0]))
    tee_outer(walls, 41)
    walls.put(11, 40, M_POST_THROUGH)
    walls.put(11, 41, M_CAP_TEE)
    walls.down(11, 42, H - 5)
    walls.put(11, H - 4, M_POST_THROUGH)
    walls.put(21, 41, M_CAP_TOP_RIGHT)
    walls.down(21, 42, H - 5)
    walls.put(21, H - 4, M_POST_THROUGH)
    clock = (x0, x1, y0, y1)
    inside = {(x, y) for x in range(x0 + 1, x1) for y in range(y0 + 3, y1 - 2)}
    walls.shade(skip=inside)
    g[1, 3], g[W - 2, 3] = blk(0x290), blk(0x291)
    # The clock room's insides, and the furnaces and terminal in its top wall
    room = compose_clock_room()
    for y in range(3, CLOCK_H - 3):
        for x in range(1, CLOCK_W - 1):
            g[ox + x, oy + y] = room[x, y]
    for (x, y) in (B1_FURNACE_MOON, B1_FURNACE_SUN, B1_TERMINAL):
        g[x, y], g[x, y - 1] = room[x - ox, y - oy], room[x - ox, y - oy - 1]
    # Furniture, from Pokemon Mansion B1F: the office's desk and chairs, the hall's statues, the lab's machines on
    # its back wall, the greenhouse's potted plants, the study's table with an open book
    mansion = load('LAYOUT_POKEMON_MANSION_B1F')
    g.copy_from(mansion, 3, 6, 6, 4, 3, 4)
    for x in (13, 19):
        g.copy_from(mansion, 27, 4, 1, 2, x, 4)
    g.copy_from(mansion, 19, 1, 6, 3, 24, 1)
    g.copy_from(mansion, 1, 17, 10, 5, 1, 44)
    g.copy_from(mansion, 19, 26, 5, 4, 14, 45)
    # The stair up, in an alcove in the bottom right corner (Pokemon Mansion B1F's, x 33-37, y 25-31)
    g.copy_from(mansion, 33, 25, 5, 7, B1_STAIRS_UP[0] - 1, B1_STAIRS_UP[1] - 4)
    furnish_b1f(g)
    if webs:
        targets = web_targets()
        for x, y, corner in B1_WEBS:
            g[x, y] = (g[x, y] & ~0x3FF) | tiles.webbed_id(g[x, y] & 0x3FF, corner, targets)
    return g


# The lone ranger's things round the ring (FURNITURE, on the basement floor): (x, y, name)
B1_FURNITURE = [
    # The office: the ranger's shelf of field guides, a cabinet, the TV, crates of supplies never unpacked
    (2, 3, 'shelf_l'), (3, 3, 'shelf_r'), (8, 3, 'tv'), (9, 3, 'cabinet_l'), (10, 3, 'cabinet_r'),
    (1, 7, 'crate_l'), (9, 7, 'crate_r'), (10, 7, 'crate_l'),
    # The statue hall: the ranger's bed, between the statues, and a plant they kept alive
    (15, 3, 'bed_l'), (16, 3, 'bed_r'), (12, 3, 'plant'),
    # The kitchen nook in the corridor, under the office's wall: sink, stove, a cupboard
    (2, 12, 'sink'), (3, 12, 'stove'), (4, 12, 'cabinet_l'), (5, 12, 'cabinet_r'),
    # The lab: cabinets, crates
    (22, 3, 'cabinet_l'), (23, 3, 'cabinet_r'), (30, 7, 'crate_l'), (31, 7, 'crate_r'), (31, 6, 'crate_l'),
    # The study: a bookshelf
    (12, 44, 'shelf_l'), (13, 44, 'shelf_r'),
    # The stair hall: supplies stacked where they were carried down
    (22, 44, 'crate_l'), (23, 44, 'crate_r'), (22, 45, 'crate_r'), (25, 49, 'crate_l'),
]
# Cobwebs (x, y, corner): baked into whatever stands there (draw_station_tiles.py makes the webbed copies)
B1_WEBS = [
    # On the furniture
    (24, 2, 'TL'), (29, 2, 'TR'), (2, 3, 'TL'), (10, 3, 'TR'), (16, 3, 'TR'),
    (12, 44, 'TL'), (2, 44, 'TL'), (10, 47, 'TR'), (15, 46, 'TL'), (22, 44, 'TL'), (5, 12, 'TR'), (31, 6, 'TR'),
    # In the corners of the walls
    (1, 1, 'TL'), (31, 1, 'TR'), (20, 1, 'TR'), (12, 1, 'TL'), (1, 42, 'TL'), (12, 42, 'TL'), (20, 42, 'TR'),
    (7, 16, 'TL'), (25, 16, 'TR'),
]


def furnish_b1f(g):
    for x, y, name in B1_FURNITURE:
        floor = g[x, y] & 0x3FF
        assert floor in tiles.FURNITURE_FLOORS, 'B1F: %s at (%d, %d) is on 0x%X, not the floor' % (name, x, y, floor)
        g[x, y] = blk(furniture_id(name, floor), True)


def web_targets():
    """The (metatile, corner) pairs B1F's cobwebs hang on, in order (draw_station_tiles.py makes a webbed copy of
    each)."""
    g = compose_b1f(webs=False)
    out = []
    for x, y, corner in B1_WEBS:
        key = (g[x, y] & 0x3FF, corner)
        if key not in out:
            out.append(key)
    return out


# ---------------------------------------------------------------------------------------------------------------------
# B2F and B3F: gTileset_General_Frlg + gTileset_CeruleanCave
CAVE_FLOOR = 0x281
WATER = 0x12B
WATER_EDGES = {  # (rock/floor above, left, right, below) -> the pond water piece with banks on those sides
    (False, False, False, False): 0x12B, (True, False, False, False): 0x123, (False, True, False, False): 0x12A,
    (False, False, True, False): 0x12C, (True, True, False, False): 0x120, (True, False, True, False): 0x121,
    (False, True, True, False): 0x12B, (True, True, True, False): 0x123,
    # Banks below (FireRed's own turned upside down, draw_station_tiles.py)
    (False, False, False, True): 0x308, (False, True, False, True): 0x309, (False, False, True, True): 0x30A,
}
WATER_NUBS = {  # Pond water with a bank's corner nub where floor touches it only diagonally
    'TL': 0x131, 'TR': 0x130, 'BL': 0x30B, 'BR': 0x30C,
}
STONE = 0x306
LADDER_UP_TOP, LADDER_UP, LADDER_DOWN = 0x307, 0x296, 0x285  # 0x307: the ladder top on the floor (draw_station_tiles.py)


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
    return (g[x, y] & 0x3FF) in set(WATER_EDGES.values()) | set(WATER_NUBS.values()) | {STONE}


def edge_water(g):
    """Banks on the pond water where it meets rock or floor (above, left or right of it)."""
    for y in range(g.h):
        for x in range(g.w):
            if (g[x, y] & 0x3FF) != WATER:
                continue
            key = tuple(not is_water(g, x + dx, y + dy) for dx, dy in ((0, -1), (-1, 0), (1, 0))) + \
                (not is_water(g, x, y + 1) and y + 1 < g.h - 1,)  # (The bottom wall has its own bank)
            g[x, y] = blk(WATER_EDGES[key], True, 1)
    # Where floor touches open water only at a corner, the banks either side meet in a nub
    for y in range(1, g.h - 1):
        for x in range(1, g.w - 1):
            if (g[x, y] & 0x3FF) != WATER:
                continue
            nubs = [name for name, (dx, dy) in (('TL', (-1, -1)), ('TR', (1, -1)), ('BL', (-1, 1)), ('BR', (1, 1)))
                    if not is_water(g, x + dx, y + dy) and not g[x + dx, y + dy] & SOLID
                    and is_water(g, x + dx, y) and is_water(g, x, y + dy)]
            if nubs:
                assert len(nubs) == 1, 'pond water at %s: nubs %s' % ((x, y), nubs)
                g[x, y] = blk(WATER_NUBS[nubs[0]], True, 1)


# B2F: stones on a lattice two squares apart (a jump over one square of water). Rows of the lattice, top to bottom:
# x a stone; S and E jetties off the start and end platforms (floor, so the bank along a landing runs round them
# rather than breaking at a stone against it). Exactly five dead ends.
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
                g[ox + 2 * i, oy + 2 * j] = blk(CAVE_FLOOR if c in 'ES' else STONE)
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
            elif 0 <= x + dx < g.w and 0 <= y + dy < g.h and (g[x + dx, y + dy] & 0x3FF) in set(WATER_EDGES.values()) | set(WATER_NUBS.values()) \
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
        if (g[p] & 0x3FF) not in (STONE,):
            continue
        ways = list(moves(*p))
        if len(ways) == 1:
            (dx, dy) = ways[0][1]
            ends.append((p, (p[0] - dx, p[1] - dy)))
    assert len(ends) == 5, 'B2F: %d dead ends' % len(ends)
    return ends


# B3F: a 9x9 field of tall grass where only the path is safe; a stone tablet near the entrance shows the path at a
# small scale (gTileset_RangerStationGrove, tools/hack/draw_station_grove.py). Rows top to bottom: X the path. It
# starts at the bottom left (entered from below) and leaves at the top.
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
B3_W, B3_H = 15, 23
B3_FIELD = (3, 5)          # The grass field's top left
B3_SIZE = 9
B3_TABLET = (5, 16)        # The tablet's top left (3x3, solid)
B3_LADDER_UP = (12, 18)    # Arrival from B2F
B3_LADDER_DOWN = (9, 3)    # To B4F (above the path's exit)


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
    """An old basement room (the Mansion's walls) with a plank floor that trees and grass grow through."""
    G = draw_station_grove.GROVE
    W, H = B3_W, B3_H
    g = Grid(W, H, blk(0x008, True, 0))
    fx, fy = B3_FIELD

    def wall(x, y, name):
        g[x, y] = blk(G[name], True, 0)
    # The floor: plank variants, mostly whole; shaded along the top wall and the left wall
    for y in range(3, H - 4):
        for x in range(1, W - 1):
            n = (x * 7 + y * 13 + (x * y) % 5) % 23
            name = 'PLANK_CRACKED' if n == 3 else 'PLANK_BROKEN' if n == 17 and y > fy + B3_SIZE else 'PLANK_%d' % (n % 4)
            g[x, y] = blk(G[name])
        g[1, y] = blk(G['PLANK_SHADE_LEFT'])
    for x in range(2, W - 1):
        g[x, 3] = blk(G['PLANK_SHADE_TOP'])
    g[1, 3] = blk(G['PLANK_SHADE_CORNER'])
    # The walls (laid out like the Pokémon Mansion basement's, as on B1F)
    for x in range(W):
        wall(x, 0, 'WALL_CAP')
        wall(x, 1, 'WALL_FACE')
        wall(x, 2, 'WALL_BASE')
        wall(x, H - 3, 'WALL_B_CAP')
        wall(x, H - 2, 'WALL_B_FACE')
        wall(x, H - 1, 'WALL_B_BASE')
        g[x, H - 4] = blk(G['PLANK_EDGE_BOTTOM'])
    g[1, H - 4] = blk(G['PLANK_EDGE_BOTTOM_LEFT'])
    for y in range(1, H):
        wall(0, y, 'WALL_L')
        wall(W - 1, y, 'WALL_R')
    for (x, y), name in (((0, 0), 'WALL_TL_CAP'), ((1, 0), 'WALL_TL_CAP2'), ((1, 1), 'WALL_L_FACE2'),
                         ((1, 2), 'WALL_L_BASE2'), ((W - 2, 0), 'WALL_TR_CAP2'), ((W - 1, 0), 'WALL_TR_CAP'),
                         ((W - 2, 1), 'WALL_R_FACE2'), ((W - 2, 2), 'WALL_R_BASE2'),
                         ((0, H - 4), 'WALL_BL_EDGE'), ((0, H - 3), 'WALL_BL_CAP'), ((0, H - 2), 'WALL_BL_FACE'),
                         ((0, H - 1), 'WALL_BL_BASE'), ((W - 1, H - 4), 'WALL_BR_EDGE'), ((W - 1, H - 3), 'WALL_BR_CAP'),
                         ((W - 1, H - 2), 'WALL_BR_FACE'), ((W - 1, H - 1), 'WALL_BR_BASE')):
        wall(x, y, name)
    # Whole trees grown up through the floor, a column down both sides of the field: the top crown just below the
    # wall, the last tree's trunk on the boards (the column's rows: 1 + 2 a tree)
    def tree_column(x0, y0, trees):
        rows = ['TREE_CROWN'] + [p for i in range(trees) for p in ('TREE', 'TREE_BETWEEN' if i < trees - 1 else 'TREE_FOOT')]
        for k, part in enumerate(rows):
            # In the walls' shadow (the left wall's column, the row under the top wall) the boards behind are shaded
            for dx, side in ((0, '_L'), (1, '_R')):
                x, y = x0 + dx, y0 + k
                shaded = (x == 1 or y == 3) and part + side + '_SH' in G
                g[x, y] = blk(G[part + side + ('_SH' if shaded else '')], True)
    assert (fy + B3_SIZE - 3) % 2 == 1
    for x0 in (1, W - 3):
        tree_column(x0, 3, (fy + B3_SIZE - 3) // 2)
    tree_column(1, H - 7, 1)  # A lone tree by the entrance
    for xy in ((9, 16), (10, 16), (2, 15), (13, 15), (11, 19 - 4 + 3), (4, 18)):
        g[xy] = blk(G['TUFT'])
    # The field: soil and grass where the boards are gone, its edge pieces the boards' broken ends
    last = B3_SIZE - 1
    for y in range(B3_SIZE):
        for x in range(B3_SIZE):
            side = ('N' if y == 0 else 'S' if y == last else '') + ('W' if x == 0 else 'E' if x == last else '')
            g[fx + x, fy + y] = blk(G['FIELD_' + side if side else 'FIELD'])
    # The tablet, the ladders
    tx, ty = B3_TABLET
    for i in range(9):
        g[tx + i % 3, ty + i // 3] = blk(G['TABLET_%d' % i], True)
    g[B3_LADDER_UP[0], B3_LADDER_UP[1] - 1] = blk(G['LADDER_UP_TOP'], True)
    g[B3_LADDER_UP] = blk(G['LADDER_UP'])
    g[B3_LADDER_DOWN] = blk(G['LADDER_DOWN'])
    return g


# ---------------------------------------------------------------------------------------------------------------------
# B4F: gTileset_General_Frlg + gTileset_MtEmber
B4_EXTRA_ROWS = 5
B4_LADDER_UP = (11, 13)    # Arrival from B3F
B4_TRIO = {'HEATRAN': (5, 6), 'SUICUNE': (7, 6), 'VIRIZION': (9, 6)}
B4_SCENE = {'MEW': (7, 7), 'DAD': (6, 8), 'MOM': (8, 8)}   # The player lines up at (7, 9) below them
B4_TRIGGER_ROW = 10        # x 3-11: close enough that the trio is on screen
B4_TRIGGER_X = range(3, 12)


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
    g[B4_LADDER_UP[0], B4_LADDER_UP[1] - 1] = blk(0x362, True)  # The ladder top on the floor (draw_station_tiles.py)
    g[B4_LADDER_UP] = blk(0x297)
    # All round the chamber is Mt. Ember's rock (0x291), seen from above, as in the original room: the rims along
    # the walls' feet (0x289) are its edge, so blacking it out would leave them floating over nothing
    return g


# ---------------------------------------------------------------------------------------------------------------------
LAYOUTS = [
    ('LAYOUT_RANGER_STATION_1F', 'RangerStation_1F', 'gTileset_BuildingFrlg', 'gTileset_GenericBuilding2', 0x008, True),
    ('LAYOUT_RANGER_STATION_B1F', 'RangerStation_B1F', 'gTileset_BuildingFrlg', 'gTileset_PokemonMansion', 0x008, True),
    ('LAYOUT_RANGER_STATION_B2F', 'RangerStation_B2F', 'gTileset_General_Frlg', 'gTileset_CeruleanCave', 0x291, True),
    ('LAYOUT_RANGER_STATION_B3F', 'RangerStation_B3F', 'gTileset_BuildingFrlg', 'gTileset_RangerStationGrove', 0x008, True),
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
        '#define STATION_1F_SWITCH_X %d' % F1_SWITCH[0],
        '#define STATION_1F_SWITCH_Y %d' % F1_SWITCH[1],
        '',
        '// B1F: the torches at 1 to 12 o\'clock, then the moon and sun furnaces (VAR_ACT2_CLOCK\'s bits)',
        'static const s8 sClockPieces[][2] = {',
    ] + ['    %s,' % pt(p) for p in B1_TORCHES + [B1_FURNACE_MOON, B1_FURNACE_SUN]] + [
        '};',
        '// The mosaic\'s middle: its slab slides away from the ladder down',
        '#define STATION_MOSAIC_X %d' % B1_LADDER[0],
        '#define STATION_MOSAIC_Y %d' % B1_LADDER[1],
        '// The clock room, walls and all: no wild Pokémon there (only in the rooms and corridor round it)',
        '#define STATION_CLOCK_X %d' % B1_CLOCK[0],
        '#define STATION_CLOCK_Y %d' % B1_CLOCK[1],
        '#define STATION_CLOCK_W %d' % CLOCK_W,
        '#define STATION_CLOCK_H %d' % CLOCK_H,
        '',
        '// B2F: the stones at the dead ends, and the square of water beyond each where its Pokémon waits',
        'static const s8 sDeadEnds[][2][2] = {',
    ] + ['    {%s, %s},' % (pt(a), pt(b)) for a, b in b2_ends] + [
        '};',
        '',
        '// B3F: the field of tall grass; the path starts from below the field\'s bottom row and leaves above its top',
        '// row. Its squares, a row each (top to bottom), bit x for column x (the tablet shows the same B3_PATH)',
        '#define STATION_FIELD_X %d' % B3_FIELD[0],
        '#define STATION_FIELD_Y %d' % B3_FIELD[1],
        '#define STATION_FIELD_SIZE %d' % B3_SIZE,
        'static const u16 sOvergrowthPath[] = {%s};' % ', '.join(
            '0x%03X' % sum(1 << x for x, c in enumerate(row) if c == 'X') for row in B3_PATH),
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
