#!/usr/bin/env python3
"""DRAFT: Celadon University's own look (docs/hack/university_remake/plan.md). Draws the campus buildings and three
interiors as GBA pixel art and writes the design mockups; later it becomes the generator of the real tilesets.

The look follows the Tamamushi University lore art: white stone collegiate buildings with grey-blue slate mansard
roofs, tall arched windows and gold trim, the clock tower over the Main Hall, the Gyarados fountain in front and the
arched glass greenhouse behind, all wrapped in dense forest. The interiors carry the same materials inside: the
facade's white stone becomes the plaster, the roof's slate the wainscot, the windows are the same windows, the gold
the trim; the floors are marble (Main Hall), parquet (lecture halls) and planks (dorms), with a Gyarados-blue carpet.

Everything is drawn only with the colors of the planned palettes (PALETTES), so every 8x8 tile fits one 15-color
palette, and on the 16x16 metatile grid, so the pieces repeat and deduplicate. `budget()` cuts the art into 8x8
tiles (flips deduplicated) and checks them against the FRLG secondary tileset limits: 384 tiles, 384 metatiles and
6 palettes (7-12). Grass, trees, paving and flowers are the real gTileset_General_Frlg metatiles (the campus keeps
that primary tileset), rendered from the game's data.

Needs Pillow:
    <venv>/bin/python tools/hack/draw_university_tiles.py
writes docs/hack/university_remake/mockup_*.png and prints the tile budget.
"""
import math
import os
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # noqa: E402

ROOT = R.ROOT
OUT = os.path.join(ROOT, 'docs/hack/university_remake')
T = 16


def gba(c):
    """A color as the GBA shows it (5 bits a channel)."""
    return tuple((v >> 3) << 3 for v in c)


# --- Palettes (each 15 colors + transparent; FRLG secondary palettes 7-12) ---------------------------------------------
OL = (40, 48, 72)        # The outline: a dark slate blue, never black
WHITE = (248, 248, 240)
S1, S2, S3 = (224, 224, 208), (192, 192, 176), (144, 144, 128)    # White stone and its shading
SL1, SL2, SL3 = (152, 168, 200), (112, 128, 168), (72, 88, 128)   # Slate roofs (the school color)
G1, G2, G3 = (200, 232, 248), (120, 176, 224), (64, 112, 168)     # Window glass
AU, AU2 = (248, 208, 88), (192, 136, 40)                          # Gold trim
WD, WD2 = (168, 104, 56), (104, 64, 40)                           # Wood (doors)

PALETTES = {
    # Shared by both tilesets: the buildings outside, the walls, windows and stairs inside
    'stone': [OL, WHITE, S1, S2, S3, SL1, SL2, SL3, G1, G2, G3, AU, AU2, WD, WD2],
    # The greenhouse and the Science Institute's glass
    'glass': [OL, WHITE, S1, (200, 208, 216), (144, 152, 168), (88, 96, 112), (224, 248, 248), (168, 224, 224),
              (112, 192, 200), (64, 144, 160), (136, 200, 96), (72, 152, 64), (40, 96, 48), (248, 152, 176),
              (216, 72, 88)],
    # Water, the fountain's Gyarados, the emblem's Magikarp
    'water': [OL, WHITE, S1, S2, S3, (176, 224, 248), (104, 176, 240), (56, 120, 208), (96, 152, 224), (40, 80, 160),
              (248, 232, 168), (224, 72, 56), (248, 136, 56), AU],
    # Inside: floors (marble, parquet, planks) and the blue carpet
    'floor': [OL, (240, 240, 232), (216, 216, 200), (184, 184, 168), (216, 168, 104), (184, 128, 72), (136, 88, 48),
              (88, 128, 200), (56, 88, 168), (32, 56, 112), AU, AU2, (248, 136, 56), (224, 72, 56)],
    # Inside: furniture
    'furn': [OL, WHITE, (208, 144, 88), WD, WD2, (64, 112, 80), (40, 72, 56), (200, 64, 56), (64, 96, 184),
             (232, 192, 72), (136, 200, 96), (72, 152, 64), (40, 96, 48), (168, 176, 192), (104, 112, 128)],
}
PALETTES = {k: [gba(c) for c in v] for k, v in PALETTES.items()}
GL = dict(zip(('ol', 'w', 's1', 'f1', 'f2', 'f3', 'q0', 'q1', 'q2', 'q3', 'p1', 'p2', 'p3', 'pk', 'rd'),
              PALETTES['glass']))
WA = dict(zip(('ol', 'w', 's1', 's2', 's3', 'a1', 'a2', 'a3', 'b1', 'b2', 'cr', 'rd', 'or', 'au'), PALETTES['water']))
FL = dict(zip(('ol', 'm0', 'm1', 'm2', 'pq1', 'pq2', 'pq3', 'c1', 'c2', 'c3', 'au', 'au2', 'or', 'rd'),
              PALETTES['floor']))
FU = dict(zip(('ol', 'w', 'wd0', 'wd', 'wd2', 'bg1', 'bg2', 'rd', 'bl', 'ye', 'p1', 'p2', 'p3', 'mt1', 'mt2'),
              PALETTES['furn']))
OL, WHITE, S1, S2, S3, SL1, SL2, SL3, G1, G2, G3, AU, AU2, WD, WD2 = PALETTES['stone']
BLACK = (0, 0, 0)


class Pic:
    """An RGBA picture drawn pixel by pixel; colors are RGB tuples, None skips."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        self.px = self.im.load()

    def p(self, x, y, c):
        if c is not None and 0 <= x < self.w and 0 <= y < self.h:
            self.px[x, y] = tuple(c) + (255,)

    def get(self, x, y):
        v = self.px[x, y]
        return v[:3] if v[3] else None

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

    def ellipse(self, cx, cy, rx, ry, c, outline=None):
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                d = ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2
                if d <= 1:
                    edge = outline and (((x - .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 > 1 or
                                        ((x + 1.5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 > 1 or
                                        ((x + .5 - cx) / rx) ** 2 + ((y - .5 - cy) / ry) ** 2 > 1 or
                                        ((x + .5 - cx) / rx) ** 2 + ((y + 1.5 - cy) / ry) ** 2 > 1)
                    self.p(x, y, outline if edge else c)

    def paste(self, other, x, y):
        self.im.alpha_composite(other.im, (x, y))


# --- Exterior pieces ---------------------------------------------------------------------------------------------------
def roof(P, x0, y0, w, h, dormers=(), dormer_row=None):
    """A slate mansard roof seen from above: 4-pixel shingle courses (staggered every 8 pixels, so every inner 16x16
    is the same metatile), a light ridge at the back, and a white dentil cornice as its last 8 rows."""
    for y in range(y0, y0 + h - 8):
        dy = y - y0
        course, row = (dy - 4) // 4, (dy - 4) % 4
        for x in range(x0, x0 + w):
            dx = x - x0
            if dy < 4:
                c = (OL, SL1, WHITE, SL1)[dy]
            elif row == 0:
                c = SL1
            elif row == 3:
                c = SL3
            else:
                c = SL3 if (dx + (course % 2) * 4) % 8 == 0 else SL2
            P.p(x, y, c)
        P.p(x0, y, OL)
        P.p(x0 + 1, y, SL3 if dy else OL)
        P.p(x0 + w - 2, y, SL3 if dy else OL)
        P.p(x0 + w - 1, y, OL)
    cornice(P, x0, y0 + h - 8, w)
    for col in dormers:
        dormer(P, x0 + col * T, y0 + (dormer_row if dormer_row is not None else h // T - 2) * T)


def cornice(P, x0, y, w):
    for x in range(x0, x0 + w):
        dx = x - x0
        P.p(x, y, OL)
        P.p(x, y + 1, WHITE)
        P.p(x, y + 2, S1)
        P.p(x, y + 3, WHITE)
        P.p(x, y + 4, S3 if dx % 4 == 1 else WHITE)
        P.p(x, y + 5, S3 if dx % 4 == 1 else S1)
        P.p(x, y + 6, S2)
        P.p(x, y + 7, S3)
    P.vl(x0, y, y + 7, OL)
    P.vl(x0 + w - 1, y, y + 7, OL)


def dormer(P, x, y):
    """A small arched dormer window in the roof (one metatile)."""
    for yy in range(y + 2, y + 15):
        for xx in range(x + 3, x + 13):
            P.p(xx, yy, WHITE)
    P.r(x + 3, y + 1, 10, 2, SL3)
    P.hl(x + 5, x + 10, y, OL)
    P.p(x + 4, y + 1, OL)
    P.p(x + 11, y + 1, OL)
    P.vl(x + 2, y + 2, y + 14, OL)
    P.vl(x + 13, y + 2, y + 14, OL)
    P.hl(x + 2, x + 13, y + 15, SL3)
    for yy in range(y + 4, y + 13):
        for xx in range(x + 5, x + 11):
            inset = 1 if yy == y + 4 and xx in (x + 5, x + 10) else 0
            P.p(xx, yy, WHITE if inset else (G1 if (xx - x) + (yy - y) < 12 else G2 if yy < y + 11 else G3))
    P.vl(x + 7, y + 5, y + 12, WHITE)
    P.hl(x + 5, x + 10, y + 13, S2)
    P.vl(x + 12, y + 3, y + 14, S2)


def ashlar(P, x0, y0, w, h):
    """White stone wall: 8-pixel courses, joints every 16 pixels staggered by 8."""
    for y in range(y0, y0 + h):
        dy = y - y0
        for x in range(x0, x0 + w):
            dx = x - x0
            joint = dy % 8 == 7 or ((dx + (dy // 8 % 2) * 8) % 16 == 0 and dy % 8 < 7)
            P.p(x, y, S1 if joint else WHITE)


def quoins(P, x, y0, h, right=False):
    """Corner stones on a building's edge: alternating long and short blocks, outlined."""
    for y in range(y0, y0 + h):
        dy = y - y0
        long_block = dy // 8 % 2 == 0
        width = 7 if long_block else 5
        for i in range(width):
            xx = x + 15 - i if right else x + i
            P.p(xx, y, S2 if dy % 8 == 7 else (S1 if (right and i == 0) else WHITE))
        P.p(x + 15 - width if right else x + width, y, S2)
        P.p(x + 15 if right else x, y, OL)


def plinth(P, x0, y, w):
    for x in range(x0, x0 + w):
        P.p(x, y, S2)
        P.p(x, y + 1, S1)
        P.p(x, y + 2, S2)
        P.p(x, y + 3, S3)


def string_course(P, x0, y, w):
    P.hl(x0, x0 + w - 1, y, WHITE)
    P.hl(x0, x0 + w - 1, y + 1, S2)


def arch_window(P, x, y, h=32, lit=False):
    """A tall arched window in a 16-wide cell: slate-outlined frame, glass with a highlight, a white cross mullion,
    a keystone and a sill."""
    L, Rr, top, bot = x + 3, x + 11, y + 5, y + h - 6
    for yy in range(top, bot + 1):
        inset = 3 if yy == top else 1 if yy == top + 1 else 0
        for xx in range(L + inset, Rr - inset + 1):
            edge = xx in (L + inset, Rr - inset) or yy in (top, bot)
            if edge:
                c = OL
            elif xx == x + 7 or yy == top + (bot - top) * 2 // 5:
                c = WHITE
            elif (xx - L) + (yy - top) < 7 or (lit and yy < bot - 3):
                c = G1
            elif yy >= bot - 3:
                c = G3
            else:
                c = G2
            P.p(xx, yy, c)
    P.p(x + 7, top - 1, WHITE)
    P.p(x + 7, top - 2, S2)
    P.hl(x + 2, x + 12, bot + 1, WHITE)
    P.hl(x + 2, x + 12, bot + 2, S3)


def door(P, x, y, glass=False):
    """A door cell of 16x32: an arched stone portal with a fanlight (the upper metatile) over a double door (the
    lower metatile, the one the door animation replaces)."""
    for yy in range(y + 4, y + 32):
        for xx in range(x + 1, x + 15):
            P.p(xx, yy, S2)
    P.hl(x + 4, x + 11, y + 3, OL)
    P.hl(x + 2, x + 13, y + 4, OL)
    P.vl(x + 1, y + 5, y + 31, OL)
    P.vl(x + 14, y + 5, y + 31, OL)
    P.vl(x + 2, y + 5, y + 31, WHITE)
    P.p(x + 7, y + 4, WHITE)
    P.p(x + 8, y + 4, WHITE)
    # Fanlight
    for yy in range(y + 7, y + 16):
        inset = 3 if yy == y + 7 else 1 if yy == y + 8 else 0
        for xx in range(x + 3 + inset, x + 13 - inset):
            P.p(xx, yy, OL if yy == y + 7 or xx in (x + 3 + inset, x + 12 - inset) else
                (G1 if (xx + yy) % 5 else WHITE) if yy < y + 12 else G2)
    # The leaves
    for yy in range(y + 16, y + 31):
        for xx in range(x + 3, x + 13):
            if glass:
                c = OL if xx in (x + 3, x + 12, x + 7, x + 8) else G1 if yy < y + 21 else G2
            else:
                c = WD2 if xx in (x + 3, x + 12) or xx in (x + 7, x + 8) and xx == x + 7 else WD
                if yy in (y + 19, y + 26) and xx not in (x + 3, x + 12, x + 7):
                    c = WD2
            P.p(xx, yy, c)
    P.hl(x + 3, x + 12, y + 16, OL)
    P.p(x + 6, y + 23, AU)
    P.p(x + 9, y + 23, AU)
    P.hl(x + 1, x + 14, y + 31, S3)


def column(P, x, y0, y1):
    """A white column (shaft 6 wide at x..x+5) with a capital and a base."""
    for y in range(y0 + 3, y1 - 2):
        P.p(x, y, OL)
        P.p(x + 1, y, WHITE)
        P.p(x + 2, y, WHITE)
        P.p(x + 3, y, S1)
        P.p(x + 4, y, S2)
        P.p(x + 5, y, OL)
    P.r(x - 2, y0, 10, 1, OL)
    P.r(x - 2, y0 + 1, 10, 1, WHITE)
    P.r(x - 1, y0 + 2, 8, 1, S2)
    P.r(x - 1, y1 - 2, 8, 1, WHITE)
    P.r(x - 2, y1 - 1, 10, 1, S2)
    P.p(x - 2, y1 - 1, OL)
    P.p(x + 7, y1 - 1, OL)


def pediment(P, cx, base_y, half, height, emblem=True):
    for dy in range(height + 1):
        y = base_y - dy
        hw = int(half * (1 - dy / height))
        for x in range(cx - hw, cx + hw + 1):
            edge = x in (cx - hw, cx + hw) or dy == 0 or dy == height
            inner = abs(x - cx) < hw - 4 and 2 < dy < height - 2
            P.p(x, y, OL if edge else (S1 if inner else WHITE))
        if 0 < dy < height:
            P.p(cx - hw + 1, y, WHITE)
            P.p(cx + hw - 1, y, S2)
    if emblem:
        P.ellipse(cx + .5, base_y - height * .38, 5, 5, AU, AU2)
        P.p(cx, base_y - int(height * .38), AU2)
        P.p(cx - 1, base_y - int(height * .38) - 1, WHITE)


def facade_windows(P, x0, y0, cols, lit=()):
    for c in cols:
        arch_window(P, x0 + c * T, y0, lit=c in lit)


def main_hall():
    """The Main Hall (central building), 15x11: a mansard roof (5 rows), three storeys, a columned portico with a
    pediment over the door (column 7). The clock tower is a separate picture standing on the roof (clock_tower)."""
    P = Pic(15 * T, 11 * T)
    roof(P, 0, 0, P.w, 5 * T, dormers=(2, 4, 10, 12), dormer_row=3)
    ashlar(P, 0, 5 * T, P.w, 6 * T)
    for s in range(3):
        y = (5 + 2 * s) * T
        facade_windows(P, 0, y, (1, 2, 3, 4, 10, 11, 12, 13), lit=(2, 12) if s == 1 else ())
        if s:
            string_course(P, 0, y, P.w)
    # The portico: shadowed hall behind four columns, entablature, pediment
    P.r(5 * T, 7 * T + 6, 5 * T, 4 * T - 10, S3)
    for c in (5, 9):
        arch_window(P, c * T, 9 * T)
    door(P, 7 * T, 9 * T)
    P.r(5 * T - 2, 7 * T, 5 * T + 4, 6, WHITE)
    P.hl(5 * T - 2, 10 * T + 1, 7 * T, OL)
    P.hl(5 * T - 2, 10 * T + 1, 7 * T + 4, S2)
    P.hl(5 * T - 2, 10 * T + 1, 7 * T + 5, OL)
    pediment(P, 7 * T + 7, 7 * T, 5 * T // 2 + 4, 26)
    for x in (5 * T + 4, 6 * T + 10, 8 * T, 9 * T + 6):
        column(P, x, 7 * T + 6, 11 * T - 4)
    for x in range(5 * T - 2, 10 * T + 2):
        P.p(x, 11 * T - 4, S1)
        P.p(x, 11 * T - 3, WHITE)
        P.p(x, 11 * T - 2, S2)
        P.p(x, 11 * T - 1, S3)
    quoins(P, 0, 5 * T, 6 * T - 4)
    quoins(P, 14 * T, 5 * T, 6 * T - 4, right=True)
    plinth(P, 0, 11 * T - 4, 5 * T - 2)
    plinth(P, 10 * T + 2, 11 * T - 4, 5 * T - 2)
    return P


def clock_tower():
    """5x6: spire, belfry, clock stage (the hands at noon, when the Gyarados fountain spouts), base on the roof."""
    P = Pic(5 * T, 6 * T)
    cx = 40
    # Base (row 5): a balustrade on the roof
    P.r(2, 5 * T, 76, 16, WHITE)
    for x in range(2, 78):
        P.p(x, 5 * T, OL)
        P.p(x, 5 * T + 1, WHITE)
        P.p(x, 5 * T + 2, S2)
        if x % 4 == 0:
            P.vl(x, 5 * T + 4, 5 * T + 11, S2)
        P.p(x, 5 * T + 12, S1)
        P.p(x, 5 * T + 13, S2)
        P.p(x, 5 * T + 14, S3)
        P.p(x, 5 * T + 15, S3)
    P.vl(2, 5 * T, 6 * T - 1, OL)
    P.vl(77, 5 * T, 6 * T - 1, OL)
    # Body (rows 2-4), 48 wide
    ashlar(P, 16, 2 * T, 48, 3 * T)
    quoins(P, 16, 2 * T, 3 * T)
    quoins(P, 48, 2 * T, 3 * T, right=True)
    # The clock stage
    P.ellipse(cx, 3 * T + 12, 14, 14, AU, AU2)
    P.ellipse(cx, 3 * T + 12, 11, 11, WHITE, S2)
    for a in range(12):
        ang = a * math.pi / 6
        P.p(int(round(cx - .5 + 9 * math.sin(ang))), int(round(3 * T + 11.5 - 9 * math.cos(ang))), OL)
    P.vl(cx - 1, 3 * T + 4, 3 * T + 11, OL)
    P.vl(cx, 3 * T + 6, 3 * T + 11, OL)
    P.p(cx - 1, 3 * T + 12, AU2)
    cornice(P, 14, 3 * T - 6, 52)
    # Belfry (row 2): two arched openings
    for ox in (24, 44):
        for yy in range(2 * T + 1, 2 * T + 9):
            inset = 1 if yy == 2 * T + 1 else 0
            for xx in range(ox + inset, ox + 8 - inset):
                P.p(xx, yy, OL if yy == 2 * T + 1 or xx in (ox, ox + 7) else SL3)
        P.hl(ox, ox + 7, 2 * T + 9, WHITE)
    # Spire (rows 0-1)
    for dy in range(2 * T - 2):
        y = 2 * T - 1 - dy
        hw = int(26 * (1 - dy / 30)) + 1
        for x in range(cx - hw, cx + hw):
            edge = x in (cx - hw, cx + hw - 1)
            course = dy // 3
            c = OL if edge else (SL1 if dy % 3 == 0 else SL3 if (x + course * 2) % 6 == 0 else SL2)
            if not edge and x >= cx + hw - 4:
                c = SL3
            P.p(x, y, c)
    P.hl(cx - 27, cx + 26, 2 * T - 1, OL)
    P.vl(cx - 1, 0, 3, AU2)
    P.vl(cx, 0, 3, AU)
    P.p(cx - 1, 1, AU)
    return P


def wing(door_col=5, w=11):
    """A lecture-hall wing, 11x9: roof 5 rows, two storeys, a door with a small pediment over it."""
    P = Pic(w * T, 9 * T)
    roof(P, 0, 0, P.w, 5 * T, dormers=(2, w - 3), dormer_row=3)
    ashlar(P, 0, 5 * T, P.w, 4 * T)
    cols = [c for c in range(1, w - 1)]
    facade_windows(P, 0, 5 * T, cols)
    facade_windows(P, 0, 7 * T, [c for c in cols if c != door_col])
    string_course(P, 0, 7 * T, P.w)
    door(P, door_col * T, 7 * T)
    pediment(P, door_col * T + 7, 7 * T + 2, 12, 9, emblem=False)
    quoins(P, 0, 5 * T, 4 * T - 4)
    quoins(P, (w - 1) * T, 5 * T, 4 * T - 4, right=True)
    plinth(P, 0, 9 * T - 4, door_col * T + 1)
    plinth(P, door_col * T + 15, 9 * T - 4, P.w - door_col * T - 15)
    return P


def pavilion(door_col=4, w=9):
    """An end pavilion, 9x8: roof 4 rows with a front gable pediment, two storeys."""
    P = Pic(w * T, 8 * T)
    roof(P, 0, 0, P.w, 4 * T)
    ashlar(P, 0, 4 * T, P.w, 4 * T)
    cols = [c for c in range(1, w - 1)]
    facade_windows(P, 0, 4 * T, cols)
    facade_windows(P, 0, 6 * T, [c for c in cols if c != door_col])
    string_course(P, 0, 6 * T, P.w)
    door(P, door_col * T, 6 * T)
    pediment(P, door_col * T + 7, 4 * T - 1, 30, 22)
    quoins(P, 0, 4 * T, 4 * T - 4)
    quoins(P, (w - 1) * T, 4 * T, 4 * T - 4, right=True)
    plinth(P, 0, 8 * T - 4, door_col * T + 1)
    plinth(P, door_col * T + 15, 8 * T - 4, P.w - door_col * T - 15)
    return P


_LEAVES = None


def leaves():
    """A 32x16 clump of leaves that tiles seamlessly (so the plants under the glass cost a few tiles, not hundreds):
    None where the glass shows, else a leaf color."""
    global _LEAVES
    if _LEAVES is None:
        P = Pic(32, 16)
        for (cx, cy, r) in ((5, 9, 6), (16, 6, 7), (27, 10, 6), (11, 15, 5), (22, 16, 5), (0, 1, 4), (32, 1, 4),
                            (32, 9, 6), (0, 16, 4), (32, 16, 4), (16, -1, 4)):
            for y in range(16):
                for x in range(32):
                    for (ox, oy) in ((0, 0), (32, 0), (-32, 0), (0, 16), (0, -16)):
                        if (x - cx - ox) ** 2 + (y - cy - oy) ** 2 <= r * r:
                            d = (x - cx - ox) + (y - cy - oy)
                            c = GL['p1'] if d < -r // 2 else GL['p3'] if d > r // 2 + 1 else GL['p2']
                            P.p(x, y, c)
        for (x, y, c) in ((15, 3, 'pk'), (26, 7, 'rd'), (6, 6, 'pk'), (21, 13, 'w')):
            P.p(x, y, GL[c])
            P.p(x + 1, y, GL[c])
            P.p(x, y + 1, GL[c])
        _LEAVES = [[P.get(x, y) for x in range(32)] for y in range(16)]
    return _LEAVES


def greenhouse(w=17, h=9, door_col=8):
    """Home Economics: the arched glass greenhouse (the lore's great glass roof), 17x9: a glass vault (6 rows) with
    rounded ends and plants showing through, a glass front on a low stone wall, glass doors under a fanlight. The
    panes repeat every 32x16 pixels so the whole roof is a handful of metatiles."""
    P = Pic(w * T, h * T)
    rh = 6 * T
    W = P.w
    L = leaves()
    r = 20
    for y in range(rh):
        for x in range(W):
            if (x < r and y < r and (x - r) ** 2 + (y - r) ** 2 > r * r) or \
               (x > W - 1 - r and y < r and (x - (W - 1 - r)) ** 2 + (y - r) ** 2 > r * r):
                continue
            if x % 16 == 0:
                c = GL['f2']
            elif x % 16 == 1:
                c = GL['w']
            elif y % 8 == 0:
                c = GL['f1']
            else:
                c = GL['q0'] if y < 16 else GL['q1'] if y < 40 else GL['q2']
                if y < 48 and (x // 16) % 2 == 0 and (x % 16) - (y % 8) in (3, 4):
                    c = GL['w']
                if y >= 48:
                    leaf = L[y % 16][x % 32]
                    if leaf is not None:
                        c = leaf if y >= 56 or leaf != GL['p3'] else GL['q3']
            P.p(x, y, c)
    for y in range(rh):
        row = [x for x in range(W) if P.get(x, y)]
        if row:
            P.p(row[0], y, GL['ol'])
            P.p(row[-1], y, GL['ol'])
            P.p(row[-1] - 1, y, GL['f3'])
    for x in range(W):
        for y in range(rh):
            if P.get(x, y):
                P.p(x, y, GL['ol'])
                break
    P.r(24, 3, W - 48, 3, GL['w'])
    P.hl(24, W - 25, 6, GL['f2'])
    P.hl(0, W - 1, rh - 2, GL['f3'])
    P.hl(0, W - 1, rh - 1, GL['ol'])
    # Front: glass panes (plants and flowers behind) over a low white wall
    for y in range(rh, P.h):
        dy = y - rh
        for x in range(W):
            if dy >= 3 * T - 10:
                c = GL['w'] if dy == 3 * T - 10 else GL['s1'] if dy < 3 * T - 3 else GL['f2']
            elif x % 16 == 0 or dy == 0:
                c = GL['f3']
            elif x % 16 == 15 or dy == 1:
                c = GL['f1']
            elif dy % 16 == 15:
                c = GL['f1']
            else:
                c = GL['q1'] if dy < 10 else GL['q2']
                leaf = L[dy % 16][x % 32]
                if dy >= 10 and leaf is not None:
                    c = leaf
            P.p(x, y, c)
    P.vl(0, rh, P.h - 1, GL['ol'])
    P.vl(W - 1, rh, P.h - 1, GL['ol'])
    P.hl(0, W - 1, P.h - 1, GL['ol'])
    dx, dy = door_col * T, (h - 2) * T
    for y in range(dy + 2, dy + 32):
        for x in range(dx + 1, dx + 15):
            P.p(x, y, GL['f3'])
    for y in range(dy + 4, dy + 15):
        for x in range(dx + 3, dx + 13):
            P.p(x, y, GL['q0'] if (x - dx + y - dy) % 4 else GL['w'])
    for y in range(dy + 17, dy + 31):
        for x in range(dx + 3, dx + 13):
            P.p(x, y, GL['f3'] if x in (dx + 7, dx + 8) else GL['q1'] if y < dy + 22 else GL['q2'])
    P.p(dx + 6, dy + 24, GL['w'])
    P.p(dx + 9, dy + 24, GL['w'])
    P.hl(dx + 1, dx + 14, dy + 31, GL['f2'])
    P.ellipse(dx + 8, dy - 6, 4, 4, GL['pk'], GL['rd'])
    P.p(dx + 7, dy - 7, GL['w'])
    return P


def fountain():
    """The Gyarados fountain, 7x5: an oval white basin, rippling water, Gyarados rising from the middle and spouting
    (the noon signal)."""
    P = Pic(7 * T, 5 * T)
    cx, cy = 56, 44
    # The basin's front face, then its rim, then the water
    P.ellipse(cx, cy + 6, 54, 30, WA['s2'], WA['ol'])
    P.ellipse(cx, cy + 3, 54, 30, WA['s3'])
    P.ellipse(cx, cy, 54, 30, WA['w'], WA['ol'])
    P.ellipse(cx, cy, 50, 26, WA['s1'])
    P.ellipse(cx, cy + 1, 46, 23, WA['a2'], WA['s3'])
    for y in range(P.h):
        for x in range(P.w):
            if P.get(x, y) == WA['a2']:
                ring = math.hypot((x - cx) / 46, (y - cy - 1) / 23)
                if abs(ring * 8 % 2 - 1) < .14:
                    P.p(x, y, WA['a1'])
                elif y < cy - 14:
                    P.p(x, y, WA['a3'])
    # The pedestal
    P.ellipse(cx, cy + 6, 12, 6, WA['s2'], WA['ol'])
    P.ellipse(cx, cy + 4, 12, 6, WA['w'], WA['ol'])
    # Gyarados: an S-curve of body segments from the pedestal up to the head
    pts = [(cx + 10 * math.sin(t * 2.4) * (1 - t * .3), cy + 2 - t * 34) for t in (i / 40 for i in range(41))]
    for i, (x, y) in enumerate(pts):
        rad = 6.5 - i * .06
        P.ellipse(x, y, rad, rad, WA['b1'], WA['ol'])
    for i, (x, y) in enumerate(pts):
        rad = 6.5 - i * .06
        P.ellipse(x - 1.5, y, rad - 2.5, rad - 2.5, WA['b1'])
        P.ellipse(x + 2, y + .5, 1.6, 1.6, WA['b2'])
        P.ellipse(x - 2.5, y + 1, 1.5, 1.2, WA['cr'])
        if i % 8 == 4:
            P.p(int(x + rad), int(y), WA['rd'])
            P.p(int(x + rad + 1), int(y - 1), WA['rd'])
    hx, hy = pts[-1]
    P.ellipse(hx - 1, hy - 3, 7, 6, WA['b1'], WA['ol'])
    P.ellipse(hx - 2, hy - 4, 4, 3, WA['b1'])
    P.ellipse(hx - 4, hy - 1, 4, 2, WA['w'], WA['ol'])          # The open mouth, white
    P.p(int(hx) + 1, int(hy) - 5, WA['ol'])                    # The eye
    P.p(int(hx) + 2, int(hy) - 6, WA['rd'])
    for k in range(5):                                          # The crest
        P.p(int(hx) + 2 + k, int(hy) - 8 - (k % 2), WA['cr'] if k % 2 else WA['b2'])
    # The jet: up from the mouth and falling in two arcs
    for k in range(14):
        P.p(int(hx) - 6, int(hy) - 4 - k, WA['w'] if k % 3 else WA['a1'])
        P.p(int(hx) - 5, int(hy) - 4 - k, WA['a1'])
    for side in (-1, 1):
        for k in range(30):
            t = k / 29
            x = hx - 5.5 + side * t * 30
            y = hy - 18 + (t * 2 - .45) ** 2 * 40
            P.p(int(x), int(y), WA['a1'] if k % 4 else WA['w'])
    return P


def pool(w, h):
    """A reflecting pool: a white stone rim, still water with highlights."""
    P = Pic(w * T, h * T)
    P.r(0, 0, P.w, P.h, WA['w'])
    P.hl(0, P.w - 1, 0, WA['ol'])
    P.vl(0, 0, P.h - 1, WA['ol'])
    P.vl(P.w - 1, 0, P.h - 1, WA['ol'])
    P.hl(0, P.w - 1, P.h - 1, WA['ol'])
    P.hl(1, P.w - 2, P.h - 2, WA['s2'])
    for y in range(4, P.h - 5):
        for x in range(4, P.w - 4):
            c = WA['a3'] if y < 7 else WA['a2']
            if (y % 16 == 10 and x % 16 in range(3, 9)) or (y % 16 == 2 and x % 16 in range(9, 14)):
                c = WA['a1']
            P.p(x, y, c)
    P.r(3, 3, P.w - 6, 1, WA['s3'])
    P.vl(3, 3, P.h - 5, WA['s3'])
    return P


def lamp():
    """A lamp post, 1x2: a slate post, a lantern with gold cap."""
    P = Pic(T, 2 * T)
    P.r(6, 10, 4, 20, SL3)
    P.vl(6, 10, 29, OL)
    P.vl(9, 10, 29, OL)
    P.p(7, 12, SL1)
    P.r(4, 28, 8, 3, SL3)
    P.hl(4, 11, 31, OL)
    P.r(4, 2, 8, 7, G1)
    P.vl(4, 2, 8, OL)
    P.vl(11, 2, 8, OL)
    P.vl(8, 2, 8, WHITE)
    P.hl(3, 12, 1, AU2)
    P.hl(5, 10, 0, AU)
    P.hl(3, 12, 9, AU2)
    return P


def bench():
    """A park bench, 2x1."""
    P = Pic(2 * T, T)
    P.r(2, 4, 28, 3, WD)
    P.hl(2, 29, 3, OL)
    P.hl(2, 29, 5, WD2)
    P.r(2, 8, 28, 3, WD)
    P.hl(2, 29, 7, OL)
    P.hl(2, 29, 11, WD2)
    for x in (4, 26):
        P.r(x, 12, 2, 3, SL3)
    return P


def dorm():
    """A residence hall, 17x10: the same stone and slate, homelier: square windows with little iron balconies on the
    upper floors, a canopy over the door (column 8)."""
    w = 17
    P = Pic(w * T, 10 * T)
    roof(P, 0, 0, P.w, 4 * T, dormers=(2, 5, 11, 14), dormer_row=2)
    ashlar(P, 0, 4 * T, P.w, 6 * T)
    for s in range(3):
        y = (4 + 2 * s) * T
        if s:
            string_course(P, 0, y, P.w)
        for c in range(1, w - 1):
            if s == 2 and c in (7, 8, 9):
                continue
            x = c * T
            # A square window
            P.r(x + 3, y + 8, 10, 14, OL)
            P.r(x + 4, y + 9, 8, 12, G2)
            P.r(x + 4, y + 9, 4, 5, G1)
            P.vl(x + 7, y + 9, y + 20, WHITE)
            P.hl(x + 4, x + 11, y + 14, WHITE)
            P.r(x + 4, y + 18, 8, 3, G3)
            P.hl(x + 2, x + 13, y + 22, WHITE)
            P.hl(x + 2, x + 13, y + 23, S3)
            if s < 2:  # Balcony railing
                P.hl(x + 1, x + 14, y + 25, SL3)
                for k in range(x + 1, x + 15, 3):
                    P.vl(k, y + 25, y + 29, SL3)
                P.hl(x + 1, x + 14, y + 29, OL)
    door(P, 8 * T, 8 * T, glass=True)
    for c in (7, 9):
        arch_window(P, c * T, 8 * T)
    # Canopy over the door
    P.r(7 * T + 4, 8 * T, 2 * T + 8, 5, SL2)
    P.hl(7 * T + 4, 9 * T + 11, 8 * T, OL)
    P.hl(7 * T + 4, 9 * T + 11, 8 * T + 5, OL)
    P.hl(7 * T + 5, 9 * T + 10, 8 * T + 1, SL1)
    quoins(P, 0, 4 * T, 6 * T - 4)
    quoins(P, (w - 1) * T, 4 * T, 6 * T - 4, right=True)
    plinth(P, 0, 10 * T - 4, 8 * T + 1)
    plinth(P, 8 * T + 15, 10 * T - 4, P.w - 8 * T - 15)
    return P


# --- Ground: the real primary metatiles ------------------------------------------------------------------------------
_ts = {}


def tileset(symbol):
    if symbol not in _ts:
        _ts[symbol] = R.load_tileset(R.tileset_folder(symbol), symbol)
    return _ts[symbol]


_mt = {}


def metatile(mid, primary='gTileset_General_Frlg', secondary='gTileset_CeladonCity'):
    key = (mid, primary, secondary)
    if key in _mt:
        return _mt[key]
    ptiles, ppals, pmeta = tileset(primary)
    stiles, spals, smeta = tileset(secondary)
    pals = [ppals[i] if i < 7 else spals[i] for i in range(16)]
    entries = pmeta[mid] if mid < 640 else smeta[mid - 640]
    img = Image.new('RGBA', (16, 16), pals[0][0] + (255,))
    px = img.load()
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
                    px[(k % 2) * 8 + x, (k // 2) * 8 + y] = (pals[0][0] if c == 0 else pals[p][c]) + (255,)
    _mt[key] = img
    return img


class Ground:
    """A grid of primary metatiles: grass, forest, lone trees, paving with edges, flower beds."""
    EDGES = {
        (False, True, False, True): 0x165, (False, True, True, True): 0x166, (False, True, True, False): 0x167,
        (True, True, False, True): 0x16D, (True, True, True, True): 0x16E, (True, True, True, False): 0x16F,
        (True, False, False, True): 0x175, (True, False, True, True): 0x176, (True, False, True, False): 0x177,
    }

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.g = [[((0x009, 0x008), (0x011, 0x010))[y % 2][x % 2] for x in range(w)] for y in range(h)]
        self.paved = set()
        self.used = set()

    def set(self, x, y, m):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.g[y][x] = m

    def forest(self, x0, y0, x1, y1):
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.set(x, y, ((0x01C, 0x01D), (0x014, 0x015))[y % 2][x % 2])
                self.used.add((x, y))

    def path(self, x0, y0, x1, y1):
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.paved.add((x, y))
                self.used.add((x, y))

    def lay(self):
        for (x, y) in self.paved:
            key = ((x, y - 1) in self.paved, (x, y + 1) in self.paved, (x - 1, y) in self.paved,
                   (x + 1, y) in self.paved)
            self.set(x, y, self.EDGES.get(key, 0x16E))

    def block(self, x0, y0, w, h):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                self.used.add((x, y))

    def tree(self, x, y):
        for (dx, dy, m) in ((0, 0, 0x00E), (1, 0, 0x00F), (0, 1, 0x01E), (1, 1, 0x01F), (0, 2, 0x026), (1, 2, 0x027)):
            self.set(x + dx, y + dy, m)
            self.used.add((x + dx, y + dy))

    def grove(self, x0, y0, x1, y1):
        """Lone trees wherever a 2x3 tree and a one-tile margin fit (trees may touch trees)."""
        for y in range(y0, y1 - 2):
            for x in range(x0, x1 - 1):
                if all((x + dx, y + dy) not in self.used for dx in range(2) for dy in range(3)) and \
                   all((x + dx, y + dy) not in self.used or self.g[y + dy][x + dx] in (0x00E, 0x00F, 0x01E, 0x01F,
                                                                                    0x026, 0x027, 0x01C, 0x01D,
                                                                                    0x014, 0x015)
                       for dx in range(-1, 3) for dy in range(-1, 4)
                       if 0 <= x + dx < self.w and 0 <= y + dy < self.h):
                    self.tree(x, y)

    def flowers(self, x0, x1, y):
        for x in range(x0, x1):
            if (x, y) not in self.used:
                self.set(x, y, 0x004)
                self.used.add((x, y))

    def render(self):
        img = Image.new('RGBA', (self.w * T, self.h * T))
        for y in range(self.h):
            for x in range(self.w):
                img.paste(metatile(self.g[y][x]), (x * T, y * T))
        return img


# --- Sprites -----------------------------------------------------------------------------------------------------------
def sprite(name, frame=0):
    im = Image.open(os.path.join(ROOT, 'graphics/object_events/pics', name + '.png'))
    w = 16
    im = im.crop((frame * w, 0, frame * w + w, 32))
    rgba = Image.new('RGBA', im.size)
    pal = im.getpalette()
    src = im.load()
    out = rgba.load()
    for y in range(im.height):
        for x in range(im.width):
            i = src[x, y]
            if i:
                out[x, y] = (pal[3 * i], pal[3 * i + 1], pal[3 * i + 2], 255)
    return rgba


def put_sprite(img, name, tx, ty, frame=0):
    """An overworld sprite standing on tile (tx, ty) (its feet on that tile, 16x32)."""
    s = sprite(name, frame)
    img.alpha_composite(s, (tx * T, ty * T - 16))


PLAYER = 'people/red/red_normal'
FACE_DOWN, FACE_UP, FACE_LEFT = 0, 1, 2


# --- The campus section ------------------------------------------------------------------------------------------------
# Tile positions in the mockup (64x40): the central building's five parts share a front line (doors on row 26)
CENTER = {
    'FACULTY_OFFICE': (4, 19, 9, 8, 4),     # x, y, w, h, door column
    'LECTURE_HALL_A': (13, 18, 11, 9, 5),
    'MAIN_HALL': (24, 16, 15, 11, 7),
    'LECTURE_HALL_B': (39, 18, 11, 9, 5),
    'CAFETERIA': (50, 19, 9, 8, 4),
    'HOME_ECONOMICS': (41, 4, 17, 9, 8),
}
TOWER_AT = (29, 11)
FOUNTAIN_AT = (28, 30)


def campus_section():
    W, H = 64, 40
    g = Ground(W, H)
    g.forest(0, 0, W, 3)
    g.forest(0, 0, 2, H)
    g.forest(W - 2, 0, W, H)
    for name, (x, y, w, h, d) in CENTER.items():
        g.block(x, y, w, h)
    g.block(29, 11, 5, 6)
    # Paths: the street in front, the square and the avenue, around the sides to the greenhouse
    g.path(2, 27, 62, 29)
    g.path(25, 29, 39, 36)
    g.path(29, 36, 34, 40)
    g.path(59, 13, 62, 27)
    g.path(41, 13, 62, 15)
    g.path(2, 13, 4, 27)
    g.path(2, 13, 24, 15)
    for name, (x, y, w, h, d) in CENTER.items():
        if name != 'HOME_ECONOMICS':
            g.path(x + d - 1, y + h, x + d + 2, y + h + 1)
    g.lay()
    # Pools along the avenue, lamps, flowers, benches
    g.block(24, 36, 4, 4)
    g.block(35, 36, 4, 4)
    g.block(*FOUNTAIN_AT, 7, 5)
    lamps = ((25, 29), (38, 29), (28, 37), (34, 37), (12, 29), (51, 29))
    for (x, y) in lamps:
        g.block(x, y, 1, 2)
    benches = ((26, 34), (36, 34))
    signs = [(x + d + 2, y + h) for (x, y, w, h, d) in CENTER.values()]
    for (x, y) in signs:
        g.set(x, y, 0x003)
        g.used.add((x, y))
    g.flowers(2, 62, 29)
    g.flowers(2, 62, 12)
    g.block(2, 3, 39, 1)
    g.grove(2, 3, 41, 12)
    g.grove(2, 30, 24, 40)
    g.grove(40, 30, 62, 40)
    g.grove(59, 3, 62, 12)
    img = g.render()
    # The custom layer (what the new tileset draws), kept apart for the tile budget
    layer = Pic(W * T, H * T)
    for name, (x, y, w, h, d) in CENTER.items():
        if name == 'MAIN_HALL':
            pic = main_hall()
        elif name.startswith('LECTURE'):
            pic = wing(d, w)
        elif name == 'HOME_ECONOMICS':
            pic = greenhouse(w, h, d)
        else:
            pic = pavilion(d, w)
        layer.paste(pic, x * T, y * T)
    layer.paste(clock_tower(), TOWER_AT[0] * T, TOWER_AT[1] * T)
    layer.paste(fountain(), FOUNTAIN_AT[0] * T, FOUNTAIN_AT[1] * T)
    layer.paste(pool(4, 4), 24 * T, 36 * T)
    layer.paste(pool(4, 4), 35 * T, 36 * T)
    for (x, y) in lamps:
        layer.paste(lamp(), x * T, y * T)
    for (x, y) in benches:
        layer.paste(bench(), x * T, y * T)
    img.alpha_composite(layer.im)
    # People
    put_sprite(img, PLAYER, 31, 29, FACE_UP)
    put_sprite(img, 'people/lass_frlg', 27, 28, FACE_DOWN)
    put_sprite(img, 'people/scientist', 44, 28, FACE_LEFT)
    put_sprite(img, 'people/youngster_frlg', 36, 33, FACE_DOWN)
    put_sprite(img, 'people/beauty_frlg', 49, 14, FACE_DOWN)
    put_sprite(img, 'people/gentleman_frlg', 9, 28, FACE_UP)
    return img, layer


# --- Interiors ---------------------------------------------------------------------------------------------------------
def back_wall(P, x0, y0, w, rows, wainscot='slate'):
    """The room's back wall: a slate cornice, white plaster with a gold picture rail, a slate (or wood) wainscot,
    a dark skirting. The facade's materials, inside."""
    h = rows * T
    lo = SL1, SL2, SL3
    if wainscot == 'wood':
        lo = WD, WD, WD2
    ws = h - (14 if rows == 3 else 10)
    for y in range(y0, y0 + h):
        dy = y - y0
        for x in range(x0, x0 + w):
            dx = x - x0
            if dy == 0:
                c = OL
            elif dy < 3:
                c = (SL3, SL2)[dy - 1]
            elif dy == 3:
                c = WHITE
            elif dy == 4:
                c = S2
            elif dy < ws - 1:
                c = S1 if (dx + dy) % 16 == 0 and dy % 4 == 0 else WHITE
                if dy == 9:
                    c = AU
                if dy == 10:
                    c = AU2
            elif dy == ws - 1:
                c = lo[0]
            elif dy < h - 2:
                c = lo[2] if dx % 16 in (0, 15) or dy == ws else lo[1]
                if dx % 16 == 1 and wainscot == 'slate':
                    c = lo[0]
            elif dy == h - 2:
                c = lo[2]
            else:
                c = OL
            P.p(x, y, c)


def side_walls(P, w, h, top_rows):
    """Wall tops down the left and right edges and along the front (the dark band the game draws for walls seen
    from above), black outside."""
    for x in (0, w * T - T):
        for y in range(0, h * T - T):
            for k in range(T):
                xx = x + k
                inner = k == (T - 1 if x == 0 else 0)
                outer = k == (0 if x == 0 else T - 1)
                P.p(xx, y, OL if inner or outer else SL3 if k in (1, 14) else SL2 if y > 2 else SL3)
        P.hl(x, x + T - 1, 0, OL)
    for x in range(0, w * T):
        for y in range((h - 1) * T, h * T):
            P.p(x, y, BLACK)
        P.p(x, (h - 1) * T, OL)
        P.p(x, (h - 1) * T + 1, SL3)
        P.p(x, (h - 1) * T + 2, SL2)
        P.p(x, (h - 1) * T + 3, OL)


def marble(P, x0, y0, w, h):
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            dx, dy = x - x0, y - y0
            light = (dx // 16 + dy // 16) % 2 == 0
            c = FL['m0'] if light else FL['m1']
            if dx % 16 == 15 or dy % 16 == 15:
                c = FL['m2']
            elif light and dx % 16 + dy % 16 in (5, 6) and dx % 16 < 8:
                c = FL['m1']
            P.p(x, y, c)


def parquet(P, x0, y0, w, h):
    """Boards 4 pixels tall, their joints staggered (period 16)."""
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            dx, dy = x - x0, y - y0
            board = dy // 4
            if dy % 4 == 3:
                c = FL['pq3']
            elif (dx + (board % 4) * 4) % 16 == 0:
                c = FL['pq3']
            elif dy % 4 == 0:
                c = FL['pq1']
            else:
                c = FL['pq2'] if (dx * 3 + board * 5) % 11 == 0 else FL['pq1']
            P.p(x, y, c)


def carpet(P, x0, y0, w, h, border=True):
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            dx, dy = x - x0, y - y0
            c = FL['c2'] if (dx + dy) % 8 else FL['c1']
            if border:
                if dx in (2, w - 3) or dy in (2, h - 3):
                    c = FL['au']
                if dx in (0, 1, w - 2, w - 1) or dy in (0, 1, h - 2, h - 1):
                    c = FL['c3']
            P.p(x, y, c)


def runner(P, x0, y0, w, h):
    """A carpet running up the hall from the door: gold edges on its sides only."""
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            dx = x - x0
            c = FL['c2'] if (dx + y) % 8 else FL['c1']
            if dx in (2, w - 3):
                c = FL['au']
            if dx in (0, 1, w - 2, w - 1):
                c = FL['c3']
            P.p(x, y, c)


def medallion(P, cx, cy, r=20):
    """The university emblem in the floor: a gold ring, Magikarp leaping inside."""
    P.ellipse(cx, cy, r, r, FL['au'], FL['au2'])
    P.ellipse(cx, cy, r - 3, r - 3, FL['c3'])
    P.ellipse(cx, cy, r - 5, r - 5, FL['m0'])
    # Magikarp: an orange body, red fins, a white eye
    P.ellipse(cx, cy, 8, 6, FL['or'], FL['ol'])
    P.ellipse(cx - 1, cy - 1, 5, 3, FL['or'])
    for k in range(5):
        P.vl(cx + 8 + k // 2, cy - 4 + k, cy + 4 - k, FL['rd'])
    P.p(cx - 4, cy - 2, FL['m0'])
    P.p(cx - 4, cy - 1, FL['ol'])
    P.hl(cx - 3, cx + 2, cy - 7, FL['rd'])
    P.hl(cx - 2, cx + 1, cy - 8, FL['rd'])
    P.p(cx - 8, cy + 1, FL['ol'])
    P.hl(cx - 6, cx - 4, cy + 3, FL['m0'])


def doormat(P, x, y):
    P.r(x + 1, y + 3, 14, 12, FL['c3'])
    P.r(x + 2, y + 4, 12, 10, FL['c2'])
    P.hl(x + 3, x + 12, y + 6, FL['au'])
    P.hl(x + 3, x + 12, y + 11, FL['au'])


def wall_clock(P, cx, cy, r=13):
    P.ellipse(cx, cy, r, r, AU, AU2)
    P.ellipse(cx, cy, r - 3, r - 3, WHITE, S2)
    for a in range(12):
        ang = a * math.pi / 6
        P.p(int(round(cx - .5 + (r - 5) * math.sin(ang))), int(round(cy - .5 - (r - 5) * math.cos(ang))), OL)
    P.vl(int(cx) - 1, int(cy) - r + 5, int(cy), OL)
    P.vl(int(cx), int(cy) - r + 6, int(cy), OL)


def banner(P, x, y, h=26):
    """A hanging slate-blue banner with the gold emblem."""
    P.hl(x - 1, x + 10, y, AU2)
    for yy in range(y + 1, y + h):
        for xx in range(x, x + 10):
            P.p(xx, yy, OL if xx in (x, x + 9) else SL2)
    for k in range(5):
        P.hl(x + k, x + 9 - k, y + h + k, OL if k == 4 else SL2)
        P.p(x + k, y + h + k, OL)
        P.p(x + 9 - k, y + h + k, OL)
    P.ellipse(x + 5, y + 10, 3.5, 3.5, AU, AU2)
    P.vl(x + 1, y + 1, y + h - 1, SL1)


def stairs_up(P, x, y, w=2, h=2):
    """Stairs climbing into the back wall: white stone steps between wooden banisters."""
    for yy in range(y, y + h * T):
        dy = yy - y
        for xx in range(x, x + w * T):
            dx = xx - x
            step = dy % 6
            c = (WHITE, WHITE, S1, S1, S2, S3)[step]
            if dy < 6:
                c = SL3 if step < 4 else OL
            if dx < 3 or dx >= w * T - 3:
                c = WD if dx in (1, w * T - 2) else WD2
            P.p(xx, yy, c)


def plant(P, x, y):
    """A potted plant, 1x2 (leaves on the upper tile)."""
    P.r(x + 4, y + 22, 8, 8, FU['wd'])
    P.hl(x + 3, x + 12, y + 21, FU['wd2'])
    P.hl(x + 4, x + 11, y + 29, FU['wd2'])
    P.vl(x + 4, y + 22, y + 29, FU['wd2'])
    P.hl(x + 5, x + 10, y + 23, FU['wd0'])
    for i, (lx, ly, rx, ry) in enumerate(((8, 14, 6, 6), (4, 16, 4, 4), (12, 16, 4, 4), (6, 9, 3, 5), (10, 9, 3, 5),
                                          (8, 6, 2, 5))):
        P.ellipse(x + lx, y + ly, rx, ry, FU['p2'], FU['p3'])
    for (lx, ly) in ((6, 12), (10, 13), (8, 8), (4, 15), (12, 15)):
        P.p(x + lx, y + ly, FU['p1'])
        P.p(x + lx + 1, y + ly - 1, FU['p1'])


def counter(P, x, y, w):
    """A wooden counter, w tiles wide, one tall: a marble-white top over a panelled front."""
    X = x + w * T
    P.r(x, y + 2, w * T, 4, FU['w'])
    P.hl(x, X - 1, y + 1, FU['ol'])
    P.hl(x, X - 1, y + 6, FU['mt1'])
    P.r(x, y + 7, w * T, 8, FU['wd'])
    P.hl(x, X - 1, y + 15, FU['ol'])
    P.vl(x, y + 1, y + 15, FU['ol'])
    P.vl(X - 1, y + 1, y + 15, FU['ol'])
    for k in range(x + 8, X - 4, 16):
        P.r(k - 4, y + 9, 8, 4, FU['wd2'])
        P.r(k - 3, y + 10, 6, 2, FU['wd0'])


def bench_in(P, x, y, w=2):
    P.r(x + 1, y + 5, w * T - 2, 6, FU['wd'])
    P.hl(x + 1, x + w * T - 2, y + 4, FU['ol'])
    P.hl(x + 1, x + w * T - 2, y + 6, FU['wd0'])
    P.hl(x + 1, x + w * T - 2, y + 11, FU['wd2'])
    P.vl(x + 1, y + 4, y + 11, FU['ol'])
    P.vl(x + w * T - 2, y + 4, y + 11, FU['ol'])
    for k in (x + 3, x + w * T - 5):
        P.r(k, y + 12, 2, 3, FU['wd2'])


def pillar(P, x, y):
    """A white column inside, 1x2 tiles."""
    column(P, x + 5, y + 2, y + 31)
    P.hl(x + 3, x + 13, y + 31, S3)


def desk_row(P, x, y, w):
    """A long lecture desk with its chairs on the tile below (students sit facing the board)."""
    X = x + w * T
    P.r(x + 1, y + 3, w * T - 2, 7, FU['wd0'])
    P.hl(x + 1, X - 2, y + 2, FU['ol'])
    P.r(x + 1, y + 10, w * T - 2, 4, FU['wd'])
    P.hl(x + 1, X - 2, y + 14, FU['ol'])
    P.vl(x + 1, y + 2, y + 14, FU['ol'])
    P.vl(X - 2, y + 2, y + 14, FU['ol'])
    for k in range(x + 4, X - 6, 16):
        P.r(k, y + 5, 6, 3, FU['w'])          # Notebooks
        P.hl(k, k + 5, y + 8, FU['mt1'])
    for k in range(x + 3, X - 3, 16):         # Chairs
        P.r(k + 2, y + 18, 8, 6, FU['wd'])
        P.hl(k + 2, k + 9, y + 17, FU['ol'])
        P.hl(k + 2, k + 9, y + 24, FU['wd2'])
        P.vl(k + 3, y + 25, y + 27, FU['wd2'])
        P.vl(k + 8, y + 25, y + 27, FU['wd2'])


def blackboard(P, x, y, w):
    """A chalkboard on the back wall, w tiles wide: Fire, Water and Grass in a triangle, a Magikarp sketch."""
    X = x + w * T
    P.r(x, y + 4, w * T, 22, FU['wd'])
    P.r(x + 2, y + 6, w * T - 4, 18, FU['bg1'])
    P.hl(x, X - 1, y + 4, FU['ol'])
    P.hl(x, X - 1, y + 25, FU['wd2'])
    P.hl(x + 2, X - 3, y + 6, FU['bg2'])
    P.hl(x + 2, X - 3, y + 26, FU['ol'])
    cx, cy = x + 20, y + 15
    for (px, py) in ((cx, cy - 6), (cx - 7, cy + 5), (cx + 7, cy + 5)):
        P.ellipse(px, py, 3, 3, FU['bg1'], FU['w'])
    for t in range(1, 6):
        P.p(cx - t, cy - 5 + 2 * t, FU['w'])
        P.p(cx + t, cy - 5 + 2 * t, FU['w'])
    P.hl(cx - 4, cx + 4, cy + 5, FU['w'])
    mx, my = x + w * T - 28, y + 14
    P.ellipse(mx, my, 7, 4, FU['bg1'], FU['w'])
    P.vl(mx + 8, my - 3, my + 3, FU['w'])
    P.p(mx - 3, my - 1, FU['w'])
    for k in range(x + 40, X - 44, 3):
        P.p(k, y + 10 + (k % 7 == 0), FU['w'])
        if k % 2:
            P.p(k, y + 16, FU['w'])
        if k % 5 < 3:
            P.p(k, y + 20, FU['w'])
    P.r(X - 14, y + 23, 4, 2, FU['w'])


def lectern(P, x, y):
    P.r(x + 3, y + 4, 10, 11, FU['wd'])
    P.r(x + 2, y + 2, 12, 3, FU['wd0'])
    P.hl(x + 2, x + 13, y + 1, FU['ol'])
    P.vl(x + 2, y + 2, y + 14, FU['ol'])
    P.vl(x + 13, y + 2, y + 14, FU['ol'])
    P.hl(x + 3, x + 12, y + 15, FU['ol'])
    P.r(x + 5, y + 1, 6, 2, FU['w'])
    P.p(x + 8, y + 9, FU['ye'])


def bookshelf(P, x, y, w=1):
    for k in range(w):
        bx = x + k * T
        P.r(bx, y + 2, T, 30, FU['wd2'])
        P.hl(bx, bx + 15, y + 1, FU['ol'])
        for sy in (y + 4, y + 15):
            for i, c in enumerate(('rd', 'bl', 'ye', 'p2', 'bl', 'rd', 'mt1')):
                P.r(bx + 2 + 2 * i, sy + (i % 3 == 1), 2, 9 - (i % 3 == 1), FU[c])
            P.hl(bx + 1, bx + 14, sy + 9, FU['wd'])
            P.hl(bx + 1, bx + 14, sy + 10, FU['wd0'])
    P.vl(x, y + 1, y + 31, FU['ol'])
    P.vl(x + w * T - 1, y + 1, y + 31, FU['ol'])


def notice_board(P, x, y, w=2):
    X = x + w * T
    P.r(x + 1, y + 5, w * T - 2, 18, FU['wd'])
    P.r(x + 3, y + 7, w * T - 6, 14, FU['wd0'])
    for i, (px, py, c) in enumerate(((5, 8, 'w'), (13, 10, 'ye'), (20, 8, 'w'), (8, 15, 'bl'), (18, 15, 'w'))):
        if x + px + 6 < X - 2:
            P.r(x + px, y + py, 6, 5, FU[c])
            P.p(x + px + 2, y + py, FU['rd'])
    P.hl(x + 1, X - 2, y + 4, FU['ol'])


def mailboxes(P, x, y, w=3):
    X = x + w * T
    P.r(x + 1, y + 4, w * T - 2, 24, FU['wd'])
    P.hl(x + 1, X - 2, y + 3, FU['ol'])
    for row in range(3):
        for col in range((w * T - 4) // 6):
            bx, by = x + 3 + col * 6, y + 6 + row * 7
            P.r(bx, by, 5, 5, FU['wd2'])
            P.hl(bx + 1, bx + 3, by + 2, FU['mt1'])
            if (row + col) % 4 == 1:
                P.r(bx + 1, by + 1, 3, 2, FU['w'])


def sofa(P, x, y, w=3):
    X = x + w * T
    P.r(x + 1, y + 2, w * T - 2, 8, FU['rd'])
    P.hl(x + 1, X - 2, y + 1, FU['ol'])
    P.r(x + 1, y + 10, w * T - 2, 5, FU['wd2'])
    P.hl(x + 3, X - 4, y + 9, FU['wd0'])
    for k in range(x + 2 + 15, X - 4, 15):
        P.vl(k, y + 3, y + 8, FU['wd2'])
    P.vl(x + 1, y + 1, y + 15, FU['ol'])
    P.vl(X - 2, y + 1, y + 15, FU['ol'])
    P.hl(x + 1, X - 2, y + 15, FU['ol'])
    P.r(x + 2, y + 3, 3, 11, FU['wd'])
    P.r(X - 5, y + 3, 3, 11, FU['wd'])


def low_table(P, x, y, w=2):
    X = x + w * T
    P.r(x + 3, y + 3, w * T - 6, 8, FU['wd0'])
    P.hl(x + 3, X - 4, y + 2, FU['ol'])
    P.hl(x + 3, X - 4, y + 11, FU['wd2'])
    P.vl(x + 3, y + 2, y + 12, FU['ol'])
    P.vl(X - 4, y + 2, y + 12, FU['ol'])
    P.r(x + 8, y + 5, 5, 4, FU['w'])
    P.p(x + 18, y + 6, FU['rd'])
    P.p(x + 19, y + 6, FU['rd'])


def vending(P, x, y):
    P.r(x + 1, y + 1, 14, 30, FU['rd'])
    P.vl(x + 1, y + 1, y + 30, FU['ol'])
    P.vl(x + 14, y + 1, y + 30, FU['ol'])
    P.hl(x + 1, x + 14, y + 1, FU['ol'])
    P.r(x + 3, y + 4, 7, 14, FU['w'])
    for k in range(3):
        for j in range(2):
            P.r(x + 4 + j * 3, y + 5 + k * 4, 2, 3, (FU['bl'], FU['ye'], FU['p2'])[k])
    P.r(x + 11, y + 6, 2, 4, FU['mt1'])
    P.r(x + 3, y + 22, 10, 4, FU['ol'])
    P.hl(x + 1, x + 14, y + 30, FU['wd2'])


def window_in(P, x, y, rows=2):
    """The facade's arched window, seen from inside (same art)."""
    arch_window(P, x, y + (rows - 2) * T, h=32, lit=True)


def interior(w, h, wall_rows, floor, wainscot='slate'):
    """(floor layer, object layer) of a room w x h: back wall rows, the floor, side and front walls."""
    F = Pic(w * T, h * T)
    O = Pic(w * T, h * T)
    floor(F, 0, wall_rows * T, w * T, (h - 1 - wall_rows) * T)
    back_wall(F, 0, 0, w * T, wall_rows, wainscot)
    side_walls(F, w, h, wall_rows)
    return F, O


def main_hall_inside():
    """The Main Hall's ground floor, 15x11 (its outside's size): marble, a blue runner from the door (7,9) up to the
    reception counter, the emblem in the floor, the clock on the back wall under the tower, white columns, stairs up
    to the gallery (12-13, 1-3)."""
    w, h = 15, 11
    F, O = interior(w, h, 3, marble)
    runner(F, 6 * T, 3 * T, 3 * T, 7 * T)
    medallion(F, 7 * T + 8, 7 * T + 8, 21)
    for c in (2, 4, 10):
        window_in(F, c * T, 0, rows=3)
    wall_clock(F, 7 * T + 8, 16, 13)
    banner(F, 5 * T + 3, 4)
    banner(F, 9 * T + 3, 4)
    stairs_up(F, 12 * T, 1 * T, 2, 3)
    doormat(F, 7 * T, 9 * T)
    counter(O, 5 * T, 4 * T, 5)
    for (x, y) in ((3, 5), (11, 5), (3, 7), (11, 7)):
        pillar(O, x * T, (y - 1) * T)
    plant(O, 1 * T, 2 * T)
    plant(O, 1 * T, 8 * T)
    plant(O, 13 * T, 8 * T)
    bench_in(O, 1 * T, 5 * T)
    bench_in(O, 12 * T, 5 * T)
    notice_board(O, 1 * T, 0 * T + 12, 1)
    return F, O, [('people/youngster_frlg', 7, 3, FACE_DOWN), (PLAYER, 7, 8, FACE_UP),
                  ('people/lass_frlg', 2, 7, FACE_LEFT), ('people/gentleman_frlg', 12, 8, FACE_DOWN)]


def lecture_hall_inside():
    """Lecture Hall A, 11x9 (its wing's size): parquet, wood wainscot, the chalkboard and the lectern, two rows of
    desks with chairs, the aisle up the middle from the door (5,7)."""
    w, h = 11, 9
    F, O = interior(w, h, 2, parquet, wainscot='wood')
    blackboard(O, 3 * T, 0, 5)
    for c in (1, 9):
        window_in(F, c * T, 0)
    doormat(F, 5 * T, 7 * T)
    lectern(O, 5 * T, 2 * T)
    for (x, y) in ((1, 3), (6, 3), (1, 5), (6, 5)):
        desk_row(O, x * T, y * T, 4 if x == 1 else 4)
    return F, O, [('people/scientist', 6, 2, FACE_DOWN), ('people/lass_frlg', 2, 4, FACE_UP),
                  ('people/youngster_frlg', 7, 6, FACE_UP), (PLAYER, 5, 6, FACE_UP)]


def dorm_lobby_inside():
    """The dorm lobby, 17x10 (the residence hall's size): plank floor, wood wainscot, stairs up (1-2, 0-2), the
    mailboxes and notice board, a lounge with a rug and sofa, the warden's counter, the door at (8,8)."""
    w, h = 17, 10
    F, O = interior(w, h, 2, parquet, wainscot='wood')
    stairs_up(F, 1 * T, 0, 2, 3)
    for c in (4, 12):
        window_in(F, c * T, 0)
    mailboxes(O, 5 * T, 0, 3)
    notice_board(O, 9 * T, 0, 2)
    carpet(F, 10 * T, 4 * T, 6 * T, 4 * T)
    doormat(F, 8 * T, 8 * T)
    sofa(O, 11 * T, 4 * T, 4)
    low_table(O, 12 * T, 6 * T, 2)
    counter(O, 2 * T, 5 * T, 4)
    vending(O, 15 * T, 1 * T)
    plant(O, 3 * T, 1 * T)
    plant(O, 15 * T, 7 * T)
    plant(O, 1 * T, 7 * T)
    bookshelf(O, 13 * T, 0, 2)
    return F, O, [('people/old_woman_frlg', 3, 4, FACE_DOWN), (PLAYER, 8, 7, FACE_UP),
                  ('people/lass_frlg', 11, 7, FACE_UP), ('people/picnicker_frlg', 6, 5, FACE_LEFT)]


# --- Budget ------------------------------------------------------------------------------------------------------------
def tiles_of(im):
    """Every non-empty 8x8 tile of an RGBA picture, as tuples of pixels (None = transparent)."""
    px = im.load()
    out = []
    for ty in range(0, im.height, 8):
        for tx in range(0, im.width, 8):
            t = tuple(tuple(px[tx + x, ty + y][:3] if px[tx + x, ty + y][3] else None for x in range(8))
                      for y in range(8))
            if any(c is not None for row in t for c in row):
                out.append(t)
    return out


def canon(t):
    variants = [t, tuple(r[::-1] for r in t), t[::-1], tuple(r[::-1] for r in t[::-1])]
    return min(variants, key=repr)


def fits(t, pals):
    cols = {c for row in t for c in row if c is not None and c != BLACK}
    return [name for name, pal in pals.items() if cols <= set(pal)]


def budget(name, layers, pals):
    tiles = [t for im in layers for t in tiles_of(im)]
    unique = {canon(t) for t in tiles}
    bad = [t for t in unique if not fits(t, pals)]
    metas = set()
    for im in layers:
        px = im.load()
        for ty in range(0, im.height, 16):
            for tx in range(0, im.width, 16):
                block = tuple(px[tx + x, ty + y] for y in range(16) for x in range(16))
                if any(p[3] for p in block):
                    metas.add(block)
    print('%-22s %4d unique 8x8 tiles (limit 384), ~%d metatile layers, %d tiles off-palette, palettes %s'
          % (name, len(unique), len(metas), len(bad), ', '.join(pals)))
    return len(unique), len(metas), len(bad)


# --- Output ------------------------------------------------------------------------------------------------------------
def font(size):
    try:
        return ImageFont.load_default(size=size)
    except TypeError:
        return ImageFont.load_default()


def scaled(img, s):
    return img.resize((img.width * s, img.height * s), Image.NEAREST)


def label(img, text, xy, size=16, fill=(255, 255, 255), bg=(32, 32, 48)):
    d = ImageDraw.Draw(img)
    f = font(size)
    box = d.textbbox(xy, text, font=f)
    d.rectangle((box[0] - 4, box[1] - 3, box[2] + 4, box[3] + 3), fill=bg)
    d.text(xy, text, font=f, fill=fill)


def outside_inside(name, outside, w, h, door_col, F, O, people, out, s=3, above=0):
    """The exterior (on grass) beside the interior, at the same scale, with the size and the door marked. `above`:
    rows of the outside picture that rise over the footprint (the clock tower), not part of it."""
    g = Ground(w + 2, h + 2 + above)
    g.path(door_col, h + 1 + above, door_col + 3, h + 2 + above)
    g.lay()
    ext = g.render()
    ext.alpha_composite(outside.im, (T, T))
    top_extra = above * T * s
    room = Image.new('RGBA', (w * T, h * T), (0, 0, 0, 255))
    room.alpha_composite(F.im)
    room.alpha_composite(O.im)
    for (spr, x, y, f) in people:
        put_sprite(room, spr, x, y, f)
    ext_s, room_s = scaled(ext, s), scaled(room, s)
    pad, top = 24, 56
    canvas = Image.new('RGBA', (ext_s.width + room_s.width + pad * 3, max(ext_s.height, room_s.height) + top + pad),
                       (24, 24, 36, 255))
    canvas.alpha_composite(ext_s, (pad, top))
    rt = top + T * s + top_extra
    canvas.alpha_composite(room_s, (ext_s.width + pad * 2, rt))
    d = ImageDraw.Draw(canvas)
    # The footprint's outline on both
    d.rectangle((pad + T * s, rt, pad + (w + 1) * T * s - 1, rt + h * T * s - 1), outline=(255, 64, 255), width=2)
    d.rectangle((ext_s.width + pad * 2, rt, ext_s.width + pad * 2 + w * T * s - 1, rt + h * T * s - 1),
                outline=(255, 64, 255), width=2)
    for (ox, oy) in ((pad + (door_col + 1) * T * s, rt + (h - 1) * T * s),
                     (ext_s.width + pad * 2 + door_col * T * s, rt + (h - 2) * T * s)):
        d.rectangle((ox, oy, ox + T * s - 1, oy + T * s - 1), outline=(255, 48, 48), width=3)
    label(canvas, '%s - outside %dx%d' % (name, w, h), (pad, 14), 22)
    label(canvas, 'inside %dx%d (door column %d)' % (w, h, door_col), (ext_s.width + pad * 2, 14), 22)
    canvas.convert('RGB').save(out)


# The proposed whole campus (76x64), as blocks: name -> (x, y, w, h, door columns, kind). A starting point for
# the real layout script. The mockup's central building is this one shifted (plan = mockup + (7, 9)); the greenhouse
# sits behind the Main Hall here, west of where the mockup shows it.
PLAN_W, PLAN_H = 76, 64
PLAN = {
    'Stadium (outdoor)': (4, 4, 22, 12, (), 'field'),
    'Literature | Library': (4, 17, 22, 9, (5, 16), 'stone'),
    'Home Economics (greenhouse)': (29, 8, 17, 9, (8,), 'glass'),
    'Ranch': (48, 5, 9, 11, (), 'field'),
    'Science Institute': (58, 8, 13, 13, (6,), 'glass'),
    'Clock tower': (36, 20, 5, 6, (), 'tower'),
    'Faculty Office': (11, 28, 9, 8, (4,), 'stone'),
    'Lecture Hall A': (20, 27, 11, 9, (5,), 'stone'),
    'Main Hall': (31, 25, 15, 11, (7,), 'stone'),
    'Lecture Hall B': (46, 27, 11, 9, (5,), 'stone'),
    'Cafeteria': (57, 28, 9, 8, (4,), 'stone'),
    'Fountain': (35, 39, 7, 5, (), 'water'),
    'Pool': (4, 41, 9, 8, (), 'water'),
    'Hospital': (14, 40, 13, 10, (6,), 'stone'),
    'Reflecting pools': (32, 47, 4, 11, (), 'water'),
    'Reflecting pools ': (41, 47, 4, 11, (), 'water'),
    'Dorm': (50, 40, 17, 10, (8,), 'stone'),
    'Dorm annex (your room)': (53, 52, 11, 8, (5,), 'stone'),
    'Gate': (35, 58, 6, 6, (2, 3), 'stone'),
}
PLAN_PATHS = ((4, 36, 72, 38), (32, 38, 45, 46), (36, 46, 41, 58), (26, 17, 28, 36), (28, 17, 57, 20),
              (56, 21, 72, 23), (70, 23, 72, 36), (4, 26, 26, 28), (4, 28, 6, 36), (13, 50, 50, 52),
              (57, 50, 60, 52), (14, 16, 16, 17))


def campus_plan(out, px=12):
    colors = {'stone': (232, 228, 214), 'glass': (168, 220, 222), 'water': (104, 168, 232), 'field': (150, 200, 120),
              'tower': (200, 200, 220)}
    img = Image.new('RGB', (PLAN_W * px, PLAN_H * px), (112, 192, 152))
    d = ImageDraw.Draw(img)
    for y in range(PLAN_H):
        for x in range(PLAN_W):
            if x < 4 or x >= PLAN_W - 4 or y < 4 or y >= PLAN_H - 4:
                d.rectangle((x * px, y * px, x * px + px - 1, y * px + px - 1), fill=(40, 104, 64))
    for (x0, y0, x1, y1) in PLAN_PATHS:
        d.rectangle((x0 * px, y0 * px, x1 * px - 1, y1 * px - 1), fill=(214, 216, 220))
    f = font(11)
    for name, (x, y, w, h, doors, kind) in PLAN.items():
        d.rectangle((x * px, y * px, (x + w) * px - 1, (y + h) * px - 1), fill=colors[kind], outline=(40, 48, 72))
        for dc in doors:
            d.rectangle(((x + dc) * px, (y + h - 1) * px, (x + dc + 1) * px - 1, (y + h) * px - 1), fill=(200, 40, 40))
        if kind != 'tower':
            d.text((x * px + 3, y * px + 2), name.strip(), font=f, fill=(24, 24, 40))
            d.text((x * px + 3, y * px + 14), '%dx%d' % (w, h), font=f, fill=(24, 24, 40))
    for i in range(0, PLAN_W, 10):
        d.text((i * px + 1, 1), str(i), font=f, fill=(255, 255, 255))
    for i in range(0, PLAN_H, 10):
        d.text((1, i * px + 1), str(i), font=f, fill=(255, 255, 255))
    img.save(out)


def main():
    os.makedirs(OUT, exist_ok=True)
    campus_plan(os.path.join(OUT, 'plan_campus_blocks.png'))
    campus, ext_layer = campus_section()
    c2 = scaled(campus, 2)
    label(c2, 'Celadon University, central section (64x40 tiles): the custom-tile mockup', (16, 12), 22)
    for name, (x, y, w, h, d) in CENTER.items():
        label(c2, name.replace('_', ' ').title() + ' %dx%d' % (w, h), (x * 32 + 8, (y + 1) * 32 + 4), 14,
              bg=(40, 48, 72))
    c2.convert('RGB').save(os.path.join(OUT, 'mockup_campus_center.png'))

    rooms = []
    specs = (
        ('Main Hall', lambda: (main_hall(), clock_tower()), 15, 11, 7, main_hall_inside,
         'mockup_main_hall.png'),
        ('Lecture Hall A', lambda: (wing(5, 11), None), 11, 9, 5, lecture_hall_inside, 'mockup_lecture_hall.png'),
        ('Dorm lobby', lambda: (dorm(), None), 17, 10, 8, dorm_lobby_inside, 'mockup_dorm_lobby.png'),
    )
    for (name, ext, w, h, d, inside, out) in specs:
        pic, tower = ext()
        above = 0
        if tower:
            # The tower stands on the roof (its base on the first roof row) and rises 5 rows above the footprint
            above = 5
            outside = Pic(w * T, (h + above) * T)
            outside.paste(pic, 0, above * T)
            outside.paste(tower, 5 * T, 0)
        else:
            outside = pic
        F, O, people = inside()
        rooms += [F.im, O.im]
        outside_inside(name, outside, w, h, d, F, O, people, os.path.join(OUT, out), above=above)
    ext_layers = [ext_layer.im, dorm().im]
    budget('campus (exterior)', ext_layers, {k: PALETTES[k] for k in ('stone', 'glass', 'water')})
    budget('interiors', rooms, {k: PALETTES[k] for k in ('stone', 'floor', 'furn')})
    sheet(ext_layers, os.path.join(OUT, 'tiles_exterior.png'))
    sheet(rooms, os.path.join(OUT, 'tiles_interior.png'))


def sheet(layers, out, per_row=16):
    """The unique 8x8 tiles, the way they'd sit in tiles.png (16 a row)."""
    seen, order = set(), []
    for im in layers:
        for t in tiles_of(im):
            k = canon(t)
            if k not in seen:
                seen.add(k)
                order.append(t)
    rows = (len(order) + per_row - 1) // per_row
    img = Image.new('RGB', (per_row * 8, rows * 8), (255, 0, 255))
    px = img.load()
    for i, t in enumerate(order):
        for y in range(8):
            for x in range(8):
                if t[y][x] is not None:
                    px[(i % per_row) * 8 + x, (i // per_row) * 8 + y] = t[y][x]
    scaled(img, 3).save(out)


if __name__ == '__main__':
    main()
