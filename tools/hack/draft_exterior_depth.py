#!/usr/bin/env python3
"""DRAFT (design phase): three options for giving the campus buildings FireRed's depth and shading
(docs/hack/university_remake/exterior_depth.md). Re-draws the campus kit of draw_university_tiles.py behind an option
switch, builds the whole campus tileset in memory (to measure the GBA budget honestly) and renders mockups from the
real tiles. It never writes the tileset, the stamps or any map.

    <venv with Pillow>/bin/python tools/hack/draft_exterior_depth.py [current|A|B|C ...]

Options (each includes the one before):
    A  shading pass: same palettes (only the outline recolored to FireRed's #52526A), the wall body one step darker so
       white is left for highlights, roof value planes (lit upper slope, curb, shaded eave), cornice shadow on the
       wall, quoins, window lintel shadow + sill shadow, door recess and steps, plinth + ground shadow. Symmetric, so
       the mirror deduplication still halves the tiles.
    B  A + FireRed palette alignment (stone/slate/glass/gold/wood re-tuned to FRLG's exterior ramps, the interiors'
       school slate) + light from the upper left where it's cheap: lit left / shaded right roof verges and wall
       corners, pilasters between the window bays, columns lit on the left, dormer caps, a two-faced spire. Needs
       the budget recovery R1-R3 (recover()).
    C  B + structure: glass highlights, end boards on the roofs (Pewter Museum style), a shaded side wall on the right,
       window reveals (left in shadow, right lit), a stepped portal, column and dormer cast shadows, a 2-px cast
       shadow to the right of every building. Over budget even with R1-R4 (rendered past the 10-bit tile limit by a
       draft-only renderer).

Writes docs/hack/university_remake/depth_*.png and prints each option's tile/metatile/palette budget.
"""
import importlib
import math
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import draw_university_tiles as D  # noqa: E402
import render_layout as R  # noqa: E402

T = 16
OUT = os.path.join(D.ROOT, 'docs/hack/university_remake')
SECTION = (11, 20, 47, 52)    # Campus cells: a pavilion, Lecture Hall A, the Main Hall + tower, fountain, hospital


def gba(c):
    return tuple((v >> 3) << 3 for v in c)


def hx(s):
    return gba((int(s[0:2], 16), int(s[2:4], 16), int(s[4:6], 16)))


# --- Palettes per option ---------------------------------------------------------------------------------------------
FRLG_OL = hx('52526A')
# B/C: FireRed's own exterior ramps (gTileset_General_Frlg 2/3/5, Celadon, Saffron, Pewter; the interiors' school
# slate from BuildingFrlg 3; glass as Celadon's windows; gold as Celadon 10; wood as the Lab's)
B_STONE = dict(OL=FRLG_OL, W=hx('F6F6FF'), S1=hx('DEDEE6'), S2=hx('ACBDCD'), S3=hx('7B7B83'),
               SLH=hx('9CBDE6'), SL1=hx('739CC5'), SL2=hx('5A7BB4'), SL3=hx('41629C'),
               G1=hx('9CD5FF'), G2=hx('62A4DE'), AU=hx('F6E68B'), AU2=hx('D5AC4A'), WD=hx('C58341'), WD2=hx('8B5229'))
B_STEEL = (hx('CDCDD5'), hx('94A4B4'), hx('5A5A73'))

OPT = 'current'
PALB = NOMIRROR = False
QUOIN_ALT = True     # Long and short quoins alternating (False: plain corner posts, fewer tiles)
FEAT = set()
# Every asymmetric (lit from the upper left) cue the kit knows; OPTIONS (below) picks them per option
ALL_CUES = ('edges', 'pilaster', 'streak', 'sill', 'column', 'dormer', 'canopy', 'door', 'pediment', 'tower', 'hospital',
            'fountain', 'boards', 'sidewall', 'portal', 'reveals', 'column_shadow', 'dormer_shadow', 'cast')


def F(name):
    return name in FEAT


C = {}       # The kit's colors: OL W S1 S2 S3 SLH SL1 SL2 SL3 G1 G2 G3 AU AU2 WD WD2


def apply(opt, features=()):
    """Reloads draw_university_tiles (a clean slate), sets the option's palettes and patches its kit with the given
    asymmetric cues (ALL_CUES). With any cue on, buildings aren't mirrored as a whole (symmetric parts are drawn
    symmetric, so their tiles still deduplicate by flipping)."""
    global OPT, PALB, NOMIRROR, FEAT, C
    importlib.reload(D)
    OPT, PALB = opt, opt in ('B', 'C')
    FEAT = set(features)
    assert FEAT <= set(ALL_CUES), FEAT - set(ALL_CUES)
    NOMIRROR = bool(FEAT)
    if opt == 'current':
        return
    P = {k: list(v) for k, v in D.PAL.items()}
    if opt == 'A':
        for k in P:
            P[k][0] = FRLG_OL
        s = P['stone']
        C = dict(OL=s[0], W=s[1], S1=s[2], S2=s[3], S3=s[4], SL1=s[5], SL2=s[6], SL3=s[7], G1=s[8], G2=s[9],
                 G3=s[10], AU=s[11], AU2=s[12], WD=s[13], WD2=s[14])
        C['SLH'] = C['SL1']
    else:
        b = B_STONE
        P['stone'] = [b[k] for k in ('OL', 'W', 'S1', 'S2', 'S3', 'SLH', 'SL1', 'SL2', 'SL3', 'G1', 'G2', 'AU', 'AU2',
                                     'WD', 'WD2')]
        g = P['glass']
        P['glass'] = [b['OL'], b['W'], b['S1']] + list(B_STEEL) + g[6:]
        w = P['water']
        P['water'] = [b['OL'], b['W'], b['S1'], b['S2'], b['S3']] + w[5:13] + [b['AU']]
        a = P['accent']
        P['accent'] = [b['OL'], b['W'], b['S1'], b['S2'], b['S3']] + a[5:13] + [B_STEEL[1], b['AU']]
        C = dict(b)
        C['G3'] = C['SL3']
    D.PAL = P
    D.GL = dict(zip(('ol', 'w', 's1', 'f1', 'f2', 'f3', 'q0', 'q1', 'q2', 'q3', 'p1', 'p2', 'p3', 'pk', 'rd'), P['glass']))
    D.WA = dict(zip(('ol', 'w', 's1', 's2', 's3', 'a1', 'a2', 'a3', 'b1', 'b2', 'cr', 'rd', 'or', 'au'), P['water']))
    D.AC = dict(zip(('ol', 'w', 's1', 's2', 's3', 't1', 't2', 'b1', 'b2', 'rd', 'rd2', 'pk', 'gn', 'mt', 'ye'),
                    P['accent']))
    s = P['stone']
    D.OL, D.WHITE, D.S1, D.S2, D.S3, D.SL1, D.SL2, D.SL3, D.G1, D.G2, D.G3, D.AU, D.AU2, D.WD, D.WD2 = s
    for name in ('roof', 'cornice', 'dormer', 'wall', 'plinth', 'window', 'sqwindow', 'door', 'column', 'pediment',
                 'academic', 'main_hall', 'tower', 'wing', 'pavilion', 'humanities', 'dorm', 'annex', 'canopy',
                 'hospital', 'fountain', 'plain_wall_cell', 'add_plaques'):
        setattr(D, name, globals()['k_' + name])


def c(name):
    return C[name]


# --- The kit ---------------------------------------------------------------------------------------------------------
def k_roof(P, x0, y0, w, h, dormers=(), drow=None):
    """Mansard slate in value planes: ridge highlight, a lit upper slope, the curb, the steep slope in 8-pixel
    courses (half as busy as before), a shaded lower course over the eave, then the cornice."""
    OL, W, S1, SLH, SL1, SL2, SL3 = (c(k) for k in ('OL', 'W', 'S1', 'SLH', 'SL1', 'SL2', 'SL3'))
    hr = h - 8
    for y in range(hr):
        for x in range(w):
            if y == 0:
                col = OL
            elif y == 1:
                col = W
            elif y == 2:
                col = S1 if PALB else SL1
            elif y < 14:                                  # The lit upper slope
                col = SL1 if (y - 3) % 5 == 4 else SLH
            elif y == 14:
                col = W if PALB else SL1                  # The curb: a lit edge, then its shadow
            elif y == 15:
                col = SL3
            elif y >= hr - 3:                             # Shaded lowest course, over the cornice
                col = SL3 if y >= hr - 2 else SL2
            else:
                course, row = (y - 16) // 8, (y - 16) % 8
                # Joints at the cell edges on even courses, mid-cell on odd ones: symmetric in every 16-pixel cell,
                # so the flipped tiles still deduplicate without mirroring the whole building
                joint = (x - x0) % 16 in ((0, 15) if course % 2 == 0 else (7, 8))
                col = SL1 if row == 0 else SL3 if row == 7 or (joint and 1 <= row <= 5) else SL2
            P.p(x0 + x, y0 + y, col)
    # Verges
    for y in range(hr):
        P.p(x0, y0 + y, OL)
        P.p(x0 + w - 1, y0 + y, OL)
        if y == 0:
            continue
        if F('boards'):   # End boards (the Pewter Museum's): lit on the left end, in shadow on the right
            for k, col in enumerate((W, S1, S1, SLH)):
                P.p(x0 + 1 + k, y0 + y, col)
            P.p(x0 + 5, y0 + y, OL)
            for k, col in enumerate((c('S3'), SL3, SL3, SL2)):
                P.p(x0 + w - 2 - k, y0 + y, col)
            P.p(x0 + w - 6, y0 + y, OL)
        elif F('edges'):
            P.p(x0 + 1, y0 + y, W)
            P.p(x0 + 2, y0 + y, SLH)
            P.p(x0 + w - 2, y0 + y, SL3)
            P.p(x0 + w - 3, y0 + y, SL3)
        else:
            P.p(x0 + 1, y0 + y, SL1)
            P.p(x0 + w - 2, y0 + y, SL1)
    if F('boards'):
        P.hl(x0 + 1, x0 + 4, y0 + 1, W)
        P.hl(x0 + w - 5, x0 + w - 2, y0 + 1, S1)
    k_cornice(P, x0, y0 + h - 8, w)
    for col_ in dormers:
        k_dormer(P, x0 + col_ * T, y0 + (drow if drow is not None else h // T - 2) * T)


def k_cornice(P, x0, y, w):
    """8 rows: the roof's edge, a lit top, the fascia, dentils, the shadowed underside (the wall below carries the
    cast shadow)."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    for x in range(x0, x0 + w):
        gap = (x - x0) % 8 in (3, 4)
        rows = (OL, W, S1, S2, S3 if gap else W, S3 if gap else S1, S3, OL)
        for k, col in enumerate(rows):
            P.p(x, y + k, col)
    P.vl(x0, y, y + 7, OL)
    P.vl(x0 + w - 1, y, y + 7, OL)
    if F('edges'):
        P.vl(x0 + 1, y + 1, y + 6, W)
        P.vl(x0 + w - 2, y + 1, y + 6, S2)


def k_dormer(P, x, y):
    """A dormer with its own little hipped cap, a lit left cheek, and its shadow on the roof below (and to the right,
    option C)."""
    OL, W, S1, S2, SLH, SL1, SL2, SL3, G1, G2, G3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'SLH', 'SL1', 'SL2', 'SL3',
                                                                   'G1', 'G2', 'G3'))
    if F('dormer_shadow'):   # Shadow to the right on the roof
        P.r(x + 14, y + 4, 2, 12, SL3)
    P.r(x + 3, y + 3, 10, 12, S1)
    P.vl(x + 3, y + 3, y + 14, W)
    P.vl(x + 12, y + 3, y + 14, S2 if F('dormer') else W)
    P.vl(x + 2, y + 3, y + 14, OL)
    P.vl(x + 13, y + 3, y + 14, OL)
    # The cap: a small hip
    P.hl(x + 5, x + 10, y, OL)
    P.hl(x + 3, x + 12, y + 1, SLH if PALB else SL1)
    P.p(x + 4, y, OL)
    P.p(x + 11, y, OL)
    P.hl(x + 2, x + 13, y + 2, SL2)
    if F('dormer'):
        P.hl(x + 8, x + 12, y + 1, SL2)
        P.hl(x + 8, x + 13, y + 2, SL3)
    P.p(x + 2, y + 1, OL)
    P.p(x + 13, y + 1, OL)
    for yy in range(y + 5, y + 13):
        for xx in range(x + 5, x + 11):
            col = G3 if yy < y + 7 else G1 if yy < y + 9 else G2
            if xx in (x + 7, x + 8):
                col = W
            P.p(xx, yy, col)
    P.hl(x + 5, x + 10, y + 4, OL)
    P.vl(x + 4, y + 4, y + 12, OL)
    P.vl(x + 11, y + 4, y + 12, OL)
    P.hl(x + 4, x + 11, y + 13, W)
    P.hl(x + 2, x + 13, y + 15, SL3)
    P.hl(x + 3, x + 12, y + 15, OL)


def k_wall(P, x0, y0, w, h):
    """The stone one step down from white (white is for highlights), 8-pixel courses, the cornice's shadow across the
    top two rows, quoins at the corners (lit on the left; option C: the right corner turns into a shaded side wall)."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    for y in range(y0, y0 + h):
        dy = y - y0
        for x in range(x0, x0 + w):
            P.p(x, y, S3 if dy == 0 else S2 if dy == 1 else S2 if dy % 8 == 7 else S1)
        P.p(x0, y, OL)
        P.p(x0 + w - 1, y, OL)
        if dy < 2:
            continue
        k = (dy // 8) % 2
        qw = (5 if k == 0 else 3) if QUOIN_ALT else 4
        joint = dy % 8 == 7
        # Left quoins: lit stone
        for x in range(x0 + 1, x0 + 1 + qw):
            P.p(x, y, S2 if joint else W)
        P.p(x0 + 1 + qw, y, S2)
        if F('sidewall'):
            # The side wall in shadow: 6 pixels, its own courses, the corner line
            for x in range(x0 + w - 7, x0 + w - 1):
                P.p(x, y, S3 if joint else S2)
            P.p(x0 + w - 8, y, S3)
            P.p(x0 + w - 7, y, S1 if not joint else S2)
        elif F('edges'):
            for x in range(x0 + w - 1 - qw, x0 + w - 1):
                P.p(x, y, S3 if joint else S2)
            P.p(x0 + w - 2 - qw, y, S3)
        else:
            for x in range(x0 + w - 1 - qw, x0 + w - 1):
                P.p(x, y, S2 if joint else W)
            P.p(x0 + w - 2 - qw, y, S2)


def k_plinth(P, x0, y, w):
    """The base: a lit step, the plinth, its dark foot, then the ground shadow on the grass (y is the stamp's last
    4 rows + 2 rows above, as the building's bottom is at y + 4)."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    yb = y + 4      # The stamp's bottom
    rows = [(yb - 6, W), (yb - 5, S1), (yb - 4, S2), (yb - 3, S3), (yb - 2, OL)]
    for (yy, col) in rows:
        P.hl(x0, x0 + w - 1, yy, col)
    P.vl(x0, yb - 6, yb - 2, OL)
    P.vl(x0 + w - 1, yb - 6, yb - 2, OL)
    P.hl(x0 + 1, x0 + w - (1 if F('sill') else 2), yb - 1, S3)


def k_window(P, x, y):
    """A tall arched window: the arch's soffit in shadow, glass graded light to dark, a lit keystone and sill with the
    sill's shadow under it. B: a highlight streak in each pane (lit from the left). C: the reveals (left in shadow,
    right lit) and an architrave that steps the window into the wall."""
    OL, W, S1, S2, S3, G1, G2, G3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3', 'G1', 'G2', 'G3'))
    L, Rr, top, bot = x + 3, x + 12, y + 5, y + 26
    if F('reveals'):   # Architrave: lit top/left, shaded right/bottom, one pixel outside the frame
        for yy in range(top + 2, bot + 1):
            P.p(L - 1, yy, W)
            P.p(Rr + 1, yy, S2)
        P.hl(L + 2, Rr - 2, top - 1, W)
    for yy in range(top, bot + 1):
        inset = 3 if yy == top else 1 if yy == top + 1 else 0
        for xx in range(L + inset, Rr - inset + 1):
            if xx in (L + inset, Rr - inset) or yy in (top, bot):
                col = OL
            elif xx in (x + 7, x + 8) or yy == top + 8:
                col = W
            elif yy <= top + 2:
                col = G3                      # The soffit's shadow under the arch
            else:
                col = G1 if yy < top + 8 else G3 if yy >= bot - 2 else G2
            P.p(xx, yy, col)
    if F('streak'):   # One clean diagonal highlight across each lower pane (Celadon's windows have one)
        for px0 in (x + 4, x + 9):
            for k in range(3):
                P.p(px0 + 2 - k, top + 10 + k, W)
                P.p(px0 + 2 - k, top + 11 + k, G1)
    if F('reveals'):   # Reveals: left in shadow, right lit
        for yy in range(top + 3, bot):
            if yy != top + 8:
                P.p(L + 1, yy, G3)
                P.p(Rr - 1, yy, G1 if yy < top + 8 else G2)
    P.p(x + 7, top - 1, W)
    P.p(x + 8, top - 1, S1 if F('streak') else W)
    P.p(x + 7, top - 2, W)
    P.p(x + 8, top - 2, S1 if F('streak') else W)
    P.hl(x + 2, x + 13, bot + 1, W)
    P.hl(x + 2, x + 13, bot + 2, S3)
    P.hl(x + 3 if F('sill') else x + 2, x + 13, bot + 3, S2)


def k_sqwindow(P, x, y):
    """A square window: a lit stone lintel over it, the lintel's shadow on the glass, graded glass, a lit sill and its
    shadow."""
    OL, W, S1, S2, S3, G1, G2, G3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3', 'G1', 'G2', 'G3'))
    P.hl(x + 2, x + 13, y + 6, W)
    P.hl(x + 2, x + 13, y + 7, S2)
    P.r(x + 3, y + 8, 10, 14, OL)
    P.r(x + 4, y + 9, 8, 12, G2)
    P.r(x + 4, y + 9, 8, 2, G3)
    P.r(x + 4, y + 11, 8, 3, G1)
    P.r(x + 4, y + 18, 8, 3, G3)
    P.vl(x + 7, y + 9, y + 20, W)
    P.vl(x + 8, y + 9, y + 20, W)
    P.hl(x + 4, x + 11, y + 14, W)
    if F('streak'):
        for px0 in (x + 4, x + 9):
            for k in range(3):
                P.p(px0 + 2 - k, y + 15 + k, W)
                P.p(px0 + 2 - k, y + 16 + k, G1)
    if F('reveals'):
        P.vl(x + 4, y + 11, y + 20, G3)
        P.vl(x + 11, y + 11, y + 13, W)
        P.vl(x + 2, y + 8, y + 21, W)
        P.vl(x + 13, y + 8, y + 21, S2)
    P.hl(x + 2, x + 13, y + 22, W)
    P.hl(x + 2, x + 13, y + 23, S3)
    P.hl(x + 3 if F('sill') else x + 2, x + 13, y + 24, S2)


def k_door(P, x, y, glass=False):
    """The portal: an arched surround (lit left, shaded right on B/C), the soffit in deep shadow, the fanlight, the
    leaves set back (C: a shadowed left reveal, a lit right one, a second step), steps at the foot."""
    OL, W, S1, S2, S3, G1, G2, G3, WD, WD2, AU = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3', 'G1', 'G2', 'G3', 'WD',
                                                                 'WD2', 'AU'))
    P.r(x + 1, y + 5, 14, 25, S1)
    P.hl(x + 4, x + 11, y + 3, OL)
    P.hl(x + 2, x + 13, y + 4, OL)
    P.p(x + 3, y + 4, OL)
    P.vl(x + 1, y + 5, y + 29, OL)
    P.vl(x + 14, y + 5, y + 29, OL)
    P.vl(x + 2, y + 5, y + 29, W)
    P.vl(x + 13, y + 5, y + 29, S2 if F('door') else W)
    P.hl(x + 4, x + 11, y + 5, W)
    # The opening, the soffit and fanlight
    for yy in range(y + 7, y + 16):
        inset = 3 if yy == y + 7 else 1 if yy == y + 8 else 0
        for xx in range(x + 3 + inset, x + 13 - inset):
            edge = yy == y + 7 or xx in (x + 3 + inset, x + 12 - inset)
            col = OL if edge else W if xx in (x + 7, x + 8) else G3 if yy < y + 10 else G1 if yy < y + 13 else G2
            P.p(xx, yy, col)
    for yy in range(y + 16, y + 29):
        for xx in range(x + 3, x + 13):
            if glass:
                col = OL if xx in (x + 3, x + 12, x + 7, x + 8) else G3 if yy < y + 18 else G1 if yy < y + 22 else G2
            else:
                col = WD2 if xx in (x + 3, x + 12, x + 7, x + 8) or yy in (y + 18, y + 25) else WD
            P.p(xx, yy, col)
    P.hl(x + 3, x + 12, y + 16, OL)
    P.hl(x + 4, x + 11, y + 17, OL if not glass else G3)     # The lintel's shadow on the leaves
    if not glass:
        P.p(x + 6, y + 22, AU)
        P.p(x + 9, y + 22, AU)
    if F('portal'):   # The reveals
        P.vl(x + 3, y + 9, y + 28, S3)
        P.vl(x + 4, y + 17, y + 28, OL)
        P.vl(x + 12, y + 9, y + 28, W)
    # Steps
    P.hl(x + 1, x + 14, y + 29, W)
    P.hl(x + 1, x + 14, y + 30, S2 if not F('portal') else S1)
    P.hl(x + 1, x + 14, y + 31, S3)
    if F('portal'):
        P.hl(x, x + 15, y + 30, S2)
        P.p(x, y + 30, OL)
        P.p(x + 15, y + 30, OL)


def k_column(P, x, y0, y1, cast=True):
    """A column: A shaded round (lit middle), B/C lit on the left, shaded right, with a capital and base; C casts a
    shadow on the porch's back wall to its right."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    shaft = (OL, W, W, S1, S2, OL) if F('column') else (OL, S1, W, W, S1, OL)
    if F('column_shadow') and cast:
        P.r(x + 11, y0 + 3, 3, y1 - y0 - 5, S3)
    for y in range(y0 + 3, y1 - 2):
        for k, col in enumerate(shaft):
            P.p(x + 5 + k, y, col)
    P.hl(x + 3, x + 12, y0, OL)
    P.hl(x + 3, x + 12, y0 + 1, W)
    P.hl(x + 4, x + 11, y0 + 2, S2)
    P.hl(x + 4, x + 11, y1 - 2, W)
    P.hl(x + 3, x + 12, y1 - 1, S2)
    P.p(x + 3, y1 - 1, OL)
    P.p(x + 12, y1 - 1, OL)
    if F('column'):
        P.p(x + 12, y0 + 1, S1)
        P.p(x + 11, y1 - 2, S1)


def k_pediment(P, cx, base_y, half, height, emblem=True):
    """The pediment: lit raking cornice on the left (both on A), the cornice's shadow along the tympanum, the
    tympanum one step down from white, the emblem with a shadow."""
    OL, W, S1, S2, AU, AU2 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'AU', 'AU2'))
    for dy in range(height + 1):
        y = base_y - dy
        hw = half * (1 - dy / height)
        for x in range(int(cx - hw) - 1, int(cx + hw) + 2):
            d = abs(x + .5 - cx)
            if d > hw:
                continue
            left = x + .5 < cx
            if d > hw - 1.5 or dy == 0:
                col = OL
            elif d > hw - 3:
                col = W if (left or not F('pediment')) else S1
            elif d > hw - 4.5:
                col = S2
            else:
                col = S1
            P.p(x, y, col)
    P.hl(int(cx - half) + 1, int(cx + half) - 2, base_y - 1, S2)
    if emblem:
        cy = base_y - int(height * .36)
        if F('pediment'):
            P.ellipse(cx + 1, cy + 1, 4.5, 4.5, S2)
        P.ellipse(cx, cy, 4.5, 4.5, AU, AU2)
        if F('pediment'):
            P.p(int(cx) - 2, int(cy) - 2, W)


def k_academic(w, roof_rows, storeys, doors, dormers=(), kind='arch', pediment_over=None, skip=(), spacing=2,
               columns=None, portico=()):
    """Mansard roof, storeys of windows, pilasters between the bays (B/C, regular arched facades), doors, the base.
    C: the building is two pixels narrower than its stamp: the last two columns are its cast shadow on the grass."""
    h = roof_rows + storeys * 2
    P = D.Pic(w * T, h * T)
    bw = P.w - (2 if F('cast') else 0)
    k_roof(P, 0, 0, bw, roof_rows * T, dormers, roof_rows - 2)
    k_wall(P, 0, roof_rows * T, bw, storeys * 2 * T)
    if F('pilaster') and kind == 'arch' and columns is None and spacing == 2:
        for col_ in range(1, w - 1):
            if not D.has_window(col_, w, spacing) and col_ not in doors and col_ not in portico:
                pilaster(P, col_ * T, roof_rows * T, h * T - 6)
    D.facade(P, 0, roof_rows * T, w, storeys, kind, skip=[(d, storeys - 1) for d in doors] + list(skip),
             spacing=spacing, columns=columns)
    k_plinth(P, 0, h * T - 4, bw)
    for d in doors:
        k_door(P, d * T, (h - 2) * T, glass=kind != 'arch')
    if pediment_over is not None:
        k_pediment(P, pediment_over * T + 8, roof_rows * T - 1, 30, 20)
    if F('cast'):
        cast_shadow(P, bw, roof_rows * T - 6)
    return P


def pilaster(P, x, y0, y1):
    """A flat pier between two window bays: a lit left edge, its body, a shaded right edge and its shadow on the wall;
    a capital under the cornice's shadow and a base on the plinth."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    for y in range(y0 + 2, y1):
        P.p(x + 5, y, W)
        for xx in range(x + 6, x + 10):
            P.p(xx, y, S1)
        P.p(x + 10, y, S2)
        P.p(x + 11, y, S3 if F('cast') else S2)
    P.hl(x + 4, x + 11, y0 + 2, W)
    P.hl(x + 4, x + 11, y0 + 3, S2)
    P.hl(x + 4, x + 11, y1 - 2, W)
    P.hl(x + 4, x + 11, y1 - 1, S2)


def cast_shadow(P, bw, y_top):
    """C: the building's shadow on the grass to its right (2 px, as FireRed's houses), from under the eave down."""
    S3 = c('S3')
    for y in range(y_top + 4, P.h):
        P.p(bw, y, S3)
        P.p(bw + 1, y, S3 if y % 2 or y >= P.h - 2 else None)


def k_main_hall():
    """15x16: the Main Hall with the clock tower and portico, in the option's kit."""
    P = D.Pic(15 * T, 16 * T)
    B = k_academic(15, 5, 3, doors=(7,), dormers=(2, 4, 10, 12),
                   skip=[(c_, s) for c_ in range(5, 10) for s in (0, 1)], portico=range(5, 10))
    P.paste(B, 0, 5 * T)
    y0 = 5 * T
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    # The porch: its back wall in the porch's shadow (darkest under the entablature)
    top, bottom = y0 + 7 * T + 6, y0 + 11 * T - 6
    P.r(5 * T, top, 5 * T, bottom - top, S2 if PALB else S3)
    P.r(5 * T, top, 5 * T, 4, S3 if PALB else OL)
    P.hl(5 * T, 10 * T - 1, top + 4, S3)
    k_door(P, 7 * T, y0 + 9 * T)
    P.r(5 * T, top, T * 5, 2, OL)
    # The entablature
    P.r(5 * T, y0 + 7 * T, 5 * T, 6, W)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T, OL)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T + 3, S1)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T + 4, S2)
    P.hl(5 * T, 10 * T - 1, y0 + 7 * T + 5, OL)
    for c_ in (5, 6, 8, 9):
        k_column(P, c_ * T, y0 + 7 * T + 6, y0 + 11 * T - 4)
    k_pediment(P, 120, y0 + 7 * T, 40, 24)
    # The porch's steps
    for x in range(5 * T, 10 * T):
        for k, col in enumerate((W, S2, OL, S3)):
            P.p(x, y0 + 11 * T - 4 + k, col)
    k_tower(P, 5 * T, 0)
    if not NOMIRROR:
        P.mirror()
    return P


def k_tower(P, x0, y0):
    """The clock tower (5x6, its base row on the Main Hall's roof), drawn on the left half and mirrored: the new kit's
    wall, cornice and lit balustrade, dark belfry openings with lit sills. 'tower': the spire redrawn with two faces
    (lit left face, shaded right face, a lit hip line between: only the spire's right half costs new tiles).
    'sidewall': the tower body's shaded right side."""
    OL, W, S1, S2, S3, SLH, SL1, SL2, SL3, AU, AU2 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3', 'SLH', 'SL1', 'SL2',
                                                                     'SL3', 'AU', 'AU2'))
    cx = x0 + 40
    by = y0 + 5 * T
    for x in range(x0 + 2, x0 + 40):     # Balustrade
        P.p(x, by, OL)
        P.p(x, by + 1, W)
        P.p(x, by + 2, S1)
        P.p(x, by + 3, S2)
        for yy in range(by + 4, by + 12):
            P.p(x, yy, (S2, W, S1, S3)[(x - x0) % 4])
        for k, col in enumerate((W, S1, S2, S3)):
            P.p(x, by + 12 + k, col)
    P.vl(x0 + 2, by, by + 15, OL)
    k_wall(P, x0 + 16, y0 + 2 * T, 48, 3 * T)     # Body (rows 2-4), 48 wide
    P.ellipse(cx, y0 + 3 * T + 12, 14, 14, AU, AU2)
    P.ellipse(cx, y0 + 3 * T + 12, 11, 11, W, S2)
    for (dx, dy) in ((0, -9), (-9, 0), (0, 8), (-7, -6), (-7, 5)):
        P.p(cx - 1 + dx, y0 + 3 * T + 12 + dy, OL)
    P.vl(cx - 1, y0 + 3 * T + 5, y0 + 3 * T + 12, OL)
    k_cornice(P, x0 + 14, y0 + 3 * T - 6, 52)
    ox, oy = x0 + 24, y0 + 2 * T + 1     # Belfry
    for yy in range(oy, oy + 8):
        inset = 1 if yy == oy else 0
        for xx in range(ox + inset, ox + 8 - inset):
            P.p(xx, yy, OL if yy <= oy + 2 or xx in (ox, ox + 7) else SL3)
    P.hl(ox, ox + 7, oy + 8, W)
    P.hl(ox, ox + 7, oy + 9, S3)

    def spire(two_faces):
        for dy in range(2 * T - 2):
            y = y0 + 2 * T - 1 - dy
            hw = 26 * (1 - dy / 30) + 1
            for x in range(int(cx - hw), int(cx + hw) + 1 if two_faces else cx):
                edge = x == int(cx - hw) or (two_faces and x == int(cx + hw))
                if not two_faces:
                    col = SL1 if dy % 3 == 0 else SL2
                elif x < cx - 1:
                    col = SL1 if dy % 3 == 0 else SLH
                elif x == cx - 1:
                    col = W
                else:
                    col = SL3 if dy % 3 == 0 else SL2
                P.p(x, y, OL if edge else col)
        P.hl(cx - 27, cx + 26 if two_faces else cx - 1, y0 + 2 * T - 1, OL)
        P.hl(cx - 26, cx + 25 if two_faces else cx - 1, y0 + 2 * T - 2, SL3)
        P.vl(cx - 1, y0, y0 + 3, AU)
        P.p(cx - 2, y0 + 1, AU2)
        if two_faces:
            P.vl(cx, y0, y0 + 3, AU2)
            P.p(cx + 1, y0 + 1, AU2)
    spire(False)
    P.mirror(x0, 80, y0, 6 * T)
    if F('tower'):
        spire(True)
    if F('sidewall'):   # The tower's shaded side
        for y in range(y0 + 2 * T + 2, y0 + 5 * T):
            for x in range(x0 + 58, x0 + 63):
                P.p(x, y, S3 if (y - y0) % 8 == 7 else S2)
            P.p(x0 + 57, y, S3)
            P.p(x0 + 63, y, OL)


def k_wing():
    P = k_academic(11, 5, 2, doors=(5,), dormers=(2, 8))
    if not NOMIRROR:
        P.mirror()
    return P


def k_pavilion():
    P = k_academic(9, 4, 2, doors=(4,), pediment_over=4)
    if not NOMIRROR:
        P.mirror()
    return P


def k_humanities():
    P = k_academic(22, 5, 2, doors=(5, 16), dormers=(3, 8, 13, 18), columns=(2, 5, 8, 10.5, 13, 16, 19))
    if not NOMIRROR:
        P.mirror()
    return P


def k_dorm():
    P = k_academic(17, 4, 3, doors=(8,), dormers=(2, 5, 11, 14), kind='square', spacing=3)
    k_canopy(P, 8, 8)
    if not NOMIRROR:
        P.mirror()
    return P


def k_annex():
    P = k_academic(11, 4, 2, doors=(5,), dormers=(2, 8), kind='square')
    k_door(P, 5 * T, 6 * T)
    if not NOMIRROR:
        P.mirror()
    return P


def k_canopy(P, col, row):
    """A slate canopy: lit top, a shaded front edge, its shadow on the wall and door below."""
    OL, W, S2, S3, SLH, SL1, SL2, SL3 = (c(k) for k in ('OL', 'W', 'S2', 'S3', 'SLH', 'SL1', 'SL2', 'SL3'))
    x, y = col * T, row * T
    P.hl(x - 7, x + T + 6, y + 6, S3)
    P.hl(x - 6, x + T + 7 if F('canopy') else x + T + 5, y + 7, S2)
    P.r(x - 8, y, T + 16, 6, SL2)
    P.hl(x - 8, x + T + 7, y, OL)
    P.hl(x - 7, x + T + 6, y + 1, SLH if PALB else SL1)
    P.hl(x - 7, x + T + 6, y + 4, SL3)
    P.hl(x - 8, x + T + 7, y + 5, OL)
    P.vl(x - 8, y, y + 5, OL)
    P.vl(x + T + 7, y, y + 5, OL)


def k_plain_wall_cell(pier=False):
    P = D.Pic(3 * T, 2 * T)
    k_wall(P, 0, 0, P.w, P.h + 32)
    if pier:
        pilaster(P, T, -64, P.h - 6)
    k_plinth(P, 0, P.h - 4, P.w)
    return P.im.crop((T, T, 2 * T, 2 * T)).tobytes()


def k_add_plaques(stamps):
    """As draw_university_tiles.add_plaques, but a plaque may also hang on a pilaster (B/C put one between every
    bay)."""
    plain = {k_plain_wall_cell(), k_plain_wall_cell(True)}
    out = {}
    for name, (pic, doors, *_rest) in stamps.items():
        if not doors:
            continue
        cells = []
        if name in D.GLASS_PLAQUES:
            for (c_, r) in D.GLASS_PLAQUES[name]:
                D.plaque(pic, c_ * T, r * T, glass=True)
                cells.append((c_, r, 0))
        else:
            for k, (dc, dr) in enumerate(doors):
                for side in (D.PLAQUE_SIDES.get(name, [(-1, 1)] * len(doors))[k]):
                    c_ = dc + side
                    while 0 < c_ < pic.w // T - 1:
                        if pic.im.crop((c_ * T, dr * T, c_ * T + T, dr * T + T)).tobytes() in plain:
                            break
                        c_ += side
                    assert 0 < c_ < pic.w // T - 1, 'no wall for a plaque on %s' % name
                    k_plaque(pic, c_ * T, dr * T)
                    cells.append((c_, dr, k))
        out[name] = cells
    return out


def k_plaque(P, x, y):
    """A brass plaque with a lit top-left and its shadow on the wall."""
    W, S2, S3, AU, AU2 = (c(k) for k in ('W', 'S2', 'S3', 'AU', 'AU2'))
    if F('sill'):     # Its shadow down and to the right
        P.r(x + 4, y + 3, 10, 8, S2)
    else:
        P.hl(x + 3, x + 12, y + 10, S2)
    P.r(x + 3, y + 2, 10, 8, AU2)
    P.r(x + 4, y + 3, 8, 6, AU)
    P.hl(x + 4, x + 11, y + 3, W)
    P.hl(x + 5, x + 10, y + 5, AU2)
    P.hl(x + 6, x + 9, y + 7, AU2)


def k_hospital():
    """The hospital: a parapet roof (the parapet's top lit, its inner faces and their shadow on the gravel), the
    red cross with a shadow, the facade in the option's kit."""
    OL, W, S1, S2, S3 = (c(k) for k in ('OL', 'W', 'S1', 'S2', 'S3'))
    AC = D.AC
    w, h = 13, 10
    P = D.Pic(w * T, h * T)
    bw = P.w - (2 if F('cast') else 0)
    rh = 4 * T
    for y in range(rh):
        for x in range(bw):
            P.p(x, y, S2 if (x % 8 in (3, 4) and y % 8 == 1) or (x % 8 in (0, 7) and y % 8 == 5) else S1)
    # Parapet: outline, lit top, inner face; the shadow it casts on the gravel (top, and left on B/C)
    for x in range(bw):
        P.p(x, 0, OL)
        P.p(x, 1, W)
        P.p(x, 2, W)
        P.p(x, 3, S2)
        P.p(x, 4, S3)
        P.p(x, 5, S2 if x % 4 in (1, 2) else S1)
    for y in range(rh - 5):
        P.p(0, y, OL)
        P.p(1, y, W)
        P.p(2, y, W)
        P.p(bw - 1, y, OL)
        P.p(bw - 2, y, S2 if F('hospital') else W)
        P.p(bw - 3, y, S2 if F('hospital') else W)
        if y >= 3:
            P.p(3, y, S2)
            P.p(bw - 4, y, W if F('hospital') else S2)
            if F('hospital'):
                P.p(4, y, S3 if y > 3 else S3)
                P.p(5, y, S2 if y % 4 in (1, 2) else S1)
    # Front parapet: the inner face (lit, it faces the light), its top, its front face (the cornice)
    for x in range(bw):
        for k, col in enumerate((W, W, S1, S2, OL)):
            P.p(x, rh - 5 + k, col)
    P.vl(0, rh - 5, rh - 1, OL)
    P.vl(bw - 1, rh - 5, rh - 1, OL)
    # The cross: a white square, red arms, its shadow
    cx, cy = P.w // 2, 2 * T
    if F('hospital'):
        P.r(cx - 14, cy - 14, 31, 31, S2)
    else:
        P.r(cx - 16, cy + 15, 32, 2, S2)
    P.r(cx - 16, cy - 16, 32, 31, AC['w'])
    P.hl(cx - 16, cx + 15, cy - 16, AC['s1'])
    P.r(cx - 5, cy - 12, 10, 23, AC['rd'])
    P.r(cx - 12, cy - 3, 24, 6, AC['rd'])
    if F('hospital'):   # The arms shaded on their right and lower edges
        P.vl(cx + 4, cy - 12, cy + 10, AC['rd2'])
        P.hl(cx + 5, cx + 11, cy + 2, AC['rd2'])
        P.hl(cx - 4, cx + 4, cy + 10, AC['rd2'])
    else:
        P.hl(cx - 12, cx + 11, cy + 2, AC['rd2'])
        P.hl(cx - 5, cx + 4, cy + 10, AC['rd2'])
    # Facade
    k_wall(P, 0, rh, bw, 6 * T)
    k_plinth(P, 0, h * T - 4, bw)
    D.facade(P, 0, rh, w, 3, 'square', skip=[(6, 2), (6, 1)], columns=(2, 4, 6, 8, 10))
    k_door(P, 6 * T, 8 * T, glass=True)
    k_canopy(P, 6, 8)
    sx, sy = 6 * T, 6 * T          # The cross sign over the door
    P.r(sx + 2, sy + 10, 12, 16, AC['w'])
    P.hl(sx + 2, sx + 13, sy + 9, AC['ol'])
    P.vl(sx + 2, sy + 9, sy + 26, AC['ol'])
    P.vl(sx + 13, sy + 9, sy + 26, AC['ol'])
    P.hl(sx + 2, sx + 13, sy + 26, AC['ol'])
    P.hl(sx + 3 if F('hospital') else sx + 2, sx + 13, sy + 27, AC['s2'])
    P.r(sx + 6, sy + 12, 4, 12, AC['rd'])
    P.r(sx + 3, sy + 16, 10, 4, AC['rd'])
    if F('hospital'):
        P.vl(sx + 9, sy + 12, sy + 23, AC['rd2'])
    if F('cast'):
        cast_shadow(P, bw, rh - 6)
    if not NOMIRROR:
        P.mirror()
    return P


def k_fountain():
    """The Gyarados fountain with a shadow on the paving, a lit rim (upper left on B/C), the rim's shadow on the
    water, the basin's outer face shaded."""
    WA = D.WA
    P = D.Pic(5 * T, 4 * T)
    cx = 40
    sx = 3 if F('fountain') else 0
    P.ellipse(cx + sx, 46, 38, 17, WA['s3'])                          # Shadow on the paving
    P.ellipse(cx, 42, 38, 19, WA['s2'], WA['ol'])                     # Outer face
    for y in range(P.h):
        for x in range(P.w):
            if P.get(x, y) == WA['s2'] and y > 46:
                P.p(x, y, WA['s3'])
    P.ellipse(cx, 38, 38, 19, WA['w'], WA['ol'])                      # Rim
    P.ellipse(cx, 38, 34, 16, WA['s1'])
    if F('fountain'):   # The rim's lower right is less lit
        for y in range(P.h):
            for x in range(P.w):
                if P.get(x, y) == WA['w'] and (x - cx) * .4 + (y - 38) > 6:
                    P.p(x, y, WA['s1'])
    P.ellipse(cx, 39, 31, 13, WA['a2'], WA['s3'])
    for y in range(P.h):
        for x in range(P.w):
            if P.get(x, y) == WA['a2']:
                ring = math.hypot((x + .5 - cx) / 31, (y + .5 - 39) / 13)
                if abs(ring * 6 % 2 - 1) < .16:
                    P.p(x, y, WA['a1'])
                # The rim's shadow on the water (top of the pool)
                if math.hypot((x + .5 - cx) / 31, (y + .5 - 41) / 13) > 1:
                    P.p(x, y, WA['a3'])
    if not NOMIRROR:
        P.mirror()
    # The pedestal, Gyarados and the jet: symmetric, drawn on their own and mirrored
    Q = D.Pic(P.w, P.h)
    Q.ellipse(cx, 42, 8, 4, WA['s2'], WA['ol'])
    Q.ellipse(cx, 40, 8, 4, WA['w'], WA['ol'])
    for y in range(14, 41):
        sway = int(round(2 * math.sin((y - 14) / 5)))
        for x in range(cx - 5 + sway, cx + 5 + sway):
            edge = x in (cx - 5 + sway, cx + 4 + sway)
            belly = x in (cx - 1 + sway, cx + sway)
            Q.p(x, y, WA['ol'] if edge else WA['cr'] if belly else WA['b1'] if x < cx + sway else WA['b2'])
    for (fy, fx) in ((30, -8), (22, -9)):
        for k in range(4):
            Q.hl(cx + fx + k, cx + fx + 3, fy + k, WA['rd'])
    Q.ellipse(cx, 11, 8, 6, WA['b1'], WA['ol'])
    Q.ellipse(cx, 13, 4, 2, WA['w'], WA['ol'])
    Q.p(cx - 5, 9, WA['ol'])
    Q.p(cx - 6, 8, WA['w'])
    for k in range(4):
        Q.p(cx - 3 - k, 4 - k // 2, WA['cr'])
    Q.vl(cx - 1, 0, 8, WA['w'])
    for k in range(26):
        t = k / 25
        Q.p(int(cx - 2 - t * 26), int(1 + (t * 1.9 - .25) ** 2 * 16), WA['a1'] if k % 3 else WA['w'])
    Q.mirror()
    P.paste(Q, 0, 0)
    return P




# --- Budget recovery (outside the mockup's section) -----------------------------------------------------------------
def capsule(P, cx, cy, hw, ry, rx_end, col, outline=None):
    """A capsule (two half-ellipses joined by a straight middle): its middle repeats, so it costs a few tiles where
    a long ellipse costs dozens."""
    def inside(x, y):
        dx = max(0.0, abs(x + .5 - cx) - (hw - rx_end))
        return (dx / rx_end) ** 2 + ((y + .5 - cy) / ry) ** 2 <= 1
    for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
        for x in range(int(cx - hw) - 1, int(cx + hw) + 2):
            if inside(x, y):
                edge = outline and not all(inside(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                P.p(x, y, outline if edge else col)


def recover(stadium_r=None, pool=False, greenhouse_r=None):
    """Frees tiles outside the mockup's section, patched into draw_university_tiles's source:
    R1 (always): the Science Institute's roof as a capsule instead of a long ellipse (the hub stays round): -13 tiles.
    R2 stadium_r: the track's corner radius (52 now; 36: -11, 32: -14; under ~28 the 26-px ring fills the infield).
    R3 pool: the lane ropes dashed every 4 pixels (6 doesn't divide the 8-pixel tiles): -4.
    R4 greenhouse_r: the glass vault's rounded ends (24 now; 12: -1)."""
    import inspect
    src = inspect.getsource(D.science)
    old = """    cap.ellipse(72, 18, 72, 14, GL['s1'], GL['ol'])
    cap.ellipse(72, 18, 50, 9, GL['w'], GL['f1'])
"""
    assert old in src
    D.capsule = capsule
    exec(src.replace(old, """    capsule(cap, 72, 18, 72, 14, 24, GL['s1'], GL['ol'])
    capsule(cap, 72, 18, 54, 9, 16, GL['w'], GL['f1'])
"""), D.__dict__)
    if stadium_r:
        src = inspect.getsource(D.stadium)
        old = 'x0, y0, x1, y1, rad = 8, 3 * T + 4, P.w - 8, h * T - 4, 52'
        assert old in src
        exec(src.replace(old, old[:-2] + str(stadium_r)), D.__dict__)
    if pool:
        src = inspect.getsource(D.pool)
        assert '(y // 3) % 2' in src
        exec(src.replace('(y // 3) % 2', '(y // 4) % 2'), D.__dict__)
    if greenhouse_r:
        src = inspect.getsource(D.greenhouse)
        old = 'L, rh, W, r = leaves(), 6 * T, w * T, 24'
        assert old in src
        exec(src.replace(old, old[:-2] + str(greenhouse_r)), D.__dict__)


# --- The options -----------------------------------------------------------------------------------------------------
# Asymmetric cues (lit from the upper left) by option, measured costs in tiles on top of the symmetric base (see
# exterior_depth.md): B keeps the cheap, repeated ones and the spire; C adds the structural ones.
B_SET = ['edges', 'pilaster', 'column', 'dormer', 'canopy', 'tower']
C_SET = B_SET + ['streak', 'boards', 'sidewall', 'portal', 'reveals', 'column_shadow',
                                             'dormer_shadow', 'cast']
OPTIONS = {
    'current': dict(features=[], recovery=None, quoins=True),
    'A': dict(features=[], recovery=None, quoins=True),
    'B': dict(features=B_SET, recovery=dict(stadium_r=32, pool=True), quoins=True),
    'C': dict(features=C_SET, recovery=dict(stadium_r=32, pool=True, greenhouse_r=12), quoins=False),
}
NAMES = {'current': 'Now (committed)', 'A': 'Option A: shading pass',
         'B': 'Option B: A + FRLG palettes, light from the upper left',
         'C': 'Option C: B + structural depth (3/4 side walls, reveals, cast shadows)'}


def wide_tileset(*args):
    """A Tileset that can also render over-budget sets (mockups only): secondary tiles are entries flagged with bit 30
    and numbered from 0, so a set past 384 tiles doesn't overflow the 10-bit tile number. The counts are the same."""
    class Wide(D.Tileset):
        FLAG = 1 << 30

        def _entry(self, n, hf, vf, pal):
            return self.FLAG | n | hf << 10 | vf << 11 | pal << 12

        def tile(self, pixels):
            cols = {c_ for row in pixels for c_ in row if c_ is not None}
            if not cols:
                return 0
            fits = [i for i, (_, pal) in enumerate(self.pals) if cols <= set(pal)]
            if not fits:
                self.off_palette += 1
                fits = [0]
                pal = self.pals[0][1]
                pixels = tuple(tuple(None if c_ is None else min(pal, key=lambda p: sum((a - b) ** 2
                                                                                       for a, b in zip(p, c_)))
                                     for c_ in row) for row in pixels)
            p = fits[0]
            pal = self.pals[p][1]
            idx = tuple(tuple(0 if c_ is None else pal.index(c_) + 1 for c_ in row) for row in pixels)
            for hf in (0, 1):
                for vf in (0, 1):
                    f = tuple(tuple(r[::-1]) if hf else r for r in (idx[::-1] if vf else idx))
                    if f in self.lookup:
                        return self._entry(self.lookup[f], hf, vf, D.FIRST_PAL + p)
            self.lookup[idx] = len(self.tiles)
            self.tiles.append(idx)
            return self._entry(len(self.tiles) - 1, 0, 0, D.FIRST_PAL + p)

        def raw_tile(self, idx):
            for hf in (0, 1):
                for vf in (0, 1):
                    f = tuple(tuple(r[::-1]) if hf else r for r in (idx[::-1] if vf else idx))
                    if f in self.lookup:
                        return self._entry(self.lookup[f], hf, vf, 0)
            self.lookup[idx] = len(self.tiles)
            self.tiles.append(idx)
            return self._entry(len(self.tiles) - 1, 0, 0, 0)

        def render_entry(self, e, img, ox, oy, skip_zero):
            if not e & self.FLAG:
                return D.Tileset.render_entry(self, e, img, ox, oy, skip_zero)
            t, hf, vf, p = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12 & 15
            idx = self.tiles[t]
            pal = [None] + list(self.pals[p - D.FIRST_PAL][1] if p >= D.FIRST_PAL else
                                D.primary_tileset(self.primary)[1][p][1:])
            px = img.load()
            for y in range(8):
                for x in range(8):
                    v = idx[7 - y if vf else y][7 - x if hf else x]
                    if v == 0 and skip_zero:
                        continue
                    c_ = pal[v] if v else D.primary_tileset(self.primary)[1][0][0]
                    px[ox + x, oy + y] = tuple(c_) + (255,)
    return Wide(*args)


def build(opt, features=None, recovery='default', quoins=None):
    """The option's whole campus tileset, in memory (never written), its stamps and the campus ground grid."""
    global QUOIN_ALT
    o = OPTIONS[opt]
    apply(opt, o['features'] if features is None else features)
    QUOIN_ALT = o['quoins'] if quoins is None else quoins
    recovery = o['recovery'] if recovery == 'default' else recovery
    if recovery is not None:
        recover(**recovery)
    MB = D.behaviors()
    grass = D.primary_entries('gTileset_General_Frlg', 0x008)
    ext = wide_tileset('gTileset_CeladonUniversity', '/nonexistent', D.EXTERIOR_PALS, 'gTileset_General_Frlg', grass)
    stamps = D.exterior_stamps()
    D.add_plaques(stamps)
    es = D.cut_exterior(ext, stamps, MB)
    g, _ = D.campus_ground(es)
    pave = D.paving_metatiles(ext, g.pave_needed)
    es['PAVING'] = {','.join(k): v for k, v in pave.items()}
    g, _ = D.campus_ground(es)
    sign = D.primary_entries('gTileset_General_Frlg', 0x003, 1)
    es['SIGN_PAVING'] = {','.join(k): ext.metatile(D.paving_entries(ext, k), sign,
                                                   D.behavior_of('gTileset_General_Frlg', 0x003), D.LAYER_COVERED)
                         for k in sorted(g.sign_needed)}
    g, _ = D.campus_ground(es)
    return ext, es, g


def budget(ext):
    n, m = len(ext.tiles), len(ext.metatiles)
    over = ', OVER by %d tiles' % (n - D.MAX_TILES) if n > D.MAX_TILES else ''
    return 'gTileset_CeladonUniversity: %d/384 tiles, %d/384 metatiles, %d/6 palettes%s' % (n, m, len(ext.pals), over)


# --- Rendering -------------------------------------------------------------------------------------------------------
def section(ext, g, rect=SECTION):
    x0, y0, x1, y1 = rect
    img = g.render(ext).crop((x0 * T, y0 * T, x1 * T, y1 * T))
    for (spr, x, y, f) in ((D.PLAYER, 38, 37, D.UP), ('people/lass_frlg', 34, 40, D.DOWN),
                           ('people/youngster_frlg', 26, 36, D.DOWN), ('people/gentleman_frlg', 20, 51, D.UP),
                           ('pokemon/chansey', 23, 51, D.DOWN)):
        if x0 <= x < x1 and y0 <= y < y1:
            D.put(img, spr, x - x0, y - y0, f)
    return img


def frlg(layout, rect):
    image, _ = R.render(layout, rect)
    im = Image.new('RGB', (len(image[0]), len(image)))
    im.putdata([c_ for row in image for c_ in row])
    return im.convert('RGBA')


def titled(img, title, sub=None, s=3, bg=(24, 24, 36)):
    big = D.scaled(img, s)
    top = 70 if sub else 44
    canvas = Image.new('RGBA', (big.width + 32, big.height + top + 16), bg + (255,))
    canvas.alpha_composite(big, (16, top))
    D.label(canvas, title, (16, 10), 22)
    if sub:
        D.label(canvas, sub, (16, 40), 15, fill=(220, 220, 230))
    return canvas


def stack(items, horizontal, gap=16, bg=(24, 24, 36)):
    w = sum(i.width for i in items) + gap * (len(items) - 1) if horizontal else max(i.width for i in items)
    h = max(i.height for i in items) if horizontal else sum(i.height for i in items) + gap * (len(items) - 1)
    out = Image.new('RGBA', (w, h), bg + (255,))
    at = 0
    for i in items:
        out.alpha_composite(i, (at, 0) if horizontal else (0, at))
        at += (i.width if horizontal else i.height) + gap
    return out


def main():
    opts = sys.argv[1:] or ['current', 'A', 'B', 'C']
    dept = frlg('LAYOUT_CELADON_CITY', (8, 4, 26, 16))
    museum = frlg('LAYOUT_PEWTER_CITY', (11, 0, 29, 9))
    mansion = frlg('LAYOUT_CINNABAR_ISLAND', (4, 0, 13, 5))
    built = {}
    for opt in opts:
        ext, es, g = build(opt)
        built[opt] = (ext, g)
        print('%-8s %s, %d tiles off-palette' % (opt, budget(ext), ext.off_palette))
        tag = opt.lower()
        titled(section(ext, g), NAMES[opt], budget(ext)).convert('RGB').save(os.path.join(OUT, 'depth_%s.png' % tag))
        # Beside FireRed, all at scale 3 on their own grass: Celadon's Dept. Store and condominium, the Lecture Hall,
        # the Pewter Museum, the hospital, the Pokemon Mansion
        row1 = stack([titled(dept, 'FRLG: Celadon Dept. Store + condominium'),
                      titled(D.render_stamp(ext, es['LECTURE_WING']), 'Lecture Hall'),
                      titled(museum, 'FRLG: Pewter Museum')], True)
        row2 = stack([titled(D.render_stamp(ext, es['HOSPITAL']), 'Hospital'),
                      titled(mansion, 'FRLG: Pokemon Mansion'),
                      titled(D.render_stamp(ext, es['PAVILION']), 'Faculty Office (pavilion)')], True)
        head = Image.new('RGBA', (max(row1.width, row2.width), 44), (24, 24, 36, 255))
        D.label(head, '%s  (%s)' % (NAMES[opt], budget(ext)), (16, 10), 20, bg=(60, 40, 90))
        stack([head, row1, row2], False).convert('RGB').save(os.path.join(OUT, 'depth_beside_frlg_%s.png' % tag))
    # Overview: the same corner (Lecture Hall A, the Main Hall's portico and tower) for every option, scale 2
    stack([titled(section(ext_, g_, (20, 20, 40, 37)), NAMES[o].split(':')[0], budget(ext_).split(': ')[1], s=2)
           for o, (ext_, g_) in built.items()], True).convert('RGB').save(os.path.join(OUT, 'depth_overview.png'))
    # The budget recoveries B and C rely on (outside the section): the Science Institute's roof, the stadium, the pool
    shots = []
    for title, rec in (('Now', None), ('Recovered (B: R1-R3; C adds R4)', OPTIONS['C']['recovery'])):
        ext, es, g = build('current', recovery=rec)
        img = g.render(ext)
        crop = img.crop((4 * T, 4 * T, 72 * T, 22 * T))
        shots.append(titled(crop, title, budget(ext), s=2))
    stack(shots, False).convert('RGB').save(os.path.join(OUT, 'depth_recovery.png'))


if __name__ == '__main__':
    main()
