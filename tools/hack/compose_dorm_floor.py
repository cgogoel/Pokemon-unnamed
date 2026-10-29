#!/usr/bin/env python3
"""Lays out Celadon University's dorm floors (2F and 3F): three bedrooms side by side over a shared corridor, stairs
down at the corridor's left end and (2F only) up at its right end.

Each bedroom is the player's dorm room (LAYOUT_CELADON_UNIVERSITY_DORM_ROOM, Lorelei's house) without its exit; black
walls part the rooms; the Safari Zone office's partition wall, with a doorway per room, closes them off from the
corridor. The stairs are the Pallet Town house's (primary tiles). A starting point to refine in Porymap: re-running it
overwrites both floors.

    python3 tools/hack/compose_dorm_floor.py
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10
ELEV = 3 << 12

ROOM_W, ROOM_H = 11, 8   # The dorm room's rows 0-7 (its row 8 is the outer wall and exit)
ROOMS = 3
W = ROOMS * ROOM_W + (ROOMS - 1)  # A black wall between rooms
H = 13
DOORWAY = (3, 4, 5)      # Each room's doorway columns, under its rug
RUG_ROW = 7              # The dorm room's rug (at its exit); moved to the doorway, against the corridor

VOID = 0x008 | SOLID     # Black outer wall
WALL_TOP, WALL_FACE = 0x281 | SOLID, 0x289 | SOLID  # Safari Zone office partition wall
CORRIDOR = 0x290 | ELEV
ROOM_FLOOR = 0x286 | ELEV


def load(layout_id):
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    l = layouts[layout_id]
    w, h = l['width'], l['height']
    path = os.path.join(ROOT, l['blockdata_filepath'])
    return w, h, list(struct.unpack('<%dH' % (w * h), open(path, 'rb').read())), path


def compose(up_stairs):
    rw, _, room, _ = load('LAYOUT_CELADON_UNIVERSITY_DORM_ROOM')
    grid = [VOID] * (W * H)

    def put(x, y, block):
        grid[y * W + x] = block

    for r in range(ROOMS):
        x0 = r * (ROOM_W + 1)
        for y in range(ROOM_H):
            for x in range(ROOM_W):
                put(x0 + x, y, room[y * rw + x])
        # The partition wall under the room, with its doorway; the rug lies in the doorway, against the corridor
        for x in range(ROOM_W):
            doorway = x in DOORWAY
            put(x0 + x, 8, ROOM_FLOOR if doorway else WALL_TOP)
            put(x0 + x, 9, room[RUG_ROW * rw + x] if doorway else WALL_FACE)
            if doorway:
                put(x0 + x, RUG_ROW, ROOM_FLOOR)
        if r < ROOMS - 1:
            put(x0 + ROOM_W, 8, WALL_TOP)
            put(x0 + ROOM_W, 9, WALL_FACE)
    for y in (10, 11):
        for x in range(W):
            put(x, y, CORRIDOR)

    # Stairs down at the left end (Pallet Town house 2F's), warp at (2,10)
    for x, y, block in ((0, 9, 0x015 | SOLID), (0, 10, 0x005 | SOLID), (1, 10, 0x006 | SOLID), (2, 10, 0x007 | ELEV),
                        (0, 11, 0x00D | SOLID), (1, 11, 0x00E | SOLID), (2, 11, 0x00F | ELEV)):
        put(x, y, block)
    # Stairs up at the right end (Pallet Town house 1F's), warp at (W-3,10)
    if up_stairs:
        x0 = W - 3
        for x, y, block in ((1, 9, 0x016 | SOLID), (2, 9, 0x017 | SOLID), (0, 10, 0x01D | ELEV), (1, 10, 0x01E | SOLID),
                            (2, 10, 0x01F | SOLID), (0, 11, 0x025 | ELEV), (1, 11, 0x026 | ELEV), (2, 11, 0x027 | ELEV)):
            put(x0 + x, y, block)
    return grid


def write(layout_id, grid):
    layouts_path = os.path.join(ROOT, 'data/layouts/layouts.json')
    data = json.load(open(layouts_path))
    for l in data['layouts']:
        if l.get('id') == layout_id:
            l['width'], l['height'] = W, H
            l['primary_tileset'], l['secondary_tileset'] = 'gTileset_BuildingFrlg', 'gTileset_SafariZoneBuilding'
            path = os.path.join(ROOT, l['blockdata_filepath'])
            border = os.path.join(ROOT, l['border_filepath'])
    open(layouts_path, 'w').write(json.dumps(data, indent=2) + '\n')
    open(path, 'wb').write(struct.pack('<%dH' % len(grid), *grid))
    open(border, 'wb').write(struct.pack('<4H', VOID, VOID, VOID, VOID))


def main():
    write('LAYOUT_CELADON_UNIVERSITY_DORM_2F', compose(up_stairs=True))
    write('LAYOUT_CELADON_UNIVERSITY_DORM_3F', compose(up_stairs=False))


if __name__ == '__main__':
    main()
