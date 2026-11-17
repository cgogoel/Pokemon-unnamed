#!/usr/bin/env python3
"""Draws the explorers' notes left round the ranger station's B1F (Act 2 finale, docs/hack/act2_finale.md).

    station_note.png   A sheet of paper, a little curled, lines of handwriting on it, in the NPC 1 palette.
                       OBJ_EVENT_GFX_STATION_NOTE, 16x16.

    python3 tools/hack/draw_station_note.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import write_png  # noqa: E402
from draw_night_bloom import NPC_1, PICS, plte, rows  # noqa: E402

# NPC 1: K navy outline, W white, w the paper's shade (light blue), h the handwriting (grey-brown)
KEYS = {'.': 0, 'K': 10, 'W': 14, 'w': 8, 'h': 12}

NOTE = [
    '................',
    '................',
    '................',
    '....KKKKKKKK....',
    '...KWWWWWWWWKK..',
    '...KWhhWhhhWWK..',
    '...KWWWWWWWWwK..',
    '..KWhhhWhhWWwK..',
    '..KWWWWWWWWWK...',
    '..KWhhWhhhhwK...',
    '..KWWWWWWWWwK...',
    '..KwWhhhWwwK....',
    '...KwwwwwwK.....',
    '....KKKKKK......',
    '................',
    '................',
]


def main():
    write_png(os.path.join(PICS, 'station_note.png'), 16, rows(NOTE, KEYS, 16), plte(NPC_1))
    print('station_note.png')


if __name__ == '__main__':
    main()
