#!/usr/bin/env python3
"""Draws the lone ranger's notes left round the ranger station's B1F, and the ranger (Act 2 finale,
docs/hack/act2_finale.md). In the NPC 1 palette.

    station_note.png     A sheet of paper, a little curled, lines of handwriting on it. OBJ_EVENT_GFX_STATION_NOTE,
                         16x16.
    ranger_remains.png   The ranger's skeleton, slumped against the study's wall in their faded uniform, the cap
                         knocked askew, the last note held in both hands on their lap. OBJ_EVENT_GFX_RANGER_REMAINS,
                         16x32 (its top half over the wall behind it).

    python3 tools/hack/draw_station_note.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import write_png  # noqa: E402
from draw_night_bloom import NPC_1, PICS, plte, rows  # noqa: E402

# NPC 1: K navy outline, W white, w the paper's shade (light blue), h the handwriting (grey-brown); for the ranger
# also b the bones' deep shade (blue), u the uniform (khaki), o its shade (olive), O its deep shade, d the boots
KEYS = {'.': 0, 'K': 10, 'W': 14, 'w': 8, 'h': 12, 'b': 9, 'u': 11, 'o': 12, 'O': 13, 'd': 4}

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

# Sitting on the floor, back to the wall, knees up, hunched: the skull sunk between the shoulders under the cap, the jacket hanging
# open on the ribs, bony hands holding the note. Lit from the upper left like the rest.
REMAINS = [
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '................',
    '.......KKKK.....',
    '......KuuuuK....',
    '.....KuuuuuoK...',
    '....KKOOOOOOKK..',
    '....KWWWWWwK....',
    '...KWKKWKKwK....',
    '...KWKKWKKbK....',
    '...KWWWbWwbK....',
    '..KKKWKWKbKKK...',
    '..KuuKWbWKuoK...',
    '..KuoKbWbKooK...',
    '.KuooKWbWKoOOK..',
    '.KWbKKKKKKKKWbK.',
    '.KWWWWWWWWwWWbK.',
    '..KWhhWhhhWwK...',
    '..KWWWWWWWwwK...',
    '.KuuKKKKKKKuoK..',
    'KuuuoK...KuuooK.',
    'KuooOK...KuoOOK.',
    'KdddddK.KdddddK.',
    '.KKKKK...KKKKK..',
]


def main():
    write_png(os.path.join(PICS, 'station_note.png'), 16, rows(NOTE, KEYS, 16), plte(NPC_1))
    write_png(os.path.join(PICS, 'ranger_remains.png'), 16, rows(REMAINS, KEYS, 32), plte(NPC_1))
    print('station_note.png, ranger_remains.png')


if __name__ == '__main__':
    main()
