#!/usr/bin/env python3
"""Draws gTileset_RangerStationGrove, the ranger station's B3F (the overgrowth; docs/hack/act2_finale.md): an old
basement room with a wooden plank floor that trees and tall grass have grown up through.

Its own secondary tileset (over gTileset_BuildingFrlg), so no other map changes. Every metatile is composed as a
16x16 picture and then cut into tiles, deduplicated (flips included) and fitted into the secondary palettes 7-12:
    - weathered plank floors (variants, cracked and broken boards, shaded under the walls), drawn here;
    - the Pokémon Mansion basement's walls (gTileset_PokemonMansion), its floor swapped for the planks;
    - the overworld trees and tall grass (gTileset_General_Frlg), their grass swapped for the planks, roots
      splitting the boards at the trunks;
    - Cerulean Cave's ladders, their cave floor swapped for the planks;
    - a stone tablet (3x3 metatiles) with the hidden path engraved in it at a small scale (from
      compose_ranger_station.B3_PATH; the start marked in red, the way out in yellow).

Metatile ids are named in GROVE (below) and src/data/act2_station_layout.h's users go through
compose_ranger_station.py, which imports this file's ids. Re-running rebuilds the tileset from scratch.

    python3 tools/hack/draw_station_grove.py
"""
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # noqa: E402
from draw_mansion_furnace import write_png  # noqa: E402

ROOT = R.ROOT
FOLDER = os.path.join(ROOT, 'data/tilesets/secondary/ranger_station_grove')
FIRST_PAL, NUM_PALS = 7, 6          # FRLG secondary palettes 7-12
MAX_TILES = 384
FIRST_METATILE = 0x280

MB_NORMAL, MB_TALL_GRASS, MB_LADDER = 0x00, 0x02, 0x61

# The metatiles, in order (ids from 0x280)
NAMES = [
    'PLANK_0', 'PLANK_1', 'PLANK_2', 'PLANK_3', 'PLANK_CRACKED', 'PLANK_BROKEN',
    'PLANK_SHADE_TOP', 'PLANK_SHADE_LEFT', 'PLANK_SHADE_CORNER', 'PLANK_EDGE_BOTTOM', 'PLANK_EDGE_BOTTOM_LEFT',
    'WALL_TL_CAP', 'WALL_TL_CAP2', 'WALL_CAP', 'WALL_TR_CAP2', 'WALL_TR_CAP',
    'WALL_L_FACE', 'WALL_L_FACE2', 'WALL_FACE', 'WALL_R_FACE2', 'WALL_R',
    'WALL_L_BASE2', 'WALL_BASE', 'WALL_R_BASE2', 'WALL_L',
    'WALL_BL_EDGE', 'WALL_BR_EDGE', 'WALL_BL_CAP', 'WALL_BR_CAP', 'WALL_BL_FACE', 'WALL_BR_FACE',
    'WALL_BL_BASE', 'WALL_BR_BASE', 'WALL_B_CAP', 'WALL_B_FACE', 'WALL_B_BASE',
    'TREE_CROWN_L', 'TREE_CROWN_R', 'TREE_L', 'TREE_R', 'TREE_BETWEEN_L', 'TREE_BETWEEN_R', 'TREE_FOOT_L', 'TREE_FOOT_R',
    'TREE_CROWN_L_SH', 'TREE_CROWN_R_SH', 'TREE_L_SH', 'TREE_BETWEEN_L_SH', 'TREE_FOOT_L_SH',
    'FIELD', 'FIELD_N', 'FIELD_S', 'FIELD_W', 'FIELD_E', 'FIELD_NW', 'FIELD_NE', 'FIELD_SW', 'FIELD_SE', 'TUFT',
    'LADDER_UP_TOP', 'LADDER_UP', 'LADDER_DOWN',
    'TABLET_0', 'TABLET_1', 'TABLET_2', 'TABLET_3', 'TABLET_4', 'TABLET_5', 'TABLET_6', 'TABLET_7', 'TABLET_8',
]
GROVE = {name: FIRST_METATILE + i for i, name in enumerate(NAMES)}

# --- Sources ---------------------------------------------------------------------------------------------------------
_cache = {}


def tileset(symbol):
    if symbol not in _cache:
        _cache[symbol] = R.load_tileset(R.tileset_folder(symbol), symbol)
    return _cache[symbol]


def metatile(primary, secondary, mid):
    """A metatile drawn as the game draws it: 16 rows of 16 RGB pixels."""
    ptiles, ppals, pmeta = tileset(primary)
    stiles, spals, smeta = tileset(secondary)
    pals = [ppals[i] if i < 7 else spals[i] for i in range(16)]
    entries = pmeta[mid] if mid < 640 else smeta[mid - 640]
    img = [[pals[0][0]] * 16 for _ in range(16)]
    for layer in (0, 1):
        for k in range(4):
            e = entries[layer * 4 + k]
            t = e & 0x3FF
            rows, t = (ptiles, t) if t < 640 else (stiles, t - 640)
            n = len(rows[0]) // 8
            if (t // n) * 8 >= len(rows):
                continue
            hf, vf, p = e >> 10 & 1, e >> 11 & 1, e >> 12
            for y in range(8):
                for x in range(8):
                    c = rows[(t // n) * 8 + (7 - y if vf else y)][(t % n) * 8 + (7 - x if hf else x)]
                    if c == 0 and layer == 1:
                        continue
                    img[(k // 2) * 8 + y][(k % 2) * 8 + x] = pals[0][0] if c == 0 else pals[p][c]
    return img


def mansion(mid):
    return metatile('gTileset_BuildingFrlg', 'gTileset_PokemonMansion', mid)


def general(mid):
    return metatile('gTileset_General_Frlg', 'gTileset_CeruleanCave', mid)


def cave(mid):
    return metatile('gTileset_General_Frlg', 'gTileset_CeruleanCave', mid)


def colors(*images):
    return {c for img in images for row in img for c in row}


def swap(img, floor_colors, floor):
    """img with every pixel of a floor color replaced by the plank floor's pixel."""
    return [[floor[y][x] if img[y][x] in floor_colors else img[y][x] for x in range(16)] for y in range(16)]


GRASS_SHADOW = (24, 164, 106)   # The trees' and the tall grass's shadow on the grass: a shadow on the planks
GRASS_SPECK = (139, 222, 189)   # A light speck in the grass


# --- Planks ----------------------------------------------------------------------------------------------------------
# FireRed's own wooden floors (Bill's Sea Cottage, gTileset_SeaCottage 0x288) are two 8-pixel boards a metatile in
# three close tones: a fill, a mid tone along each board's edge and a dark seam between boards, with a dashed line
# on the board below the seam. These follow that, in an aged brown: low contrast, no speckle, joints only now and
# then, so the floor reads as one surface and every edge meets its neighbour.
SEAM, MID, WOOD = (123, 98, 65), (156, 128, 90), (180, 150, 106)
DARK, GAP = SEAM, (57, 41, 24)       # A crack's line; the dark under a broken board
LIGHT = (197, 172, 123)              # The lit cut end of a board (hole edges)
GRAIN = MID


def planks(joints=(), cracked=False, broken=False):
    """Two boards a metatile, after gTileset_SeaCottage's floor: [mid, fill x6, seam], [dashed, fill x6, mid].
    joints: (board, x) board ends, a seam with a mid tone after it. Rows line up with every other plank metatile,
    so they tile in any order."""
    img = [[WOOD] * 16 for _ in range(16)]
    for x in range(16):
        img[0][x] = MID
        img[7][x] = SEAM
        img[8][x] = MID if x % 4 in (0, 3) else SEAM
        img[15][x] = MID
    for board, x in joints:
        y0 = board * 8
        for y in range(y0 + 1, y0 + 7):
            img[y][x] = SEAM
            img[y][(x + 1) % 16] = MID
    if cracked:  # A split along the top board's grain
        for x, y in ((3, 3), (4, 3), (5, 4), (6, 4), (7, 4), (8, 5), (9, 5), (10, 4), (11, 4)):
            img[y][x] = SEAM
    if broken:   # A length of the lower board gone: the dark beneath, its far edge in shadow, the near cut end lit
        for y in range(9, 15):
            for x in range(4, 12):
                img[y][x] = GAP
        for x in range(4, 12):
            img[9][x] = SEAM
            img[15][x] = LIGHT
        img[10][4], img[10][11] = SEAM, SEAM
    return img


PLANK = planks()


def shade(img, factor=0.8):
    """In the shadow of a wall: FireRed's floors darken by about a fifth (gTileset_SeaCottage 0x288 -> 0x290)."""
    return [[tuple(int(v * factor) for v in c) for c in row] for row in img]


FOLIAGE_EDGE = (57, 90, 16)    # The trees' darkest green, their outline


def foot(img, floor, trunk_colors, right):
    """A tree's foot on the boards: the overworld tree's skirt (drawn to sit on grass) rounded off into the trunk, a
    dark edge where it is cut, and a soft shadow on the boards around the trunk. One half of the pair (right: the
    right half), measured across both halves so the curve and the shadow meet in the middle."""
    dark = shade(floor)
    out = [r[:] for r in img]
    foliage = lambda x, y: img[y][x] not in trunk_colors and img[y][x] != floor[y][x] and img[y][x] != dark[y][x]
    kept = [[False] * 16 for _ in range(16)]
    for y in range(16):
        for x in range(16):
            X = x + (16 if right else 0)
            if foliage(x, y) and ((X - 15.5) / 14.5) ** 2 + ((y - 2) / 10.5) ** 2 < 1:
                kept[y][x] = True
            elif foliage(x, y) or img[y][x] == dark[y][x] != floor[y][x]:  # Cut skirt, or the grass's shadow
                out[y][x] = dark[y][x] if ((X - 15.5) / 9) ** 2 + ((y - 13) / 3.2) ** 2 < 1 else floor[y][x]
    for y in range(16):
        for x in range(16):
            if kept[y][x] and any(0 <= x + dx < 16 and y + dy < 16 and foliage(x + dx, y + dy)
                                  and not kept[y + dy][x + dx] for dx, dy in ((0, 1), (-1, 0), (1, 0))):
                out[y][x] = FOLIAGE_EDGE
    return out


def roots(img, trunk_colors):
    """Boards split around the trunk: short seams running out from the trunk's foot along the boards."""
    out = [r[:] for r in img]
    feet = [(x, y) for y in range(16) for x in range(16) if img[y][x] in trunk_colors
            and (y == 15 or img[y + 1][x] not in trunk_colors)]
    for x, y in feet[::3]:
        for dx in (-2, -1, 1, 2):
            cx = x + dx
            if 0 <= cx < 16 and 0 <= y < 16 and out[y][cx] not in trunk_colors and out[y][cx] in (WOOD, MID):
                out[y][cx] = SEAM
    return out


# --- Ladders ----------------------------------------------------------------------------------------------------------
# A wooden ladder drawn like FireRed's objects: a dark outline, rails rounded with a lit left face and a shaded right,
# rungs with a highlight on top and a shadow under, and a shadow on the boards at its feet.
OUTLINE = (49, 32, 16)
RAIL_DARK, RAIL, RAIL_LIGHT = (115, 74, 41), (164, 115, 65), (213, 164, 106)


def ladder_up(top_floor, floor):
    """A ladder standing on the planks, its top rising out of sight into the opening above (fading into the dark).
    Two metatiles, top and bottom."""
    img = [r[:] for r in top_floor] + [r[:] for r in floor]
    for y in range(32):
        for x0 in (3, 10):  # Rails, 3 px and an outline either side
            img[y][x0], img[y][x0 + 4] = OUTLINE, OUTLINE
            img[y][x0 + 1], img[y][x0 + 2], img[y][x0 + 3] = RAIL_LIGHT, RAIL, RAIL_DARK
        if y % 5 == 3 and y < 30:  # Rungs between the rails
            for x in range(8, 10):
                img[y - 1][x] = OUTLINE
                img[y][x] = RAIL_LIGHT
                img[y + 1][x] = RAIL_DARK
                img[y + 2][x] = OUTLINE
            for x in (7, 10):
                img[y][x], img[y + 1][x] = RAIL, RAIL_DARK
    for x in range(2, 16):  # Its shadow on the boards
        img[31][x] = shade([[img[31][x]]], 0.75)[0][0] if img[31][x] not in (OUTLINE, RAIL, RAIL_LIGHT, RAIL_DARK) else OUTLINE
    fade = [GAP, GAP, (74, 52, 33), (74, 52, 33)]
    for y, color in enumerate(fade):  # Into the dark above
        for x in range(2, 15):
            img[y][x] = color if y < 2 else (blend(img[y][x], color) if img[y][x] in (RAIL, RAIL_LIGHT, RAIL_DARK, OUTLINE) else img[y][x])
    return img[:16], img[16:]


def blend(a, b):
    return tuple((x + y) // 2 for x, y in zip(a, b))


def hatch(floor):
    """A square hole cut through the boards, a ladder going down into the dark: the boards' cut ends around it,
    the hole's far side (its north wall) catching some light."""
    img = [r[:] for r in floor]
    x0, x1, y0, y1 = 2, 13, 2, 13
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            img[y][x] = GAP
    for x in range(x0, x1 + 1):
        img[y0][x] = DARK          # The far side's lip
        img[y0 + 1][x] = SEAM
        img[y1 + 1][x] = LIGHT     # The near boards' cut ends
    for y in range(y0, y1 + 2):
        img[y][x0 - 1] = SEAM
        img[y][x1 + 1] = LIGHT if y > y0 else SEAM
    for y in range(y0 + 1, y1 + 1):  # The ladder: rails and rungs, darker the deeper
        depth = (y - y0) / (y1 - y0)
        rail = RAIL_LIGHT if depth < 0.3 else RAIL if depth < 0.6 else RAIL_DARK
        for x in (4, 11):
            img[y][x] = rail
            img[y][x + 1] = RAIL_DARK if depth < 0.6 else (57, 41, 24)
        if (y - y0) % 4 == 1:
            for x in range(6, 11):
                img[y][x] = rail
    return img


# --- The tablet ------------------------------------------------------------------------------------------------------
# A stone slab drawn like FireRed's stonework: an outline, a bevel lit from the upper left and shaded at the lower
# right, a few flecks in the stone, and the path cut into its face as a groove (darker along the groove's top edge,
# where the cut throws its shadow).
STONE_OUT, STONE_HI, STONE, STONE_FLECK, STONE_LO = (57, 57, 65), (213, 213, 205), (180, 180, 172), (156, 156, 148), (123, 123, 123)
GROOVE, GROOVE_DEEP, START, EXIT = (106, 106, 115), (74, 74, 82), (222, 74, 49), (238, 205, 74)


def tablet(path_rows):
    """48x48: the slab (outline, bevel) and on its face the 9x9 grid's path (4 px a square, 1 px between) cut into
    the stone, the path's squares joined up; the start (bottom) red, the way out (top) yellow."""
    img = [[STONE] * 48 for _ in range(48)]
    rnd = random.Random(3)
    for _ in range(60):
        img[rnd.randint(3, 44)][rnd.randint(3, 44)] = STONE_FLECK
    for i in range(48):
        img[1][i] = img[i][1] = STONE_HI
        img[46][i] = img[i][46] = STONE_LO
        img[0][i] = img[47][i] = img[i][0] = img[i][47] = STONE_OUT
    img[1][46] = img[46][1] = STONE
    size = len(path_rows)
    cells = [(x, y) for y, row in enumerate(path_rows) for x, c in enumerate(row) if c == 'X']
    start = [c for c in cells if c[1] == size - 1][0]
    end = [c for c in cells if c[1] == 0][0]
    groove = set()

    def fill(x0, y0, w, h):
        groove.update((x, y) for y in range(y0, y0 + h) for x in range(x0, x0 + w))
    for (cx, cy) in cells:
        fill(2 + cx * 5, 2 + cy * 5, 4, 4)
        if (cx + 1, cy) in cells:
            fill(2 + cx * 5 + 4, 2 + cy * 5, 1, 4)
        if (cx, cy + 1) in cells:
            fill(2 + cx * 5, 2 + cy * 5 + 4, 4, 1)
    for (x, y) in groove:
        img[y][x] = GROOVE_DEEP if (x, y - 1) not in groove or (x - 1, y) not in groove else GROOVE
    for (cx, cy), color in ((start, START), (end, EXIT)):
        for y in range(3 + cy * 5, 2 + cy * 5 + 4):
            for x in range(3 + cx * 5, 2 + cx * 5 + 4):
                img[y][x] = color
    return [[img[y0 + y][x0:x0 + 16] for y in range(16)] for y0 in (0, 16, 32) for x0 in (0, 16, 32)]


# --- The overgrown field ---------------------------------------------------------------------------------------------
# Where the field is, the boards have rotted away and the tall grass grows out of the soil (gTileset_General_Frlg's
# own tall grass, 0x00D, on its own lawn). Its outer squares show the broken boards round the edge. Each side's edge
# wanders, but starts and ends 3 px in, so the squares along a side join up whatever their order.
EDGE_N = [3, 3, 4, 4, 3, 2, 2, 3, 4, 4, 5, 4, 3, 3, 2, 3]
EDGE_S = [3, 2, 2, 3, 4, 4, 3, 3, 2, 3, 4, 5, 4, 3, 3, 3]
EDGE_W = [3, 4, 4, 3, 2, 2, 3, 4, 4, 3, 3, 2, 3, 4, 4, 3]
EDGE_E = [3, 3, 2, 3, 4, 4, 3, 2, 2, 3, 4, 4, 3, 3, 2, 3]


def field(grass, floor, sides, inset=0):
    """The tall grass square with boards along the given sides ('N', 'S', 'E', 'W'). Where a board's broken end
    meets the soil: on the far side (N) its lit cut end shows and it shades the grass below; elsewhere a dark
    edge."""
    def soil(x, y):
        return not (('N' in sides and y < EDGE_N[x] + inset) or ('S' in sides and y > 15 - EDGE_S[x] - inset)
                    or ('W' in sides and x < EDGE_W[y] + inset) or ('E' in sides and x > 15 - EDGE_E[y] - inset))
    img = [[grass[y][x] if soil(x, y) else floor[y][x] for x in range(16)] for y in range(16)]
    for y in range(16):
        for x in range(16):
            if soil(x, y):
                if y > 0 and not soil(x, y - 1):
                    img[y][x] = GRASS_SHADOW
                continue
            below = y < 15 and soil(x, y + 1)
            beside = any(0 <= x + dx < 16 and soil(x + dx, y) for dx in (-1, 1)) or (y > 0 and soil(x, y - 1))
            if below:
                img[y][x] = LIGHT
            elif beside:
                img[y][x] = SEAM
    return img


FIELD_PARTS = {
    'FIELD': '', 'FIELD_N': 'N', 'FIELD_S': 'S', 'FIELD_W': 'W', 'FIELD_E': 'E',
    'FIELD_NW': 'NW', 'FIELD_NE': 'NE', 'FIELD_SW': 'SW', 'FIELD_SE': 'SE',
}


# --- Composition -----------------------------------------------------------------------------------------------------
def compose(path_rows):
    m286, m287 = mansion(0x286), mansion(0x287)
    grass_colors = colors(general(0x008), general(0x009), general(0x010), general(0x011)) | {GRASS_SPECK}
    shaded = shade(PLANK)

    def floored(mid, lit, dark):
        """A Mansion metatile with its floor (lit or shaded, matched pixel by pixel) swapped for the planks."""
        pic = mansion(mid)
        return [[lit[y][x] if pic[y][x] == m286[y][x] else dark[y][x] if pic[y][x] == m287[y][x] else pic[y][x]
                 for x in range(16)] for y in range(16)]
    pics = {
        'PLANK_0': PLANK, 'PLANK_1': planks([(0, 5)]), 'PLANK_2': planks([(1, 11)]),
        'PLANK_3': planks([(0, 2), (1, 9)]),
        'PLANK_CRACKED': planks([(1, 6)], cracked=True), 'PLANK_BROKEN': planks(broken=True),
        'PLANK_SHADE_TOP': shaded, 'PLANK_SHADE_LEFT': shade(planks([(1, 11)])),
        'PLANK_SHADE_CORNER': floored(0x290, PLANK, shaded),
        'PLANK_EDGE_BOTTOM': floored(0x29F, PLANK, shaded), 'PLANK_EDGE_BOTTOM_LEFT': floored(0x352, PLANK, shaded),
    }
    for name, mid in (('WALL_TL_CAP', 0x2A0), ('WALL_TL_CAP2', 0x2A1), ('WALL_CAP', 0x2A2), ('WALL_TR_CAP2', 0x2A3),
                      ('WALL_TR_CAP', 0x2A4), ('WALL_L_FACE', 0x2A8), ('WALL_L_FACE2', 0x2A9), ('WALL_FACE', 0x2AA),
                      ('WALL_R_FACE2', 0x2AB), ('WALL_R', 0x2AC), ('WALL_L_BASE2', 0x2B1), ('WALL_BASE', 0x2B2),
                      ('WALL_R_BASE2', 0x2B3), ('WALL_L', 0x2A8),
                      ('WALL_BL_EDGE', 0x32D), ('WALL_BR_EDGE', 0x32E), ('WALL_BL_CAP', 0x331), ('WALL_BR_CAP', 0x332),
                      ('WALL_BL_FACE', 0x339), ('WALL_BR_FACE', 0x33A), ('WALL_BL_BASE', 0x341),
                      ('WALL_BR_BASE', 0x342), ('WALL_B_CAP', 0x2A2), ('WALL_B_FACE', 0x2AA), ('WALL_B_BASE', 0x2B2)):
        pics[name] = floored(mid, PLANK, shaded)
    # Trees grown up through the floor. A column of trees, top to bottom (as the overworld stacks them): the crown's
    # tip, then per tree its body and either the next tree's tip over its foot (between) or, for the last, its foot
    # with the trunk on the boards. Their grass becomes the boards around them; _SH: on the boards in a wall's shadow
    # (the left wall's column and the row under the top wall), so the shadow runs on behind the trees.
    trunk_colors = {c for c in colors(general(0x024), general(0x025)) - grass_colors if c[0] >= c[1]}
    for name, mid in (('TREE_CROWN_L', 0x00E), ('TREE_CROWN_R', 0x00F), ('TREE_L', 0x01E), ('TREE_R', 0x01F),
                      ('TREE_BETWEEN_L', 0x016), ('TREE_BETWEEN_R', 0x017), ('TREE_FOOT_L', 0x024),
                      ('TREE_FOOT_R', 0x025)):
        for suffix, floor in (('', PLANK), ('_SH', shaded)):
            if suffix and name in ('TREE_R', 'TREE_BETWEEN_R', 'TREE_FOOT_R'):
                continue
            pic = swap(swap(general(mid), grass_colors, floor), {GRASS_SHADOW}, shade(floor))
            if 'FOOT' in name:
                pic = roots(foot(pic, floor, trunk_colors, name.endswith('_R')), trunk_colors)
            pics[name + suffix] = pic
    # The field: the overworld's tall grass in the soil where the boards are gone, and lone tufts in the gaps of
    # single broken boards
    grass = general(0x00D)
    for name, sides in FIELD_PARTS.items():
        pics[name] = field(grass, PLANK, sides)
    pics['TUFT'] = field(grass, PLANK, 'NSWE', inset=1)
    pics['LADDER_UP_TOP'], pics['LADDER_UP'] = ladder_up(PLANK, PLANK)
    pics['LADDER_DOWN'] = hatch(shaded)
    for i, pic in enumerate(tablet(path_rows)):
        pics['TABLET_%d' % i] = pic
    return pics


# --- Tiles and palettes ----------------------------------------------------------------------------------------------
def to5(c):
    return tuple(v >> 3 for v in c)


def cut(pics):
    """{metatile name: [4 tiles of 8x8 5-bit colors]}"""
    return {name: [[[to5(pic[y0 + y][x0 + x]) for x in range(8)] for y in range(8)]
                   for y0, x0 in ((0, 0), (0, 8), (8, 0), (8, 8))] for name, pic in pics.items()}


def pack(tiles):
    """Palettes (lists of up to 15 colors) holding every tile's colors, or None."""
    sets = sorted({frozenset(c for row in t for c in row) for t in tiles}, key=len, reverse=True)
    pals = []
    for s in sets:
        best = None
        for p in pals:
            union = p | s
            if len(union) <= 15 and (best is None or len(union) < len(best[1])):
                best = (p, union)
        if best:
            pals[pals.index(best[0])] = best[1]
        elif len(pals) < NUM_PALS:
            pals.append(set(s))
        else:
            return None
    return pals


def reduce_colors(tiles):
    """Merges the two closest colors (everywhere), so the tiles fit fewer palettes."""
    cols = sorted({c for t in tiles for row in t for c in row})
    best = None
    for i, a in enumerate(cols):
        for b in cols[i + 1:]:
            d = sum((x - y) ** 2 for x, y in zip(a, b))
            if best is None or d < best[0]:
                best = (d, a, b)
    _, a, b = best
    return [[[a if c == b else c for c in row] for row in t] for t in tiles]


def build(path_rows):
    pics = compose(path_rows)
    by_name = cut(pics)
    tiles = [t for name in NAMES for t in by_name[name]]
    while (pals := pack(tiles)) is None:
        tiles = reduce_colors(tiles)
    pals = [sorted(p) for p in pals]
    # Each tile: its palette, its pixels as indices; deduplicated with flips
    unique, entries = [], []
    for t in tiles:
        p = next(i for i, pal in enumerate(pals) if all(c in pal for row in t for c in row))
        px = tuple(tuple(pals[p].index(c) + 1 for c in row) for row in t)
        for hf in (0, 1):
            for vf in (0, 1):
                f = tuple(tuple(r[::-1]) if hf else r for r in (px[::-1] if vf else px))
                if f in unique:
                    entries.append((unique.index(f), hf, vf, p))
                    break
            else:
                continue
            break
        else:
            unique.append(px)
            entries.append((len(unique) - 1, 0, 0, p))
    assert len(unique) <= MAX_TILES, 'too many tiles: %d' % len(unique)
    return pals, unique, entries


def behavior(name):
    if name.startswith('FIELD') or name == 'TUFT':
        return MB_TALL_GRASS
    if name in ('LADDER_UP', 'LADDER_DOWN'):
        return MB_LADDER
    return MB_NORMAL


def main():
    import compose_ranger_station as C
    pals, unique, entries = build(C.B3_PATH)
    os.makedirs(os.path.join(FOLDER, 'palettes'), exist_ok=True)
    # tiles.png: 16 tiles a row
    rows_n = (len(unique) + 15) // 16
    image = [[0] * 128 for _ in range(rows_n * 8)]
    for n, px in enumerate(unique):
        for y in range(8):
            image[(n // 16) * 8 + y][(n % 16) * 8:(n % 16) * 8 + 8] = px[y]
    grey = b''.join(bytes((v * 17, v * 17, v * 17)) for v in range(16))
    write_png(os.path.join(FOLDER, 'tiles.png'), 128, image, grey)
    for i in range(16):
        slot = i - FIRST_PAL
        cols = [(0, 0, 0)] + ([(r << 3, g << 3, b << 3) for r, g, b in pals[slot]] if 0 <= slot < len(pals) else [])
        cols += [(0, 0, 0)] * (16 - len(cols))
        open(os.path.join(FOLDER, 'palettes/%02d.pal' % i), 'w').write(
            'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in cols))
    data, attrs = b'', b''
    for i, name in enumerate(NAMES):
        bottom = [(640 + t) | (hf << 10) | (vf << 11) | ((FIRST_PAL + p) << 12) for t, hf, vf, p in entries[i * 4:i * 4 + 4]]
        data += struct.pack('<8H', *(bottom + [0, 0, 0, 0]))
        attrs += struct.pack('<I', behavior(name))   # Layer type normal: the bottom layer, under everyone
    open(os.path.join(FOLDER, 'metatiles.bin'), 'wb').write(data)
    open(os.path.join(FOLDER, 'metatile_attributes.bin'), 'wb').write(attrs)
    print('gTileset_RangerStationGrove: %d metatiles, %d tiles, %d palettes' % (len(NAMES), len(unique), len(pals)))


if __name__ == '__main__':
    main()
