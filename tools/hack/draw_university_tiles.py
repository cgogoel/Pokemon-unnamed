#!/usr/bin/env python3
"""Celadon University's own tiles (docs/hack/university_remake/plan.md): draws every campus building and the
campus's windows the rooms use, writes the campus tileset, has draw_university_interiors.py compose the rooms and write
their tilesets, and renders the design mockups from them.

    <venv with Pillow>/bin/python tools/hack/draw_university_tiles.py

Writes
    data/tilesets/secondary/celadon_university/    gTileset_CeladonUniversity (over gTileset_General_Frlg): the campus
    data/tilesets/secondary/university_{classroom,dorm,lab}/   the rooms' tilesets (draw_university_interiors.py)
    tools/hack/university_stamps.json              every building / room / furniture piece as a grid of metatile ids,
                                                   for the layout scripts (null = leave the ground as it is)
    docs/hack/university_remake/*.png              the mockups, rendered from the written tilesets
and prints the tile budget (FRLG secondary: 384 tiles, 384 metatiles, palettes 7-12).

The look follows the Tamamushi University lore art: white stone with grey-blue slate mansard roofs, tall arched
windows and gold trim, the clock tower over the Main Hall, the Gyarados fountain, the glass greenhouse, the glass
Science tower, the stadium with its track and floodlights, all in dense forest. The rooms carry the same materials
inside (draw_university_interiors.py: FireRed's interior look, with the campus's own windows). To fit one campus tileset the art is kept plain and is drawn on the metatile grid, in the palettes' colors only,
and mirror-symmetric wherever it can be (the right half of a building is the left half flipped, which the tile
deduplication gets for free).

Metatiles: the campus art sits in the middle layer over the primary's grass (layer type COVERED, under the player);
the clock tower's spire, which rises over the path behind the Main Hall, is in the top layer (NORMAL) so people walk
behind it. Doors are MB_ANIMATED_DOOR (no
door animation yet: the game just doesn't animate them), the gate's passage and room door mats MB_SOUTH_ARROW_WARP,
stairs the behavior of the Celadon Condominiums' stairs. Everything else is MB_NORMAL: collision goes in the layouts.
"""
import json
import math
import os
import re
import struct
import sys

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # noqa: E402
from draw_mansion_furnace import write_png  # noqa: E402

ROOT = R.ROOT
OUT = os.path.join(ROOT, 'docs/hack/university_remake')
STAMPS = os.path.join(ROOT, 'tools/hack/university_stamps.json')
T = 16
PRIMARY = 640
FIRST_PAL = 7
MAX_TILES = MAX_METATILES = 384
LAYER_NORMAL, LAYER_COVERED = 0, 1


def gba(c):
    return tuple((v >> 3) << 3 for v in c)


# --- Palettes ----------------------------------------------------------------------------------------------------------
OL = (40, 48, 72)        # The outline: a dark slate blue, never black
WHITE = (248, 248, 240)
S1, S2, S3 = (224, 224, 208), (192, 192, 176), (144, 144, 128)    # White stone
SL1, SL2, SL3 = (152, 168, 200), (112, 128, 168), (72, 88, 128)   # Slate (the school color)
G1, G2, G3 = (200, 232, 248), (120, 176, 224), (64, 112, 168)     # Window glass
AU, AU2 = (248, 208, 88), (192, 136, 40)                          # Gold
WD, WD2 = (168, 104, 56), (104, 64, 40)                           # Wood

PAL = {
    'stone': [OL, WHITE, S1, S2, S3, SL1, SL2, SL3, G1, G2, G3, AU, AU2, WD, WD2],
    'glass': [OL, WHITE, S1, (200, 208, 216), (144, 152, 168), (88, 96, 112), (224, 248, 248), (168, 224, 224),
              (112, 192, 200), (64, 144, 160), (136, 200, 96), (72, 152, 64), (40, 96, 48), (248, 152, 176),
              (216, 72, 88)],
    'water': [OL, WHITE, S1, S2, S3, (176, 224, 248), (104, 176, 240), (56, 120, 208), (96, 152, 224), (40, 80, 160),
              (248, 232, 168), (224, 72, 56), (248, 136, 56), AU],
    'accent': [OL, WHITE, S1, S2, S3, (200, 96, 72), (152, 64, 56), (88, 120, 200), (48, 72, 144), (224, 56, 56),
               (160, 32, 40), (248, 176, 192), (64, 160, 96), (104, 112, 128), (248, 216, 96)],
}
PAL = {k: [gba(c) for c in v] for k, v in PAL.items()}
EXTERIOR_PALS = ('stone', 'glass', 'water', 'accent')
GL = dict(zip(('ol', 'w', 's1', 'f1', 'f2', 'f3', 'q0', 'q1', 'q2', 'q3', 'p1', 'p2', 'p3', 'pk', 'rd'), PAL['glass']))
WA = dict(zip(('ol', 'w', 's1', 's2', 's3', 'a1', 'a2', 'a3', 'b1', 'b2', 'cr', 'rd', 'or', 'au'), PAL['water']))
AC = dict(zip(('ol', 'w', 's1', 's2', 's3', 't1', 't2', 'b1', 'b2', 'rd', 'rd2', 'pk', 'gn', 'mt', 'ye'),
              PAL['accent']))
OL, WHITE, S1, S2, S3, SL1, SL2, SL3, G1, G2, G3, AU, AU2, WD, WD2 = PAL['stone']
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

    def clear(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[x, y] = (0, 0, 0, 0)

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
        """Pixels whose centers are inside; symmetric about x = cx - 0.5 (so cx = 8 is symmetric in a tile)."""
        def inside(x, y):
            return ((x + .5 - cx) / rx) ** 2 + ((y + .5 - cy) / ry) ** 2 <= 1
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                if inside(x, y):
                    edge = outline and not all(inside(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                    self.p(x, y, outline if edge else c)

    def paste(self, other, x, y):
        self.im.alpha_composite(other.im, (x, y))

    def mirror(self, x0=0, w=None, y0=0, h=None):
        """Copies the left half of a region onto its right half, flipped."""
        w = self.w - x0 if w is None else w
        h = self.h - y0 if h is None else h
        for y in range(y0, y0 + h):
            for x in range(w // 2):
                self.px[x0 + w - 1 - x, y] = self.px[x0 + x, y]


# --- The stone kit (exteriors) ---------------------------------------------------------------------------------------
def roof(P, x0, y0, w, h, dormers=(), drow=None):
    """A slate mansard roof: a light ridge, 4-pixel shingle courses (every inner metatile the same), a white dentil
    cornice as its last 8 rows."""
    for y in range(h - 8):
        for x in range(w):
            if y < 4:
                c = (OL, SL1, WHITE, SL1)[y]
            else:
                course, row = (y - 4) // 4, (y - 4) % 4
                c = SL1 if row == 0 else SL3 if row == 3 else SL3 if (x + (course % 2) * 4) % 8 == 0 else SL2
            P.p(x0 + x, y0 + y, c)
        P.p(x0, y0 + y, OL)
        P.p(x0 + 1, y0 + y, SL3 if y else OL)
        P.p(x0 + w - 2, y0 + y, SL3 if y else OL)
        P.p(x0 + w - 1, y0 + y, OL)
    cornice(P, x0, y0 + h - 8, w)
    for col in dormers:
        dormer(P, x0 + col * T, y0 + (drow if drow is not None else h // T - 2) * T)


def cornice(P, x0, y, w):
    for x in range(x0, x0 + w):
        dx = (x - x0) % 8
        dent = dx in (1, 6)
        for k, c in enumerate((OL, WHITE, S1, WHITE, S3 if dent else WHITE, S3 if dent else S1, S2, S3)):
            P.p(x, y + k, c)
    P.vl(x0, y, y + 7, OL)
    P.vl(x0 + w - 1, y, y + 7, OL)


def dormer(P, x, y):
    """A small arched dormer (one metatile, symmetric)."""
    P.r(x + 3, y + 2, 10, 13, WHITE)
    P.r(x + 3, y + 1, 10, 2, SL3)
    P.hl(x + 5, x + 10, y, OL)
    P.p(x + 4, y + 1, OL)
    P.p(x + 11, y + 1, OL)
    P.vl(x + 2, y + 2, y + 14, OL)
    P.vl(x + 13, y + 2, y + 14, OL)
    P.hl(x + 2, x + 13, y + 15, SL3)
    for yy in range(y + 4, y + 13):
        for xx in range(x + 5, x + 11):
            corner = yy == y + 4 and xx in (x + 5, x + 10)
            P.p(xx, yy, WHITE if corner or xx in (x + 7, x + 8) else G1 if yy < y + 8 else G2 if yy < y + 11 else G3)
    P.hl(x + 5, x + 10, y + 13, S2)


def wall(P, x0, y0, w, h):
    """White stone in 8-pixel courses (horizontal joints only: plain, and the same flipped), outlined sides."""
    for y in range(y0, y0 + h):
        for x in range(x0, x0 + w):
            P.p(x, y, S1 if (y - y0) % 8 == 7 else WHITE)
        P.p(x0, y, OL)
        P.p(x0 + w - 1, y, OL)


def plinth(P, x0, y, w):
    for x in range(x0, x0 + w):
        for k, c in enumerate((S2, S1, S2, S3)):
            P.p(x, y + k, c)
    P.vl(x0, y, y + 3, OL)
    P.vl(x0 + w - 1, y, y + 3, OL)


def window(P, x, y):
    """A tall arched window in a 16x32 cell, symmetric: slate outline, glass light above the transom, a white cross,
    a keystone and a sill."""
    L, Rr, top, bot = x + 3, x + 12, y + 5, y + 26
    for yy in range(top, bot + 1):
        inset = 3 if yy == top else 1 if yy == top + 1 else 0
        for xx in range(L + inset, Rr - inset + 1):
            if xx in (L + inset, Rr - inset) or yy in (top, bot):
                c = OL
            elif xx in (x + 7, x + 8) or yy == top + 8:
                c = WHITE
            else:
                c = G1 if yy < top + 8 else G3 if yy >= bot - 3 else G2
            P.p(xx, yy, c)
    P.p(x + 7, top - 1, WHITE)
    P.p(x + 8, top - 1, WHITE)
    P.hl(x + 2, x + 13, bot + 1, WHITE)
    P.hl(x + 2, x + 13, bot + 2, S3)


def sqwindow(P, x, y):
    """A square window (dorms, hospital) in a 16x32 cell, symmetric."""
    P.r(x + 3, y + 8, 10, 14, OL)
    P.r(x + 4, y + 9, 8, 12, G2)
    P.r(x + 4, y + 9, 8, 4, G1)
    P.r(x + 4, y + 18, 8, 3, G3)
    P.vl(x + 7, y + 9, y + 20, WHITE)
    P.vl(x + 8, y + 9, y + 20, WHITE)
    P.hl(x + 4, x + 11, y + 14, WHITE)
    P.hl(x + 2, x + 13, y + 22, WHITE)
    P.hl(x + 2, x + 13, y + 23, S3)


def door(P, x, y, glass=False):
    """A 16x32 door cell (symmetric): an arched stone portal with a fanlight over a double door on the lower
    metatile (the one a door animation would replace)."""
    P.r(x + 1, y + 5, 14, 27, S2)
    P.hl(x + 4, x + 11, y + 3, OL)
    P.hl(x + 2, x + 13, y + 4, OL)
    P.p(x + 3, y + 4, OL)
    P.vl(x + 1, y + 5, y + 31, OL)
    P.vl(x + 14, y + 5, y + 31, OL)
    P.vl(x + 2, y + 5, y + 31, WHITE)
    P.vl(x + 13, y + 5, y + 31, WHITE)
    for yy in range(y + 7, y + 16):
        inset = 3 if yy == y + 7 else 1 if yy == y + 8 else 0
        for xx in range(x + 3 + inset, x + 13 - inset):
            edge = yy == y + 7 or xx in (x + 3 + inset, x + 12 - inset)
            P.p(xx, yy, OL if edge else WHITE if xx in (x + 7, x + 8) else G1 if yy < y + 12 else G2)
    for yy in range(y + 16, y + 31):
        for xx in range(x + 3, x + 13):
            if glass:
                c = OL if xx in (x + 3, x + 12, x + 7, x + 8) else G1 if yy < y + 21 else G2
            else:
                c = WD2 if xx in (x + 3, x + 12, x + 7, x + 8) or yy in (y + 19, y + 26) else WD
            P.p(xx, yy, c)
    P.hl(x + 3, x + 12, y + 16, OL)
    if not glass:
        P.p(x + 6, y + 23, AU)
        P.p(x + 9, y + 23, AU)
    P.hl(x + 1, x + 14, y + 31, S3)


def column(P, x, y0, y1):
    """A white column centered in the 16-wide cell at x (symmetric)."""
    for y in range(y0 + 3, y1 - 2):
        for k, c in enumerate((OL, S1, WHITE, WHITE, S1, OL)):
            P.p(x + 5 + k, y, c)
    P.hl(x + 3, x + 12, y0, OL)
    P.hl(x + 3, x + 12, y0 + 1, WHITE)
    P.hl(x + 4, x + 11, y0 + 2, S2)
    P.hl(x + 4, x + 11, y1 - 2, WHITE)
    P.hl(x + 3, x + 12, y1 - 1, S2)
    P.p(x + 3, y1 - 1, OL)
    P.p(x + 12, y1 - 1, OL)


def pediment(P, c, base_y, half, height, emblem=True):
    """A triangle pediment symmetric about the pixel boundary c."""
    for dy in range(height + 1):
        y = base_y - dy
        hw = half * (1 - dy / height)
        for x in range(int(c - hw) - 1, int(c + hw) + 2):
            d = abs(x + .5 - c)
            if d > hw:
                continue
            if d > hw - 1.5 or dy == 0:
                col = OL
            elif d < hw - 5 and 2 < dy < height - 3:
                col = S1
            else:
                col = WHITE
            P.p(x, y, col)
    if emblem:
        cy = base_y - int(height * .36)
        P.ellipse(c, cy, 4.5, 4.5, AU, AU2)


def has_window(c, w_tiles, spacing):
    """Windows every `spacing` columns, centered so a mirrored building stays symmetric (on an even width with
    spacing 2, the middle pair are both windows)."""
    d2 = abs(2 * c - (w_tiles - 1))  # Twice the distance from the middle
    if spacing == 2:
        return d2 % 4 in (0, 1)
    if w_tiles % 2:
        return (d2 // 2) % spacing == 0
    return d2 % (2 * spacing) == spacing


def facade(P, x0, y0, w_tiles, storeys, kind='arch', skip=(), spacing=2, columns=None):
    """Storeys of windows (2 rows each) every `spacing` columns within columns 1..w-2 (or exactly `columns`, which may
    include half columns such as 10.5 for the middle of an even-width building), except (col, storey) in skip
    (storey 0 = the top one)."""
    for s in range(storeys):
        for c in [c for c in (columns or ()) if c != int(c)]:  # Half-column windows, between two tiles
            (window if kind == 'arch' else sqwindow)(P, x0 + int(c * T), y0 + s * 2 * T)
        for c in range(1, w_tiles - 1):
            if (c not in columns) if columns is not None else not has_window(c, w_tiles, spacing):
                continue
            if (c, s) in skip or c in skip:
                continue
            (window if kind == 'arch' else sqwindow)(P, x0 + c * T, y0 + s * 2 * T)


def academic(w, roof_rows, storeys, doors, dormers=(), kind='arch', pediment_over=None, skip=(), spacing=2,
             columns=None):
    """The academic building: mansard roof, storeys of windows, doors (columns) on the ground storey, plinth.
    Symmetric when the doors and dormers are."""
    h = roof_rows + storeys * 2
    P = Pic(w * T, h * T)
    roof(P, 0, 0, P.w, roof_rows * T, dormers, roof_rows - 2)
    wall(P, 0, roof_rows * T, P.w, storeys * 2 * T)
    facade(P, 0, roof_rows * T, w, storeys, kind, skip=[(d, storeys - 1) for d in doors] + list(skip), spacing=spacing,
           columns=columns)
    for d in doors:
        door(P, d * T, (h - 2) * T, glass=kind != 'arch')
    plinth(P, 0, h * T - 4, P.w)
    for d in doors:  # The plinth stops at the door
        P.r(d * T + 2, h * T - 4, 12, 3, S2)
        P.hl(d * T + 1, d * T + 14, h * T - 1, S3)
    if pediment_over is not None:
        pediment(P, pediment_over * T + 8, roof_rows * T - 1, 30, 20)
    return P


# --- The buildings ---------------------------------------------------------------------------------------------------
def main_hall():
    """15x16: the Main Hall (15x11, rows 5-15) with the clock tower standing on its roof (rows 0-5, columns 5-9).
    Mansard roof, three storeys, a portico of four columns under a pediment, the door at column 7."""
    P = Pic(15 * T, 16 * T)
    B = academic(15, 5, 3, doors=(7,), dormers=(2, 4, 10, 12), skip=[(c, s) for c in range(5, 10) for s in (0, 1)])
    P.paste(B, 0, 5 * T)
    y0 = 5 * T
    # The portico: columns 5-9, rows 7-10 of the building; a shadowed hall behind columns
    P.r(5 * T, y0 + 7 * T + 6, 5 * T, 4 * T - 10, S3)
    for c in (5, 6, 8, 9):
        P.r(c * T, y0 + 7 * T + 6, T, 4 * T - 10, S3)
    door(P, 7 * T, y0 + 9 * T)
    P.r(5 * T, y0 + 7 * T, 5 * T, 6, WHITE)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T, OL)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T + 4, S2)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T + 5, OL)
    for c in (5, 6, 8, 9):
        column(P, c * T, y0 + 7 * T + 6, y0 + 11 * T - 4)
    pediment(P, 120, y0 + 7 * T, 40, 24)
    for x in range(5 * T, 10 * T):
        for k, c in enumerate((S1, WHITE, S2, S3)):
            P.p(x, y0 + 11 * T - 4 + k, c)
    tower(P, 5 * T, 0)
    P.mirror()
    return P


def tower(P, x0, y0):
    """The clock tower, 5x6 (its base row sits on the Main Hall's roof): balustrade, body with the clock at noon,
    belfry, slate spire and gold finial. Drawn on the left half; main_hall() mirrors it."""
    cx = x0 + 40
    # Base: a balustrade (row 5)
    by = y0 + 5 * T
    P.r(x0 + 2, by, 76, 16, WHITE)
    for x in range(x0 + 2, x0 + 78):
        P.p(x, by, OL)
        P.p(x, by + 1, WHITE)
        P.p(x, by + 2, S2)
        if (x - x0) % 4 == 2:
            P.vl(x, by + 4, by + 10, S2)
        P.p(x, by + 12, S1)
        P.p(x, by + 13, S2)
        P.p(x, by + 14, S3)
        P.p(x, by + 15, S3)
    P.vl(x0 + 2, by, by + 15, OL)
    # Body (rows 2-4), 48 wide
    wall(P, x0 + 16, y0 + 2 * T, 48, 3 * T)
    P.ellipse(cx, y0 + 3 * T + 12, 14, 14, AU, AU2)
    P.ellipse(cx, y0 + 3 * T + 12, 11, 11, WHITE, S2)
    for (dx, dy) in ((0, -9), (-9, 0), (0, 8), (-7, -6), (-7, 5)):
        P.p(cx - 1 + dx if dx <= 0 else cx + dx, y0 + 3 * T + 12 + dy, OL)
    P.vl(cx - 1, y0 + 3 * T + 5, y0 + 3 * T + 12, OL)
    cornice(P, x0 + 14, y0 + 3 * T - 6, 52)
    # Belfry (row 2): an arched opening
    ox, oy = x0 + 24, y0 + 2 * T + 1
    for yy in range(oy, oy + 8):
        inset = 1 if yy == oy else 0
        for xx in range(ox + inset, ox + 8 - inset):
            P.p(xx, yy, OL if yy == oy or xx in (ox, ox + 7) else SL3)
    P.hl(ox, ox + 7, oy + 8, WHITE)
    # Spire (rows 0-1)
    for dy in range(2 * T - 2):
        y = y0 + 2 * T - 1 - dy
        hw = 26 * (1 - dy / 30) + 1
        for x in range(int(cx - hw), cx):
            edge = x == int(cx - hw)
            P.p(x, y, OL if edge else SL1 if dy % 3 == 0 else SL2)
    P.hl(cx - 27, cx - 1, y0 + 2 * T - 1, OL)
    P.vl(cx - 1, y0, y0 + 3, AU)
    P.p(cx - 2, y0 + 1, AU2)


def wing():
    """A lecture-hall wing, 11x9: two storeys, the door at column 5."""
    P = academic(11, 5, 2, doors=(5,), dormers=(2, 8))
    P.mirror()
    return P


def pavilion():
    """An end pavilion (Faculty Office, Cafeteria), 9x8: two storeys, a pediment over the cornice, door at 4."""
    P = academic(9, 4, 2, doors=(4,), pediment_over=4)
    P.mirror()
    return P


def humanities():
    """Literature & Library, 22x9: one building, two doors (columns 5 and 16)."""
    # Windows three columns either side of each door (2, 8 and 13, 19: just outside the end dormers, and under the
    # middle ones), one in the dead center (between columns 10 and 11), and one above each door
    P = academic(22, 5, 2, doors=(5, 16), dormers=(3, 8, 13, 18), columns=(2, 5, 8, 10.5, 13, 16, 19))
    P.mirror()
    return P


def dorm():
    """The dorm, 17x10: the same stone and slate, square windows, a glass door under a slate canopy (column 8)."""
    P = academic(17, 4, 3, doors=(8,), dormers=(2, 5, 11, 14), kind='square', spacing=3)
    canopy(P, 8, 8)
    P.mirror()
    return P


def annex():
    """The dorm annex (the player's room), 11x8: two storeys, a wooden door (column 5)."""
    P = academic(11, 4, 2, doors=(5,), dormers=(2, 8), kind='square')
    door(P, 5 * T, 6 * T)
    P.mirror()
    return P


def canopy(P, col, row):
    x, y = col * T, row * T
    P.r(x - 8, y, T + 16, 5, SL2)
    P.hl(x - 8, x + T + 7, y, OL)
    P.hl(x - 8, x + T + 7, y + 5, OL)
    P.hl(x - 7, x + T + 6, y + 1, SL1)


def hospital():
    """The University Hospital, 13x10: a modern white block, flat roof with a red cross, three storeys of square
    windows, a glass door under a canopy (column 6), the cross over it."""
    w, h = 13, 10
    P = Pic(w * T, h * T)
    # Flat roof (rows 0-3)
    for y in range(4 * T):
        for x in range(P.w):
            P.p(x, y, S2 if (x % 8 == 7 and y % 8 == 1) or (x % 8 == 3 and y % 8 == 5) else S1)
    for y in range(4 * T):
        P.p(0, y, OL)
        P.p(1, y, S2)
        P.p(2, y, WHITE)
    for x in range(P.w):
        P.p(x, 0, OL)
        P.p(x, 1, S2)
        P.p(x, 2, WHITE)
        for k, c in enumerate((WHITE, S1, S2, S3, OL)):
            P.p(x, 4 * T - 5 + k, c)
    # The cross on the roof (accent palette: a white square, red arms)
    cx, cy = P.w // 2, 2 * T
    P.r(cx - 16, cy - 16, 16, 31, AC['w'])
    P.r(cx - 16, cy - 16, 16, 1, AC['s2'])
    P.vl(cx - 16, cy - 16, cy + 14, AC['s2'])
    P.r(cx - 5, cy - 12, 5, 23, AC['rd'])
    P.r(cx - 12, cy - 3, 12, 6, AC['rd'])
    P.vl(cx - 5, cy - 12, cy + 10, AC['rd2'])
    # Facade
    wall(P, 0, 4 * T, P.w, 6 * T)
    # Two windows either side of the door (and one over the cross sign on the top storey)
    facade(P, 0, 4 * T, w, 3, 'square', skip=[(6, 2), (6, 1)], columns=(2, 4, 6, 8, 10))
    door(P, 6 * T, 8 * T, glass=True)
    canopy(P, 6, 8)
    # The cross sign over the door
    sx, sy = 6 * T, 6 * T
    P.r(sx + 2, sy + 10, 6, 16, AC['w'])
    P.hl(sx + 2, sx + 7, sy + 9, AC['ol'])
    P.vl(sx + 2, sy + 9, sy + 26, AC['ol'])
    P.hl(sx + 2, sx + 7, sy + 26, AC['ol'])
    P.r(sx + 6, sy + 12, 2, 12, AC['rd'])
    P.r(sx + 3, sy + 16, 5, 4, AC['rd'])
    plinth(P, 0, h * T - 4, P.w)
    P.r(6 * T + 2, h * T - 4, 6, 3, S2)
    P.mirror()
    return P


LEAF = None


def leaves():
    """A 16x16 seamless leaf pattern (glass palette): None where the glass shows."""
    global LEAF
    if LEAF is None:
        P = Pic(16, 16)
        for (cx, cy, r) in ((4, 5, 4.5), (12, 3, 4), (9, 11, 5), (1, 13, 3.5), (17, 13, 3.5), (12, 19, 4),
                            (4, 21, 4.5), (20, 5, 4.5)):
            for y in range(16):
                for x in range(16):
                    for ox in (0, 16, -16):
                        for oy in (0, 16, -16):
                            dx, dy = x + .5 - cx - ox, y + .5 - cy - oy
                            if dx * dx + dy * dy <= r * r:
                                s = dx + dy
                                P.p(x, y, GL['p1'] if s < -r / 2 else GL['p3'] if s > r / 2 else GL['p2'])
        for (x, y, c) in ((11, 2, 'pk'), (4, 4, 'rd'), (9, 10, 'pk')):
            P.p(x, y, GL[c])
            P.p(x + 1, y, GL[c])
        LEAF = [[P.get(x, y) for x in range(16)] for y in range(16)]
    return LEAF


def greenhouse():
    """Home Economics: the glass greenhouse, 17x9: a vault of glass (6 rows) with rounded ends, leaves through the
    lower panes, a glass front on a low white wall, glass doors at column 8. Symmetric."""
    w, h = 17, 9
    P = Pic(w * T, h * T)
    L, rh, W, r = leaves(), 6 * T, w * T, 24
    for y in range(rh):
        for x in range(W // 2 + 8):
            if x < r and y < r and (x + .5 - r) ** 2 + (y + .5 - r) ** 2 > r * r:
                continue
            if x % 16 == 0:
                c = GL['f2']
            elif y % 16 == 0:
                c = GL['f1']
            elif y < 32:
                c = GL['q0'] if y < 16 else GL['q1']
            else:
                leaf = L[y % 16][x % 16]
                c = leaf if leaf is not None and y >= 48 else GL['q2'] if y >= 48 else GL['q1']
            P.p(x, y, c)
    for y in range(rh):  # The vault's outline
        xs = [x for x in range(W // 2) if P.get(x, y)]
        if xs:
            P.p(xs[0], y, GL['ol'])
            if xs[0] + 1 < W // 2:
                P.p(xs[0] + 1, y, GL['w'])
    for x in range(W // 2):
        ys = [y for y in range(rh) if P.get(x, y)]
        if ys:
            P.p(x, ys[0], GL['ol'])
    P.hl(0, W - 1, rh - 2, GL['f3'])
    P.hl(0, W - 1, rh - 1, GL['ol'])
    for y in range(rh, h * T):
        dy = y - rh
        for x in range(W // 2 + 8):
            if dy >= 3 * T - 8:
                c = (GL['w'], GL['s1'], GL['s1'], GL['s1'], GL['s1'], GL['f2'], GL['f2'], GL['ol'])[dy - (3 * T - 8)]
            elif x % 16 == 0 or dy % 16 == 0:
                c = GL['f3']
            elif x % 16 == 15 or dy % 16 == 1:
                c = GL['f1']
            else:
                leaf = L[dy % 16][x % 16]
                c = leaf if leaf is not None and dy >= 16 else GL['q1']
            P.p(x, y, c)
        P.p(0, y, GL['ol'])
    # The door (glass, framed) with a fanlight
    dx, dy = 8 * T, 7 * T
    P.r(dx, dy, T, 2 * T, GL['f3'])
    for y in range(dy + 3, dy + 14):
        for x in range(dx + 3, dx + 13):
            P.p(x, y, GL['w'] if x in (dx + 7, dx + 8) else GL['q0'])
    for y in range(dy + 16, dy + 31):
        for x in range(dx + 3, dx + 13):
            P.p(x, y, GL['f3'] if x in (dx + 7, dx + 8) else GL['q1'] if y < dy + 22 else GL['q2'])
    P.hl(dx, dx + 15, dy + 31, GL['f2'])
    P.ellipse(dx + 8, dy - 6, 4, 4, GL['pk'], GL['rd'])
    P.mirror()
    return P


def science():
    """The Science Research Institute, 13x13: a glass cylinder (columns 0-8, light from the left, a floor band every
    metatile) with a white disc roof, beside a steel lattice tower (columns 9-12) with a beacon; a white podium and
    a glass door at column 6."""
    w, h = 13, 13
    P = Pic(w * T, h * T)
    shades = ('q3', 'q1', 'q0', 'q0', 'q1', 'q1', 'q2', 'q2', 'q3')
    for y in range(T, 12 * T):
        for x in range(9 * T):
            col = x // 16
            if x == 0 or x == 9 * T - 1:
                c = GL['ol']
            elif x % 16 == 0:
                c = GL['f2']
            elif y % 16 == 0:
                c = GL['f2']
            elif y % 16 == 1:
                c = GL['f1']
            else:
                c = GL[shades[col]]
            P.p(x, y, c)
    # The roof: a white disc (symmetric over the cylinder)
    cap = Pic(9 * T, 2 * T)
    cap.ellipse(72, 18, 72, 14, GL['s1'], GL['ol'])
    cap.ellipse(72, 18, 50, 9, GL['w'], GL['f1'])
    cap.ellipse(72, 18, 12, 4, GL['f2'])
    cap.mirror()
    P.paste(cap, 0, 0)
    # The lattice tower: an X in every metatile over a dark core; a mast and beacon on top
    for y in range(T, 12 * T):
        for x in range(9 * T, 13 * T):
            lx, ly = x % 16, y % 16
            if lx in (0, 15) or ly == 0:
                c = GL['f3']
            elif abs(lx - ly) <= 0 or abs(lx + ly - 15) <= 0:
                c = GL['f1']
            elif abs(lx - ly) == 1 or abs(lx + ly - 15) == 1:
                c = GL['f2']
            else:
                c = GL['q3']
            P.p(x, y, c)
    P.vl(13 * T - 1, T, 12 * T, GL['ol'])
    for y in range(0, T):
        P.p(11 * T - 1, y, GL['f3'])
        P.p(11 * T, y, GL['f3'])
    P.r(11 * T - 2, 1, 4, 3, GL['rd'])
    P.hl(9 * T + 2, 13 * T - 3, T - 1, GL['f3'])
    # Podium (row 12) and the door
    for y in range(12 * T, 13 * T):
        for x in range(P.w):
            dy = y - 12 * T
            P.p(x, y, GL['w'] if dy < 2 else GL['ol'] if dy == 15 else GL['s1'] if dy % 8 != 7 else GL['f1'])
        P.p(0, y, GL['ol'])
        P.p(P.w - 1, y, GL['ol'])
    gx, gy = 6 * T, 11 * T
    P.r(gx + 1, gy + 2, 14, 30, GL['f3'])
    for y in range(gy + 4, gy + 31):
        for x in range(gx + 3, gx + 13):
            P.p(x, y, GL['f3'] if x in (gx + 7, gx + 8) else GL['q0'] if y < gy + 14 else GL['q1'])
    P.hl(gx + 1, gx + 14, gy + 31, GL['f2'])
    return P


def stadium():
    """The stadium, 22x12: stands with blue seats along the top (rows 0-2) with two floodlights, an oval running
    track (rows 3-11) around a grass infield (transparent: the primary's grass), two more floodlights at the bottom
    corners. Symmetric both ways (the track); accent palette."""
    w, h = 22, 12
    P = Pic(w * T, h * T)
    # Stands
    for y in range(3 * T):
        for x in range(P.w // 2):
            if y < 4:
                c = (AC['ol'], AC['s2'], AC['w'], AC['s3'])[y]
            elif y >= 3 * T - 2:
                c = AC['ol'] if y == 3 * T - 1 else AC['s3']
            else:
                tier = (y - 4) % 8
                c = AC['s1'] if tier == 0 else AC['b2'] if tier == 7 else AC['b2'] if x % 4 == 0 else AC['b1']
                if x % 64 in (30, 31, 32, 33):
                    c = AC['s1'] if tier < 4 else AC['s2']
            P.p(x, y, c)
        P.p(0, y, AC['ol'])
    floodlight(P, 1 * T, 0)
    # Track: a rounded rectangle ring, 24 px wide, two lane lines
    x0, y0, x1, y1, rad = 8, 3 * T + 4, P.w - 8, h * T - 4, 52
    for y in range(3 * T, h * T):
        for x in range(P.w // 2):
            px, py = x + .5, y + .5
            cxr = min(max(px, x0 + rad), x1 - rad)
            cyr = min(max(py, y0 + rad), y1 - rad)
            d = rad - math.hypot(px - cxr, py - cyr)   # Distance inside the outer edge
            if 0 <= d < 26:
                if d < 1.2 or d >= 24.8:
                    c = AC['ol'] if d < 1.2 else AC['t2']
                elif abs(d - 9) < .6 or abs(d - 17) < .6:
                    c = AC['w']
                elif d < 3:
                    c = AC['t2']
                else:
                    c = AC['t1']
                P.p(x, y, c)
    floodlight(P, 0, 9 * T)
    P.mirror()
    return P


def floodlight(P, x, y):
    """A floodlight tower, 1x3: a steel pole, a panel of lamps on top."""
    P.r(x + 1, y + 1, 14, 9, AC['ol'])
    for ly in (y + 2, y + 5):
        for lx in range(x + 2, x + 14, 3):
            P.r(lx, ly, 2, 2, AC['w'])
    P.hl(x + 1, x + 14, y + 8, AC['mt'])
    P.r(x + 7, y + 10, 2, 36, AC['mt'])
    P.vl(x + 6, y + 10, y + 45, AC['ol'])
    P.vl(x + 9, y + 10, y + 45, AC['ol'])
    P.r(x + 4, y + 44, 8, 3, AC['s3'])
    P.hl(x + 4, x + 11, y + 47, AC['ol'])


def pool():
    """The outdoor pool, 9x8: a tiled deck, the water with two lane ropes (water palette). Symmetric."""
    P = Pic(9 * T, 8 * T)
    for y in range(P.h):
        for x in range(P.w // 2 + 8):
            P.p(x, y, WA['s2'] if x % 8 == 0 or y % 8 == 0 else WA['s1'])
    for y in range(T, 7 * T):
        for x in range(T, P.w // 2 + 8):
            if y < T + 3:
                c = WA['a3'] if y < T + 2 else WA['a2']
            else:
                c = WA['a1'] if (y % 16 == 8 and x % 16 in (4, 5, 6)) or (y % 16 == 0 and x % 16 in (11, 12)) \
                    else WA['a2']
            P.p(x, y, c)
        P.p(T, y, WA['s3'])
    for y in range(T + 3, 7 * T):
        for x in (2 * T + 8, 2 * T + 9):
            P.p(x, y, WA['rd'] if (y // 3) % 2 else WA['w'])
    P.hl(0, P.w - 1, 0, WA['ol'])
    P.hl(0, P.w - 1, P.h - 1, WA['ol'])
    P.vl(0, 0, P.h - 1, WA['ol'])
    P.mirror()
    return P


def barn():
    """The ranch's barn, 5x4: the roof kit over white walls, a wooden barn door (column 2). Symmetric."""
    P = Pic(5 * T, 4 * T)
    roof(P, 0, 0, P.w, 2 * T)
    wall(P, 0, 2 * T, P.w, 2 * T)
    x, y = 2 * T, 2 * T
    P.r(x + 1, y + 3, 14, 29, WD)
    P.r(x, y + 2, 16, 1, OL)
    P.vl(x, y + 2, y + 31, OL)
    P.vl(x + 15, y + 2, y + 31, OL)
    for k in range(26):
        P.p(x + 2 + k * 12 // 26, y + 4 + k, WD2)
        P.p(x + 13 - k * 12 // 26, y + 4 + k, WD2)
    P.vl(x + 7, y + 3, y + 31, WD2)
    P.vl(x + 8, y + 3, y + 31, WD2)
    sqwindow(P, 0, 2 * T - 4)
    P.mirror()
    return P


def gate():
    """The gate seen from the campus, 6x5: two stone pillars with gold lamps either side of the passage
    (columns 2-3 of row 0, the warp tiles), the gatehouse's slate roof below them. Symmetric."""
    P = Pic(6 * T, 5 * T)
    roof(P, 0, T, P.w, 4 * T)
    wall(P, 0, 0, 2 * T, T)
    P.hl(0, 2 * T - 1, 0, OL)
    P.hl(0, 2 * T - 1, 1, WHITE)
    P.vl(2 * T - 1, 0, T - 1, OL)
    P.ellipse(T, 8, 4, 4, AU, AU2)
    for y in range(T):
        for x in range(2 * T, 3 * T):
            P.p(x, y, OL if x == 2 * T else SL3 if y < 6 else SL2 if y < 12 else SL1)
        P.p(2 * T + 1, y, WHITE)
    P.mirror()
    return P


def fountain():
    """The Gyarados fountain, 5x4: an oval white basin, Gyarados rising straight up from the pedestal, the jet
    falling both ways (the noon signal). Symmetric (water palette)."""
    P = Pic(5 * T, 4 * T)
    cx = 40
    P.ellipse(cx, 42, 38, 19, WA['s2'], WA['ol'])
    P.ellipse(cx, 38, 38, 19, WA['w'], WA['ol'])
    P.ellipse(cx, 38, 34, 16, WA['s1'])
    P.ellipse(cx, 39, 31, 13, WA['a2'], WA['s3'])
    for y in range(P.h):
        for x in range(P.w):
            if P.get(x, y) == WA['a2']:
                ring = math.hypot((x + .5 - cx) / 31, (y + .5 - 39) / 13)
                if abs(ring * 6 % 2 - 1) < .16:
                    P.p(x, y, WA['a1'])
    P.ellipse(cx, 42, 8, 4, WA['s2'], WA['ol'])
    P.ellipse(cx, 40, 8, 4, WA['w'], WA['ol'])
    # Gyarados: the body rising from the pedestal, fins either side, the head with an open mouth
    for y in range(14, 41):
        sway = int(round(2 * math.sin((y - 14) / 5)))
        for x in range(cx - 5 + sway, cx + 5 + sway):
            edge = x in (cx - 5 + sway, cx + 4 + sway)
            belly = x in (cx - 1 + sway, cx + sway)
            P.p(x, y, WA['ol'] if edge else WA['cr'] if belly else WA['b1'] if x < cx + sway else WA['b2'])
    for (fy, fx) in ((30, -8), (22, -9)):
        for k in range(4):
            P.hl(cx + fx + k, cx + fx + 3, fy + k, WA['rd'])
    P.ellipse(cx, 11, 8, 6, WA['b1'], WA['ol'])
    P.ellipse(cx, 13, 4, 2, WA['w'], WA['ol'])
    P.p(cx - 5, 9, WA['ol'])
    P.p(cx - 6, 8, WA['w'])
    for k in range(4):
        P.p(cx - 3 - k, 4 - k // 2, WA['cr'])
    # The jet: up and falling into the basin on both sides
    P.vl(cx - 1, 0, 8, WA['w'])
    for k in range(26):
        t = k / 25
        P.p(int(cx - 2 - t * 26), int(1 + (t * 1.9 - .25) ** 2 * 16), WA['a1'] if k % 3 else WA['w'])
    P.mirror()
    return P


def reflecting_pool():
    """A long reflecting pool, 4x11: white rim, still water, a highlight now and then. Symmetric."""
    P = Pic(4 * T, 11 * T)
    P.r(0, 0, P.w, P.h, WA['w'])
    for y in range(4, P.h - 4):
        for x in range(4, P.w - 4):
            c = WA['a3'] if y < 6 else WA['a1'] if y % 16 == 8 and x % 16 in (6, 7, 8, 9) else WA['a2']
            P.p(x, y, c)
    P.hl(0, P.w - 1, 0, WA['ol'])
    P.hl(0, P.w - 1, P.h - 1, WA['ol'])
    P.hl(1, P.w - 2, P.h - 2, WA['s2'])
    P.vl(0, 0, P.h - 1, WA['ol'])
    P.hl(3, P.w - 4, 3, WA['s3'])
    P.vl(3, 3, P.h - 5, WA['s3'])
    P.mirror()
    return P


def lamp():
    """A lamp post, 1x2 (symmetric)."""
    P = Pic(T, 2 * T)
    P.r(6, 10, 4, 20, SL3)
    P.vl(6, 10, 29, OL)
    P.r(4, 28, 8, 3, SL3)
    P.hl(4, 11, 31, OL)
    P.r(4, 2, 8, 7, G1)
    P.vl(4, 2, 8, OL)
    P.vl(7, 2, 8, WHITE)
    P.hl(3, 12, 1, AU2)
    P.hl(5, 10, 0, AU)
    P.hl(3, 12, 9, AU2)
    P.mirror()
    return P


def bench():
    """A park bench, 2x1 (symmetric)."""
    P = Pic(2 * T, T)
    for (y, c) in ((3, OL), (4, WD), (5, WD2), (7, OL), (8, WD), (9, WD), (10, WD), (11, WD2)):
        P.hl(2, 29, y, c)
    P.r(4, 12, 2, 3, SL3)
    P.mirror()
    return P


# --- Exterior stamps: name -> (picture, door cells, warp cells, cells in the top layer) --------------------------------
def exterior_stamps():
    return {
        'MAIN_HALL': (main_hall(), [(7, 15)], [], [(c, r) for r in range(5) for c in range(5, 10)]),
        'LECTURE_WING': (wing(), [(5, 8)], [], []),
        'PAVILION': (pavilion(), [(4, 7)], [], []),
        'HUMANITIES': (humanities(), [(5, 8), (16, 8)], [], []),
        'GREENHOUSE': (greenhouse(), [(8, 8)], [], []),
        'SCIENCE': (science(), [(6, 12)], [], []),
        'HOSPITAL': (hospital(), [(6, 9)], [], []),
        'DORM': (dorm(), [(8, 9)], [], []),
        'ANNEX': (annex(), [(5, 7)], [], []),
        'STADIUM': (stadium(), [], [], []),
        'POOL': (pool(), [], [], []),
        'BARN': (barn(), [], [], []),
        'GATE': (gate(), [], [(2, 0), (3, 0)], []),
        'FOUNTAIN': (fountain(), [], [], [], 'paving'),
        'REFLECTING_POOL': (reflecting_pool(), [], [], []),
        'LAMP': (lamp(), [], [], []),
        'BENCH': (bench(), [], [], [], 'paving'),
    }


def plaque(P, x, y, glass=False):
    """A name plaque on a building's wall, in the bottom row beside a door (symmetric): a brass plate on stone, a
    steel one on the glass buildings' white base."""
    if glass:
        frame, plate, line = GL['f3'], GL['f1'], GL['f2']
    else:
        frame, plate, line = AU2, AU, AU2
    P.r(x + 3, y + 2, 10, 8, frame)
    P.r(x + 4, y + 3, 8, 6, plate)
    P.hl(x + 5, x + 10, y + 5, line)
    P.hl(x + 6, x + 9, y + 7, line)
    P.hl(x + 3, x + 12, y + 10, S3 if not glass else GL['f2'])


def plain_wall_cell():
    """The pixels of a plain stone wall cell in a facade's bottom row (no window), to find where plaques can go."""
    P = Pic(3 * T, 2 * T)
    wall(P, 0, 0, P.w, P.h)
    plinth(P, 0, P.h - 4, P.w)
    return P.im.crop((T, T, 2 * T, 2 * T)).tobytes()


# Plaques: stamp -> the sides of each door that get one (the Literature & Library building: the outer sides only).
# The glass buildings' plaques go on their white base, beside the door.
PLAQUE_SIDES = {'HUMANITIES': [(-1,), (1,)]}
GLASS_PLAQUES = {'GREENHOUSE': [(7, 8), (9, 8)], 'SCIENCE': [(5, 12), (7, 12)]}


def add_plaques(stamps):
    """Draws the plaques onto the door stamps; returns {stamp: [(col, row, door index)]}."""
    plain = plain_wall_cell()
    out = {}
    for name, (pic, doors, *_rest) in stamps.items():
        if not doors:
            continue
        cells = []
        if name in GLASS_PLAQUES:
            for (c, r) in GLASS_PLAQUES[name]:
                plaque(pic, c * T, r * T, glass=True)
                cells.append((c, r, 0))
        else:
            for k, (dc, dr) in enumerate(doors):
                for side in (PLAQUE_SIDES.get(name, [(-1, 1)] * len(doors))[k]):
                    c = dc + side
                    while 0 < c < pic.w // T - 1:
                        if pic.im.crop((c * T, dr * T, c * T + T, dr * T + T)).tobytes() == plain:
                            break
                        c += side
                    assert 0 < c < pic.w // T - 1, 'no wall for a plaque on %s' % name
                    plaque(pic, c * T, dr * T)
                    cells.append((c, dr, k))
        out[name] = cells
    return out


# --- Tileset writing -------------------------------------------------------------------------------------------------
def behaviors():
    """MB_* name -> value, from the enum (counting entries as the compiler does)."""
    src = open(os.path.join(ROOT, 'include/constants/metatile_behaviors.h')).read()
    body = src[src.index('enum'):]
    body = re.sub(r'//[^\n]*|/\*.*?\*/', '', body, flags=re.S)
    out, n = {}, -1
    for m in re.finditer(r'\b(MB_\w+)\s*(?:=\s*([^,}]+))?\s*[,}]', body):
        n = eval(m.group(2).strip(), {}, dict(out)) if m.group(2) else n + 1
        out[m.group(1)] = n
    return out


def behavior_of(symbol, metatile):
    folder = R.tileset_folder(symbol)
    data = open(os.path.join(folder, 'metatile_attributes.bin'), 'rb').read()
    return struct.unpack_from('<I', data, (metatile - (PRIMARY if metatile >= PRIMARY else 0)) * 4)[0] & 0x1FF


def tile_pixels(im, x, y):
    px = im.load()
    return tuple(tuple(px[x + i, y + j][:3] if px[x + i, y + j][3] else None for i in range(8)) for j in range(8))


class Tileset:
    """Collects 8x8 tiles (deduplicated with flips, each in one of the palettes) and metatiles."""

    def __init__(self, symbol, folder, pals, primary, under):
        self.symbol, self.folder, self.primary = symbol, folder, primary
        self.pals = [(name, PAL[name]) for name in pals]
        self.under = under                 # The 4 entries of the bottom layer under transparent art (or None)
        self.tiles, self.lookup = [], {}
        self.metatiles, self.attrs, self.mlookup = [], [], {}
        self.off_palette = 0

    def tile(self, pixels):
        """The metatile entry for an 8x8 RGB/None tile."""
        cols = {c for row in pixels for c in row if c is not None}
        if not cols:
            return 0
        fits = [i for i, (_, pal) in enumerate(self.pals) if cols <= set(pal)]
        if not fits:
            self.off_palette += 1
            fits = [0]
            pal = self.pals[0][1]
            pixels = tuple(tuple(None if c is None else min(pal, key=lambda p: sum((a - b) ** 2 for a, b in zip(p, c)))
                                 for c in row) for row in pixels)
        p = fits[0]
        pal = self.pals[p][1]
        idx = tuple(tuple(0 if c is None else pal.index(c) + 1 for c in row) for row in pixels)
        for hf in (0, 1):
            for vf in (0, 1):
                f = tuple(tuple(r[::-1]) if hf else r for r in (idx[::-1] if vf else idx))
                if f in self.lookup:
                    return (PRIMARY + self.lookup[f]) | hf << 10 | vf << 11 | (FIRST_PAL + p) << 12
        self.lookup[idx] = len(self.tiles)
        self.tiles.append(idx)
        return (PRIMARY + len(self.tiles) - 1) | (FIRST_PAL + p) << 12

    def raw_tile(self, idx):
        """The entry (no palette bits) for a tile given as rows of color indices (another tileset's palette)."""
        for hf in (0, 1):
            for vf in (0, 1):
                f = tuple(tuple(r[::-1]) if hf else r for r in (idx[::-1] if vf else idx))
                if f in self.lookup:
                    return (PRIMARY + self.lookup[f]) | hf << 10 | vf << 11
        self.lookup[idx] = len(self.tiles)
        self.tiles.append(idx)
        return PRIMARY + len(self.tiles) - 1

    def quarter(self, im, x, y):
        return [self.tile(tile_pixels(im, x + dx, y + dy)) for dy in (0, 8) for dx in (0, 8)]

    def metatile(self, bottom, middle, behavior=0, layer=LAYER_COVERED):
        entries = tuple(bottom) + tuple(middle)
        attr = behavior | layer << 29
        key = (entries, attr)
        if key not in self.mlookup:
            self.mlookup[key] = PRIMARY + len(self.metatiles)
            self.metatiles.append(entries)
            self.attrs.append(attr)
        return self.mlookup[key]

    def write(self):
        os.makedirs(os.path.join(self.folder, 'palettes'), exist_ok=True)
        rows_n = (len(self.tiles) + 15) // 16
        image = [[0] * 128 for _ in range(rows_n * 8)]
        for n, px in enumerate(self.tiles):
            for y in range(8):
                image[(n // 16) * 8 + y][(n % 16) * 8:(n % 16) * 8 + 8] = px[y]
        grey = b''.join(bytes((v * 17, v * 17, v * 17)) for v in range(16))
        write_png(os.path.join(self.folder, 'tiles.png'), 128, image, grey)
        for i in range(16):
            slot = i - FIRST_PAL
            cols = [(0, 0, 0)] + (list(self.pals[slot][1]) if 0 <= slot < len(self.pals) else [])
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
        return '%s: %d/%d tiles, %d/%d metatiles, %d palettes (%s), %d tiles off-palette' % (
            self.symbol, len(self.tiles), MAX_TILES, len(self.metatiles), MAX_METATILES, len(self.pals),
            ', '.join(n for n, _ in self.pals), self.off_palette)

    # Rendering what was written (the mockups are drawn from this, so they show the real tileset)
    def render_entry(self, e, img, ox, oy, skip_zero):
        t, hf, vf, p = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
        if t >= PRIMARY:
            idx = self.tiles[t - PRIMARY]
            pal = [None] + list(self.pals[p - FIRST_PAL][1] if p >= FIRST_PAL else
                                primary_tileset(self.primary)[1][p][1:])
        else:
            ptiles, ppals, _ = primary_tileset(self.primary)
            n = len(ptiles[0]) // 8
            idx = [[ptiles[(t // n) * 8 + y][(t % n) * 8 + x] for x in range(8)] for y in range(8)]
            pal = [None] + list(ppals[p][1:])
        px = img.load()
        for y in range(8):
            for x in range(8):
                v = idx[7 - y if vf else y][7 - x if hf else x]
                if v == 0 and skip_zero:
                    continue
                c = pal[v] if v else primary_tileset(self.primary)[1][0][0]
                px[ox + x, oy + y] = tuple(c) + (255,)

    def render(self, mid):
        img = Image.new('RGBA', (16, 16), (0, 0, 0, 255))
        entries = self.metatiles[mid - PRIMARY]
        for layer in (0, 1):
            for k in range(4):
                self.render_entry(entries[layer * 4 + k], img, (k % 2) * 8, (k // 2) * 8, layer == 1)
        return img


_pts = {}


def primary_tileset(symbol):
    if symbol not in _pts:
        _pts[symbol] = R.load_tileset(R.tileset_folder(symbol), symbol)
    return _pts[symbol]


def primary_entries(symbol, mid, layer=0):
    data = open(os.path.join(R.tileset_folder(symbol), 'metatiles.bin'), 'rb').read()
    return list(struct.unpack_from('<8H', data, mid * 16))[layer * 4:layer * 4 + 4]


# Walkable cells of a stamp (the rest is solid): the path behind the clock tower, the stadium's track, the pool's
# deck, the gate's passage
WALKABLE = {
    'MAIN_HALL': lambda c, r: r < 5,
    'STADIUM': lambda c, r: r >= 3 and 1 <= c <= 20,
    'POOL': lambda c, r: not (1 <= c <= 7 and 1 <= r <= 6),
    'GATE': lambda c, r: r == 0 and c in (2, 3),
}


def cut_exterior(ts, stamps, MB):
    """Each stamp -> rows of metatile ids (None where the art is empty) and its collision."""
    out = {}
    paving = primary_entries('gTileset_General_Frlg', 0x16E)
    for name, (pic, doors, warps, above, *ground) in stamps.items():
        under = paving if ground == ['paving'] else ts.under
        rows, solid = [], []
        walkable = WALKABLE.get(name, lambda c, r: False)
        for r in range(pic.h // T):
            row = []
            solid.append([0 if walkable(c, r) else 1 for c in range(pic.w // T)])
            for c in range(pic.w // T):
                cell = pic.im.crop((c * T, r * T, c * T + T, r * T + T))
                alphas = [a for (_, _, _, a) in cell.getdata()]
                if not any(alphas):
                    row.append(None)
                    solid[-1][c] = 0
                    continue
                art = ts.quarter(pic.im, c * T, r * T)
                beh = MB['MB_ANIMATED_DOOR'] if (c, r) in doors else MB['MB_SOUTH_ARROW_WARP'] if (c, r) in warps \
                    else MB['MB_NORMAL']
                if (c, r) in above:
                    row.append(ts.metatile(under, art, beh, LAYER_NORMAL))
                else:
                    row.append(ts.metatile(under, art, beh, LAYER_COVERED))
            rows.append(row)
        out[name] = {'w': pic.w // T, 'h': pic.h // T, 'rows': rows, 'solid': solid, 'doors': doors, 'warps': warps}
    return out


# --- Paving edges ----------------------------------------------------------------------------------------------------
# The primary's paving (palette 1): its nine metatiles by the states of their quarters (TL, TR, BL, BR). A quarter is
# 'outer' (grass on both sides it touches), 'horiz' (grass above/below), 'vert' (grass beside), 'plain', or 'inner'
# (both sides paved, the diagonal not: where two paths meet).
PRIMARY_PAVING = {
    ('outer', 'horiz', 'vert', 'plain'): 0x165, ('horiz', 'horiz', 'plain', 'plain'): 0x166,
    ('horiz', 'outer', 'plain', 'vert'): 0x167, ('vert', 'plain', 'vert', 'plain'): 0x16D,
    ('plain', 'plain', 'plain', 'plain'): 0x16E, ('plain', 'vert', 'plain', 'vert'): 0x16F,
    ('vert', 'plain', 'outer', 'horiz'): 0x175, ('plain', 'plain', 'horiz', 'horiz'): 0x176,
    ('plain', 'vert', 'horiz', 'outer'): 0x177,
}
PAVE_PAL = 1 << 12
HF = 1 << 10
# The primary's quarter tiles for each corner of a cell (with the flips its metatiles use); the inner corners are drawn
PAVE_QUARTERS = {
    0: {'outer': 0x132, 'horiz': 0x133, 'vert': 0x152, 'plain': 0x143},
    1: {'outer': 0x132 | HF, 'horiz': 0x133, 'vert': 0x152 | HF, 'plain': 0x153},
    2: {'outer': 0x162, 'horiz': 0x163, 'vert': 0x152, 'plain': 0x153},
    3: {'outer': 0x162 | HF, 'horiz': 0x163, 'vert': 0x152 | HF, 'plain': 0x143},
}
# Inner corners (palette 1 indices): the grass strips of the two paved neighbours meet in a 2x2 corner of grass, their
# dotted lines turn it. Top-left (the diagonal above-left is lawn); bottom-left (below-left). Mirrored for the right.
INNER_TL = ('f73' + '1' * 5, '773' + '1' * 5, '33' + '1' * 6) + ('1' * 8,) * 5
INNER_BL = ('1' * 8,) * 5 + ('33' + '1' * 6, '773' + '1' * 5, 'f73' + '1' * 5)


def paving_entries(ts, states):
    """The four bottom-layer entries of a paved cell with these quarter states (the primary's own where it has
    that metatile)."""
    if states in PRIMARY_PAVING:
        return primary_entries('gTileset_General_Frlg', PRIMARY_PAVING[states])
    inner = {}
    for corner, rows in ((0, INNER_TL), (2, INNER_BL)):
        e = ts.raw_tile(tuple(tuple(int(c, 16) for c in r) for r in rows)) | PAVE_PAL
        inner[corner], inner[corner + 1] = e, e ^ HF
    return [inner[k] if s == 'inner' else PAVE_QUARTERS[k][s] | PAVE_PAL for k, s in enumerate(states)]


def paving_metatiles(ts, needed):
    """Campus tileset metatiles for the paving mixes in `needed` (quarter states): {states: id}."""
    return {states: ts.metatile(paving_entries(ts, states), [0, 0, 0, 0], 0, LAYER_NORMAL)
            for states in sorted(needed)}


# --- Ground (the primary's metatiles) --------------------------------------------------------------------------------
class Ground:
    """A grid of metatile ids: grass, forest, lone trees, paving with edges, flower beds, fences; stamps on top."""
    EDGES = {
        (False, True, False, True): 0x165, (False, True, True, True): 0x166, (False, True, True, False): 0x167,
        (True, True, False, True): 0x16D, (True, True, True, True): 0x16E, (True, True, True, False): 0x16F,
        (True, False, False, True): 0x175, (True, False, True, True): 0x176, (True, False, True, False): 0x177,
    }
    TREE_IDS = (0x00E, 0x00F, 0x01E, 0x01F, 0x026, 0x027, 0x01C, 0x01D, 0x014, 0x015)

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.g = [[((0x009, 0x008), (0x011, 0x010))[y % 2][x % 2] for x in range(w)] for y in range(h)]
        self.paved, self.used = set(), set()

    def set(self, x, y, m):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.g[y][x] = m
            self.used.add((x, y))

    def forest(self, x0, y0, x1, y1):
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.set(x, y, ((0x01C, 0x01D), (0x014, 0x015))[y % 2][x % 2])

    def path(self, x0, y0, x1, y1):
        for y in range(y0, y1):
            for x in range(x0, x1):
                self.paved.add((x, y))
                self.used.add((x, y))

    def lay(self, pave_ids=None):
        """Paving with its edges, quarter by quarter: each 8x8 quarter of a paved cell looks at the two cells it
        touches and the diagonal between them (outer corner, edge, plain, or an inner corner where two paths meet),
        so junctions, 1-wide stretches and dead ends get their dotted edge lines too. A cell whose quarters match one
        of the primary's nine paving metatiles uses it; any other mix is one of the campus tileset's paving metatiles
        (`pave_ids`: quarter states -> id). Mixes not in `pave_ids` are collected in `self.pave_needed` (drawn as
        plain paving meanwhile)."""
        self.pave_needed, self.sign_needed, self.pave_states = set(), set(), {}
        p = self.paved
        for (x, y) in p:
            def q(dy, dx):
                v, h, d = (x, y + dy) in p, (x + dx, y) in p, (x + dx, y + dy) in p
                return 'outer' if not v and not h else 'horiz' if not v else 'vert' if not h else \
                    'plain' if d else 'inner'
            states = (q(-1, -1), q(-1, 1), q(1, -1), q(1, 1))
            self.pave_states[(x, y)] = states
            if states in PRIMARY_PAVING:
                self.g[y][x] = PRIMARY_PAVING[states]
            elif pave_ids is not None and states in pave_ids:
                self.g[y][x] = pave_ids[states]
            else:
                self.pave_needed.add(states)
                self.g[y][x] = 0x16E

    def fill_notches(self):
        """Paves any free lawn cell with paving on three sides or more (a notch the paths left), until none is left."""
        changed = True
        while changed:
            changed = False
            for y in range(self.h):
                for x in range(self.w):
                    if (x, y) in self.used:
                        continue
                    sides = sum((x + dx, y + dy) in self.paved for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                    if sides >= 3:
                        self.path(x, y, x + 1, y + 1)
                        changed = True

    def block(self, x0, y0, w, h):
        for y in range(y0, y0 + h):
            for x in range(x0, x0 + w):
                self.used.add((x, y))

    def stamp(self, st, x0, y0):
        for r, row in enumerate(st['rows']):
            for c, mid in enumerate(row):
                if mid is not None:
                    self.set(x0 + c, y0 + r, mid)
        self.block(x0, y0, st['w'], st['h'])

    def fence(self, x0, y0, x1, y1, gap=(0, 0)):
        for x in range(x0 + 1, x1 - 1):
            self.set(x, y0, 0x0E7)
            if not gap[0] <= x < gap[1]:
                self.set(x, y1 - 1, 0x0E7)
        for y in range(y0 + 1, y1 - 1):
            self.set(x0, y, 0x0F4)
            self.set(x1 - 1, y, 0x0F5)
        self.set(x0, y0, 0x0EC)
        self.set(x1 - 1, y0, 0x0ED)
        self.set(x0, y1 - 1, 0x0F4)       # (Celadon's own bottom corners are in its secondary tileset)
        self.set(x1 - 1, y1 - 1, 0x0F5)
        if gap != (0, 0):  # A post on each side of the gap, so the rails don't stop mid-air
            self.set(gap[0] - 1, y1 - 1, 0x0ED)
            self.set(gap[1], y1 - 1, 0x0EC)
        self.block(x0, y0, x1 - x0, y1 - y0)

    def tree(self, x, y):
        for (dx, dy, m) in ((0, 0, 0x00E), (1, 0, 0x00F), (0, 1, 0x01E), (1, 1, 0x01F), (0, 2, 0x026), (1, 2, 0x027)):
            self.set(x + dx, y + dy, m)

    def grove(self, x0, y0, x1, y1):
        for y in range(y0, y1 - 2):
            for x in range(x0, x1 - 1):
                if all((x + dx, y + dy) not in self.used for dx in range(2) for dy in range(3)) and \
                   all((x + dx, y + dy) not in self.used or self.g[y + dy][x + dx] in self.TREE_IDS
                       for dx in range(-1, 3) for dy in range(-1, 4)
                       if 0 <= x + dx < self.w and 0 <= y + dy < self.h):
                    self.tree(x, y)

    def flowers(self, x0, x1, y):
        for x in range(x0, x1):
            if (x, y) not in self.used:
                self.set(x, y, 0x004)

    def render(self, ts):
        img = Image.new('RGBA', (self.w * T, self.h * T))
        for y in range(self.h):
            for x in range(self.w):
                m = self.g[y][x]
                img.paste(ts.render(m) if m >= PRIMARY else primary_metatile(m), (x * T, y * T))
        return img


_pm = {}


def primary_metatile(mid, primary='gTileset_General_Frlg', secondary='gTileset_CeladonCity'):
    """A metatile of the primary (or Celadon's, for the fence corners) as the game draws it."""
    key = (mid, primary)
    if key not in _pm:
        ptiles, ppals, pmeta = primary_tileset(primary)
        stiles, spals, smeta = primary_tileset(secondary)
        pals = [ppals[i] if i < 7 else spals[i] for i in range(16)]
        entries = pmeta[mid] if mid < PRIMARY else smeta[mid - PRIMARY]
        img = Image.new('RGBA', (16, 16), pals[0][0] + (255,))
        px = img.load()
        for layer in (0, 1):
            for k in range(4):
                e = entries[layer * 4 + k]
                t = e & 0x3FF
                rows, t = (ptiles, t) if t < PRIMARY else (stiles, t - PRIMARY)
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
        _pm[key] = img
    return _pm[key]


# --- Sprites -----------------------------------------------------------------------------------------------------------
def sprite(name, frame=0, w=16, h=32):
    im = Image.open(os.path.join(ROOT, 'graphics', name + '.png'))
    im = im.crop((frame * w, 0, frame * w + w, h))
    pal, src = im.getpalette(), im.load()
    out = Image.new('RGBA', im.size)
    px = out.load()
    for y in range(h):
        for x in range(w):
            i = src[x, y]
            if i:
                px[x, y] = (pal[3 * i], pal[3 * i + 1], pal[3 * i + 2], 255)
    return out


def put(img, name, tx, ty, frame=0):
    """A person (16x32) or a Pokémon (32x32, pokemon/<species>) standing on tile (tx, ty)."""
    if name.startswith('pokemon/'):
        if os.path.exists(os.path.join(ROOT, 'graphics', name, 'overworld.png')):
            img.alpha_composite(sprite(name + '/overworld', frame, 32, 32), (tx * T - 8, ty * T - 16))
    else:
        img.alpha_composite(sprite('object_events/pics/' + name, frame), (tx * T, ty * T - 16))


PLAYER = 'people/red/red_normal'
DOWN, UP, LEFT = 0, 1, 2


# --- The campus (76x64) ------------------------------------------------------------------------------------------------
CAMPUS_W, CAMPUS_H = 76, 64
# Stamp name, top-left; the Main Hall's stamp starts 5 rows over its footprint (the tower)
CAMPUS = [
    ('STADIUM', 4, 4), ('HUMANITIES', 4, 17), ('GREENHOUSE', 29, 8), ('BARN', 50, 6), ('SCIENCE', 58, 8),
    ('PAVILION', 11, 28), ('LECTURE_WING', 20, 27), ('MAIN_HALL', 31, 20), ('LECTURE_WING', 46, 27),
    ('PAVILION', 57, 28), ('FOUNTAIN', 36, 39), ('POOL', 4, 41), ('HOSPITAL', 14, 40), ('REFLECTING_POOL', 32, 47),
    ('REFLECTING_POOL', 41, 47), ('DORM', 48, 38), ('ANNEX', 51, 50), ('GATE', 35, 59),
    ('LAMP', 31, 38), ('LAMP', 45, 38), ('LAMP', 8, 38), ('LAMP', 68, 38),
    ('BENCH', 33, 43), ('BENCH', 42, 43),
]
CAMPUS_PATHS = [
    (4, 36, 72, 38),     # The street in front of the central building
    (32, 38, 45, 46),    # The fountain square
    (36, 46, 41, 59),    # The avenue down to the gate
    (27, 46, 36, 47), (27, 46, 29, 52), (13, 50, 29, 52),   # To the hospital's front
    (41, 46, 47, 47), (45, 46, 47, 59), (45, 48, 66, 50), (45, 58, 63, 59),   # The dorm's and the annex's fronts
    # (they stop a row short of the south forest: its trees' tips stand on the lawn in front of it)
    (4, 26, 31, 27), (4, 27, 6, 36),             # In front of Literature & Library, down to the street
    (26, 17, 28, 26), (26, 17, 58, 20),          # Behind the central building, the greenhouse's door
    (14, 16, 16, 17), (4, 16, 27, 17),           # The stadium's entrance
    (56, 21, 72, 23), (70, 23, 72, 36),          # The Science Institute, down the east side
    (52, 16, 54, 17),                            # The ranch's gate
]


# Freestanding signposts (primary metatile 0x003), by their text (CeladonUniversity's Sign<n>), only where there's
# no building to put a plaque on: the welcome by the gate, the stadium, the ranch, the pool. They stand on the lawn,
# beside the paths. The buildings' names are plaques by their doors (add_plaques; build_university_maps.py).
CAMPUS_SIGNS = {0: (34, 58), 12: (26, 15), 13: (47, 16), 14: (13, 47)}
# Ground metatiles that block: forest, tree trunks, signs, fences
SOLID_GROUND = {0x01C, 0x01D, 0x014, 0x015, 0x01E, 0x01F, 0x026, 0x027, 0x003, 0x0E7, 0x0F4, 0x0F5, 0x0EC, 0x0ED}


def campus_ground(st, reserve=()):
    """The campus as a Ground grid (metatile ids) and its collision {(x, y): solid}; `reserve` cells stay free of
    trees (where people stand)."""
    g = Ground(CAMPUS_W, CAMPUS_H)
    g.forest(0, 0, CAMPUS_W, 4)
    g.forest(0, 0, 4, CAMPUS_H)
    g.forest(CAMPUS_W - 4, 0, CAMPUS_W, CAMPUS_H)
    # From an even row and to an even column on each side of the gate, so every tree is whole (a tree is 2x2)
    g.forest(0, CAMPUS_H - 4, 34, CAMPUS_H)
    g.forest(42, CAMPUS_H - 4, CAMPUS_W, CAMPUS_H)
    for (x0, y0, x1, y1) in CAMPUS_PATHS:
        g.path(x0, y0, x1, y1)
    # Fill the lawn notches the paths leave (not under buildings, the ranch, the stadium or the signs), then pave
    for (name, x, y) in CAMPUS:
        g.block(x, y, st[name]['w'], st[name]['h'])
    g.block(48, 5, 10, 12)
    g.block(4, 4, 22, 12)
    for (x, y) in CAMPUS_SIGNS.values():
        g.used.add((x, y))
    g.fill_notches()
    g.lay({tuple(k.split(',')): v for k, v in st.get('PAVING', {}).items()})
    stamp_solid = {}
    for (name, x, y) in CAMPUS:
        g.stamp(st[name], x, y)
        for r, row in enumerate(st[name]['solid']):
            for c, s in enumerate(row):
                if st[name]['rows'][r][c] is not None:
                    stamp_solid[(x + c, y + r)] = s
    g.fence(48, 5, 58, 17, gap=(52, 54))
    g.block(4, 4, 22, 12)
    for (x, y) in CAMPUS_SIGNS.values():
        # On the paving, the sign stands on that cell's paving, edges and all (the primary's sign has grass under it)
        if (x, y) in g.paved:
            key = ','.join(g.pave_states[(x, y)])
            if key in st.get('SIGN_PAVING', {}):
                g.set(x, y, st['SIGN_PAVING'][key])
            else:
                g.sign_needed.add(g.pave_states[(x, y)])
                g.set(x, y, 0x003)
            stamp_solid[(x, y)] = 1
        else:
            g.set(x, y, 0x003)
    for (x, y) in reserve:
        g.used.add((x, y))
    g.flowers(4, 72, 38)
    g.flowers(4, 72, 35)
    g.grove(4, 4, 72, 61)
    whole_trees(g)
    solid = {}
    for y in range(CAMPUS_H):
        for x in range(CAMPUS_W):
            m = g.g[y][x]
            solid[(x, y)] = stamp_solid[(x, y)] if (x, y) in stamp_solid else int(m in SOLID_GROUND)
    return g, solid


FOREST_CROWN, FOREST_LOWER = (0x01C, 0x01D), (0x014, 0x015)     # A forest row: crowns; their lower halves + tips
TREE_TIP, TREE_CROWN, TREE_TRUNK = (0x00E, 0x00F), (0x01E, 0x01F), (0x026, 0x027)   # A lone tree, top to bottom
GRASS = (0x008, 0x009, 0x010, 0x011)


def whole_trees(g):
    """Every tree whole: a forest's last row (crowns' lower halves with the next row's tips) over something that
    isn't forest gets the lone tree's trunk row instead (the crown's lower half on the lawn, no stray tips); a forest
    row of crowns under lawn gets the lone tree's tips on the lawn above it. The map's top and bottom rows are left:
    in the game the border (forest, the same 2x2 pattern) carries the trees on past the edge."""
    for y in range(g.h - 1):
        for x in range(g.w):
            m = g.g[y][x]
            if m in FOREST_LOWER and g.g[y + 1][x] not in FOREST_CROWN:
                g.g[y][x] = TREE_TRUNK[FOREST_LOWER.index(m)]
    for y in range(1, g.h):
        for x in range(g.w):
            m = g.g[y][x]
            if m in FOREST_CROWN and g.g[y - 1][x] in GRASS and (x, y - 1) not in g.paved:
                g.g[y - 1][x] = TREE_TIP[FOREST_CROWN.index(m)]


def tree_problems(g):
    """Cut-off trees on the campus: [(x, y, what)]. Halves must pair (left/right), a crown needs a tip or the
    forest's lower row above it, a lower row needs crowns or a trunk below, a tip a crown below, a trunk a crown
    above (the map's top and bottom rows count as continued by the border)."""
    out = []
    pairs = (FOREST_CROWN, FOREST_LOWER, TREE_TIP, TREE_CROWN, TREE_TRUNK)
    for y in range(g.h):
        for x in range(g.w):
            m = g.g[y][x]
            for pr in pairs:
                if m == pr[0] and (x + 1 >= g.w or g.g[y][x + 1] != pr[1]):
                    out.append((x, y, 'left half %03X without its right' % m))
                if m == pr[1] and (x == 0 or g.g[y][x - 1] != pr[0]):
                    out.append((x, y, 'right half %03X without its left' % m))
            up = g.g[y - 1][x] if y else None
            down = g.g[y + 1][x] if y + 1 < g.h else None
            if m in FOREST_CROWN and y and up not in FOREST_LOWER + TREE_TIP + TREE_TRUNK:
                out.append((x, y, 'forest crown without its tip'))
            if m in FOREST_LOWER and down is not None and down not in FOREST_CROWN:
                out.append((x, y, 'tips of trees that aren\'t there'))
            if m in TREE_TIP and down not in TREE_CROWN + FOREST_CROWN:
                out.append((x, y, 'tip without a crown'))
            if m in TREE_CROWN and down not in TREE_TRUNK:
                out.append((x, y, 'crown without its trunk'))
            if m in TREE_TRUNK and up not in TREE_CROWN + FOREST_CROWN:
                out.append((x, y, 'trunk without a crown'))
    return out


def campus(ts, st):
    g, _ = campus_ground(st)
    img = g.render(ts)
    for (spr, x, y, f) in ((PLAYER, 38, 37, UP), ('people/lass_frlg', 34, 40, DOWN), ('people/scientist', 63, 22, LEFT),
                           ('people/youngster_frlg', 26, 36, DOWN), ('people/beauty_frlg', 37, 18, DOWN),
                           ('people/gentleman_frlg', 20, 51, UP), ('pokemon/ponyta', 51, 12, LEFT),
                           ('pokemon/ponyta', 55, 14, DOWN), ('pokemon/chansey', 23, 51, DOWN),
                           ('people/camper_frlg', 9, 10, DOWN), ('people/picnicker_frlg', 18, 13, LEFT),
                           ('pokemon/psyduck', 7, 49, DOWN), ('people/woman_2_frlg', 57, 51, UP)):
        put(img, spr, x, y, f)
    return img


# --- Interiors ---------------------------------------------------------------------------------------------------------
# The rooms are drawn by draw_university_interiors.py (FireRed's interior look); it uses the campus's windows above
# and this glass wall.
def glass_wall(P, x0, w, rows):
    """The greenhouse's back wall from inside: glass panes in steel, leaves outside."""
    for y in range(rows * T):
        for x in range(x0, x0 + w):
            lx = (x - x0) % 16
            if y < 2:
                c = GL['ol'] if y == 0 else GL['f3']
            elif y >= rows * T - 3:
                c = GL['s1'] if y < rows * T - 1 else GL['ol']
            elif lx == 0 or y % 16 == 2:
                c = GL['f2']
            else:
                c = GL['q0'] if y < 10 else GL['q1']
            P.p(x, y, c)


# --- Mockups -----------------------------------------------------------------------------------------------------------
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
    d.rectangle((box[0] - 3, box[1] - 2, box[2] + 3, box[3] + 2), fill=bg)
    d.text(xy, text, font=f, fill=fill)


def render_stamp(ts, st, extra_rows=0):
    g = Ground(st['w'] + 2, st['h'] + 2)
    door = st['doors'][0] if st['doors'] else (0, 0)
    g.path(door[0], st['h'] + 1, door[0] + 3, st['h'] + 2)
    g.lay()
    g.stamp(st, 1, 1)
    return g.render(ts)


def outside_inside(name, ext_ts, st, img, rst, out, footprint_top=0, s=3):
    """The building (on grass) and its room (img, rendered) side by side at the same scale; the door columns
    marked."""
    ext = render_stamp(ext_ts, st)
    ext_s, room_s = scaled(ext, s), scaled(img, s)
    pad, top = 24, 60
    ew, eh = st['w'], st['h'] - footprint_top
    rw, rh = rst['w'], rst['h']
    fy = top + (1 + footprint_top) * T * s          # The footprint's top, outside; the room starts there too
    canvas = Image.new('RGBA', (ext_s.width + room_s.width + pad * 3,
                                max(top + ext_s.height, fy + room_s.height) + pad), (24, 24, 36, 255))
    canvas.alpha_composite(ext_s, (pad, top))
    rx, ry = ext_s.width + pad * 2, fy
    canvas.alpha_composite(room_s, (rx, ry))
    d = ImageDraw.Draw(canvas)
    d.rectangle((pad + T * s, fy, pad + (ew + 1) * T * s - 1, fy + eh * T * s - 1), outline=(255, 64, 255), width=2)
    door_ext = st['doors'][0][0]
    dx_in = [int(k.split(',')[0]) for k, v in rst['marks'].items() if v == 'mat'][0]
    d.rectangle((pad + (door_ext + 1) * T * s, fy + (eh - 1) * T * s, pad + (door_ext + 2) * T * s - 1,
                 fy + eh * T * s - 1), outline=(255, 48, 48), width=3)
    d.rectangle((rx + dx_in * T * s, ry + (rh - 2) * T * s, rx + (dx_in + 1) * T * s - 1, ry + (rh - 1) * T * s - 1),
                outline=(255, 48, 48), width=3)
    label(canvas, '%s - outside %dx%d, door column %d' % (name, ew, eh, door_ext), (pad, 16), 22)
    label(canvas, 'inside %dx%d, door column %d' % (rw, rh, dx_in), (rx, 16), 22)
    canvas.convert('RGB').save(out)


def sheet(ts, out, per_row=16, s=3):
    rows_n = (len(ts.metatiles) + per_row - 1) // per_row
    img = Image.new('RGBA', (per_row * T, rows_n * T), (255, 0, 255, 255))
    for i in range(len(ts.metatiles)):
        img.paste(ts.render(PRIMARY + i), ((i % per_row) * T, (i // per_row) * T))
    scaled(img, s).convert('RGB').save(out)


def main():
    MB = behaviors()
    assert MB['MB_ANIMATED_DOOR'] == behavior_of('gTileset_CeladonCity', 0x294), 'behavior numbering'
    os.makedirs(OUT, exist_ok=True)

    # The campus tileset
    grass = primary_entries('gTileset_General_Frlg', 0x008)
    ext = Tileset('gTileset_CeladonUniversity', os.path.join(ROOT, 'data/tilesets/secondary/celadon_university'),
                  EXTERIOR_PALS, 'gTileset_General_Frlg', grass)
    stamps = exterior_stamps()
    plaques = add_plaques(stamps)
    ext_stamps = cut_exterior(ext, stamps, MB)
    for name, cells in plaques.items():
        ext_stamps[name]['plaques'] = cells
    # The paving's junctions, 1-wide stretches and ends: the mixes of quarters the campus needs
    g, _ = campus_ground(ext_stamps)
    pave = paving_metatiles(ext, g.pave_needed)
    ext_stamps['PAVING'] = {','.join(k): v for k, v in pave.items()}
    # Signposts on the paving: the primary's signpost (its top layer, its behavior) over the cell's paving
    g, _ = campus_ground(ext_stamps)
    sign = primary_entries('gTileset_General_Frlg', 0x003, 1)
    ext_stamps['SIGN_PAVING'] = {
        ','.join(k): ext.metatile(paving_entries(ext, k), sign, behavior_of('gTileset_General_Frlg', 0x003),
                                  LAYER_COVERED) for k in sorted(g.sign_needed)}

    print(ext.budget())
    assert len(ext.tiles) <= MAX_TILES and len(ext.metatiles) <= MAX_METATILES, 'over budget'
    ext.write()
    # The rooms and their tilesets (gTileset_UniversityClassroom, gTileset_UniversityDorm, gTileset_UniversityLab)
    import draw_university_interiors as I
    room_stamps, room_sets = I.build(MB)
    json.dump({'exterior': ext_stamps, 'interior': room_stamps}, open(STAMPS, 'w'), indent=None,
              separators=(',', ':'))

    # Mockups, rendered from the tilesets
    c = campus(ext, ext_stamps)
    c2 = scaled(c, 2)
    label(c2, 'Celadon University (76x64), drawn with gTileset_CeladonUniversity', (72, 12), 24)
    names = {'STADIUM': 'Stadium', 'HUMANITIES': 'Literature & Library', 'GREENHOUSE': 'Home Economics',
             'SCIENCE': 'Science Institute', 'MAIN_HALL': 'Main Hall', 'HOSPITAL': 'Hospital', 'DORM': 'Dorm',
             'ANNEX': 'Your room', 'POOL': 'Pool', 'BARN': 'Ranch'}
    pav = iter(('Faculty Office', 'Cafeteria'))
    wings = iter(('Lecture Hall A', 'Lecture Hall B'))
    for (name, x, y) in CAMPUS:
        text = next(pav) if name == 'PAVILION' else next(wings) if name == 'LECTURE_WING' else names.get(name)
        if text:
            label(c2, text, (x * 32 + 6, (y + (5 if name == 'MAIN_HALL' else 0)) * 32 + 6), 14, bg=(40, 48, 72))
    c2.convert('RGB').save(os.path.join(OUT, 'mockup_campus.png'))
    for (rname, sname, title, top) in (('MAIN_HALL', 'MAIN_HALL', 'Main Hall', 5),
                                       ('LECTURE_HALL_A', 'LECTURE_WING', 'Lecture Hall A', 0),
                                       ('DORM_LOBBY', 'DORM', 'Dorm lobby', 0)):
        rst = room_stamps[rname]
        img = room_sets[rst['tileset']].render_room(rst)
        I.people(img, rname)
        outside_inside(title, ext, ext_stamps[sname], img, rst, os.path.join(OUT, 'mockup_%s.png' % rname.lower()),
                       footprint_top=top)
    sheet(ext, os.path.join(OUT, 'tiles_exterior.png'))
    g, _ = campus_ground(ext_stamps)
    for (x, y, what) in tree_problems(g):
        print('campus tree at %d,%d: %s' % (x, y, what))
    for ts in room_sets.values():
        ts.sheet(os.path.join(OUT, 'tiles_%s.png' % ts.symbol.replace('gTileset_University', '').lower()))
    I.mockups(room_stamps, room_sets)

if __name__ == '__main__':
    main()
