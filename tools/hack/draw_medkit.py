#!/usr/bin/env python3
"""Draws the ranger's MEDKIT on the ranger station's B2F (Act 2 finale, docs/hack/act2_finale.md).

    medkit.png   A red first-aid case with a white cross, a carry handle and a latch, in the NPC 1 palette.
                 OBJ_EVENT_GFX_MEDKIT, 16x16.

    python3 tools/hack/draw_medkit.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import write_png  # noqa: E402
from draw_night_bloom import NPC_1, PICS, plte, rows  # noqa: E402

# NPC 1: K navy outline, R red, r dark red, p pink (the lit edge), W white, w the cross's shade (light blue), h the
# handle's grey-brown
KEYS = {'.': 0, 'K': 10, 'R': 6, 'r': 7, 'p': 5, 'W': 14, 'w': 8, 'h': 12}

MEDKIT = [
    '................',
    '................',
    '.....KKKKKK.....',
    '....KhKKKKhK....',
    '....Kh....hK....',
    '.KKKKKKKKKKKKKK.',
    'KppppppWWpppppRK',
    'KpRRRRRWWRRRRRrK',
    'KpRRRWWWWWWRRRrK',
    'KpRRRwwWWwwRRRrK',
    'KpRRRRRWwRRRRRrK',
    'KrrrrrrwwrrrrrrK',
    'KKKKKKKKKKKKKKKK',
    'KrrrrrrKKrrrrrrK',
    '.KKKKKKKKKKKKKK.',
    '................',
]


def main():
    write_png(os.path.join(PICS, 'medkit.png'), 16, rows(MEDKIT, KEYS, 16), plte(NPC_1))
    print('medkit.png')


if __name__ == '__main__':
    main()
