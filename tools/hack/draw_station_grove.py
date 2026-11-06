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
    'PINE_TOP_L', 'PINE_TOP_R', 'PINE_BOTTOM_L', 'PINE_BOTTOM_R',
    'TREE_ROW_TOP_L', 'TREE_ROW_TOP_R', 'TREE_ROW_BOTTOM_L', 'TREE_ROW_BOTTOM_R',
    'TALL_GRASS',
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


def shade(img, factor=0.72):
    return [[tuple(int(v * factor) for v in c) for c in row] for row in img]


GRASS_SHADOW = (24, 164, 106)   # The trees' and the tall grass's shadow on the grass: a shadow on the planks
GRASS_SPECK = (139, 222, 189)   # A light speck in the grass


# --- Planks ----------------------------------------------------------------------------------------------------------
SEAM, DARK, GRAIN, WOOD, LIGHT, GAP = (90, 66, 41), (123, 98, 57), (148, 123, 82), (180, 156, 106), (205, 180, 131), (41, 32, 24)


def planks(seed, cracked=False, broken=False):
    """Four boards a metatile, 4 pixels each (a seam and three of wood), their ends staggered."""
    rnd = random.Random(seed)
    img = [[WOOD] * 16 for _ in range(16)]
    for board in range(4):
        y0 = board * 4
        joint = rnd.choice([None, None, 3, 6, 9, 12])
        for x in range(16):
            img[y0][x] = SEAM
            img[y0 + 1][x] = LIGHT if rnd.random() < 0.8 else WOOD
        for _ in range(rnd.randint(1, 3)):  # Grain streaks
            gy, gx, length = y0 + rnd.randint(2, 3), rnd.randint(0, 13), rnd.randint(2, 6)
            for x in range(gx, min(16, gx + length)):
                img[gy][x] = GRAIN
        if rnd.random() < 0.5:  # A knot
            kx, ky = rnd.randint(1, 14), y0 + 2
            img[ky][kx] = DARK
        if joint is not None:
            for y in range(y0 + 1, y0 + 4):
                img[y][joint] = SEAM
            img[y0 + 1][joint + 1] = LIGHT
        for x in (0, 15):  # Nail heads at the ends, now and then
            if rnd.random() < 0.15:
                img[y0 + 2][x if x == 0 else x - 1] = DARK
    if cracked:
        x = rnd.randint(4, 10)
        for y in range(5, 12):
            img[y][x] = DARK if y % 4 else SEAM
            x += rnd.choice([-1, 0, 0, 1])
            x = max(1, min(14, x))
    if broken:
        for y in range(9, 12):
            for x in range(5 - (y == 9), 12 + (y == 11)):
                img[y][x] = GAP
        for x in range(4, 13):
            img[8][x] = DARK
        img[12][6], img[12][9] = DARK, DARK
    return img


PLANK = planks(1)


def roots(img, trunk_colors):
    """Boards split around the trunk: dark cracks running out from the trunk's foot into the planks."""
    out = [r[:] for r in img]
    feet = [(x, y) for y in range(16) for x in range(16) if img[y][x] in trunk_colors
            and (y == 15 or img[y + 1][x] not in trunk_colors)]
    rnd = random.Random(7)
    for x, y in feet[::2]:
        cx, cy = x, y + 1
        for _ in range(3):
            if not (0 <= cx < 16 and 0 <= cy < 16) or out[cy][cx] in trunk_colors:
                break
            out[cy][cx] = SEAM
            cx += rnd.choice([-1, 1])
            cy += rnd.choice([0, 1])
    return out


# --- The tablet ------------------------------------------------------------------------------------------------------
STONE_DARK, STONE_LIGHT, SLATE, SLATE_CELL, PATH_CELL, START, EXIT, OUTLINE = \
    (82, 82, 90), (164, 164, 172), (32, 40, 49), (57, 66, 82), (230, 230, 213), (230, 57, 41), (246, 205, 57), (16, 16, 24)


def tablet(path_rows):
    """48x48: a stone slab framing a dark slate with the 9x9 grid engraved in it (4 px a square, 1 px between),
    the path's squares light and joined up; the start (bottom) red, the way out (top) yellow."""
    img = [[SLATE] * 48 for _ in range(48)]
    for i in range(48):
        img[1][i] = img[i][1] = STONE_LIGHT
        img[46][i] = img[i][46] = STONE_DARK
        img[0][i] = img[47][i] = img[i][0] = img[i][47] = OUTLINE
    size = len(path_rows)
    cells = [(x, y) for y, row in enumerate(path_rows) for x, c in enumerate(row) if c == 'X']
    start = [c for c in cells if c[1] == size - 1][0]
    end = [c for c in cells if c[1] == 0][0]

    def fill(x0, y0, w, h, color):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                img[y][x] = color
    for cy in range(size):
        for cx in range(size):
            fill(2 + cx * 5, 2 + cy * 5, 4, 4, SLATE_CELL)
    for (cx, cy) in cells:
        fill(2 + cx * 5, 2 + cy * 5, 4, 4, START if (cx, cy) == start else EXIT if (cx, cy) == end else PATH_CELL)
        if (cx + 1, cy) in cells:
            fill(2 + cx * 5 + 4, 2 + cy * 5, 1, 4, PATH_CELL)
        if (cx, cy + 1) in cells:
            fill(2 + cx * 5, 2 + cy * 5 + 4, 4, 1, PATH_CELL)
    return [[img[y0 + y][x0:x0 + 16] for y in range(16)] for y0 in (0, 16, 32) for x0 in (0, 16, 32)]


# --- Composition -----------------------------------------------------------------------------------------------------
def compose(path_rows):
    m286, m287 = mansion(0x286), mansion(0x287)
    grass_colors = colors(general(0x008), general(0x009), general(0x010), general(0x011))
    cave_floor_colors = colors(cave(0x281))
    shaded = shade(PLANK)

    def floored(mid, lit, dark):
        """A Mansion metatile with its floor (lit or shaded, matched pixel by pixel) swapped for the planks."""
        pic = mansion(mid)
        return [[lit[y][x] if pic[y][x] == m286[y][x] else dark[y][x] if pic[y][x] == m287[y][x] else pic[y][x]
                 for x in range(16)] for y in range(16)]
    pics = {
        'PLANK_0': PLANK, 'PLANK_1': planks(2), 'PLANK_2': planks(3), 'PLANK_3': planks(4),
        'PLANK_CRACKED': planks(5, cracked=True), 'PLANK_BROKEN': planks(6, broken=True),
        'PLANK_SHADE_TOP': shaded, 'PLANK_SHADE_LEFT': shade(planks(2)),
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
    # Trees and grass through the floor
    grass_colors |= {GRASS_SPECK}
    trunk_colors = {c for c in colors(general(0x024), general(0x025)) - grass_colors if c[0] >= c[1]}
    for name, mid in (('PINE_TOP_L', 0x01E), ('PINE_TOP_R', 0x01F), ('PINE_BOTTOM_L', 0x016), ('PINE_BOTTOM_R', 0x017),
                      ('TREE_ROW_TOP_L', 0x01C), ('TREE_ROW_TOP_R', 0x01D), ('TREE_ROW_BOTTOM_L', 0x024),
                      ('TREE_ROW_BOTTOM_R', 0x025)):
        pic = swap(swap(general(mid), grass_colors, PLANK), {GRASS_SHADOW}, shaded)
        if 'BOTTOM' in name:
            pic = roots(pic, trunk_colors)
        pics[name] = pic
    pics['TALL_GRASS'] = swap(swap(general(0x00D), grass_colors, planks(8)), {GRASS_SHADOW}, shade(planks(8)))
    for name, mid in (('LADDER_UP_TOP', 0x28E), ('LADDER_UP', 0x296), ('LADDER_DOWN', 0x285)):
        pics[name] = swap(cave(mid), cave_floor_colors, PLANK)
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
    if name == 'TALL_GRASS':
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
