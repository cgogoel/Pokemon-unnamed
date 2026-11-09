#!/usr/bin/env python3
"""Celadon University's rooms in FireRed's own interior look (docs/hack/university_remake/interior_art.md).

Run by draw_university_tiles.py (which writes the campus and calls build() for the rooms); on its own it only renders:

    <venv with Pillow>/bin/python tools/hack/draw_university_interiors.py [--before DIR] [--audit]

Every room is composed from FireRed metatiles where one fits (the School's classroom, the Cinnabar Lab, the houses'
furniture in gTileset_BuildingFrlg, the Pokemon Center's counter) and new art drawn by FireRed's rules (outline
#52526A, 3-4 shade ramps, a lit top edge, drop shadows) where it doesn't. The windows are the campus's own (the arched
window of the stone buildings, the square one of the dorms and the hospital, the glass of the greenhouse and the
Science tower), in the columns of the building's windows outside. Each room keeps its collision, door mat and stairs
cells (COLLISION, the layouts' state when the rooms were redone), so every event stays where it is.

The rooms go into three secondary tilesets over gTileset_BuildingFrlg, by look:
    gTileset_UniversityClassroom   the stone academic buildings and the Study Hall (School + Lab looks)
    gTileset_UniversityDorm        the dorm floors and lobby, the cafeteria (FireRed's house set)
    gTileset_UniversityLab         the Science Institute's floors and the hospital (Lab + Pokemon Center)
Each is written by FrlgTileset: both layers cut into 8x8 tiles; a tile identical to one of gTileset_BuildingFrlg's
(in one of its palettes 0-5) is referenced, not copied; the rest are packed into palettes (a primary palette when one
holds the tile's colors, else secondary slots 7-12) and deduplicated with flips.
"""
import json
import os
import random
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # noqa: E402
import draw_university_tiles as U  # noqa: E402

ROOT = R.ROOT
OUT = os.path.join(ROOT, 'docs/hack/university_remake')
T = 16
PRIMARY = 'gTileset_BuildingFrlg'
PRIMARY_BASE = 640
FIRST_PAL = 7
MAX_TILES = MAX_METATILES = 384
MAX_SECONDARY_PALS = 6       # Slots 7-12
SNAP = 2                     # Colors this close (5-bit steps, summed over r, g, b) are stored as one
LAYER_COVERED = 1


def hx(s):
    return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4))


def key(c):
    """A color as the GBA stores it (5 bits a channel): FireRed's .pal files say 0x52 where we'd write 0x50."""
    return tuple(v >> 3 for v in c[:3])


def unkey(k):
    return tuple((v << 3) | (v >> 2) for v in k)


# --- FireRed's colors (from the .pal files; interior_art.md says which tileset each comes from) --------------------
OL = hx('52526A')            # The outline of every FRLG interior
WH = hx('FFFFFF')
GR1, GR2, GR3 = hx('D5D5D5'), hx('B4B4A4'), hx('7B7B7B')
CREAM1, CREAM2, CREAM3 = hx('FFE6AC'), hx('D5BD83'), hx('AC945A')       # The School's plaster
BOARD1, BOARD2 = hx('94CD7B'), hx('629C4A')                              # Its chalkboard
WOOD1, WOOD2, WOOD3, WOOD4 = hx('FFCD8B'), hx('E6A462'), hx('C58341'), hx('AC5A18')    # The Lab's furniture wood
BLUE1, BLUE2, BLUE3 = hx('BDE6FF'), hx('83BDDE'), hx('5294C5')          # The Lab's glass and upholstery
STRAW1, STRAW2 = hx('F6EE94'), hx('C5B44A')
LAB_G1 = hx('BDC5D5')
# gTileset_BuildingFrlg palette 3: slate blue and gold, the school colors, already in the primary
P3 = {'g': hx('7B7B7B'), 'au2': hx('E6B44A'), 'au1': hx('FFF683'), 'sl3': hx('41629C'), 'sl2': hx('5A7BB4'),
      'sl1': hx('739CC5'), 'st2': hx('A4A4AC'), 'st1': hx('CDCDD5'), 'st0': hx('EEEEF6'), 'gn2': hx('6AAC73'),
      'gn1': hx('9CCD8B')}
RED1, RED2, RED3 = hx('FF8373'), hx('CD5241'), hx('A43129')
PINK1, PINK2 = hx('FFC5C5'), hx('E69C9C')
SOIL1, SOIL2 = hx('A47B52'), hx('7B5A39')
BLACK = (0, 0, 0)

# The campus's window colors -> the interiors' (same shapes; the outline FireRed's, the glass the Lab's blues)
WINDOW_COLORS = {U.OL: OL, U.WHITE: WH, U.G1: BLUE1, U.G2: BLUE2, U.G3: BLUE3, U.S1: GR1, U.S2: GR1, U.S3: GR2,
                 U.GL['ol']: OL, U.GL['w']: WH, U.GL['s1']: GR1}


# --- FireRed metatiles as pictures -------------------------------------------------------------------------------
class Source:
    """A FireRed secondary tileset over gTileset_BuildingFrlg: cell(id) -> (bottom, top) 16x16 RGBA pictures, drawn
    as the game draws them (bottom layer: color 0 is the backdrop; top layer: color 0 is transparent)."""
    _cache = {}

    def __init__(self, secondary):
        if secondary not in Source._cache:
            pt, pp, pm = R.load_tileset(R.tileset_folder(PRIMARY), PRIMARY)
            st, sp, sm = R.load_tileset(R.tileset_folder(secondary), secondary)
            Source._cache[secondary] = (pt, st, [pp[i] if i < 7 else sp[i] for i in range(16)], pm, sm)
        self.pt, self.st, self.pals, self.pm, self.sm = Source._cache[secondary]

    def tile(self, t):
        rows, t = (self.pt, t) if t < PRIMARY_BASE else (self.st, t - PRIMARY_BASE)
        n = len(rows[0]) // 8
        if (t // n) * 8 >= len(rows):
            return None
        return [rows[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]

    def cell(self, mid):
        entries = self.pm[mid] if mid < PRIMARY_BASE else self.sm[mid - PRIMARY_BASE]
        out = []
        for layer in (0, 1):
            im = Image.new('RGBA', (T, T), (0, 0, 0, 0))
            px = im.load()
            for k in range(4):
                e = entries[layer * 4 + k]
                pix = self.tile(e & 0x3FF)
                if pix is None:
                    continue
                hf, vf, p = e >> 10 & 1, e >> 11 & 1, e >> 12
                for y in range(8):
                    for x in range(8):
                        c = pix[7 - y if vf else y][7 - x if hf else x]
                        if c == 0 and layer == 1:
                            continue
                        col = self.pals[0][0] if c == 0 else self.pals[p][c]
                        px[(k % 2) * 8 + x, (k // 2) * 8 + y] = tuple(col) + (255,)
            out.append(im)
        return out[0], out[1]

    def flat(self, mid):
        b, t = self.cell(mid)
        b = b.copy()
        b.alpha_composite(t)
        return b


SCHOOL = 'gTileset_School'
LAB = 'gTileset_Lab_Frlg'
HOME = 'gTileset_GenericBuilding1'
CONDO = 'gTileset_Condominiums'
DEPT = 'gTileset_DepartmentStore'
CENTER = 'gTileset_PokemonCenterFrlg'


# --- Pictures -------------------------------------------------------------------------------------------------------
class Pix:
    """A small RGBA picture drawn pixel by pixel."""

    def __init__(self, w, h):
        self.im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        self.px = self.im.load()
        self.w, self.h = w, h

    def p(self, x, y, c):
        if c is not None and 0 <= x < self.w and 0 <= y < self.h:
            self.px[x, y] = tuple(c) + (255,)

    def r(self, x, y, w, h, c):
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.p(xx, yy, c)

    def hl(self, x0, x1, y, c):
        for x in range(x0, x1 + 1):
            self.p(x, y, c)

    def vl(self, x, y0, y1, c):
        for y in range(y0, y1 + 1):
            self.p(x, y, c)

    def box(self, x, y, w, h, fill, outline=OL):
        self.r(x, y, w, h, fill)
        self.hl(x, x + w - 1, y, outline)
        self.hl(x, x + w - 1, y + h - 1, outline)
        self.vl(x, y, y + h - 1, outline)
        self.vl(x + w - 1, y, y + h - 1, outline)


def hslice(im, left, mid, right, width):
    """A strip `width` px wide from a FireRed piece: its first `left` px, its middle columns [mid0, mid1) repeated,
    its last `right` px (a 2-wide board or desk becomes any width with FireRed's own ends)."""
    h = im.height
    out = Image.new('RGBA', (width, h), (0, 0, 0, 0))
    out.paste(im.crop((0, 0, left, h)), (0, 0))
    m0, m1 = mid
    x = left
    while x < width - right:
        piece = im.crop((m0, 0, m1, h))
        out.paste(piece.crop((0, 0, min(m1 - m0, width - right - x), h)), (x, 0))
        x += m1 - m0
    out.paste(im.crop((im.width - right, 0, im.width, h)), (width - right, 0))
    # The joins must continue the piece as it was drawn: the column before each one must be the column the piece
    # has before the column after it (else the strip shows a seam)
    seams = []
    for jx in range(left, width - right, m1 - m0):
        if jx and m0 and list(out.crop((jx - 1, 0, jx, h)).getdata()) != list(im.crop((m0 - 1, 0, m0, h)).getdata()):
            seams.append(jx)
    last = width - right
    src_before = im.width - right - 1
    if last and list(out.crop((last - 1, 0, last, h)).getdata()) != list(im.crop((src_before, 0, src_before + 1,
                                                                                     h)).getdata()):
        seams.append(last)
    if seams:
        HSLICE_SEAMS.append((im, left, mid, right, width, seams))
    return out


HSLICE_SEAMS = []


def narrow(im):
    """A two-cell FireRed piece made one cell wide: its left and right edges joined where a shelf or panel boundary
    runs through it (the column pair that matches best), so the cut doesn't show."""
    w = im.width
    best, at = None, None
    for c in range(4, w - 4 - T + 1):
        # Keep columns [0, c) and [c + w - 2T ... ) : choose c where the columns meeting look alike
        d = c + (w - T) - 1
        a = list(im.crop((c - 1, 0, c, im.height)).getdata())
        b = list(im.crop((d, 0, d + 1, im.height)).getdata())
        score = sum(p != q for p, q in zip(a, b))
        if best is None or score < best:
            best, at = score, c
    out = Image.new('RGBA', (T, im.height), (0, 0, 0, 0))
    out.paste(im.crop((0, 0, at, im.height)), (0, 0))
    out.paste(im.crop((at + w - T, 0, w, im.height)), (at, 0))
    return out


def strip(source, mids, top_only=False):
    """FireRed metatiles side by side (a row of ids, or rows), flattened (or only their top layers)."""
    rows = [mids] if isinstance(mids[0], int) else mids
    im = Image.new('RGBA', (len(rows[0]) * T, len(rows) * T), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        for x, mid in enumerate(row):
            if mid is not None:
                pic = Source(source).cell(mid)[1] if top_only else Source(source).flat(mid)
                im.alpha_composite(pic, (x * T, y * T))
    return im


def recolor(im, mapping):
    m = {key(a): b for a, b in mapping.items()}
    out = im.copy()
    px = out.load()
    for y in range(out.height):
        for x in range(out.width):
            c = px[x, y]
            if c[3] and key(c) in m:
                px[x, y] = tuple(m[key(c)]) + (255,)
    return out


def fit(im, colors):
    """Every color of a picture moved to the nearest of `colors` (a piece from another FireRed tileset brought into
    the palette the room already uses)."""
    out = im.copy()
    px = out.load()
    for y in range(out.height):
        for x in range(out.width):
            c = px[x, y]
            if c[3]:
                n = min(colors, key=lambda d: sum((a - b) ** 2 for a, b in zip(c[:3], d)))
                px[x, y] = tuple(n) + (255,)
    return out


def campus_pic(draw, w, h, *args):
    """A piece of the campus's own art (draw_university_tiles: window, sqwindow, glass_wall...) as an RGBA picture in
    the interiors' colors."""
    P = U.Pic(w, h)
    draw(P, *args)
    return recolor(P.im, WINDOW_COLORS)


# --- A room ---------------------------------------------------------------------------------------------------------
class Room:
    """A room being composed: F (the bottom layer: floor, walls, low pieces) and O (the middle layer: furniture over
    the floor), its collision and door/stairs marks (from COLLISION: they don't change), and the people for the
    mockups."""

    def __init__(self, name):
        marks, rows = COLLISION[name]
        self.name, self.w, self.h = name, len(rows[0]), len(rows)
        self.solid = [[int(c == '#') for c in row] for row in rows]
        self.marks = {tuple(map(int, k.split(','))): v for k, v in marks.items()}
        self.F = Image.new('RGBA', (self.w * T, self.h * T), BLACK + (255,))
        self.O = Image.new('RGBA', (self.w * T, self.h * T), (0, 0, 0, 0))
        self.decor = set()          # Walkable cells with something in the middle layer (rugs, mats, stairs)
        self.walls = set()          # Solid cells drawn in the bottom layer (partitions, wall pieces)
        self.wall_rows = 3 if name.startswith('MAIN_HALL') else 2

    def fr(self, x, y, source, mid, mode='both'):
        """FireRed metatile `mid` of `source` at cell (x, y): both layers, its bottom, or only its top layer."""
        b, t = Source(source).cell(mid)
        if mode in ('both', 'bottom'):
            self.F.paste(b, (x * T, y * T))
        if mode in ('both', 'top'):
            self.O.alpha_composite(t, (x * T, y * T))

    def fr_block(self, x, y, source, rows, mode='both'):
        for dy, row in enumerate(rows):
            for dx, mid in enumerate(row):
                if mid is not None:
                    self.fr(x + dx, y + dy, source, mid, mode)

    def put(self, im, x, y, layer='O'):
        """A picture at pixel (x, y) on a layer."""
        (self.F if layer == 'F' else self.O).alpha_composite(im, (x, y))

    def floor(self, source, mid, shade=None, x0=1, y0=None, x1=None, y1=None):
        """The floor over the room's inside (and its shaded variant down the first column, by the left wall)."""
        y0 = self.wall_rows if y0 is None else y0
        x1 = self.w - 1 if x1 is None else x1
        y1 = self.h - 1 if y1 is None else y1
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.fr(x, y, source, shade if (shade is not None and x == x0) else mid, 'bottom')

    def shade(self, ramp, box):
        """Floor shadow (FireRed's: the floor one shade darker) over a pixel box (x0, y0, x1, y1) of F."""
        px = self.F.load()
        down = {key(a): b for a, b in ramp.items()}
        for y in range(max(0, box[1]), min(self.F.height, box[3])):
            for x in range(max(0, box[0]), min(self.F.width, box[2])):
                c = px[x, y]
                if key(c) in down:
                    px[x, y] = tuple(down[key(c)]) + (255,)

    def drop_shadow(self, ramp, x, y, w, h, below=3, right=0):
        """The shadow under a piece of furniture over cells (x, y, w, h): a band below it (and down its right)."""
        X, Y = x * T, y * T
        self.shade(ramp, (X + 2, Y + h * T, X + w * T + right, Y + h * T + below))
        if right:
            self.shade(ramp, (X + w * T, Y + 4, X + w * T + right, Y + h * T))

    def image(self):
        im = self.F.copy()
        im.alpha_composite(self.O)
        return im

    def check(self):
        """Cells where what's drawn and the collision disagree: furniture on a walkable cell (not decor), or a
        solid cell inside the room with nothing on it."""
        out = []
        for y in range(self.wall_rows, self.h - 1):
            for x in range(1, self.w - 1):
                cell = self.O.crop((x * T, y * T, x * T + T, y * T + T))
                n = sum(1 for p in cell.getdata() if p[3])
                if n > 24 and not self.solid[y][x] and (x, y) not in self.decor and (x, y) not in self.marks:
                    out.append('furniture on walkable %d,%d' % (x, y))
                if self.solid[y][x] and n == 0 and (x, y) not in self.walls:
                    out.append('solid but empty %d,%d' % (x, y))
        return out


def shadow_ramp(source, plain, shaded):
    """A floor's shadow colors, read off a FireRed pair (the plain floor and its shaded variant)."""
    a, b = Source(source).flat(plain), Source(source).flat(shaded)
    pa, pb = a.load(), b.load()
    ramp = {}
    for y in range(T):
        for x in range(T):
            if pa[x, y] != pb[x, y]:
                ramp.setdefault(pa[x, y][:3], pb[x, y][:3])
    return ramp
COLLISION = {
    'MAIN_HALL': ({'9,11': 'mat', '1,3': 'up', '2,3': 'up', '16,3': 'up', '17,3': 'up'}, [
        '###################',
        '###################',
        '###################',
        '#.................#',
        '#...#.........#...#',
        '#...#..#####..#...#',
        '###.............###',
        '#...#.........#...#',
        '#...#.........#...#',
        '##...............##',
        '##...............##',
        '#.................#',
        '###################',
    ]),
    'MAIN_HALL_2F': ({'1,3': 'down', '2,3': 'down', '16,3': 'down', '17,3': 'down'}, [
        '###################',
        '###################',
        '###################',
        '#.................#',
        '#.................#',
        '#.................#',
        '#..##........###..#',
        '#............###..#',
        '#.................#',
        '#.......###.......#',
        '##...............##',
        '##...............##',
        '###################',
    ]),
    'LECTURE_HALL_A': ({'7,9': 'mat'}, [
        '###############',
        '###############',
        '#......#......#',
        '######...######',
        '######...######',
        '######...######',
        '######...######',
        '##...........##',
        '##...........##',
        '#.............#',
        '###############',
    ]),
    'LECTURE_HALL_B': ({'7,9': 'mat'}, [
        '###############',
        '###############',
        '#......#......#',
        '######...######',
        '######...######',
        '######...######',
        '######...######',
        '##...........##',
        '##...........##',
        '#.............#',
        '###############',
    ]),
    'FACULTY_OFFICE': ({'6,8': 'mat'}, [
        '#############',
        '#############',
        '#...........#',
        '#....###....#',
        '##.........##',
        '#...........#',
        '##......###.#',
        '##......###.#',
        '#...........#',
        '#############',
    ]),
    'CAFETERIA': ({'6,8': 'mat'}, [
        '#############',
        '#############',
        '#...........#',
        '######.##.###',
        '#......##.###',
        '#...........#',
        '##.....##.###',
        '##.....##.###',
        '#...........#',
        '#############',
    ]),
    'LITERATURE': ({'7,9': 'mat'}, [
        '###############',
        '###############',
        '#.............#',
        '##...........##',
        '##....###....##',
        '#.....###.....#',
        '#.............#',
        '#.###.....#.#.#',
        '#.............#',
        '#.............#',
        '###############',
    ]),
    'LIBRARY': ({'7,9': 'mat'}, [
        '###############',
        '###############',
        '#.............#',
        '#.....###.....#',
        '#####.....#####',
        '#####.....#####',
        '#.....###.....#',
        '##....###....##',
        '##...........##',
        '#.............#',
        '###############',
    ]),
    'DORM_LOBBY': ({'10,10': 'mat', '1,2': 'up', '2,2': 'up'}, [
        '#####################',
        '#####################',
        '#...#..............##',
        '#...................#',
        '#...................#',
        '#..####......#####..#',
        '#...................#',
        '#.............###...#',
        '##.................##',
        '##.................##',
        '#...................#',
        '#####################',
    ]),
    'DORM_2F': ({'1,2': 'down', '2,2': 'down', '18,2': 'up', '19,2': 'up'}, [
        '#####################',
        '#####################',
        '#...................#',
        '#...................#',
        '###.######.#####.####',
        '##...#.##..#.##..####',
        '##.....##....##..#..#',
        '#......#.....#.....##',
        '#....#.#...#.#.....##',
        '#....#.#...#.#......#',
        '#......#.....#......#',
        '#####################',
    ]),
    'DORM_3F': ({'18,2': 'down', '19,2': 'down'}, [
        '#####################',
        '#####################',
        '##..................#',
        '##..................#',
        '###.######.#####.####',
        '##...#.##..#.##...#.#',
        '##.....##....##.....#',
        '#......#.....#......#',
        '#....#.#...#.#....#.#',
        '#....#.#...#.#....#.#',
        '#......#.....#......#',
        '#####################',
    ]),
    'STUDY_HALL': ({'7,8': 'mat'}, [
        '###############',
        '###############',
        '#............##',
        '#.###........##',
        '#.###....####.#',
        '#.............#',
        '#.###.....##..#',
        '#####.........#',
        '##............#',
        '###############',
    ]),
    'HOME_ECONOMICS': ({'10,9': 'mat'}, [
        '#####################',
        '#####################',
        '#...................#',
        '#.##.##..###..##.##.#',
        '#........###........#',
        '#...................#',
        '#.##.##.......##.##.#',
        '#...................#',
        '##.................##',
        '##.................##',
        '#####################',
    ]),
    'SCIENCE_INSTITUTE': ({'8,13': 'mat', '14,2': 'up', '15,2': 'up'}, [
        '#################',
        '#################',
        '#...............#',
        '#...............#',
        '####.........####',
        '####.........####',
        '#...............#',
        '#...............#',
        '#...............#',
        '#...............#',
        '##.............##',
        '##.............##',
        '#...............#',
        '#...............#',
        '#################',
    ]),
    'SCIENCE_LOUNGE': ({'14,2': 'down', '15,2': 'down', '1,2': 'up', '2,2': 'up'}, [
        '#################',
        '#################',
        '#...............#',
        '#...............#',
        '#...............#',
        '#....####.##....#',
        '#..............##',
        '#.....###......##',
        '#...............#',
        '#...............#',
        '#...............#',
        '##.............##',
        '##.............##',
        '#...............#',
        '#################',
    ]),
    'SCIENCE_RESEARCH_ROOM': ({'1,2': 'down', '2,2': 'down', '14,2': 'up', '15,2': 'up'}, [
        '#################',
        '#################',
        '#...............#',
        '#...............#',
        '#..............##',
        '#..............##',
        '#...............#',
        '#..........###..#',
        '#..........###..#',
        '#..###..........#',
        '#..###.....###..#',
        '#..........###..#',
        '#...............#',
        '#...............#',
        '#################',
    ]),
    'SCIENCE_EXPERIMENT_ROOM': ({'14,2': 'down', '15,2': 'down'}, [
        '#################',
        '#################',
        '#...............#',
        '#...............#',
        '#...............#',
        '#.###..###......#',
        '#.###..###......#',
        '#...............#',
        '#...............#',
        '#.###..###......#',
        '#.###..###......#',
        '#..............##',
        '#..............##',
        '#...............#',
        '#################',
    ]),
    'HOSPITAL': ({'8,10': 'mat', '14,2': 'up', '15,2': 'up'}, [
        '#################',
        '#################',
        '#...............#',
        '#...............#',
        '#.....#####.....#',
        '#...............#',
        '###.............#',
        '#...............#',
        '###............##',
        '#..............##',
        '#...............#',
        '#################',
    ]),
    'HOSPITAL_2F': ({'14,2': 'down', '15,2': 'down'}, [
        '#################',
        '#################',
        '##.#.#..........#',
        '##.#.#..........#',
        '#...............#',
        '#...............#',
        '#...............#',
        '##.#.#..........#',
        '##.#.#.........##',
        '#..............##',
        '#...............#',
        '#################',
    ]),
}


# --- The shell: walls, windows, edges, mats, stairs -----------------------------------------------------------------
WALL_TOP = 5                 # The wall's top band (rows 0-4): wall-mounted pieces hang below it


def wall_top(room, y=0):
    """The wall's top edge seen from above, across the top of row y (outline, white, white, light grey, outline),
    running on over the edge columns' wall tops so the corners join (their outer outline closes it)."""
    x0, x1 = 10, room.w * T - 11
    P = Pix(x1 - x0 + 1, WALL_TOP)
    for k, c in enumerate((OL, WH, WH, GR1, OL)):
        P.hl(0, P.w - 1, k, c)
    P.vl(0, 0, WALL_TOP - 1, OL)
    P.vl(P.w - 1, 0, WALL_TOP - 1, OL)
    room.put(P.im, x0, y * T, 'F')


def edges(room):
    """The solid edge columns: black beyond, the wall's top (outline, white, light grey, outline) down the room side,
    like the Condominiums' and Silph's walls; the last row black (FireRed's rooms end in the void)."""
    W, H = room.w, room.h
    for side in (0, 1):
        P = Pix(T, (H - 1) * T)
        P.r(0, 0, T, P.h, BLACK)
        for i, c in enumerate((OL, WH, GR1, GR1, GR1, OL)):
            P.vl(10 + i if side == 0 else 5 - i, 0, P.h - 1, c)
        room.F.paste(P.im, (0 if side == 0 else (W - 1) * T, 0))
    room.F.paste(Image.new('RGBA', (W * T, T), BLACK + (255,)), (0, (H - 1) * T))


def school_wall(room):
    """The School's back wall (gTileset_School): cream plaster (0x283), the tan rail, wainscot and teal skirting of its
    window wall (0x28E's lower half), the skirting's foot on the first floor row (0x296)."""
    plain = Pix(T, 12)
    plain.r(0, 0, T, 7, CREAM1)
    plain.hl(0, T - 1, 7, CREAM3)
    plain.r(0, 8, T, 4, CREAM2)
    for x in range(1, room.w - 1):
        room.fr(x, 0, SCHOOL, 0x283, 'bottom')
        room.F.paste(Source(SCHOOL).flat(0x28E), (x * T, T))
        room.F.paste(plain.im, (x * T, T))
        room.fr(x, 2, SCHOOL, 0x296, 'bottom')


def lab_wall(room, foot=True):
    """The Lab's cream plaster with its gold-brown rail (gTileset_BuildingFrlg 0x069 over 0x071; a third row of plain
    plaster for the Main Hall); 0x079, the white tiles along the wall's foot, on the first floor row."""
    rows = room.wall_rows
    for x in range(1, room.w - 1):
        room.fr(x, 0, LAB, 0x069, 'bottom')
        if rows == 3:
            room.F.paste(Image.new('RGBA', (T, T), Source(LAB).flat(0x069).getpixel((0, T - 1))), (x * T, T))
        room.fr(x, rows - 1, LAB, 0x071, 'bottom')
        if foot:
            room.fr(x, rows, LAB, 0x079, 'bottom')


def house_wall(room):
    """The houses' grey-green plaster (0x0A8 over 0x0B0: the wall top, teal skirting)."""
    for x in range(1, room.w - 1):
        room.fr(x, 0, HOME, 0x0A8, 'bottom')
        room.fr(x, 1, HOME, 0x0B0, 'bottom')


def center_wall(room):
    """The Pokemon Center's wall (0x284 over 0x2AC: cream with the salmon band)."""
    for x in range(1, room.w - 1):
        room.fr(x, 0, CENTER, 0x284, 'bottom')
        room.fr(x, 1, CENTER, 0x2AC, 'bottom')


def glass_wall(room):
    """The greenhouse's and the Science tower's glass, from inside (the campus's glass_wall, its outline FireRed's)."""
    room.put(campus_pic(U.glass_wall, (room.w - 2) * T, room.wall_rows * T, 0, (room.w - 2) * T, room.wall_rows),
             T, 0, 'F')


# Where each wall's rail (or skirting band) starts on its last row: windows end above it
RAIL = {'school': 7, 'lab': 7, 'house': 8, 'center': 4}


def campus_window(room, draw, sill, cut_at):
    """The campus window (16x32), made as much shorter as the wall's rail needs (rows of its lower pane taken out from
    `cut_at`, so the arch or frame, the cross and the sill keep their shape), the sill resting on the rail."""
    im = campus_pic(draw, T, 2 * T, 0, 0)
    rail = T + RAIL.get(room.wall_kind, T)          # The rail's first row, from the window's top (row wall_rows-2)
    cut = max(0, sill + 1 - (rail - 1))
    if cut:
        short = Image.new('RGBA', im.size, (0, 0, 0, 0))
        short.paste(im.crop((0, 0, T, cut_at)), (0, 0))
        short.paste(im.crop((0, cut_at + cut, T, 2 * T)), (0, cut_at))
        im = short
    return im


def arched(room, *cols):
    """The stone buildings' tall arched window (the campus's window()), on the wall's last two rows."""
    for c in cols:
        room.put(campus_window(room, U.window, 28, 14), c * T, (room.wall_rows - 2) * T, 'F')


def square(room, *cols):
    """The dorms' and the hospital's square window (the campus's sqwindow())."""
    for c in cols:
        room.put(campus_window(room, U.sqwindow, 23, 15), c * T, (room.wall_rows - 2) * T, 'F')


MATS = {'school': (SCHOOL, (0x2A5, 0x2A6, 0x2A7)), 'lab': (LAB, (0x290, 0x291, 0x292))}


def door_mat(room, style='lab'):
    """FireRed's red door mat: three cells over the floor on the row above the void, its lower edge in the void
    (gTileset_BuildingFrlg 0x01A-0x01C); only the middle one warps."""
    door = [x for (x, y), v in room.marks.items() if v == 'mat']
    if not door:
        return
    x = door[0]
    source, mats = MATS[style]
    for i, mid in enumerate(mats):
        room.fr(x - 1 + i, room.h - 2, source, mid, 'top')
        room.decor.add((x - 1 + i, room.h - 2))
    for i, mid in enumerate((0x01A, 0x01B, 0x01C)):
        room.fr(x - 1 + i, room.h - 1, SCHOOL, mid, 'both')


def stairs_up(room, x):
    """The Copycat's stairs (rail 0x0EA, steps 0x0DA-0x0DB, landing 0x0E2-0x0E3), the landing on the first floor
    row (the warp cells)."""
    y = room.wall_rows
    room.fr(x, y - 2, HOME, 0x0EA, 'top')
    room.fr_block(x, y - 1, HOME, [[0x0DA, 0x0DB], [0x0E2, 0x0E3]])
    for k in (0, 1):
        refloor(room, x + k, y, HOME, (PLANKS, PLANKS_SHADE))


def stairs_down(room, x):
    """The Pallet house's stairs down (rail 0x015, steps 0x005-0x006 over 0x00D-0x00E), the last row the warp
    cells."""
    y = room.wall_rows
    room.fr(x, y - 2, HOME, 0x015, 'top')
    room.fr_block(x, y - 1, HOME, [[0x005, 0x006], [0x00D, 0x00E]])
    for k in (0, 1):
        refloor(room, x + k, y, HOME, (PARQUET, PARQUET_SHADE))


# --- FireRed furniture ------------------------------------------------------------------------------------------------
def palm(room, x, y):
    """The potted palm (gTileset_BuildingFrlg 0x08C over 0x094, top layers), 1x2."""
    room.fr(x, y, LAB, 0x08C, 'top')
    room.fr(x, y + 1, LAB, 0x094, 'top')


def shelves(room, x, y, w, standing=False):
    """The Lab's bookshelf (0x073-0x074 over 0x07B-0x07C, top layers: 24 px tall), w cells wide (odd widths end on a
    narrowed one), against the wall's rows; standing on the floor it's made 32 px tall with a third shelf (0x073's
    shelf rows again) so it fills its two cells."""
    top = strip(LAB, [0x073, 0x074], True)
    bot = strip(LAB, [0x07B, 0x07C], True)
    unit = Image.new('RGBA', (2 * T, 2 * T), (0, 0, 0, 0))
    unit.alpha_composite(top, (0, 0))
    if standing:
        unit.alpha_composite(top.crop((0, 6, 2 * T, 14)), (0, T))
        unit.alpha_composite(bot.crop((0, 0, 2 * T, 8)), (0, T + 8))
    else:
        unit.alpha_composite(bot, (0, T))
    wide = Image.new('RGBA', (w * T, 2 * T), (0, 0, 0, 0))
    if w == 1:
        wide = narrow(unit)
    else:
        for k in range(0, w - 1, 2):
            wide.alpha_composite(unit, (k * T, 0))
        if w % 2:
            wide.alpha_composite(narrow(unit), ((w - 1) * T, 0))
    if y == 0 and not standing:
        against_wall(room, x, wide)
    else:
        room.put(wide, x * T, y * T)


def notices(room, *cols):
    """Notices on strings (Oak's lab: 0x28D over 0x295, top layers) on the wall's last two rows."""
    for c in cols:
        room.fr(c, room.wall_rows - 2, LAB, 0x28D, 'top')
        room.fr(c, room.wall_rows - 1, LAB, 0x295, 'top')


def notice_house(room, *cols):
    """The houses' notice (0x0D6's top layer) on the wall's lower row."""
    for c in cols:
        room.fr(c, room.wall_rows - 1, HOME, 0x0D6, 'top')


def straw_table(room, x, y, w=3, plate=False):
    """The Lab lounge's woven table (0x2E0-0x2E2 over 0x2F0-0x2F2, top layers), w cells wide, 2 tall."""
    rows = [strip(LAB, [0x2E0, 0x2E1, 0x2E2], True), strip(LAB, [0x2F0, 0x2F1, 0x2F2], True)]
    for k, im in enumerate(rows):
        room.put(hslice(im, 16, (16, 32), 16, w * T), x * T, (y + k) * T)
    if plate:
        for c in range(w):
            P = Pix(T, T)
            P.box(3, 8, 10, 6, WH, OL)
            P.r(5, 9, 6, 3, GR1)
            P.r(6, 9, 4, 2, (WOOD2, BOARD1, RED1)[(x + c) % 3])
            room.put(P.im, (x + c) * T, y * T)


def low_table(room, x, y, w):
    """The School's table (0x2AA-0x2AB: straw top, rail, legs), one row, w cells wide."""
    room.put(hslice(strip(SCHOOL, [0x2AA, 0x2AB], True), 8, (8, 24), 8, w * T), x * T, y * T)


def sofa(room, x, y, w):
    """The Condominiums' sofa (0x2E0-0x2E2) widened to w cells, in the Lab's blues."""
    im = hslice(strip(CONDO, [0x2E0, 0x2E1, 0x2E2], True), 16, (16, 32), 16, w * T)
    room.put(fit(im, LAB_COLORS), x * T, y * T)


def stool(room, x, y):
    room.fr(x, y, LAB, 0x2CB, 'top')


def vending(room, x, y):
    """The Department Store roof's vending machine (0x325 over 0x32D), 1x2."""
    for k, mid in enumerate((0x325, 0x32D)):
        room.put(fit(Source(DEPT).flat(mid), LAB_COLORS + [RED1, RED2, RED3, BOARD1, BOARD2]), x * T, (y + k) * T)


def lab_desk(room, x, y):
    """The Lab's desk with a PC and its towers (0x305-0x307 over 0x30D-0x30F, top layers), 3x2."""
    room.fr_block(x, y, LAB, [[0x305, 0x306, 0x307], [0x30D, 0x30E, 0x30F]], 'top')


def cabinet_desk(room, x, y, book=True):
    """The houses' little desk (0x030's top layer: straw top, a drawer), 1x1, an open book on it."""
    room.fr(x, y, HOME, 0x030, 'top')
    if book:
        room.put(Source(SCHOOL).cell(0x299)[1].crop((3, 0, 13, 7)), x * T + 3, y * T + 1)


def lab_rug(room, x, y, w, h, top=True, bottom=True):
    """The Lab's blue rug (edges 0x2EC-0x2F7, middle 0x2F3), w x h cells (walkable)."""
    for dy in range(h):
        for dx in range(w):
            edge_t, edge_b = dy == 0 and top, dy == h - 1 and bottom
            col = 0 if dx == 0 else 2 if dx == w - 1 else 1
            if edge_t:
                mid = (0x2EC, 0x2ED, 0x2EE)[col]
            elif edge_b:
                mid = (0x2F4, 0x2F5, 0x2F6)[col]
            else:
                mid = (0x2F7, 0x2F3, 0x2EF)[col]
            room.fr(x + dx, y + dy, LAB, mid, 'bottom' if mid == 0x2F3 else 'top')
            room.decor.add((x + dx, y + dy))


def house_rug(room, x, y, w, h):
    """The Copycat's orange rug (corners and edges 0x0C3-0x0D5, middle 0x0CC), w x h cells (walkable)."""
    for dy in range(h):
        for dx in range(w):
            r = 0 if dy == 0 else 2 if dy == h - 1 else 1
            c = 0 if dx == 0 else 2 if dx == w - 1 else 1
            room.fr(x + dx, y + dy, HOME, ((0x0C3, 0x0C4, 0x0C5), (0x0CB, 0x0CC, 0x0CD), (0x0D3, 0x0D4, 0x0D5))[r][c],
                    'both')
            room.decor.add((x + dx, y + dy))


# --- New pieces (FireRed's rules: outline, a lit top edge, 3-4 shades, the shadow on the floor) ------------------------
def counter(room, x, y, w, papers=True):
    """A counter, w cells: a pale stone top (white rim, light grey, a darker lip) over a wood front with panels."""
    P = Pix(w * T, T)
    P.box(0, 1, w * T, 15, WOOD3)
    P.hl(1, w * T - 2, 2, WH)
    P.r(1, 3, w * T - 2, 3, GR1)
    P.hl(1, w * T - 2, 6, GR2)
    P.hl(1, w * T - 2, 7, OL)
    P.hl(1, w * T - 2, 8, WOOD2)
    for k in range(w):
        P.box(k * T + 3, 9, 10, 5, WOOD3, WOOD4)
        P.hl(k * T + 4, k * T + 11, 10, WOOD2)
    if papers:
        for k in range(0, w, 2):
            P.box(k * T + 4, 2, 7, 4, WH, GR2)
            P.hl(k * T + 5, k * T + 8, 3, LAB_G1)
    room.put(P.im, x * T, y * T)


def bench(room, x, y, w):
    """A wooden bench, w cells: slatted seat with a lit edge, legs."""
    P = Pix(w * T, T)
    P.box(1, 3, w * T - 2, 7, WOOD2)
    P.hl(2, w * T - 3, 4, WOOD1)
    P.hl(2, w * T - 3, 6, WOOD3)
    P.hl(2, w * T - 3, 8, WOOD3)
    P.hl(1, w * T - 2, 10, WOOD4)
    for lx in (3, w * T - 6):
        P.box(lx, 10, 3, 5, WOOD4)
    room.put(P.im, x * T, y * T)


def pillar(room, x, y):
    """A white stone pillar, 1x2: capital, a fluted shaft lit on the left, base."""
    P = Pix(T, 2 * T)
    P.box(1, 0, 14, 4, WH)
    P.hl(2, 13, 2, GR1)
    P.box(3, 3, 10, 26, GR1)
    P.vl(4, 4, 27, WH)
    P.vl(5, 4, 27, WH)
    P.vl(8, 4, 27, GR2)
    P.vl(11, 4, 27, GR2)
    P.box(1, 28, 14, 4, GR1)
    P.hl(2, 13, 29, WH)
    room.put(P.im, x * T, y * T)


def mailboxes(room, x, w):
    """The dorm's mailboxes on the wall's two rows: a grid of slate doors with brass name slots."""
    P = Pix(w * T, 2 * T)
    P.box(1, 6, w * T - 2, 24, P3['sl1'])
    for r in range(3):
        for c in range(w * 2):
            bx, by = 3 + c * 7, 8 + r * 7
            if bx + 6 > w * T - 2:
                continue
            P.box(bx, by, 6, 6, P3['sl2'], P3['sl3'])
            P.hl(bx + 1, bx + 4, by + 1, P3['sl1'])
            P.hl(bx + 2, bx + 3, by + 3, P3['au1'])
    room.put(P.im, x * T, 0)


def wall_clock(room, cx, cy, r=12):
    """The Main Hall's clock (the campus clock's colors: a gold ring, white face)."""
    P = Pix(2 * r + 2, 2 * r + 2)
    for y in range(P.h):
        for x in range(P.w):
            d = ((x + .5 - r - 1) ** 2 + (y + .5 - r - 1) ** 2) ** .5
            if d <= r:
                P.p(x, y, OL if d > r - 1 else P3['au2'] if d > r - 3 else P3['au1'] if d > r - 4 else WH)
    c = r + 1
    P.vl(c - 1, c - 7, c, OL)
    P.vl(c, c - 7, c, OL)
    for (dx, dy) in ((0, -9), (-9, 0), (8, 0), (0, 8)):
        P.p(c + dx - (1 if dx <= 0 else 0), c + dy, P3['g'])
    room.put(P.im, cx - c, cy - c, 'F')


def banner(room, x, y, h=18):
    """A school banner in slate and gold, hanging from a rod."""
    P = Pix(12, h + 6)
    P.hl(0, 11, 0, P3['au2'])
    for yy in range(1, h):
        P.hl(1, 10, yy, P3['sl2'])
        P.p(1, yy, OL)
        P.p(10, yy, OL)
        P.p(2, yy, P3['sl1'])
    for k in range(5):
        P.hl(1 + k, 10 - k, h + k, P3['sl2'])
        P.p(1 + k, h + k, OL)
        P.p(10 - k, h + k, OL)
    for (dx, dy) in ((5, 7), (6, 7), (4, 8), (7, 8), (4, 9), (7, 9), (5, 10), (6, 10)):
        P.p(dx, dy + 1, P3['au1'])
    room.put(P.im, x, y, 'F')


def emblem(room, cx, cy):
    """The school's emblem inlaid in the runner: a gold ring around a Magikarp (the campus's medallion's motif)."""
    P = Pix(28, 28)
    for y in range(28):
        for x in range(28):
            d = ((x + .5 - 14) ** 2 + (y + .5 - 14) ** 2) ** .5
            if d <= 13:
                P.p(x, y, OL if d > 12 else P3['au2'] if d > 11 else P3['au1'] if d > 10 else P3['sl3'])
    for y in range(-4, 5):
        for x in range(-7, 7):
            if (x / 7) ** 2 + (y / 4.5) ** 2 <= 1:
                P.p(14 + x, 14 + y, WOOD2 if y < 2 else WOOD1)
    for k in range(4):
        P.vl(21 + k // 2, 11 + k, 17 - k, RED2)
    P.p(10, 13, OL)
    room.put(P.im, cx - 14, cy - 14)


def display_case(room, x, y, w, h, kind):
    """A museum case: glass (the Lab's blues, white glints) on a wood base; moon stones, or a model satellite."""
    P = Pix(w * T, h * T)
    base = 6
    P.box(1, 1, w * T - 2, h * T - base, BLUE1)
    P.hl(2, w * T - 3, 2, WH)
    for k in range(4, w * T - 6, 12):
        P.p(k, 4, WH)
        P.p(k + 1, 5, WH)
    P.box(0, h * T - base - 1, w * T, base + 1, WOOD3)
    P.hl(1, w * T - 2, h * T - base, WOOD2)
    P.hl(1, w * T - 2, h * T - 2, WOOD4)
    inner_y = h * T - base - 6
    if kind == 'stones':
        for k in range(6, w * T - 6, 9):
            P.box(k, inner_y, 6, 5, GR1, GR3)
            P.p(k + 1, inner_y + 1, WH)
    else:
        cx, cy = w * T // 2, h * T // 2 - 4
        P.box(cx - 4, cy - 4, 8, 8, GR1, GR3)
        P.p(cx - 2, cy - 2, WH)
        for s in (-1, 1):
            x0 = cx + 6 if s > 0 else cx - 18
            P.box(x0, cy - 2, 12, 4, P3['sl2'], P3['sl3'])
            P.vl(x0 + 6, cy - 2, cy + 1, P3['sl1'])
        P.vl(cx, cy - 10, cy - 5, GR3)
        P.p(cx, cy - 11, RED2)
        P.box(cx - 2, h * T - base - 6, 4, 5, WOOD4)
    room.put(P.im, x * T, y * T)


def screen(room, x, w):
    """A projection screen on the wall's two rows: grey case, a white sheet showing a slide (Lapras on the waves)."""
    P = Pix(w * T, 2 * T)
    y0 = WALL_TOP + 1
    P.box(2, y0, w * T - 4, 4, GR1)
    P.hl(3, w * T - 5, y0 + 1, WH)
    P.box(4, y0 + 3, w * T - 8, 18, WH, GR2)
    P.box(14, y0 + 5, w * T - 28, 13, BLUE2, BLUE3)
    P.hl(15, w * T - 16, y0 + 6, BLUE1)
    for k in range(15, w * T - 15):         # Waves, a whole number of 6-pixel periods across
        P.p(k, y0 + 14 + ((k - 15) // 3) % 2, WH)
    cx = w * T // 2
    for yy in range(-3, 3):
        for xx in range(-8, 8):
            if (xx / 8) ** 2 + (yy / 3) ** 2 <= 1:
                P.p(cx + xx, y0 + 10 + yy, BLUE3 if yy > 0 else hx('7BA4DE'))
    P.vl(cx + 7, y0 + 5, y0 + 10, BLUE3)
    P.p(cx + 8, y0 + 6, BLUE3)
    room.put(P.im, x * T, 0)


def lectern(room, x, y):
    """A lectern: a sloped top with a lit edge and notes, a body with a front panel inset and a brass plaque."""
    P = Pix(T, T)
    P.box(2, 1, 12, 4, WOOD2)
    P.hl(3, 12, 2, WOOD1)
    P.hl(3, 12, 4, WOOD3)
    P.r(5, 2, 6, 2, WH)
    P.hl(6, 9, 3, LAB_G1)
    P.box(3, 5, 10, 10, WOOD3)
    P.hl(4, 11, 6, WOOD4)
    P.r(5, 8, 6, 5, WOOD4)
    P.r(6, 9, 4, 3, WOOD3)
    P.p(7, 10, STRAW1)
    P.p(8, 10, STRAW1)
    P.vl(4, 7, 13, WOOD2)
    room.put(P.im, x * T, y * T)


def chalkboard(room, x, w):
    """The School's board (0x28B-0x28C, chalk tray 0x293-0x294) widened to w cells with its own ends, wiped, and the
    Fire-Water-Grass triangle chalked on it."""
    board = hslice(strip(SCHOOL, [0x28B, 0x28C]), 8, (8, 24), 8, w * T)
    tray = hslice(strip(SCHOOL, [0x293, 0x294]), 8, (8, 24), 8, w * T)
    px = board.load()
    for yy in range(2, 13):
        for xx in range(3, w * T - 3):
            if key(px[xx, yy]) in (key(WH), key(BOARD1)):
                px[xx, yy] = BOARD2 + (255,)
    # The eraser and chalk on the board's ledge (0x28B's columns 6-11) once, not at every repeat of the middle
    for yy in range(13, T):
        for xx in range(12, w * T - 8):
            px[xx, yy] = BOARD2 + (255,)
    tp = tray.load()
    plain = [tp[2, yy] for yy in range(T)]
    for yy in range(0, 2):
        for xx in range(T, w * T - T):
            tp[xx, yy] = plain[yy]
    P = Pix(w * T, T)
    cx = w * T // 2

    def ring(x0, y0):
        for dx, dy in ((1, 0), (2, 0), (0, 1), (3, 1), (0, 2), (3, 2), (1, 3), (2, 3)):
            P.p(x0 + dx, y0 + dy, WH)
    ring(cx - 2, 2)
    ring(cx - 10, 9)
    ring(cx + 6, 9)
    for t in range(1, 5):
        P.p(cx - 3 - t, 5 + t, BOARD1)
        P.p(cx + 2 + t, 5 + t, BOARD1)
    P.hl(cx - 5, cx + 4, 11, BOARD1)
    for k, xx in enumerate(range(6, cx - 16, 2)):
        P.p(xx, 4 + (k % 3 == 0), WH)
        if k % 4:
            P.p(xx, 8, WH)
    for k, xx in enumerate(range(cx + 16, w * T - 10, 2)):
        P.p(xx, 4, WH if k % 5 else BOARD1)
        if k % 3:
            P.p(xx, 8, WH)
    board.alpha_composite(P.im)
    room.put(board, x * T, T, 'F')
    room.put(tray, x * T, 2 * T, 'F')
    for k in range(w):                  # The tray's own floor: the board's shadow, as the School's 0x293-0x294 have it
        room.decor.add((x + k, 2))


def school_bookcase(room, x, w=1):
    """The School's bookcase (0x29B-0x29C over 0x2A3-0x2A4, top layers) against the wall, 1 or 2 cells wide."""
    im = strip(SCHOOL, [[0x29B, 0x29C], [0x2A3, 0x2A4]], True)
    against_wall(room, x, im if w == 2 else narrow(im))


def long_desk(room, x, y, w, books=()):
    """The School's table (0x2AA-0x2AB) as one long desk of w cells, open books (from 0x299) at the given seats."""
    desk = hslice(strip(SCHOOL, [0x2AA, 0x2AB], True), 8, (8, 24), 8, w * T)
    book = Source(SCHOOL).cell(0x299)[1].crop((3, 0, 13, 7))
    for k in books:
        desk.alpha_composite(book, (k * T + 3, 1))
    room.put(desk, x * T, y * T)


def bed(room, x, y, blanket=None, frame=None):
    """A bed, 1x2: the houses' blanket (0x294, top layer) recolored per room under a new head cell drawn to match
    (headboard with a lit top edge, the pillow from 0x287)."""
    a, b, c, d = frame or (WOOD1, WOOD2, WOOD3, WOOD4)
    P = Pix(T, T)
    P.box(1, 1, 14, 15, c)
    P.hl(2, 13, 2, a)
    P.hl(2, 13, 3, b)
    P.hl(2, 13, 4, d)
    P.r(2, 5, 12, 11, WH)
    P.vl(13, 5, 15, GR1)
    P.im.alpha_composite(Source(HOME).cell(0x287)[1].crop((0, 8, 16, 16)), (0, 6))
    room.put(P.im, x * T, y * T)
    foot = Source(HOME).cell(0x294)[1]
    if blanket:
        cols = sorted({p[:3] for p in foot.getdata() if p[3]}, key=lambda q: -sum(q))
        light = [q for q in cols if sum(q) > 450]
        foot = recolor(foot, {q: blanket[min(i, 2)] for i, q in enumerate(light)})
    room.put(foot, x * T, (y + 1) * T)


def mirror(room, x, y):
    """A standing mirror, 1x2 (wood frame, blue glass with a glint, the course schedule taped to it)."""
    P = Pix(T, 2 * T)
    P.box(3, 1, 10, 28, WOOD3)
    P.vl(4, 2, 27, WOOD1)
    P.hl(4, 11, 2, WOOD1)
    P.vl(11, 2, 27, WOOD4)
    P.box(5, 4, 6, 21, BLUE2)
    P.r(6, 5, 4, 19, BLUE1)
    P.vl(9, 6, 22, BLUE2)
    P.vl(6, 6, 10, WH)
    P.p(7, 6, WH)
    P.box(7, 14, 5, 6, STRAW1)
    P.hl(8, 10, 16, STRAW2)
    P.hl(8, 10, 18, STRAW2)
    P.r(4, 29, 8, 2, WOOD4)
    room.put(P.im, x * T, y * T)


def photo_cabinet(room, x, y):
    """The houses' low drawers (0x0BB's top layer) with a framed photo of home on top."""
    room.fr(x, y, HOME, 0x0BB, 'top')
    P = Pix(T, T)
    P.box(4, 0, 8, 7, STRAW2)
    P.r(5, 1, 6, 3, BLUE1)
    P.r(5, 4, 6, 2, P3['gn1'])
    P.p(7, 3, WH)
    room.put(P.im, x * T, y * T)


def garden_bed(room, x, y, w=2):
    """A greenhouse planter, w cells: a wood box with a lit rim and a darker front, soil, and leafy plants (two
    greens with a highlight, an outline under each clump), a flower here and there."""
    P = Pix(w * T, T)
    P.box(0, 7, w * T, 9, WOOD3)
    P.hl(1, w * T - 2, 8, WOOD1)
    P.r(2, 9, w * T - 4, 2, SOIL2)
    P.hl(1, w * T - 2, 12, WOOD4)
    for k in range(0, w * 2):
        cx = 4 + k * 8
        for yy in range(-4, 4):
            for xx in range(-4, 4):
                d = ((xx + .5) / 4) ** 2 + ((yy + .5) / 4) ** 2
                if d <= 1:
                    P.p(cx + xx, 6 + yy, OL if d > .7 and yy > 0 else P3['gn1'] if yy < -1 and xx < 1 else P3['gn2'])
        P.p(cx - 2, 3, WH)
        if k % 3 == 1:
            P.p(cx + 1, 4, RED1)
            P.p(cx + 2, 4, RED2)
    room.put(P.im, x * T, y * T)


def against_wall(room, x, im):
    """A piece standing against the back wall: its feet 2 px onto the first floor row, as FireRed's stand (or, if it's
    too tall for that, its top just under the wall's top band)."""
    box = im.getbbox()
    floor_y = room.wall_rows * T
    y = floor_y + 2 - box[3]
    if y + box[1] < WALL_TOP:
        y = WALL_TOP - box[1]
    room.put(im, x * T, y)
    if y + box[3] > floor_y:
        for k in range(im.width // T):
            room.decor.add((x + k, room.wall_rows))


def terminal(room, x, kind='pc'):
    """A computer against the wall: the Lab's PC (0x062 over 0x30B's top layers), or the TM MACHINE (the Lab's
    analysis machine 0x2A6 over 0x2B6)."""
    a, b = (0x062, 0x30B) if kind == 'pc' else (0x2A6, 0x2B6)
    against_wall(room, x, strip(LAB, [[a], [b]], True))


def consoles(room, x):
    """The Lab's experiment consoles (0x320-0x322 over 0x328-0x32A), 3 wide, against the wall."""
    against_wall(room, x, strip(LAB, [[0x320, 0x321, 0x322], [0x328, 0x329, 0x32A]], True))


def picture(room, x):
    """The Lab's framed picture (0x324 over 0x325, top layers)."""
    room.fr(x, 0, LAB, 0x324, 'top')
    room.fr(x, 1, LAB, 0x325, 'top')


LAB_COLORS = [tuple(c) for c in Source(LAB).pals[7][1:]]
WHITE_TILE, WHITE_TILE_SHADE = 0x081, 0x082         # gTileset_BuildingFrlg (the Lab's floor)
PARQUET, PARQUET_SHADE = 0x001, 0x009               # gTileset_BuildingFrlg (the Pallet houses')
PLANKS, PLANKS_SHADE = 0x0A9, 0x0B1                 # gTileset_BuildingFrlg (the Copycat's bedroom)
SCHOOL_FLOOR, SCHOOL_SHADE = 0x29E, 0x29D           # gTileset_School


def ramp(name):
    return {'white': lambda: shadow_ramp(LAB, WHITE_TILE, WHITE_TILE_SHADE),
            'parquet': lambda: shadow_ramp(HOME, PARQUET, PARQUET_SHADE),
            'planks': lambda: shadow_ramp(HOME, PLANKS, PLANKS_SHADE),
            'school': lambda: shadow_ramp(SCHOOL, SCHOOL_FLOOR, SCHOOL_SHADE)}[name]()


def shell(room, floor, wall):
    """Floor, back wall, the wall's top, the edge columns and the void row; returns the floor's shadow ramp."""
    src, mid, shade = {'white': (LAB, WHITE_TILE, WHITE_TILE_SHADE), 'parquet': (HOME, PARQUET, PARQUET_SHADE),
                       'planks': (HOME, PLANKS, PLANKS_SHADE), 'school': (SCHOOL, SCHOOL_FLOOR, SCHOOL_SHADE)}[floor]
    room.floor(src, mid, shade)
    room.floor_spec = (src, mid, shade, floor)
    room.wall_kind = wall
    room.foot = (SCHOOL, 0x296) if wall == 'school' else (LAB, 0x079) if (wall == 'lab' and floor == 'white') else None
    if wall == 'school':
        school_wall(room)
    elif wall == 'lab':
        lab_wall(room, foot=floor == 'white')
    elif wall == 'house':
        house_wall(room)
    elif wall == 'center':
        center_wall(room)
    elif wall == 'glass':
        glass_wall(room)
    if wall == 'school':                # The School's skirting foot in the left wall's shadow (the Lab's foot runs on
        shade_cell(room, 1, 2)          # to the wall unshaded, as in the Cinnabar Lab)
    edges(room)
    wall_top(room)
    return ramp(floor)


def shade_cell(room, x, y):
    """The left wall's shadow on a floor cell that isn't plain floor (the wall's foot): the floor's shadow ramp on the
    pixels FireRed's shaded floor darkens."""
    src, mid, shade, kind = room.floor_spec
    a, b = Source(src).flat(mid).load(), Source(src).flat(shade).load()
    sh = {key(k): v for k, v in ramp(kind).items()}
    px = room.F.load()
    for yy in range(T):
        for xx in range(T):
            if a[xx, yy] != b[xx, yy]:
                c = px[x * T + xx, y * T + yy]
                if key(c) in sh:
                    px[x * T + xx, y * T + yy] = tuple(sh[key(c)]) + (255,)


def refloor(room, x, y, source, floors):
    """A FireRed piece drawn on another floor (its bottom layer) put on this room's: its pixels that are that floor
    (any of `floors`, in phase) become the room's floor (the shaded one in the first column)."""
    src, mid, shade, _ = room.floor_spec
    mine = Source(src).flat(shade if x == 1 else mid).load()
    theirs = [Source(source).flat(f).load() for f in floors]
    px = room.F.load()
    for yy in range(T):
        for xx in range(T):
            c = px[x * T + xx, y * T + yy]
            if any(key(c) == key(t[xx, yy]) for t in theirs):
                px[x * T + xx, y * T + yy] = mine[xx, yy]


# --- The rooms (collision and marks: COLLISION; the comments give the solid cells each piece stands on) --------------
def main_hall():
    """Main Hall, 19x13 (3 wall rows): marble-white tiles, a blue runner with the emblem from the door to the
    reception, pillars, benches, palms, stairs up on both sides; the arched windows of the facade (5, 7, 11, 13), the
    clock under the tower between banners, notice boards (3, 15)."""
    r = Room('MAIN_HALL')
    sh = shell(r, 'white', 'lab')
    arched(r, 5, 7, 11, 13)
    wall_clock(r, 9 * T + 8, 22)
    banner(r, 8 * T + 2, 20)
    banner(r, 10 * T + 2, 20)
    notices(r, 3, 15)
    stairs_up(r, 1)
    stairs_up(r, 16)
    lab_rug(r, 8, 6, 3, 5)
    emblem(r, 9 * T + 8, 8 * T + 8)
    counter(r, 7, 5, 5)                                     # (7-11,5)
    r.drop_shadow(sh, 7, 5, 5, 1)
    for (x, y) in ((4, 4), (14, 4), (4, 7), (14, 7)):       # (4|14, 4-5 and 7-8)
        pillar(r, x, y)
        r.drop_shadow(sh, x, y, 1, 2, below=2, right=3)
    bench(r, 1, 6, 2)                                       # (1-2,6), (16-17,6)
    bench(r, 16, 6, 2)
    palm(r, 1, 9)                                           # (1,9-10), (17,9-10)
    palm(r, 17, 9)
    door_mat(r)
    return r


def main_hall_2f():
    """The Main Hall's gallery, 19x13: stairs down both sides, a rug, display cases (moon stones; a model satellite),
    a bench, palms; the same windows and clock."""
    r = Room('MAIN_HALL_2F')
    sh = shell(r, 'white', 'lab')
    arched(r, 5, 7, 11, 13)
    wall_clock(r, 9 * T + 8, 22)
    banner(r, 8 * T + 2, 20)
    banner(r, 10 * T + 2, 20)
    stairs_down(r, 1)
    stairs_down(r, 16)
    lab_rug(r, 6, 5, 7, 5)
    display_case(r, 3, 6, 2, 1, 'stones')                  # (3-4,6)
    display_case(r, 13, 6, 3, 2, 'satellite')              # (13-15,6-7)
    r.drop_shadow(sh, 3, 6, 2, 1)
    r.drop_shadow(sh, 13, 6, 3, 2)
    bench(r, 8, 9, 3)                                       # (8-10,9)
    palm(r, 1, 10)                                          # (1,10-11), (17,10-11)
    palm(r, 17, 10)
    return r


def lecture_hall(name, board):
    """A lecture hall, 15x11: the School's classroom grown up. The board (A) or a projection screen (B) across 4-10,
    bookcases either side, the arched windows (3, 11), the lectern (7,2), two long desks a side (rows 3 and 5) with
    stools behind them (rows 4 and 6), palms (1|13, 7-8)."""
    r = Room(name)
    sh = shell(r, 'school', 'school')
    arched(r, 3, 11)
    if board:
        chalkboard(r, 4, 7)
        school_bookcase(r, 1)
        school_bookcase(r, 13)
    else:
        screen(r, 4, 7)
        school_bookcase(r, 1, 2)
        school_bookcase(r, 12, 2)
    for y in (3, 5):
        for (x, books) in ((1, (0, 3)), (9, (1, 4))):
            long_desk(r, x, y, 5, books)
            r.drop_shadow(sh, x, y, 5, 1, below=2)
            for k in range(5):
                r.fr(x + k, y + 1, SCHOOL, 0x288, 'top')
    lectern(r, 7, 2)
    r.drop_shadow(sh, 7, 2, 1, 1, below=2)
    palm(r, 1, 7)
    palm(r, 13, 7)
    door_mat(r, 'school')
    return r


def faculty_office():
    """Faculty Office, 13x10: parquet, bookshelves (1-2, 10-11), the arched windows (4, 6, 8), the staff counter
    (5-7,3), a desk each side (1|11, 4), a sofa and low table (8-10, 6-7), a palm (1, 6-7)."""
    r = Room('FACULTY_OFFICE')
    sh = shell(r, 'parquet', 'lab')
    arched(r, 4, 6, 8)
    shelves(r, 1, 0, 2)
    shelves(r, 10, 0, 2)
    counter(r, 5, 3, 3)
    r.drop_shadow(sh, 5, 3, 3, 1)
    for x in (1, 11):
        cabinet_desk(r, x, 4)
        r.drop_shadow(sh, x, 4, 1, 1, below=2)
    sofa(r, 8, 6, 3)
    low_table(r, 8, 7, 3)
    r.drop_shadow(sh, 8, 7, 3, 1, below=2)
    palm(r, 1, 6)
    door_mat(r)
    return r


def cafeteria():
    """Cafeteria, 13x10: white tiles, the kitchen along the back wall (1-5: sink, stove, cupboards, fridge), the
    serving counter (1-5,3), four tables with plates (7-8|10-11, 3-4 and 6-7), a palm (1, 6-7); windows 6, 8."""
    r = Room('CAFETERIA')
    sh = shell(r, 'white', 'lab')
    arched(r, 6, 8)
    # The houses' kitchen (sink and stove 0x140-0x141 over 0x148-0x149, the cupboard 0x114-0x115 over
    # 0x11C-0x11D, the fridge 0x117 over 0x11F), against the wall
    against_wall(r, 1, strip(HOME, [[0x140, 0x141], [0x148, 0x149]], True))
    against_wall(r, 3, strip(HOME, [[0x114, 0x115], [0x11C, 0x11D]], True))
    against_wall(r, 5, strip(HOME, [[0x117], [0x11F]], True))
    counter(r, 1, 3, 5)
    r.drop_shadow(sh, 1, 3, 5, 1)
    for (x, y) in ((7, 3), (10, 3), (7, 6), (10, 6)):
        straw_table(r, x, y, 2, plate=True)
        r.drop_shadow(sh, x, y, 2, 2, below=2)
    palm(r, 1, 6)
    door_mat(r)
    return r


def literature():
    """Literature, 15x11: parquet, bookshelves (1-2, 12-13), notices (6-8), the arched windows (4, 10), palms
    (1|13, 3-4), a reading table (6-8, 4-5), a sofa (2-4, 7), two desks (10|12, 7)."""
    r = Room('LITERATURE')
    sh = shell(r, 'parquet', 'lab')
    arched(r, 4, 10)
    shelves(r, 1, 0, 2)
    shelves(r, 12, 0, 2)
    notices(r, 6, 7, 8)
    palm(r, 1, 3)
    palm(r, 13, 3)
    straw_table(r, 6, 4, 3)
    r.drop_shadow(sh, 6, 4, 3, 2, below=2)
    sofa(r, 2, 7, 3)
    for x in (10, 12):
        cabinet_desk(r, x, 7)
        r.drop_shadow(sh, x, 7, 1, 1, below=2)
    door_mat(r)
    return r


def library():
    """Library, 15x11: parquet, wall shelves (1-3, 11-13) round the arched windows (4, 7, 10), the librarian's
    counter (6-8,3), freestanding shelves (1-4|10-13, 4-5), a reading table (6-8, 6-7), palms (1|13, 7-8)."""
    r = Room('LIBRARY')
    sh = shell(r, 'parquet', 'lab')
    arched(r, 4, 7, 10)
    shelves(r, 1, 0, 3)
    shelves(r, 11, 0, 3)
    counter(r, 6, 3, 3)
    r.drop_shadow(sh, 6, 3, 3, 1)
    for x in (1, 10):
        shelves(r, x, 4, 4, standing=True)
        r.drop_shadow(sh, x, 4, 4, 2, below=3)
    straw_table(r, 6, 6, 3)
    r.drop_shadow(sh, 6, 6, 3, 2, below=2)
    palm(r, 1, 7)
    palm(r, 13, 7)
    door_mat(r)
    return r


def study_hall():
    """Study Hall, 15x10 (the small building by the dorms): the Lab lounge's look. Shelves of old exam papers (1-2,
    12-13), notices (6-8), the square windows (3, 5, 9, 11), study tables (2-4, 3-4 and 6-7) with stools where people
    sit, the lounge on a rug (9-13, 4-7: sofa 9-12, low table 10-11), the vending machine (13, 2-3), a palm (1, 7-8)."""
    r = Room('STUDY_HALL')
    sh = shell(r, 'white', 'lab')
    square(r, 3, 5, 9, 11)
    notices(r, 6, 7, 8)
    shelves(r, 1, 0, 2)
    shelves(r, 12, 0, 2)
    lab_rug(r, 9, 4, 5, 4)
    sofa(r, 9, 4, 4)
    low_table(r, 10, 6, 2)
    for y in (3, 6):
        straw_table(r, 2, y, 3)
        r.drop_shadow(sh, 2, y, 3, 2, below=3, right=2)
    for (x, y) in ((5, 4), (3, 5), (5, 7)):
        stool(r, x, y)
        r.decor.add((x, y))
    vending(r, 13, 2)
    r.drop_shadow(sh, 13, 2, 1, 2, below=0, right=0)
    palm(r, 1, 7)
    door_mat(r)
    return r


def home_economics():
    """Home Economics, 21x11, in the greenhouse: the glass wall, white tiles, eight planters (2x1), the tasting table
    (9-11, 3-4), notices (4, 16), palms (1|19, 8-9)."""
    r = Room('HOME_ECONOMICS')
    sh = shell(r, 'white', 'glass')
    notices(r, 4, 16)
    for (x, y) in ((2, 3), (5, 3), (14, 3), (17, 3), (2, 6), (5, 6), (14, 6), (17, 6)):
        garden_bed(r, x, y)
        r.drop_shadow(sh, x, y, 2, 1, below=2)
    straw_table(r, 9, 3, 3, plate=True)
    r.drop_shadow(sh, 9, 3, 3, 2, below=2)
    palm(r, 1, 8)
    palm(r, 19, 8)
    door_mat(r)
    return r


def dorm_lobby():
    """Dorm lobby, 21x12: the houses' look. Stairs up (1-2), the square windows (4, 13), mailboxes (7-9), notices
    (11-12), a bookshelf (16-17), the vending machine (19, 1-2), the warden's counter (3-6,5), the lounge on a rug
    (12-18, 5-8: sofa 13-17, low table 14-16), a low palm under the window (4,2), palms (1|19, 8-9)."""
    r = Room('DORM_LOBBY')
    sh = shell(r, 'planks', 'house')
    square(r, 4, 13)
    stairs_up(r, 1)
    mailboxes(r, 7, 3)
    notice_house(r, 11, 12)
    shelves(r, 16, 0, 2)
    vending(r, 19, 1)
    counter(r, 3, 5, 4)
    r.drop_shadow(sh, 3, 5, 4, 1)
    house_rug(r, 12, 5, 7, 4)
    sofa(r, 13, 5, 5)
    low_table(r, 14, 7, 3)
    r.fr(4, 2, LAB, 0x094, 'top')           # A low palm under the window (the palm's pot cell alone), (4,2)
    palm(r, 1, 8)
    palm(r, 19, 8)
    door_mat(r)
    return r


def partition(room, y, gaps):
    """The wall between the corridor and the bedrooms, one row: its top seen from above over the bedrooms' wall face
    and teal skirting (0x0B0's lower half)."""
    face = Source(HOME).flat(0x0B0)
    for x in range(1, room.w - 1):
        if x in gaps:
            continue
        P = Pix(T, T)
        P.im.paste(face, (0, 0))
        for k, c in enumerate((OL, WH, GR1, GR1, GR2, OL)):
            P.hl(0, T - 1, k, c)
        room.F.paste(P.im, (x * T, y * T))
        room.walls.add((x, y))
    for x in gaps:
        for xx in (x * T - 1, (x + 1) * T):
            for k in range(T):
                room.F.putpixel((xx, y * T + k), OL + (255,))
    # The top runs on over the edge columns' wall tops
    for x0 in (10, (room.w - 1) * T):
        P = Pix(6, 6)
        for k, c in enumerate((OL, WH, GR1, GR1, GR2, OL)):
            P.hl(0, 5, k, c)
        room.F.paste(P.im, (x0, y * T))


def partition_v(room, x, y0, y1, sh):
    """A wall between two bedrooms, seen from above: a light bar, the floor either side in its shadow."""
    for y in range(y0, y1):
        P = Pix(T, T)
        for i, c in enumerate((OL, WH, GR1, GR1, GR1, GR1, GR2, OL)):
            P.vl(4 + i, 0, T - 1, c)
        room.F.alpha_composite(P.im, (x * T, y * T))
        room.walls.add((x, y))
    # Up through the corridor wall's face to its top, which runs across it
    P = Pix(T, T - 5)
    for i, c in enumerate((OL, WH, GR1, GR1, GR1, GR1, GR2, OL)):
        P.vl(4 + i, 0, P.h - 1, c)
    room.F.alpha_composite(P.im, (x * T, (y0 - 1) * T + 5))
    room.shade(sh, (x * T + 12, y0 * T, x * T + 20, y1 * T))


def dorm_floor(top):
    """A dorm floor, 21x12: the corridor (rows 2-3; stairs down 1-2 on 2F, 18-19 on 3F; up 18-19 on 2F), notices
    (9-10), the square windows (4, 7, 13, 16); the partition (row 4, doorways 3, 10, 16; walls 7, 13) and three
    bedrooms: bed (1|8|14, 5-6), desk (5|11|18, 5), palm (5|11|18, 8-9). 2F's third room is the player's: the bed,
    a mirror with the schedule (17, 5-6), the PC desk (18,5), a photo of home (19,5), textbooks (19, 7-8), a rug."""
    r = Room('DORM_3F' if top else 'DORM_2F')
    sh = shell(r, 'planks', 'house')
    square(r, 4, 7, 13, 16)
    notice_house(r, 9, 10)
    if top:
        stairs_down(r, 18)
        palm(r, 1, 2)
    else:
        stairs_down(r, 1)
        stairs_up(r, 18)
    partition(r, 4, (3, 10, 16))
    partition_v(r, 7, 5, 11, sh)
    partition_v(r, 13, 5, 11, sh)
    rooms = [(1, 5, (BLUE1, BLUE2, BLUE3)), (8, 11, (RED1, RED2, RED3))]
    if top:
        rooms.append((14, 18, (P3['gn1'], P3['gn2'], hx('4A8B52'))))
    for (bx, dx, blanket) in rooms:
        bed(r, bx, 5, blanket)
        r.drop_shadow(sh, bx, 5, 1, 2, below=3)
        cabinet_desk(r, dx, 5)
        r.drop_shadow(sh, dx, 5, 1, 1, below=2)
        palm(r, dx, 8)
    if not top:
        bed(r, 14, 5, (P3['gn1'], P3['gn2'], hx('4A8B52')))
        r.drop_shadow(sh, 14, 5, 1, 2, below=3)
        house_rug(r, 15, 7, 3, 3)
        mirror(r, 17, 5)
        r.fr(18, 5, HOME, 0x28F, 'top')
        photo_cabinet(r, 19, 5)
        r.drop_shadow(sh, 17, 5, 3, 1, below=2)
        r.fr(19, 7, HOME, 0x0B6, 'top')
        r.fr(19, 8, HOME, 0x0BE, 'top')
    return r


def science(name):
    """The Science Institute's floors, 17x15, in the glass tower: the glass wall, white tiles, the Lab's furniture."""
    r = Room(name)
    sh = shell(r, 'white', 'glass')
    if name == 'SCIENCE_INSTITUTE':
        # The director's hall: a runner to the stairs, a picture and the floor directory, desks either side
        lab_rug(r, 7, 2, 3, 11)
        stairs_up(r, 14)
        picture(r, 4)
        notices(r, 12)
        for x in (1, 13):                                   # (1-3|13-15, 4-5)
            lab_desk(r, x, 4)
            r.drop_shadow(sh, x, 4, 3, 2)
        palm(r, 1, 10)                                      # (1|15, 10-11)
        palm(r, 15, 10)
    elif name == 'SCIENCE_LOUNGE':
        stairs_down(r, 14)
        stairs_up(r, 1)
        shelves(r, 4, 0, 1)
        lab_rug(r, 4, 5, 9, 5)
        sofa(r, 5, 5, 4)                                    # (5-8,5)
        sofa(r, 10, 5, 2)                                   # (10-11,5)
        low_table(r, 6, 7, 3)                               # (6-8,7)
        vending(r, 15, 6)                                   # (15, 6-7)
        palm(r, 1, 11)                                      # (1|15, 11-12)
        palm(r, 15, 11)
    elif name == 'SCIENCE_RESEARCH_ROOM':
        stairs_down(r, 1)
        stairs_up(r, 14)
        terminal(r, 4)
        terminal(r, 8)
        terminal(r, 11, 'tm')                               # The TM MACHINE (11,1)
        for (x, y) in ((11, 7), (3, 9), (11, 10)):          # (11-13, 7-8), (3-5, 9-10), (11-13, 10-11)
            lab_desk(r, x, y)
            r.drop_shadow(sh, x, y, 3, 2)
        palm(r, 15, 4)                                      # (15, 4-5)
    else:
        stairs_down(r, 14)
        terminal(r, 3)
        terminal(r, 5)
        consoles(r, 7)
        consoles(r, 10)
        for (x, y) in ((2, 5), (7, 5), (2, 9), (7, 9)):     # 3x2 benches
            lab_desk(r, x, y)
            r.drop_shadow(sh, x, y, 3, 2)
        palm(r, 15, 11)
    door_mat(r)
    return r


def hospital(upstairs):
    """The University Hospital, 17x12: the Pokemon Center's wall and counter, white tiles, the square windows of the
    facade. 1F: stairs up (14-15), the counter (6-10,4) with the healing machine behind it, benches (1-2, 6 and 8),
    a palm (15, 8-9). 2F, the ward: stairs down (14-15), six beds (1|3|5, 2-3 and 7-8), a palm."""
    r = Room('HOSPITAL_2F' if upstairs else 'HOSPITAL')
    sh = shell(r, 'white', 'center')
    if upstairs:
        square(r, 4, 6, 8, 10, 12)
        stairs_down(r, 14)
        for y in (2, 7):
            for x in (1, 3, 5):
                bed(r, x, y, (PINK1, PINK2, hx('CD7B83')), (GR1, WH, GR2, GR3))
                r.drop_shadow(sh, x, y, 1, 2, below=2)
    else:
        square(r, 4, 6, 10, 12)
        stairs_up(r, 14)
        machine = strip(CENTER, [[0x2AA, 0x2AB], [0x2B2, 0x2B3]], True)
        against_wall(r, 8, machine.crop((0, 0, 2 * T, T + 9)))      # Its counter is the one in front
        r.put(center_counter(5), 6 * T, 4 * T)
        r.drop_shadow(sh, 6, 4, 5, 1)
        for y in (6, 8):
            bench(r, 1, y, 2)
    palm(r, 15, 8)
    door_mat(r)
    return r


def center_counter(w):
    """The Pokemon Center's counter front (0x2B9, 0x2BC, 0x298 with its card, 0x2BD; top layers), w cells, the
    flower pots of the row above it wiped off its ends (the top edge taken from the plain middle, 0x2BC)."""
    im = strip(CENTER, [0x2B9, 0x2BC, 0x298, 0x2BC, 0x2BD], True)
    mid = Source(CENTER).cell(0x2BC)[1]
    for x0, xs in ((0, range(4, T)), (4 * T, range(0, 12))):
        for y in range(10):
            for x in xs:
                im.putpixel((x0 + x, y), mid.getpixel((x, y)))
    return hslice(im, 16, (16, 32), 16, w * T) if w != 5 else im


# name -> (builder, tileset group)
ROOMS = {
    'MAIN_HALL': (main_hall, 'classroom'),
    'MAIN_HALL_2F': (main_hall_2f, 'classroom'),
    'LECTURE_HALL_A': (lambda: lecture_hall('LECTURE_HALL_A', True), 'classroom'),
    'LECTURE_HALL_B': (lambda: lecture_hall('LECTURE_HALL_B', False), 'classroom'),
    'FACULTY_OFFICE': (faculty_office, 'classroom'),
    'LITERATURE': (literature, 'classroom'),
    'LIBRARY': (library, 'classroom'),
    'CAFETERIA': (cafeteria, 'dorm'),
    'DORM_LOBBY': (dorm_lobby, 'dorm'),
    'DORM_2F': (lambda: dorm_floor(False), 'dorm'),
    'DORM_3F': (lambda: dorm_floor(True), 'dorm'),
    'SCIENCE_INSTITUTE': (lambda: science('SCIENCE_INSTITUTE'), 'lab'),
    'SCIENCE_LOUNGE': (lambda: science('SCIENCE_LOUNGE'), 'lab'),
    'SCIENCE_RESEARCH_ROOM': (lambda: science('SCIENCE_RESEARCH_ROOM'), 'lab'),
    'SCIENCE_EXPERIMENT_ROOM': (lambda: science('SCIENCE_EXPERIMENT_ROOM'), 'lab'),
    'HOSPITAL': (lambda: hospital(False), 'lab'),
    'HOSPITAL_2F': (lambda: hospital(True), 'lab'),
    'STUDY_HALL': (study_hall, 'lab'),
    'HOME_ECONOMICS': (home_economics, 'lab'),
}
TILESETS = {
    'classroom': ('gTileset_UniversityClassroom', 'data/tilesets/secondary/university_classroom'),
    'dorm': ('gTileset_UniversityDorm', 'data/tilesets/secondary/university_dorm'),
    'lab': ('gTileset_UniversityLab', 'data/tilesets/secondary/university_lab'),
}


def make_rooms():
    out = {}
    for name, (build, group) in ROOMS.items():
        r = build()
        r.group = group
        out[name] = r
    return out


# --- The seam audit ----------------------------------------------------------------------------------------------
def audit(room):
    """Where the pieces of a room don't meet cleanly. Returns (kind, x, y, detail) for:
    - 'floor': floor pixels (not covered by the middle layer) that aren't the room's floor in its own phase, its
      shaded column, or either one in the floor's shadow ramp (another floor showing through a piece's bottom layer,
      a pattern restarting mid-floor);
    - 'jump': a vertical cell boundary where the picture's lines (rows of a color running across it) jump by 1-3
      rows (a rail, skirting or edge a pixel off), anywhere in the room;
    - 'step': a horizontal cell boundary in the floor where the floor doesn't continue (shadow bands that stop, a
      wall foot that ends)."""
    out = []
    src, mid, shade, kind = room.floor_spec
    sh = ramp(kind)
    plain, shaded = Source(src).flat(mid).load(), Source(src).flat(shade).load()
    down = {key(a): key(b) for a, b in sh.items()}
    F, O = room.F.load(), room.O.load()
    img = room.image()
    px = img.load()
    foot = Source(room.foot[0]).flat(room.foot[1]).load() if getattr(room, 'foot', None) else None
    for cy in range(room.wall_rows, room.h - 1):
        for cx in range(1, room.w - 1):
            if (cx, cy) in room.decor or (cx, cy) in room.walls or (cx, cy) in room.marks:
                continue
            bad = 0
            for y in range(T):
                for x in range(T):
                    X, Y = cx * T + x, cy * T + y
                    if O[X, Y][3]:
                        continue
                    ok = {key(plain[x, y]), key(shaded[x, y])}
                    if foot and cy == room.wall_rows:
                        ok.add(key(foot[x, y]))
                        ok |= {down[key(foot[x, y])]} if key(foot[x, y]) in down else set()
                    ok |= {down[k] for k in list(ok) if k in down}
                    if key(F[X, Y]) not in ok:
                        bad += 1
            if bad:
                out.append(('floor', cx, cy, bad))
    W, H = img.size
    for bx in range(T, W, T):
        col_a = [key(px[bx - 1, y]) for y in range(H)]
        col_b = [key(px[bx, y]) for y in range(H)]
        for cy in range(room.h):
            ys = range(cy * T, cy * T + T)
            mism = [y for y in ys if col_a[y] != col_b[y]]
            if 1 <= len(mism) <= 3:
                # A line that jumps: the left cell's edge column matches the right one's shifted by a row or two
                for d in (-2, -1, 1, 2):
                    if all(0 <= y + d < H and col_a[y] == col_b[y + d] for y in mism):
                        out.append(('jump', bx // T, cy, 'rows %s shift %d' % (mism, d)))
                        break
    for by in range((room.wall_rows + 1) * T, (room.h - 1) * T, T):
        for cx in range(1, room.w - 1):
            a = [key(F[cx * T + x, by - 1]) for x in range(T)]
            b = [key(F[cx * T + x, by]) for x in range(T)]
            # The floor's own pattern across the boundary, for comparison
            pa = [key(plain[x, 15]) for x in range(T)]
            pb = [key(plain[x, 0]) for x in range(T)]
            covered = any(O[cx * T + x, by - 1][3] or O[cx * T + x, by][3] for x in range(T))
            if not covered and sum(p != q for p, q in zip(a, b)) > sum(p != q for p, q in zip(pa, pb)) + 4:
                out.append(('step', cx, by // T, ''))
    return out


# --- Writing the tilesets -------------------------------------------------------------------------------------------
class FrlgTileset:
    """A secondary tileset over gTileset_BuildingFrlg built from composed rooms. Both layers of every cell are cut
    into 8x8 tiles; near-identical colors are merged (SNAP); a tile identical to one of the primary's (in one of its
    palettes 0-5) is referenced; any other tile gets a primary palette when one holds its colors, else one of up to
    six secondary palettes (packed first-fit, the slot needing the fewest new colors), and is stored once (flips
    deduplicated, a primary tile's pixels reused under another palette). Writes the folder like the game's tilesets
    (tiles.png indexed, palettes/00-15.pal, metatiles.bin, metatile_attributes.bin)."""

    def __init__(self, symbol, folder):
        self.symbol, self.folder = symbol, os.path.join(ROOT, folder)
        pt, pp, _ = R.load_tileset(R.tileset_folder(PRIMARY), PRIMARY)
        self.ppt = pt
        self.ppals = [[key(c) for c in pp[i]] for i in range(16)]
        self.bg = self.ppals[0][0]
        self.reps = list(dict.fromkeys(c for p in self.ppals[:6] for c in p[1:]))
        n = len(pt[0]) // 8
        self.ptiles = {}            # (layer, colors) -> primary entry
        self.pidx = {}              # index rows -> (tile, hflip, vflip)
        for t in range(min(PRIMARY_BASE, (len(pt) // 8) * n)):
            rows = tuple(tuple(pt[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8]) for y in range(8))
            for hf in (0, 1):
                for vf in (0, 1):
                    f = tuple(tuple(r[::-1]) if hf else r for r in (rows[::-1] if vf else rows))
                    self.pidx.setdefault(f, (t, hf, vf))
                    for p in range(6):
                        for layer in (0, 1):
                            k = tuple(tuple((None if layer else self.bg) if v == 0 else self.ppals[p][v] for v in r)
                                      for r in f)
                            self.ptiles.setdefault((layer, k), t | hf << 10 | vf << 11 | p << 12)
        self.tiles, self.lookup = [], {}
        self.metatiles, self.attrs, self.mlookup = [], [], {}
        self.pals = []

    def snap(self, c):
        for r in self.reps:
            if sum(abs(a - b) for a, b in zip(c, r)) <= SNAP:
                return r
        self.reps.append(c)
        return c

    def quarters(self, im, cx, cy, layer):
        px = im.load()
        out = []
        for qy in (0, 8):
            for qx in (0, 8):
                out.append(tuple(tuple(
                    self.snap(key(px[cx * T + qx + x, cy * T + qy + y])) if px[cx * T + qx + x, cy * T + qy + y][3]
                    else None for x in range(8)) for y in range(8)))
        return out

    def colorset(self, layer, k):
        return frozenset(c for r in k for c in r if c is not None and not (layer == 0 and c == self.bg))

    def build(self, rooms, MB):
        """rooms: {name: Room} -> {name: stamp} (metatile rows, collision, marks, this tileset's symbol)."""
        beh = {'mat': MB['MB_SOUTH_ARROW_WARP'], 'up': MB['MB_NON_ANIMATED_DOOR'], 'down': MB['MB_NON_ANIMATED_DOOR']}
        cells = {}
        new = set()
        for name, r in rooms.items():
            for y in range(r.h):
                for x in range(r.w):
                    q = [(0, k) for k in self.quarters(r.F, x, y, 0)] + [(1, k) for k in self.quarters(r.O, x, y, 1)]
                    cells[(name, x, y)] = q
                    for (layer, k) in q:
                        if (layer, k) not in self.ptiles and not (layer and all(v is None for row in k for v in row)):
                            new.add((layer, k))
        # Palettes for the new tiles
        prim = [set(p[1:]) for p in self.ppals[:6]]
        sets = {self.colorset(layer, k) for (layer, k) in new}
        rest = [c for c in sets if not any(c <= p for p in prim)]
        too_many = [c for c in rest if len(c) > 15]
        assert not too_many, '%s: a tile with %d colors' % (self.symbol, len(too_many[0]))
        maximal = [c for c in rest if not any(c < d for d in rest)]
        best = None
        rng = random.Random(1)
        orders = [sorted(maximal, key=lambda c: (-len(c), sorted(c)))]
        for _ in range(400):                   # First fit over shuffled orders (seeded: the result is stable)
            o = sorted(maximal, key=lambda c: (-len(c), sorted(c)))
            rng.shuffle(o)
            orders.append(o)
        for order in orders:
            sec = []
            for cols in order:
                fits = [p for p in sec if len(p | cols) <= 15]
                if fits:
                    min(fits, key=lambda p: (len(cols - p), -len(p))).update(cols)
                else:
                    sec.append(set(cols))
            if best is None or len(sec) < len(best):
                best = sec
        assert len(best) <= MAX_SECONDARY_PALS, '%s: %d palettes' % (self.symbol, len(best))
        self.pals = [sorted(p) for p in best]
        # Metatiles
        out = {}
        for name, r in rooms.items():
            rows = []
            for y in range(r.h):
                row = []
                for x in range(r.w):
                    entries = [self.entry(layer, k) for (layer, k) in cells[(name, x, y)]]
                    attr = beh.get(r.marks.get((x, y)), MB['MB_NORMAL']) | LAYER_COVERED << 29
                    mkey = (tuple(entries), attr)
                    if mkey not in self.mlookup:
                        self.mlookup[mkey] = PRIMARY_BASE + len(self.metatiles)
                        self.metatiles.append(tuple(entries))
                        self.attrs.append(attr)
                    row.append(self.mlookup[mkey])
                rows.append(row)
            solid = [[0 if (x, y) in r.marks else r.solid[y][x] for x in range(r.w)] for y in range(r.h)]
            out[name] = {'w': r.w, 'h': r.h, 'rows': rows, 'solid': solid, 'tileset': self.symbol,
                         'marks': {'%d,%d' % k: v for k, v in r.marks.items()}}
        assert len(self.tiles) <= MAX_TILES and len(self.metatiles) <= MAX_METATILES, self.budget()
        return out

    def entry(self, layer, k):
        if layer and all(v is None for row in k for v in row):
            return 0
        if (layer, k) in self.ptiles:
            return self.ptiles[(layer, k)]
        cols = self.colorset(layer, k)
        for p in range(6):
            pal = self.ppals[p]
            if cols <= set(pal[1:]):
                return self.store(layer, k, pal, p)
        for i, pal in enumerate(self.pals):
            if cols <= set(pal):
                return self.store(layer, k, [None] + pal, FIRST_PAL + i)
        raise AssertionError('no palette for a tile')

    def store(self, layer, k, pal, slot):
        idx = tuple(tuple(0 if (c is None or (layer == 0 and c == self.bg)) else pal.index(c, 1) for c in row)
                    for row in k)
        if idx in self.pidx:
            t, hf, vf = self.pidx[idx]
            return t | hf << 10 | vf << 11 | slot << 12
        for hf in (0, 1):
            for vf in (0, 1):
                f = tuple(tuple(r[::-1]) if hf else r for r in (idx[::-1] if vf else idx))
                if f in self.lookup:
                    return (PRIMARY_BASE + self.lookup[f]) | hf << 10 | vf << 11 | slot << 12
        self.lookup[idx] = len(self.tiles)
        self.tiles.append(idx)
        return (PRIMARY_BASE + len(self.tiles) - 1) | slot << 12

    def write(self):
        os.makedirs(os.path.join(self.folder, 'palettes'), exist_ok=True)
        rows_n = (len(self.tiles) + 15) // 16
        image = [[0] * 128 for _ in range(rows_n * 8)]
        for n, px in enumerate(self.tiles):
            for y in range(8):
                image[(n // 16) * 8 + y][(n % 16) * 8:(n % 16) * 8 + 8] = px[y]
        grey = b''.join(bytes((v * 17, v * 17, v * 17)) for v in range(16))
        U.write_png(os.path.join(self.folder, 'tiles.png'), 128, image, grey)
        for i in range(16):
            slot = i - FIRST_PAL
            cols = [(0, 0, 0)] + ([unkey(c) for c in self.pals[slot]] if 0 <= slot < len(self.pals) else [])
            cols += [(0, 0, 0)] * (16 - len(cols))
            open(os.path.join(self.folder, 'palettes/%02d.pal' % i), 'w', newline='').write(
                'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in cols))
        with open(os.path.join(self.folder, 'metatiles.bin'), 'wb') as f:
            for e in self.metatiles:
                f.write(struct.pack('<8H', *e))
        with open(os.path.join(self.folder, 'metatile_attributes.bin'), 'wb') as f:
            for a in self.attrs:
                f.write(struct.pack('<I', a))

    def budget(self):
        return '%s: %d/%d tiles, %d/%d metatiles, %d/%d secondary palettes (%s colors)' % (
            self.symbol, len(self.tiles), MAX_TILES, len(self.metatiles), MAX_METATILES, len(self.pals),
            MAX_SECONDARY_PALS, '+'.join(str(len(p)) for p in self.pals))

    # Rendering what was written (the mockups come from this, so they show what the game will)
    def render_room(self, stamp):
        img = Image.new('RGBA', (stamp['w'] * T, stamp['h'] * T), (0, 0, 0, 255))
        px = img.load()
        n = len(self.ppt[0]) // 8
        for cy, row in enumerate(stamp['rows']):
            for cx, mid in enumerate(row):
                entries = self.metatiles[mid - PRIMARY_BASE]
                for layer in (0, 1):
                    for q in range(4):
                        e = entries[layer * 4 + q]
                        t, hf, vf, p = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
                        if t >= PRIMARY_BASE:
                            idx = self.tiles[t - PRIMARY_BASE]
                        else:
                            idx = [self.ppt[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]
                        pal = self.ppals[p] if p < FIRST_PAL else [self.bg] + self.pals[p - FIRST_PAL]
                        for y in range(8):
                            for x in range(8):
                                v = idx[7 - y if vf else y][7 - x if hf else x]
                                if v == 0 and layer == 1:
                                    continue
                                c = self.bg if v == 0 else pal[v]
                                px[cx * T + (q % 2) * 8 + x, cy * T + (q // 2) * 8 + y] = unkey(c) + (255,)
        return img


def _sheet(self, path, per_row=16, s=3):
    """Every metatile of the tileset, in order (scale 3)."""
    ids = list(range(PRIMARY_BASE, PRIMARY_BASE + len(self.metatiles)))
    rows = [ids[i:i + per_row] for i in range(0, len(ids), per_row)]
    rows[-1] += [rows[-1][-1]] * (per_row - len(rows[-1]))
    img = self.render_room({'w': per_row, 'h': len(rows), 'rows': rows})
    U.scaled(img, s).convert('RGB').save(path)


FrlgTileset.sheet = _sheet


def build(MB):
    """Composes every room, writes the three tilesets, returns the rooms' stamps (for university_stamps.json) and the
    tilesets (for the mockups)."""
    rooms = make_rooms()
    stamps, sets = {}, {}
    for group, (symbol, folder) in TILESETS.items():
        ts = FrlgTileset(symbol, folder)
        stamps.update(ts.build({n: r for n, r in rooms.items() if r.group == group}, MB))
        print(ts.budget())
        ts.write()
        sets[symbol] = ts
    for name, r in rooms.items():
        for problem in r.check():
            print('%s: %s' % (name, problem))
    return stamps, sets


# --- Mockups ----------------------------------------------------------------------------------------------------------
def map_folder(room):
    return 'CeladonUniversity_' + ''.join(p.capitalize() for p in room.split('_')).replace('2f', '2F').replace(
        '3f', '3F')


def people(img, room):
    """The room's people (its object events, by their graphics) drawn where they stand."""
    m = json.load(open(os.path.join(ROOT, 'data/maps', map_folder(room), 'map.json')))
    for o in m.get('object_events') or []:
        gfx = o.get('graphics_id', '').replace('OBJ_EVENT_GFX_', '').lower()
        for name in ('people/' + gfx, 'people/' + gfx.replace('_frlg', ''), 'pokemon/' + gfx):
            path = os.path.join(ROOT, 'graphics/object_events/pics', name + '.png')
            if os.path.exists(path):
                facing = {'MOVEMENT_TYPE_FACE_UP': U.UP, 'MOVEMENT_TYPE_FACE_LEFT': U.LEFT}.get(
                    o.get('movement_type'), U.DOWN)
                U.put(img, name, o['x'], o['y'], facing)
                break


def mockups(stamps, sets, before_dir=None):
    """interior_<room>.png for every room (scale 3, its people), interiors.png (all of them), and with a folder of
    the rooms as they were (before_dir/<ROOM>.png, scale 1), interiors_before_after.png."""
    order = list(ROOMS)
    shots = {}
    for name in order:
        st = stamps[name]
        img = sets[st['tileset']].render_room(st)
        people(img, name)
        shots[name] = img
        U.scaled(img, 3).convert('RGB').save(os.path.join(OUT, 'interior_%s.png' % name.lower()))
    titles = {n: n.replace('_', ' ').title().replace('2F', '2F').replace('3F', '3F') for n in order}

    def grid(pairs, cols, s, path, header):
        cw = max(p[0].width for p in pairs) * s
        ch = max(p[0].height for p in pairs) * s
        rows_n = (len(pairs) + cols - 1) // cols
        pad, cap = 16, 26
        sheet = Image.new('RGBA', (cols * (cw + pad) + pad, rows_n * (ch + cap + pad) + 50), (24, 24, 36, 255))
        U.label(sheet, header, (pad, 12), 20)
        for i, (im, text) in enumerate(pairs):
            x = pad + (i % cols) * (cw + pad)
            y = 50 + (i // cols) * (ch + cap + pad)
            U.label(sheet, text, (x, y), 14)
            sheet.alpha_composite(U.scaled(im, s), (x, y + cap))
        sheet.convert('RGB').save(path)

    grid([(shots[n], '%s (%s)' % (titles[n], stamps[n]['tileset'].replace('gTileset_', ''))) for n in order], 4, 1,
         os.path.join(OUT, 'interiors.png'), 'Celadon University interiors (FireRed look)')
    if before_dir:
        pairs = []
        for n in order:
            before = Image.open(os.path.join(before_dir, n + '.png')).convert('RGBA')
            people(before, n)
            both = Image.new('RGBA', (before.width * 2 + 8, before.height), (24, 24, 36, 255))
            both.alpha_composite(before, (0, 0))
            both.alpha_composite(shots[n], (before.width + 8, 0))
            pairs.append((both, '%s: before | after' % titles[n]))
        grid(pairs, 2, 1, os.path.join(OUT, 'interiors_before_after.png'),
             'Celadon University interiors: v1 (left) and the FireRed look (right)')


def main():
    """Renders the rooms from the tilesets as written (run draw_university_tiles.py to write them)."""
    MB = U.behaviors()
    rooms = make_rooms()
    stamps, sets = {}, {}
    for group, (symbol, folder) in TILESETS.items():
        ts = FrlgTileset(symbol, folder)
        stamps.update(ts.build({n: r for n, r in rooms.items() if r.group == group}, MB))
        print(ts.budget())
        sets[symbol] = ts
    before = sys.argv[sys.argv.index('--before') + 1] if '--before' in sys.argv else None
    mockups(stamps, sets, before)
    if '--audit' in sys.argv:
        # What's left is by design: curves (the clock, the emblem, chalk) crossing cell edges read as line jumps,
        # and the stairs' landings carry FireRed's own rail feet and shadow
        for name, r in rooms.items():
            for issue in audit(r):
                print(name, *issue)
        for (im, left, mid, right, width, seams) in HSLICE_SEAMS:
            print('widened %dx%d piece (ends %d+%d, middle %s) to %d: joins differ at %s' % (
                im.width, im.height, left, right, mid, width, seams))


if __name__ == '__main__':
    main()
