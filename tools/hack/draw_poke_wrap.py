#!/usr/bin/env python3
"""Draws the POKé WRAP's bag icon (ITEM_POKE_WRAP): KAI's dads' homemade wrap, rolled, with greens and a berry poking
out of the open end. A 24x24 4-bit icon and its palette:

    graphics/items/icons/poke_wrap.png
    graphics/items/icon_palettes/poke_wrap.pal

    python3 tools/hack/draw_poke_wrap.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import write_png  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))

PALETTE = [
    (180, 180, 180),  # 0 transparent
    (57, 41, 32),     # K outline
    (238, 206, 140),  # T tortilla
    (213, 164, 98),   # t tortilla shade
    (164, 111, 57),   # D toasted stripe
    (255, 240, 197),  # L highlight
    (98, 189, 65),    # G greens
    (41, 123, 49),    # g dark greens
    (222, 57, 57),    # R berry
    (148, 32, 41),    # r berry shade
    (255, 246, 230),  # W cream
] + [(0, 0, 0)] * 5
KEYS = {'.': 0, 'K': 1, 'T': 2, 't': 3, 'D': 4, 'L': 5, 'G': 6, 'g': 7, 'R': 8, 'r': 9, 'W': 10}

ART = [
    '........................',
    '........................',
    '...............g.G......',
    '.............gGGgGG.....',
    '............KGRGgGRgK...',
    '...........KgRrGGGRrK...',
    '..........KTKgGWGgGK....',
    '.........KTLTKGGgGK.....',
    '........KTLTTtKKKK......',
    '.......KTLTTtDTK........',
    '......KTLTTtDTTK........',
    '.....KTLTTtDTTtK........',
    '....KTLTTtDTTtK.........',
    '...KTLTTtDTTtK..........',
    '...KTTTtDTTtK...........',
    '...KTTtDTTtK............',
    '...KtTDTTtK.............',
    '...KttDTtK..............',
    '....KtttK...............',
    '.....KKK................',
    '........................',
    '........................',
    '........................',
    '........................',
]


def main():
    rows = []
    for row in ART:
        assert len(row) <= 24, row
        rows.append([KEYS[c] for c in row.ljust(24, '.')])
    plte = b''.join(bytes(c) for c in PALETTE)
    write_png(os.path.join(ROOT, 'graphics/items/icons/poke_wrap.png'), 24, rows, plte)
    open(os.path.join(ROOT, 'graphics/items/icon_palettes/poke_wrap.pal'), 'w').write(
        'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in PALETTE))


if __name__ == '__main__':
    main()
