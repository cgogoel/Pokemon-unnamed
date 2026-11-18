#!/usr/bin/env python3
"""The stone mosaic in the middle of B1F's clock (Act 2 finale, docs/hack/act2_finale.md): pictures only, cut into
metatiles by draw_station_tiles.py (Pokémon Mansion tileset, palette 10, MOSAIC_PALETTE).

A round dial of stone tesserae set into the basement floor, 5x5 squares (80x80 px), lit from the upper left like the
rest of FireRed:
    the rim      a raised ring of dressed stone: an outline, lit on its upper left, shaded on its lower right, and a
                 thin shadow inside it where the mosaic sits lower than the rim
    the band     a ring of terracotta tesserae laid in courses, twelve gold stones at the hours (12 at the top)
    the field    the left half night (deep blue, a crescent moon, a few stars), the right half day (sky blue, a sun),
                 parted by a gold line running from 12 to 6 (the moon furnace stands on the left, the sun on the right)
    the slab     the middle square: a plain stone plug with a carved ring, a dark seam round it. When the clock is
                 set it slides away (SLAB_HALF, then SLAB_OPEN): a shaft with a ladder going down.

    python3 tools/hack/draw_station_mosaic.py out.png   (a preview, 4x)
"""
import math
import os
import sys

SIZE = 80                       # 5x5 squares
C = (SIZE - 1) / 2              # The dial's centre
R_RIM_OUT, R_RIM_IN, R_BAND_IN = 38.6, 34.6, 26.0
SLAB = (32, 48)                 # The middle square's pixels (x and y)

# Palette 10: index -> RGB. 0 is see-through (the floor shows).
MOSAIC_PALETTE = [
    (0, 0, 0),
    (41, 33, 41),     # 1  grout, outlines
    (99, 90, 99),     # 2  stone, dark
    (148, 139, 139),  # 3  stone
    (197, 189, 180),  # 4  stone, light
    (238, 230, 213),  # 5  stone, glint
    (131, 57, 41),    # 6  terracotta, dark
    (189, 98, 57),    # 7  terracotta
    (222, 148, 98),   # 8  terracotta, light
    (32, 41, 98),     # 9  night, dark
    (57, 82, 156),    # 10 night
    (123, 172, 213),  # 11 day sky
    (189, 131, 32),   # 12 gold, dark
    (246, 197, 65),   # 13 gold
    (255, 241, 172),  # 14 gold, light (stars, the sun's middle)
    (90, 131, 189),   # 15 day sky, shaded (its grout)
]
GROUT, STONE_DARK, STONE, STONE_LIGHT, GLINT = 1, 2, 3, 4, 5
TERRA_DARK, TERRA, TERRA_LIGHT = 6, 7, 8
NIGHT_DARK, NIGHT, SKY = 9, 10, 11
GOLD_DARK, GOLD, GOLD_LIGHT, SKY_DARK = 12, 13, 14, 15
PIT = GROUT  # The shaft's dark


def _hash(*v):
    h = 2166136261
    for n in v:
        h = ((h ^ (n & 0xFFFF)) * 16777619) & 0xFFFFFFFF
    return h


def _vary(base, dark, light, cell):
    """A tessera's color: mostly the base, now and then a shade darker or lighter (hand-laid stones aren't even)."""
    r = _hash(*cell) % 9
    return dark if r == 0 else light if r == 1 else base


def dial():
    """The 80x80 picture (palette 10 indices), the slab closed."""
    img = [[0] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            dx, dy = x - C, y - C
            r = math.hypot(dx, dy)
            a = math.atan2(dx, -dy) % (2 * math.pi)        # 0 at 12 o'clock, clockwise
            light = -(dx + dy) / max(r, 1e-6)                # >0 facing the upper left
            if r >= R_RIM_OUT:
                continue
            if r >= R_RIM_OUT - 1:
                img[y][x] = GROUT
            elif r >= R_RIM_IN:
                # The rim: dressed stone, rounded (lit upper left, shaded lower right), joints every 30 degrees
                joint = abs(((a * 6 / math.pi) + 0.5) % 1 - 0.5) < 0.035 * (R_RIM_OUT / r)
                if joint:
                    img[y][x] = STONE_DARK
                elif r > R_RIM_OUT - 2:
                    img[y][x] = GLINT if light > 0.55 else STONE_LIGHT if light > -0.2 else STONE
                elif r < R_RIM_IN + 1:
                    img[y][x] = STONE_LIGHT if light < -0.55 else STONE if light < 0.2 else STONE_DARK
                else:
                    img[y][x] = STONE_LIGHT if light > 0.2 else STONE if light > -0.5 else STONE_DARK
            elif r >= R_RIM_IN - 1.2 and light > -0.3:
                img[y][x] = GROUT  # The mosaic sits lower than the rim: its shadow on the upper left
            elif r >= R_BAND_IN:
                img[y][x] = band(r, a)
            else:
                img[y][x] = field(x, y, dx, dy, r, a)
    slab(img, 'closed')
    return img


def band(r, a):
    """The terracotta band: two courses of tesserae, a gold stone at each hour."""
    course = 0 if r < (R_BAND_IN + R_RIM_IN - 1.2) / 2 else 1
    r0, r1 = (R_BAND_IN, (R_BAND_IN + R_RIM_IN - 1.2) / 2) if course == 0 else ((R_BAND_IN + R_RIM_IN - 1.2) / 2, R_RIM_IN - 1.2)
    if r - r0 < 0.9:
        return GROUT
    per = int(2 * math.pi * r1 / 4.2)                     # Tesserae ~4 px along the ring
    t = (a / (2 * math.pi) * per + course * 0.5)          # The courses are offset half a stone, like brickwork
    if (t % 1) < 0.9 / (2 * math.pi * r / per) * per / per * 1.0 and False:
        return GROUT
    seam = (t % 1) * (2 * math.pi * r / per)              # Pixels from the tessera's leading edge
    if seam < 0.85:
        return GROUT
    hour = (a * 6 / math.pi + 0.5) % 1 - 0.5              # Distance (in hours) to the nearest hour
    if abs(hour) * r * math.pi / 6 < 2.6:
        # The hour stones: gold, lit on their upper left
        return GOLD_LIGHT if seam < 2.0 and course == 1 else GOLD if course == 1 else GOLD_DARK
    return _vary(TERRA, TERRA_DARK, TERRA_LIGHT, (int(t), course))


def field(x, y, dx, dy, r, a):
    """Night on the left, day on the right, tesserae on a square grid (grout a shade darker than the stones, so the
    pictures read first); a gold line from 12 to 6."""
    if abs(dx) < 1.0:
        return GOLD if dy < 0 else GOLD_DARK
    if r > R_BAND_IN - 1.0:
        return GROUT
    seam = x % 4 == 0 or y % 4 == 0
    cell = (x // 4, y // 4)
    if dx < 0:
        # A crescent moon opening to the upper right, rimmed in dark gold, and a few stars
        mx, my = dx + 17.0, dy
        moon, cut = math.hypot(mx, my), math.hypot(mx - 3.4, my + 2.4)
        if moon < 7.4 and cut > 5.8:
            return GOLD_DARK if moon > 6.5 or cut < 6.6 else GOLD_LIGHT if mx + my < -3 else GOLD
        if seam:
            return NIGHT_DARK
        if cell in ((5, 4), (8, 6), (4, 15), (8, 14), (7, 10)):
            return GOLD_LIGHT
        return NIGHT_DARK if _hash(*cell) % 6 == 0 else NIGHT
    sx, sy = dx - 17.0, dy
    s = math.hypot(sx, sy)
    if s < 4.2:
        return GOLD_LIGHT if sx + sy < 0.5 else GOLD
    if s < 5.2:
        return GOLD_DARK
    ray = (math.atan2(sx, -sy) * 4 / math.pi + 0.5) % 1
    if 6.2 < s < 9.2 and abs(ray - 0.5) > 0.36:
        return GOLD if s < 8.0 else GOLD_DARK
    if seam:
        return SKY_DARK
    return SKY_DARK if _hash(*cell) % 7 == 0 else SKY


SLAB_STONE = [
    # The slab, 16x16 over the middle square. s: stone; l: lit edge; d: shaded edge; K: seam; r/R: the carved ring
    'KKKKKKKKKKKKKKKK',
    'KllllllllllllldK',
    'KlsssssssssssddK',
    'KlssssssssssssdK',
    'KlssssKKKKsssddK',
    'KlsssKdrrRKsssdK',
    'KlssKdsssslKssdK',
    'KlssKrssssRKssdK',
    'KlssKrssssRKssdK',
    'KlssKdsssslKssdK',
    'KlsssKRRRlKsssdK',
    'KlssssKKKKssssdK',
    'KlsssssssssssddK',
    'KldssssssssssddK',
    'KddddddddddddddK',
    'KKKKKKKKKKKKKKKK',
]
SLAB_KEYS = {'K': GROUT, 'l': STONE_LIGHT, 'd': STONE_DARK, 's': STONE, 'r': STONE_DARK, 'R': STONE_LIGHT}


def shaft():
    """The open shaft, 16x16: its stone lip, the far wall going down into the dark, a ladder's rails and rungs (the
    lower ones fading)."""
    pic = [[PIT] * 16 for _ in range(16)]
    for x in range(16):
        pic[0][x] = GROUT
        pic[1][x] = STONE_LIGHT if x < 15 else STONE
    for y in range(16):
        pic[y][0] = GROUT
        pic[y][15] = GROUT
    for y in range(2, 6):    # The far wall, in shadow, darker as it goes down
        for x in range(1, 15):
            pic[y][x] = (STONE_DARK, STONE_DARK, GROUT, GROUT)[y - 2] if (x + y) % 5 else GROUT
    for y in range(1, 16):
        pic[y][1] = STONE_DARK if y < 8 else GROUT  # The near-left wall's edge, catching a little light
    depth = lambda y: TERRA_LIGHT if y < 6 else TERRA if y < 10 else TERRA_DARK if y < 13 else STONE_DARK
    for y in range(2, 16):
        for x in (5, 10):
            pic[y][x] = depth(y)
    for y in (4, 7, 10, 13):
        for x in range(6, 10):
            pic[y][x] = depth(y)
    return pic


def slab(img, state):
    """Draws the middle square: 'closed', 'half' (the slab slid halfway under the dial's left side) or 'open'."""
    x0, y0 = SLAB[0], SLAB[0]
    stone = [[SLAB_KEYS[c] for c in row] for row in SLAB_STONE]
    hole = shaft()
    shift = {'closed': 0, 'half': 9, 'open': 16}[state]
    for y in range(16):
        for x in range(16):
            sx = x + shift
            if sx < 16:
                c = stone[y][sx]
                if x == 15 - 0 and shift and sx < 16:
                    c = GROUT
            else:
                c = hole[y][x]
            img[y0 + y][x0 + x] = c
    if shift and shift < 16:
        # The slab's edge throws a shadow into the shaft
        for y in range(1, 15):
            img[y0 + y][x0 + 16 - shift] = PIT
    return img


def middle(state):
    """Just the middle square's 16x16 picture."""
    img = dial()
    slab(img, state)
    return [row[SLAB[0]:SLAB[1]] for row in img[SLAB[0]:SLAB[1]]]


def preview(path, scale=4):
    sys.path.insert(0, os.path.dirname(__file__))
    from render_layout import write_png
    floor = (222, 205, 156)
    pics = [dial(), slab(dial(), 'half'), slab(dial(), 'open')]
    w = SIZE * 3 + 16
    out = [[floor] * w for _ in range(SIZE)]
    for i, p in enumerate(pics):
        for y in range(SIZE):
            for x in range(SIZE):
                if p[y][x]:
                    out[y][i * (SIZE + 8) + x] = MOSAIC_PALETTE[p[y][x]]
    write_png(path, out, scale)


if __name__ == '__main__':
    preview(sys.argv[1] if len(sys.argv) > 1 else 'mosaic.png')
