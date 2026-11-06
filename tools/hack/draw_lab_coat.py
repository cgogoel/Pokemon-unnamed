#!/usr/bin/env python3
"""Draws the player's Act 3 sprite: the Silph worker (the player's walk sprite from Act 2) in a long white lab coat,
open at the front over his khaki uniform.

Each of the worker's 9 walk frames keeps its head (hair, face); below the chin, the body is the FRLG scientist's
(OBJ_EVENT_GFX_SCIENTIST) long white coat from the same frame, which lines up with the worker's pose frame for frame.
In the front views the coat is opened: the worker's khaki uniform shows down the middle, between grey edges.

Both sprites use npc_white.pal (white 14, light greys 11-13 for the coat), so no palette changes: the player
already loads npc_white into the player's slot from Act 2.

    python3 tools/hack/draw_lab_coat.py              # writes graphics/object_events/pics/people/player_lab_coat.png
    python3 tools/hack/draw_lab_coat.py preview.png  # also writes a scaled-up preview: the worker above, the coat below
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from draw_mansion_furnace import read_png, write_png  # noqa: E402

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
PEOPLE = os.path.join(ROOT, 'graphics/object_events/pics/people')
SOURCE = os.path.join(PEOPLE, 'worker_m.png')
COAT = os.path.join(PEOPLE, 'scientist.png')
OUTPUT = os.path.join(PEOPLE, 'player_lab_coat.png')
PALETTE = os.path.join(ROOT, 'graphics/object_events/palettes/npc_white.pal')
FRAMES = 9

# npc_white.pal
KHAKI, KHAKI_DARK = 6, 7
LIGHT_GREY, WHITE = 11, 14

# Per frame: the first row taken from the scientist (rows above it are the worker's head), and the rows where the
# coat hangs open in the front views (the worker's khaki uniform shows between its edges, columns 7-8).
FRAME_SHAPE = {
    0: (22, range(26, 29)),
    1: (22, ()),
    2: (23, ()),
    3: (23, range(27, 30)),
    4: (23, range(27, 30)),
    5: (22, ()),
    6: (22, ()),
    7: (23, ()),
    8: (23, ()),
}
# In the walking front views the arms swing up beside the chin: they come from the scientist one row higher
ARMS = {3: (22, (0, 1, 2, 3)), 4: (22, (12, 13, 14, 15))}
OPEN_COLS = (7, 8)


def draw_frame(worker, scientist, n):
    top, open_rows = FRAME_SHAPE[n]
    out = [row[:] for row in worker[:top]] + [row[:] for row in scientist[top:]]
    if n in ARMS:
        y, cols = ARMS[n]
        for x in cols:
            out[y][x] = scientist[y][x]
    for i, y in enumerate(open_rows):
        for x in OPEN_COLS:
            out[y][x] = KHAKI if i == 0 else KHAKI_DARK
        for x in (OPEN_COLS[0] - 1, OPEN_COLS[-1] + 1):
            if out[y][x] == WHITE:
                out[y][x] = LIGHT_GREY
    return out


def main():
    _, rows, plte = read_png(SOURCE)
    _, coat_rows, _ = read_png(COAT)
    frames = [[row[n * 16:(n + 1) * 16] for row in rows] for n in range(FRAMES)]
    coat_frames = [[row[n * 16:(n + 1) * 16] for row in coat_rows] for n in range(FRAMES)]
    coats = [draw_frame(f, coat_frames[n], n) for n, f in enumerate(frames)]
    joined = [sum((c[y] for c in coats), []) for y in range(32)]
    write_png(OUTPUT, FRAMES * 16, joined, plte)
    print('wrote', os.path.relpath(OUTPUT, ROOT))

    if len(sys.argv) > 1:
        scale = 8
        pal = [line.split() for line in open(PALETTE).read().splitlines()[3:19]]
        pal[0] = ['160', '200', '160']  # Transparent shows as pale green
        preview_plte = bytes(int(v) for color in pal for v in color)
        before = [sum((f[y] for f in frames), []) for y in range(32)]
        both = before + [[0] * (FRAMES * 16)] * 4 + joined
        big = [[row[x // scale] for x in range(FRAMES * 16 * scale)] for row in both for _ in range(scale)]
        write_png(sys.argv[1], FRAMES * 16 * scale, big, preview_plte)
        print('wrote', sys.argv[1])


if __name__ == '__main__':
    main()
