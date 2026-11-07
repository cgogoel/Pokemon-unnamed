#!/usr/bin/env python3
"""Draws the Grass course's night bloom sprites (Act 2, docs/hack/act2_story.md): object event pictures.

    glowcap_dim.png     A glowcap asleep: dull blue-grey caps, in the NPC 1 palette (tinted by the night like
                        everything else). OBJ_EVENT_GFX_GLOWCAP_DIM, 16x16.
    glowcap_lit.png     The same glowcap woken by a GRASS move: bright cyan caps with white spots.
                        OBJ_EVENT_GFX_GLOWCAP_LIT, 16x16.
    moonveil_bud.png    The MOONVEIL, closed: a lavender-tipped bud on a leafy stem. OBJ_EVENT_GFX_MOONVEIL_BUD, 16x32.
    moonveil_bloom.png  The MOONVEIL open at dawn: white petals around a golden center. OBJ_EVENT_GFX_MOONVEIL_BLOOM.

The lit glowcap and the MOONVEIL use their own palette, night_bloom.pal (OBJ_EVENT_PAL_TAG_NIGHT_BLOOM, blend-immune:
the night's tint passes them by, so they glow in the dark). Re-running redraws all five files.

    python3 tools/hack/draw_night_bloom.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import write_png  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
PICS = os.path.join(ROOT, 'graphics/object_events/pics/misc')
PALETTES = os.path.join(ROOT, 'graphics/object_events/palettes')

# night_bloom.pal
GLOW = [
    (115, 197, 164),  # 0 transparent
    (20, 40, 70),     # 1 K outline
    (30, 150, 190),   # 2 c cap shade
    (80, 215, 240),   # 3 C cap
    (180, 250, 255),  # 4 L cap light
    (255, 255, 255),  # 5 W white glow
    (140, 170, 200),  # 6 s stem shade
    (225, 240, 250),  # 7 S stem
    (30, 90, 70),     # 8 g moss dark
    (60, 140, 100),   # 9 G moss
    (150, 130, 230),  # 10 v petal lavender
    (210, 200, 255),  # 11 p petal light
    (250, 248, 255),  # 12 P petal white
    (255, 225, 110),  # 13 y center gold
    (60, 140, 80),    # 14 b leaf
    (130, 200, 130),  # 15 B leaf light
]
GLOW_KEYS = {'.': 0, 'K': 1, 'c': 2, 'C': 3, 'L': 4, 'W': 5, 's': 6, 'S': 7, 'g': 8, 'G': 9,
             'v': 10, 'p': 11, 'P': 12, 'y': 13, 'b': 14, 'B': 15}

# The NPC 1 palette (graphics/object_events/palettes/npc_1.pal): the dim glowcap in its blues and olives
NPC_1 = [
    (115, 197, 164), (255, 213, 180), (255, 197, 148), (222, 148, 115), (123, 65, 65), (255, 164, 180),
    (213, 106, 123), (139, 65, 82), (148, 197, 246), (90, 139, 189), (16, 49, 82), (180, 164, 98),
    (123, 115, 65), (57, 57, 24), (255, 255, 255), (0, 0, 0),
]
DIM_KEYS = {'.': 0, 'K': 10, 'c': 10, 'C': 9, 'L': 9, 'W': 8, 's': 12, 'S': 11, 'g': 13, 'G': 12}

GLOWCAP = [
    '................',
    '................',
    '....KKKKK.......',
    '...KCLLLCK......',
    '..KCLWLCCCK.....',
    '..KCCCCWCCK.....',
    '..KcCCCCCcK.KKK.',
    '...KKsSSKK.KCLCK',
    '....KsSK...KcWcK',
    '....KsSK....KSK.',
    '....KsSK....KSK.',
    '....KsSK....KSK.',
    '...gKsSKg..gKSKg',
    '..gGGGGGGGGGGGg.',
    '...gggggggggggg.',
    '................',
]

STEM = [
    '.......KbK......',
    '..KK...KbK..KK..',
    '.KBbK..KbK.KbBK.',
    '.KBBbK.KbKKbBBK.',
    '..KBbbbKbbbbBK..',
    '...KKbbbbbbKK...',
    '.......KbK......',
    '.......KbK......',
    '.....gKbbKg.....',
    '...gGGGGGGGGg...',
    '....gggggggg....',
]

BUD = [
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '.......KK.......',
    '......KvpK......',
    '.....KvppPK.....',
    '.....KvpPpK.....',
    '....KBvppPbK....',
    '....KBBvpbbK....',
    '....KBBbbbbK....',
    '.....KBBbbK.....',
    '......KBbK......',
    '.......KK.......',
] + STEM

BLOOM = [
    '................',
    '................',
    '................',
    '................',
    '................',
    '......KKKK......',
    '....KKpPPpKK....',
    '...KpPPPPPPpK...',
    '..KvpPPyyPPpvK..',
    '.KvpPPyyyyPPpvK.',
    '.KvppPyyyyPppvK.',
    '..KvpPPyyPPpvK..',
    '..KvvpPPPPpvvK..',
    '...KKvvppvvKK...',
    '.....KKbbKK.....',
    '.......KbK......',
] + STEM


def rows(art, keys, height):
    art = art + ['.' * 16] * (height - len(art))
    for row in art:
        assert len(row) == 16, row
    return [[keys[c] for c in row] for row in art]


def plte(colors):
    return b''.join(bytes(c) for c in colors)


def write_pal(path, colors):
    open(path, 'w').write('JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in colors))


def main():
    write_png(os.path.join(PICS, 'glowcap_dim.png'), 16, rows(GLOWCAP, DIM_KEYS, 16), plte(NPC_1))
    write_png(os.path.join(PICS, 'glowcap_lit.png'), 16, rows(GLOWCAP, GLOW_KEYS, 16), plte(GLOW))
    write_png(os.path.join(PICS, 'moonveil_bud.png'), 16, rows(BUD, GLOW_KEYS, 32), plte(GLOW))
    write_png(os.path.join(PICS, 'moonveil_bloom.png'), 16, rows(BLOOM, GLOW_KEYS, 32), plte(GLOW))
    write_pal(os.path.join(PALETTES, 'night_bloom.pal'), GLOW)


if __name__ == '__main__':
    main()
