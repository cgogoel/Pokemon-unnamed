#!/usr/bin/env python3
"""Celadon University interiors, v2 (DRAFT): the rooms redone in FireRed's own interior look
(docs/hack/university_remake/interior_art.md).

    <venv with Pillow>/bin/python tools/hack/draw_university_interiors_v2.py

v1 (draw_university_tiles.py) drew every room from scratch in the campus's palettes: navy outlines, saturated flat
fills, no highlights or drop shadows, 1-2 shade furniture. FireRed's interiors read differently: a warm grey-violet
outline (#52526A) everywhere, soft desaturated ramps of 3-4 shades per material, a white highlight along every top
edge, drop shadows on the floor, and furniture built from small parts (legs, drawers, books, chalk trays).

So v2 builds rooms from FireRed's own metatiles wherever one fits (the School's classroom, the Lab's lounge, the
houses' bedrooms: gTileset_BuildingFrlg furniture costs nothing, the rooms already use it as their primary), and
draws only what FireRed doesn't have, in FireRed's colors and with its detail rules.

This draft writes NO tileset. It writes, into docs/hack/university_remake/:
    interior_v2_<room>.png            the room redone (scale 3)
    interior_v2_<room>_compare.png    before (the current layout, rendered from the game data) | after
    interior_v2_reference.png         the FireRed rooms the look is taken from
and prints the GBA budget of a secondary tileset holding the redone rooms: tiles and metatiles (deduplicated with
flips; tiles identical to a gTileset_BuildingFrlg tile cost nothing) and the palettes they pack into (tiles that fit
one of the primary's palettes 0-5 need no secondary palette slot).
"""
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # noqa: E402
import draw_university_tiles as U  # noqa: E402

ROOT = R.ROOT
OUT = os.path.join(ROOT, 'docs/hack/university_remake')
T = 16
PRIMARY = 'gTileset_BuildingFrlg'
MAX_TILES = MAX_METATILES = 384
MAX_SECONDARY_PALS = 6       # Slots 7-12
SNAP = 2                     # Colors this close (5-bit steps, summed over r, g, b) count as one


def hx(s):
    return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4))


def key(c):
    """A color as the GBA stores it (5 bits a channel): FireRed's .pal files say 0x52 where we'd write 0x50."""
    return tuple(v >> 3 for v in c[:3])


# --- FireRed's colors (from the .pal files; which tileset each comes from is in interior_art.md) ------------------
OL = hx('52526A')            # The outline of every FRLG interior
WH = hx('FFFFFF')
GR1, GR2, GR3 = hx('D5D5D5'), hx('B4B4A4'), hx('7B7B7B')               # Neutral greys (Condominiums, Lab)
# The School (gTileset_School palette 7): cream plaster, tan trim, teal skirting, the chalkboard
CREAM1, CREAM2, CREAM3 = hx('FFE6AC'), hx('D5BD83'), hx('AC945A')
TEAL1, TEAL2, TEAL3, TEAL4 = hx('BDE6F6'), hx('9CCDDE'), hx('7BB4C5'), hx('629CAC')
BOARD1, BOARD2 = hx('94CD7B'), hx('629C4A')
SCH_GREY, SCH_LILAC = hx('83838B'), hx('C5BDD5')
# The Lab (gTileset_Lab_Frlg palette 7): orange wood furniture, blue glass/upholstery, straw table tops
WOOD1, WOOD2, WOOD3, WOOD4 = hx('FFCD8B'), hx('E6A462'), hx('C58341'), hx('AC5A18')
BLUE1, BLUE2, BLUE3 = hx('BDE6FF'), hx('83BDDE'), hx('5294C5')
STRAW1, STRAW2 = hx('F6EE94'), hx('C5B44A')
LAB_G1, LAB_G2, LAB_G3 = hx('BDC5D5'), hx('B4ACB4'), hx('8B8394')
# gTileset_BuildingFrlg palette 3: slate blue and gold, our school colors, already in the primary (no palette slot)
P3 = {'ol': OL, 'g': hx('7B7B7B'), 'g2': hx('ACACA4'), 'w': WH, 'au2': hx('E6B44A'), 'au1': hx('FFF683'),
      'sl3': hx('41629C'), 'sl2': hx('5A7BB4'), 'sl1': hx('739CC5'), 'cr': hx('DED583'), 'st2': hx('A4A4AC'),
      'st1': hx('CDCDD5'), 'st0': hx('EEEEF6'), 'gn2': hx('6AAC73'), 'gn1': hx('9CCD8B')}
# gTileset_BuildingFrlg palette 0: reds for the door mat, rugs
RED1, RED2 = hx('FF8373'), hx('CD5241')
BLACK = (0, 0, 0)


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
        rows, t = (self.pt, t) if t < 640 else (self.st, t - 640)
        n = len(rows[0]) // 8
        if (t // n) * 8 >= len(rows):
            return None
        return [rows[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]

    def cell(self, mid):
        entries = self.pm[mid] if mid < 640 else self.sm[mid - 640]
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
HOUSE = 'gTileset_GenericBuilding2'
CONDO = 'gTileset_Condominiums'
DEPT = 'gTileset_DepartmentStore'


def src(name):
    return Source(name)


# --- A room being composed ---------------------------------------------------------------------------------------
class Room:
    """Two RGBA layers like a map's metatiles: F (bottom: floor, walls, low furniture) and O (top: furniture over the
    floor). Cells come from FireRed metatiles or new art; collision and the room's events stay where they are."""

    def __init__(self, name, w, h):
        self.name, self.w, self.h = name, w, h
        self.F = Image.new('RGBA', (w * T, h * T), BLACK + (255,))
        self.O = Image.new('RGBA', (w * T, h * T), (0, 0, 0, 0))
        self.people = []

    def fr(self, x, y, source, mid, mode='both'):
        """FireRed metatile `mid` of `source` at cell (x, y): both layers, or only its top layer over our floor."""
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

    def paste(self, layer, im, x, y):
        (self.F if layer == 'F' else self.O).alpha_composite(im, (x, y))

    def fill_floor(self, source, mid, x0, y0, x1, y1):
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.fr(x, y, source, mid, 'bottom')

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
        """The shadow under a piece of furniture at pixel box (x, y, w, h): a band below it (and on its right)."""
        self.shade(ramp, (x + 2, y + h, x + w + right, y + h + below))
        if right:
            self.shade(ramp, (x + w, y + 3, x + w + right, y + h))

    def image(self, people=True):
        im = self.F.copy()
        im.alpha_composite(self.O)
        if people:
            for (spr, x, y, f) in self.people:
                U.put(im, spr, x, y, f)
        return im


def shadow_ramp(source, plain, shaded):
    """The floor's shadow colors, read off a FireRed pair (the plain floor and its shaded variant)."""
    a, b = Source(source).flat(plain), Source(source).flat(shaded)
    pa, pb = a.load(), b.load()
    ramp = {}
    for y in range(T):
        for x in range(T):
            if pa[x, y] != pb[x, y]:
                ramp.setdefault(pa[x, y][:3], pb[x, y][:3])
    return ramp


# --- Drawing helpers (new art, FireRed rules) --------------------------------------------------------------------
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
    its last `right` px (so a 2-wide board or desk becomes any width with FireRed's own ends)."""
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
    return out


def strip(source, mids):
    """FireRed metatiles side by side (rows of ids), flattened (both layers)."""
    rows = [mids] if isinstance(mids[0], int) else mids
    im = Image.new('RGBA', (len(rows[0]) * T, len(rows) * T), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        for x, mid in enumerate(row):
            if mid is not None:
                im.alpha_composite(Source(source).flat(mid), (x * T, y * T))
    return im


def strip_top(source, mids):
    """Only the top layers of FireRed metatiles, side by side."""
    rows = [mids] if isinstance(mids[0], int) else mids
    im = Image.new('RGBA', (len(rows[0]) * T, len(rows) * T), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        for x, mid in enumerate(row):
            if mid is not None:
                im.alpha_composite(Source(source).cell(mid)[1], (x * T, y * T))
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


# --- The shell: walls the FireRed way ----------------------------------------------------------------------------
def wall_top_band(room, x0, x1, y=0):
    """The wall's top edge seen from above, across the top of row 0 (FireRed: outline, white, light grey, outline)."""
    P = Pix((x1 - x0) * T, 5)
    for k, c in enumerate((OL, WH, WH, GR1, OL)):
        P.hl(0, P.w - 1, k, c)
    room.paste('F', P.im, x0 * T, y)


def side_walls(room, top_rows=0):
    """The solid edge columns: black beyond, a light wall top (outline, white, light grey, outline) on the room side,
    like the Condominiums' and Silph's walls; the bottom row black (FireRed's rooms end in the void)."""
    W, H = room.w, room.h
    for side in (0, 1):
        P = Pix(T, (H - 1) * T)
        P.r(0, 0, T, P.h, BLACK)
        cols = (OL, WH, GR1, GR1, GR1, OL)
        for i, c in enumerate(cols):
            x = 10 + i if side == 0 else 5 - i
            P.vl(x, 0, P.h - 1, c)
        P.hl(10 if side == 0 else 0, 15 if side == 0 else 5, 0, OL)
        room.F.paste(P.im, (0 if side == 0 else (W - 1) * T, 0))
    room.F.paste(Image.new('RGBA', (W * T, T), BLACK + (255,)), (0, (H - 1) * T))


def exit_mat(room, door, source, mats):
    """FireRed's door mat: three cells over the floor on the row above the void, its lower edge in the void
    (gTileset_BuildingFrlg 0x01A-0x01C)."""
    for i, mid in enumerate(mats):
        room.fr(door - 1 + i, room.h - 2, source, mid, 'top')
    for i, mid in enumerate((0x01A, 0x01B, 0x01C)):
        room.fr(door - 1 + i, room.h - 1, SCHOOL, mid, 'both')


# --- The budget --------------------------------------------------------------------------------------------------
class Budget:
    """What a secondary tileset holding these rooms would cost: every 8x8 tile of both layers, deduplicated with
    flips; a tile identical to one of gTileset_BuildingFrlg's (in one of its palettes 0-5) is free; the rest are
    packed into palettes (the primary's 0-5 when a tile fits one, else secondary slots of 15 colors)."""

    def __init__(self):
        pt, pp, _ = R.load_tileset(R.tileset_folder(PRIMARY), PRIMARY)
        self.ppals = [[key(c) for c in pp[i]] for i in range(6)]
        self.bg = key(pp[0][0])
        self.primary = set()
        n = len(pt[0]) // 8
        for t in range(min(640, (len(pt) // 8) * n)):
            rows = [pt[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]
            for p in range(6):
                for layer in (0, 1):
                    k = tuple(tuple((None if layer else self.bg) if v == 0 else self.ppals[p][v] for v in r)
                              for r in rows)
                    for f in self.flips(k):
                        self.primary.add((layer, f))
        self.tiles = {}            # canonical (layer, pixels) -> color set
        self.metatiles = set()
        self.reused = 0

    @staticmethod
    def flips(k):
        return (k, tuple(r[::-1] for r in k), k[::-1], tuple(r[::-1] for r in k[::-1]))

    def add(self, room):
        F, O = room.F.load(), room.O.load()
        for cy in range(room.h):
            for cx in range(room.w):
                cell = []
                for layer, px in ((0, F), (1, O)):
                    for qy in (0, 8):
                        for qx in (0, 8):
                            k = tuple(tuple(
                                (key(px[cx * T + qx + x, cy * T + qy + y]) if px[cx * T + qx + x, cy * T + qy + y][3]
                                 else None) for x in range(8)) for y in range(8))
                            cell.append(self.entry(layer, k))
                self.metatiles.add(tuple(cell))

    def entry(self, layer, k):
        if layer == 1 and all(v is None for r in k for v in r):
            return 'empty'
        if (layer, k) in self.primary:
            self.reused += 1
            return ('P', layer, k)
        canon = min(self.flips(k), key=repr)
        if (layer, canon) not in self.tiles:
            self.tiles[(layer, canon)] = frozenset(v for r in k for v in r if v is not None)
        return ('S', layer, canon)

    def snapped(self):
        """The tiles' color sets with near-identical colors merged (FireRed's tilesets each have their own white,
        light grey and mid grey a step or two apart; one shared tileset would use one of each), primary colors kept
        as they are so tiles that fit a primary palette still do."""
        prim = [c for p in self.ppals for c in p[1:]]
        reps = list(dict.fromkeys(prim))

        def snap(c):
            for r in reps:
                if sum(abs(a - b) for a, b in zip(c, r)) <= SNAP:
                    return r
            reps.append(c)
            return c
        return [frozenset(snap(c) for c in sorted(cols)) for cols in self.tiles.values()]

    def palettes(self):
        """Packing of the new tiles' color sets: a primary palette when one holds them (no slot), else secondary
        slots of 15 colors (first fit decreasing, the slot that needs the fewest new colors; a few orders tried)."""
        prim = [set(p[1:]) for p in self.ppals]
        sets = set(self.snapped())
        rest = [c for c in sets if not any(c <= p for p in prim)]
        in_primary = len(sets) - len(rest)
        unfit = sum(1 for c in rest if len(c) > 15)
        rest = [c for c in rest if len(c) <= 15 and not any(c < d for d in rest)]
        best = None
        for order in (lambda c: -len(c), lambda c: (-len(c), sorted(c)), lambda c: sorted(c)):
            sec = []
            for cols in sorted(rest, key=order):
                fits = [p for p in sec if len(p | cols) <= 15]
                if fits:
                    min(fits, key=lambda p: len(cols - p)).update(cols)
                else:
                    sec.append(set(cols))
            if best is None or len(sec) < len(best):
                best = sec
        return best, in_primary, unfit

    def report(self, label):
        sec, in_primary, unfit = self.palettes()
        return ('%s: %d/%d new tiles, %d/%d metatiles, %d/%d secondary palettes (%s colors), %d color sets fit a '
                'primary palette, %d tiles over 15 colors' % (
                    label, len(self.tiles), MAX_TILES, len(self.metatiles), MAX_METATILES, len(sec),
                    MAX_SECONDARY_PALS, '+'.join(str(len(p)) for p in sec), in_primary, unfit))


# --- Pieces ------------------------------------------------------------------------------------------------------
def school_wall(room, x0, x1):
    """The School's back wall (gTileset_School): cream plaster (row 0, 0x283), and on row 1 the tan rail, the
    wainscot and the teal skirting of its window wall (0x28E's lower half) under plain plaster."""
    for x in range(x0, x1):
        room.fr(x, 0, SCHOOL, 0x283, 'bottom')
        P = Pix(T, T)
        P.r(0, 0, T, 7, CREAM1)
        P.hl(0, T - 1, 7, CREAM3)
        P.r(0, 8, T, 4, CREAM2)
        room.F.paste(Source(SCHOOL).flat(0x28E), (x * T, T))
        room.F.paste(P.im.crop((0, 0, T, 12)), (x * T, T))
        room.fr(x, 2, SCHOOL, 0x296, 'bottom')


def school_window(room, x):
    """A one-cell window from the School's three-cell one (0x285-0x287 over 0x28D-0x28F: its two ends)."""
    for row, (a, b) in ((0, (0x285, 0x287)), (1, (0x28D, 0x28F)), (2, (0x295, 0x297))):
        room.F.paste(strip(SCHOOL, [a, b]).crop((8, 0, 24, T)), (x * T, row * T))


def chalkboard(room, x, w, y=1):
    """The School's board (0x28B-0x28C, chalk tray 0x293-0x294) widened to w cells with its own ends, wiped, and the
    Fire-Water-Grass triangle chalked on it (white and pale green chalk, the board's own colors)."""
    board = hslice(strip(SCHOOL, [0x28B, 0x28C]), 8, (8, 24), 8, w * T)
    tray = hslice(strip(SCHOOL, [0x293, 0x294]), 8, (8, 24), 8, w * T)
    px = board.load()
    for yy in range(2, 13):
        for xx in range(3, w * T - 3):
            if key(px[xx, yy]) in (key(WH), key(BOARD1)):
                px[xx, yy] = BOARD2 + (255,)
    # The tray's chalk and eraser, once, at the right
    for xx in range(4, w * T - 30):
        for yy in range(13, 16):
            if key(px[xx, yy]) in (key(CREAM1),):
                px[xx, yy] = OL + (255,)
    P = Pix(w * T, T)
    cx = w * T // 2

    def ring(x0, y0, c):
        for dx, dy in ((1, 0), (2, 0), (0, 1), (3, 1), (0, 2), (3, 2), (1, 3), (2, 3)):
            P.p(x0 + dx, y0 + dy, c)
    ring(cx - 2, 2, WH)                 # Fire (top)
    ring(cx - 10, 9, WH)                # Water
    ring(cx + 6, 9, WH)                 # Grass
    for t in range(1, 5):               # The arrows between them
        P.p(cx - 3 - t, 5 + t, BOARD1)
        P.p(cx + 2 + t, 5 + t, BOARD1)
    P.hl(cx - 5, cx + 4, 11, BOARD1)
    for k, xx in enumerate(range(6, cx - 16, 2)):    # Notes in chalk
        P.p(xx, 4 + (k % 3 == 0), WH)
        if k % 4:
            P.p(xx, 8, WH)
    for k, xx in enumerate(range(cx + 16, w * T - 10, 2)):
        P.p(xx, 4, WH if k % 5 else BOARD1)
        if k % 3:
            P.p(xx, 8, WH)
    board.alpha_composite(P.im)
    room.F.paste(board, (x * T, y * T))
    room.F.paste(tray, (x * T, (y + 1) * T))


def bookcase_1x2(room, x, y, source=SCHOOL, top=(0x29B, 0x29C), bottom=(0x2A3, 0x2A4)):
    """The School's two-cell bookcase (0x29B-0x29C over 0x2A3-0x2A4) narrowed to one cell (its two ends), top layer."""
    for row, ids in ((0, top), (1, bottom)):
        im = hslice(strip_top(source, list(ids)), 8, (8, 9), 8, T)
        room.O.alpha_composite(im, (x * T, (y + row) * T))


def long_desk(room, x, y, w, books=()):
    """The School's table (0x2AA-0x2AB: straw top, front rail, legs) as one long desk of w cells, with open books
    (from the School's desk 0x299) at the given seats."""
    desk = hslice(strip_top(SCHOOL, [0x2AA, 0x2AB]), 8, (8, 24), 8, w * T)
    book = Source(SCHOOL).cell(0x299)[1].crop((3, 0, 13, 7))
    for k in books:
        desk.alpha_composite(book, (k * T + 3, 1))
    room.O.alpha_composite(desk, (x * T, y * T))


def lectern(room, x, y):
    """A lectern (new art, the Lab's orange wood ramp, FireRed's rules: outline, a lit top edge, a front panel with
    a darker inset, a shadow under the lip), with a gold plaque in the middle."""
    P = Pix(T, T)
    P.box(2, 1, 12, 4, WOOD2)           # The sloped top
    P.hl(3, 12, 2, WOOD1)
    P.hl(3, 12, 4, WOOD3)
    P.r(5, 2, 6, 2, WH)                 # Notes on it
    P.hl(6, 9, 3, LAB_G1)
    P.box(3, 5, 10, 10, WOOD3)          # The body
    P.hl(4, 11, 6, WOOD4)               # The lip's shadow
    P.r(5, 8, 6, 5, WOOD4)              # The front panel's inset
    P.r(6, 9, 4, 3, WOOD3)
    P.p(7, 10, STRAW1)
    P.p(8, 10, STRAW1)
    P.vl(4, 7, 13, WOOD2)               # The lit left edge
    room.O.alpha_composite(P.im, (x * T, y * T))


def plant(room, x, y, flip=False):
    """The School's potted palm (0x281 over 0x289; 0x282 over 0x28A is its mirror), top layer."""
    a, b = (0x282, 0x28A) if flip else (0x281, 0x289)
    room.fr(x, y, SCHOOL, a, 'top')
    room.fr(x, y + 1, SCHOOL, b, 'top')


# --- The rooms ---------------------------------------------------------------------------------------------------
UP, DOWN, LEFT = U.UP, U.DOWN, U.LEFT
PLAYER = U.PLAYER
SCHOOL_FLOOR = 0x29E
SCHOOL_SHADE = None


def school_shade():
    global SCHOOL_SHADE
    if SCHOOL_SHADE is None:
        SCHOOL_SHADE = shadow_ramp(SCHOOL, 0x29E, 0x29D)
    return SCHOOL_SHADE


def lecture_hall():
    """Lecture Hall A, 15x11, the School's classroom grown up: same cells as v1 (bookcases (1,0-1) and (13,0-1),
    windows 2 and 12, the board 4-10, the lectern (7,2), desks on rows 3 and 5 with seats below, palms (1,7) and
    (13,7), the door mat (7,9)), so collision and events don't move."""
    r = Room('LECTURE_HALL_A', 15, 11)
    r.fill_floor(SCHOOL, SCHOOL_FLOOR, 1, 2, 14, 10)
    school_wall(r, 1, 14)
    r.fr(1, 2, SCHOOL, 0x29D, 'bottom')
    for y in range(3, 10):
        r.fr(1, y, SCHOOL, 0x29D, 'bottom')             # The left wall's shadow down the first column
    for c in (2, 12):
        school_window(r, c)
    chalkboard(r, 4, 7)
    side_walls(r)
    wall_top_band(r, 1, 14)
    sh = school_shade()
    for y in (3, 5):
        for (x, books) in ((1, (0, 3)), (9, (1, 4))):
            long_desk(r, x, y, 5, books)
            r.drop_shadow(sh, x * T, y * T, 5 * T, 14, below=2)
            for k in range(5):
                r.fr(x + k, y + 1, SCHOOL, 0x288, 'top')
                r.drop_shadow(sh, (x + k) * T + 3, (y + 1) * T + 4, 10, 8, below=1)
    lectern(r, 7, 2)
    r.drop_shadow(sh, 7 * T + 3, 2 * T + 1, 10, 14, below=2)
    bookcase_1x2(r, 1, 0)
    bookcase_1x2(r, 13, 0)
    plant(r, 1, 7)
    plant(r, 13, 7, flip=True)
    exit_mat(r, 7, SCHOOL, (0x2A5, 0x2A6, 0x2A7))
    r.people += [('people/scientist', 8, 2, DOWN), ('people/lass_frlg', 5, 4, UP), ('people/camper_frlg', 2, 6, UP),
                 ('people/picnicker_frlg', 3, 6, UP), ('people/boy', 11, 6, UP), (PLAYER, 7, 8, UP)]
    return r


# --- The dorm: FireRed's house set (all gTileset_BuildingFrlg, so free) -------------------------------------------
HOUSE_FLOOR, HOUSE_FLOOR_SHADE = 0x0A9, 0x0B1       # The Copycat's bedroom: light planks, the shaded column


def house_wall(room, x0, x1, y=0):
    """The houses' grey-green plaster (0x0A8 over 0x0B0: white wall top, teal skirting)."""
    for x in range(x0, x1):
        room.fr(x, y, HOME, 0x0A8, 'bottom')
        room.fr(x, y + 1, HOME, 0x0B0, 'bottom')


def partition(room, x0, x1, y, gaps=()):
    """A wall between the corridor and the bedrooms, one row: its top seen from above (outline, white, light grey,
    outline) over the bedrooms' wall face and teal skirting (the lower half of 0x0B0)."""
    face = Source(HOME).flat(0x0B0)
    for x in range(x0, x1):
        if x in gaps:
            continue
        P = Pix(T, T)
        P.im.paste(face, (0, 0))
        for k, c in enumerate((OL, WH, GR1, GR1, GR2, OL)):
            P.hl(0, T - 1, k, c)
        room.F.paste(P.im, (x * T, y * T))
    for x in gaps:                      # The wall's cut ends either side of a doorway
        for xx in (x * T - 1, (x + 1) * T):
            for k in range(T):
                room.F.putpixel((xx, y * T + k), OL + (255,))


def partition_v(room, x, y0, y1):
    """A wall between two bedrooms, seen from above: a light bar with outlines, the floor either side (the right-hand
    room's floor in the wall's shadow)."""
    for y in range(y0, y1):
        room.fr(x, y, HOME, HOUSE_FLOOR, 'bottom')
        P = Pix(T, T)
        for i, c in enumerate((OL, WH, GR1, GR1, GR1, GR1, GR2, OL)):
            P.vl(4 + i, 0, T - 1, c)
        room.F.alpha_composite(P.im, (x * T, y * T))
    room.shade(shadow_ramp(HOME, HOUSE_FLOOR, HOUSE_FLOOR_SHADE), (x * T + 12, y0 * T, x * T + 20, y1 * T))


def bed(room, x, y, blanket=None):
    """A bed, 1x2: the houses' blanket (0x294, top layer) under a new head cell drawn to match (wood headboard with a
    lit top edge, the pillow from 0x287); the blanket recolored per room."""
    P = Pix(T, T)
    P.box(1, 1, 14, 15, WOOD3)          # Headboard
    P.hl(2, 13, 2, WOOD1)
    P.hl(2, 13, 3, WOOD2)
    P.hl(2, 13, 4, WOOD4)
    P.r(2, 5, 12, 11, WH)               # Sheet
    P.vl(13, 5, 15, GR1)
    pillow = Source(HOME).cell(0x287)[1]
    P.im.alpha_composite(pillow.crop((0, 8, 16, 16)), (0, 6))
    room.O.alpha_composite(P.im, (x * T, y * T))
    foot = Source(HOME).cell(0x294)[1]
    if blanket:
        foot = recolor(foot, blanket)
    room.O.alpha_composite(foot, (x * T, (y + 1) * T))


def blanket_ramp(name):
    """The blanket's light greys (FireRed's exact values, read off 0x294) onto a color ramp."""
    b = Source(HOME).cell(0x294)[1]
    cols = sorted({px[:3] for px in b.getdata() if px[3]}, key=lambda c: -sum(c))
    ramp = {'blue': (BLUE1, BLUE2, BLUE3), 'red': (RED1, RED2, hx('A43129')),
            'green': (P3['gn1'], P3['gn2'], hx('4A8B52'))}[name]
    light = [c for c in cols if sum(c) > 450]
    return {c: ramp[min(i, 2)] for i, c in enumerate(light)}


def desk(room, x, y, pc=False):
    """The houses' desk, 1x2 (top layer): 0x289 (straw top) over 0x291 (drawer), or the PC desk 0x28F over 0x290
    (its blue chair pulled in)."""
    a, b = (0x28F, 0x290) if pc else (0x289, 0x291)
    room.fr(x, y, HOME, a, 'top')
    room.fr(x, y + 1, HOME, b, 'top')


def mirror(room, x, y):
    """A standing mirror, 1x2 (new art: Lab wood frame, Lab blue glass with a white glint, the course schedule
    taped to it), with the outline and lit edges FireRed's furniture has."""
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
    P.box(7, 14, 5, 6, STRAW1)          # The schedule
    P.hl(8, 10, 16, STRAW2)
    P.hl(8, 10, 18, STRAW2)
    P.r(4, 29, 8, 2, WOOD4)             # Feet
    P.p(3, 30, OL)
    P.p(12, 30, OL)
    room.O.alpha_composite(P.im, (x * T, y * T))


def photo_cabinet(room, x, y):
    """The houses' low drawers (0x0BB, top layer) with a framed photo of home on top (new art)."""
    room.fr(x, y, HOME, 0x0BB, 'top')
    P = Pix(T, T)
    P.box(4, 0, 8, 7, STRAW2)
    P.r(5, 1, 6, 3, BLUE1)              # Sky
    P.r(5, 4, 6, 2, P3['gn1'])          # Home's fields
    P.p(7, 3, WH)
    room.O.alpha_composite(P.im, (x * T, y * T))


def dorm_floor(top=False):
    """A dorm floor, 21x12, v1's plan (corridor rows 2-3, the partition on row 4 with doorways at 3, 10, 16, walls at
    7 and 13, three bedrooms) in the houses' look."""
    r = Room('DORM_3F' if top else 'DORM_2F', 21, 12)
    r.fill_floor(HOME, HOUSE_FLOOR, 1, 2, 20, 11)
    house_wall(r, 1, 20)
    for y in range(2, 11):
        r.fr(1, y, HOME, HOUSE_FLOOR_SHADE, 'bottom')
    for c in (5, 13):
        r.fr(c, 0, HOME, 0x0B8, 'both')
        r.fr(c, 1, HOME, 0x0C0, 'both')
    for c in (9, 10):
        r.fr(c, 1, HOME, 0x0D6, 'both')
    # Stairs: down (the Pallet house's, 0x015 / 0x005-0x006 / 0x00D-0x00E), up (the Copycat's, 0x0EA / 0x0DA-0x0DB /
    # 0x0E2-0x0E3), shifted up a row from FireRed's so the corridor stays clear
    down_x, up_x = (18, None) if top else (1, 18)
    r.fr(down_x, 0, HOME, 0x015, 'both')
    r.fr_block(down_x, 1, HOME, [[0x005, 0x006], [0x00D, 0x00E]])
    if up_x:
        r.fr(up_x, 0, HOME, 0x0EA, 'both')
        r.fr_block(up_x, 1, HOME, [[0x0DA, 0x0DB], [0x0E2, 0x0E3]])
    side_walls(r)
    wall_top_band(r, 1, 20)
    partition(r, 1, 20, 4, gaps=(3, 10, 16))
    partition_v(r, 7, 5, 11)
    partition_v(r, 13, 5, 11)
    sh = shadow_ramp(HOME, HOUSE_FLOOR, HOUSE_FLOOR_SHADE)
    rooms = [(1, 5, 'blue'), (8, 11, 'red')] + ([(14, 18, 'green')] if top else [])
    for (bx, dx, col) in rooms:
        bed(r, bx, 5, blanket_ramp(col))
        r.drop_shadow(sh, bx * T + 1, 5 * T, 14, 2 * T, below=3)
        desk(r, dx, 5)
        r.drop_shadow(sh, dx * T, 5 * T, 16, 2 * T - 1, below=2)
        r.fr(dx, 8, HOME, 0x0F5, 'top')
        r.fr(dx, 9, HOME, 0x0FD, 'top')
    if not top:                          # The player's room
        bed(r, 14, 5, blanket_ramp('green'))
        r.drop_shadow(sh, 14 * T + 1, 5 * T, 14, 2 * T, below=3)
        r.fr_block(15, 7, HOME, [[0x0C3, 0x0C4, 0x0C5], [0x0CB, 0x0CC, 0x0CD], [0x0D3, 0x0D4, 0x0D5]], 'both')
        mirror(r, 17, 5)
        desk(r, 18, 5, pc=True)
        r.drop_shadow(sh, 17 * T, 5 * T, 3 * T, 2 * T - 1, below=2)
        photo_cabinet(r, 19, 5)
        r.fr(19, 7, HOME, 0x0B6, 'top')
        r.fr(19, 8, HOME, 0x0BE, 'top')
        r.people += [(PLAYER, 15, 6, DOWN)]
    r.people += [('people/cooltrainer_m', 12, 3, LEFT), ('people/lass_frlg', 10, 3, DOWN),
                 ('people/camper_frlg', 3, 7, DOWN), ('people/picnicker_frlg', 10, 7, DOWN)]
    return r


def dorm_2f():
    return dorm_floor(False)


# --- The Study Hall: the Cinnabar Lab's lounge (gTileset_Lab_Frlg over gTileset_BuildingFrlg's white tiles) --------
LAB_FLOOR, LAB_FLOOR_SHADE = 0x081, 0x082


def lab_wall(room, x0, x1):
    """The Lab's cream wall with the gold-brown rail (0x069 over 0x071; 0x079: the floor along its foot)."""
    for x in range(x0, x1):
        room.fr(x, 0, LAB, 0x069, 'bottom')
        room.fr(x, 1, LAB, 0x071, 'bottom')
        room.fr(x, 2, LAB, 0x079, 'bottom')


def study_hall():
    """The Study Hall, 15x10, v1's plan (shelves (1-2,0-1) and (12-13,0-1), the notice board 6-8, windows 4 and 10,
    two study tables (2-4, rows 3-4 and 6-7), the lounge on a rug (9-13, 4-7) with a sofa and a low table, the
    vending machine (13,2-3), a palm (1,7), the mat (7,8)) in the Lab lounge's look."""
    r = Room('STUDY_HALL', 15, 10)
    r.fill_floor(LAB, LAB_FLOOR, 1, 2, 14, 9)
    lab_wall(r, 1, 14)
    for y in range(3, 9):
        r.fr(1, y, LAB, LAB_FLOOR_SHADE, 'bottom')
    for c in (4, 10):                                   # Small windows (0x314 over 0x31C)
        r.fr(c, 0, LAB, 0x314, 'both')
        r.fr(c, 1, LAB, 0x31C, 'both')
    for c in (6, 7, 8):                                 # Notices on strings (Oak's lab: 0x28D over 0x295)
        r.fr(c, 0, LAB, 0x28D, 'top')
        r.fr(c, 1, LAB, 0x295, 'top')
    for x in (1, 12):                                   # Bookshelves (0x073-0x074 over 0x07B-0x07C), a row up
        r.fr_block(x, 0, LAB, [[0x073, 0x074], [0x07B, 0x07C]], 'top')
    side_walls(r)
    wall_top_band(r, 1, 14)
    sh = shadow_ramp(LAB, LAB_FLOOR, LAB_FLOOR_SHADE)
    # The lounge: the Lab's rug (0x2EC-0x2F7, plain 0x2F3), the Condominiums' sofa widened, the School's low table
    rug = [[0x2EC, 0x2ED, 0x2ED, 0x2ED, 0x2EE]] + [[0x2F7, 0x2F3, 0x2F3, 0x2F3, 0x2EF]] * 2 + \
          [[0x2F4, 0x2F5, 0x2F5, 0x2F5, 0x2F6]]
    for dy, row in enumerate(rug):
        for dx, mid in enumerate(row):
            r.fr(9 + dx, 4 + dy, LAB, mid, 'bottom' if mid == 0x2F3 else 'top')
    sofa = hslice(strip_top(CONDO, [0x2E0, 0x2E1, 0x2E2]), 16, (16, 32), 16, 4 * T)
    r.O.alpha_composite(sofa, (9 * T, 4 * T))
    low = strip_top(SCHOOL, [0x2AA, 0x2AB])
    r.O.alpha_composite(low, (10 * T, 6 * T))
    # Study tables: the Lab lounge's straw table (0x2E0-0x2E2 over 0x2F0-0x2F2), orange stools (0x2CB) either end
    for y in (3, 6):
        r.fr_block(2, y, LAB, [[0x2E0, 0x2E1, 0x2E2], [0x2F0, 0x2F1, 0x2F2]], 'top')
        r.drop_shadow(sh, 2 * T + 1, y * T + 2, 3 * T - 2, 2 * T - 2, below=3, right=2)
        for (sx, sy) in ((1, y), (5, y), (5, y + 1)):
            if (sx, sy) != (1, 7):
                r.fr(sx, sy, LAB, 0x2CB, 'top')
    r.fr(3, 5, LAB, 0x2CB, 'top')
    # The vending machine (the Department Store roof's, 0x325 over 0x32D)
    r.fr(13, 2, DEPT, 0x325, 'both')
    r.fr(13, 3, DEPT, 0x32D, 'both')
    r.drop_shadow(sh, 13 * T, 2 * T, T, 2 * T, below=0, right=3)
    r.fr(1, 7, HOME, 0x0F5, 'top')
    r.fr(1, 8, HOME, 0x0FD, 'top')
    exit_mat(r, 7, LAB, (0x290, 0x291, 0x292))
    r.people += [('people/youngster_frlg', 5, 4, LEFT), ('people/lass_frlg', 3, 5, UP),
                 ('people/cooltrainer_f', 11, 5, UP), ('people/fat_man_frlg', 12, 3, DOWN), (PLAYER, 7, 7, UP)]
    return r


# --- Mockups -----------------------------------------------------------------------------------------------------
ROOMS = (('lecture_hall_a', 'LAYOUT_CELADON_UNIVERSITY_LECTURE_HALL_A', 'Lecture Hall A', lecture_hall),
         ('dorm_2f', 'LAYOUT_CELADON_UNIVERSITY_DORM_2F', 'Dorm 2F (bedrooms)', dorm_2f),
         ('study_hall', 'LAYOUT_CELADON_UNIVERSITY_STUDY_HALL', 'Study Hall', study_hall))

REFERENCES = (('LAYOUT_VIRIDIAN_CITY_SCHOOL', 'Viridian School (gTileset_School)'),
              ('LAYOUT_CINNABAR_ISLAND_POKEMON_LAB_LOUNGE', 'Cinnabar Lab lounge (gTileset_Lab_Frlg)'),
              ('LAYOUT_SAFFRON_CITY_COPYCATS_HOUSE_2F', "Copycat's bedroom (BuildingFrlg's house set)"),
              ('LAYOUT_HOUSE3_FRLG', 'A house (BuildingFrlg + GenericBuilding2)'))


def layout_image(layout_id):
    rows, _ = R.render(layout_id)
    im = Image.new('RGBA', (len(rows[0]), len(rows)))
    im.putdata([tuple(c) + (255,) for row in rows for c in row])
    return im


def titled(panels, title, s=3, pad=24, top=64):
    """Pictures side by side (scaled), each with its caption, under a title."""
    sc = [U.scaled(p, s) for p, _ in panels]
    w = sum(p.width for p in sc) + pad * (len(sc) + 1)
    h = max(p.height for p in sc) + top + pad + 30
    canvas = Image.new('RGBA', (w, h), (24, 24, 36, 255))
    U.label(canvas, title, (pad, 14), 22)
    x = pad
    for p, (_, cap) in zip(sc, panels):
        canvas.alpha_composite(p, (x, top + 30))
        U.label(canvas, cap, (x, top), 16)
        x += p.width + pad
    return canvas


def main():
    budget = Budget()
    for slug, layout, title, build in ROOMS:
        r = build()
        budget.add(r)
        print(Budget.report(_one(r), title))
        after = r.image()
        U.scaled(after, 3).convert('RGB').save(os.path.join(OUT, 'interior_v2_%s.png' % slug))
        before = layout_image(layout)
        for (spr, x, y, f) in r.people:
            U.put(before, spr, x, y, f)
        titled([(before, 'Before: v1 (gTileset_UniversityInterior), as the game draws it'),
                (after, 'After: v2 draft, FireRed metatiles + matching new art')],
               '%s, %dx%d' % (title, r.w, r.h)).convert('RGB').save(
            os.path.join(OUT, 'interior_v2_%s_compare.png' % slug))
    print(budget.report('All three rooms in one secondary'))
    refs = [(layout_image(l), cap) for l, cap in REFERENCES]
    titled(refs[:2], 'FireRed interiors the v2 look comes from', s=2).convert('RGB').save(
        os.path.join(OUT, 'interior_v2_reference.png'))
    a = Image.open(os.path.join(OUT, 'interior_v2_reference.png'))
    b = titled(refs[2:], '', s=2, top=4)
    both = Image.new('RGB', (max(a.width, b.width), a.height + b.height), (24, 24, 36))
    both.paste(a, (0, 0))
    both.paste(b.convert('RGB'), (0, a.height))
    both.save(os.path.join(OUT, 'interior_v2_reference.png'))


def _one(room):
    b = Budget()
    b.add(room)
    return b


if __name__ == '__main__':
    main()
