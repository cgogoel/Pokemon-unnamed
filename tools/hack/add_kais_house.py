#!/usr/bin/env python3
"""Adds KAI's family home to Cinnabar Island (docs/hack/act2_story.md, "The friends' families").

FireRed's Cinnabar has no houses, so this stamps the small route house of Kanto's general tileset (as on Route 2:
roof edge, roof, a window and the door on the right) into the corner between the Pokémon Mansion and the Gym, at
(12-16, 0-2). The door is (15, 2); the warp to MAP_CINNABAR_ISLAND_KAIS_HOUSE is in the map's map.json. Only these
blocks are written, so re-running is harmless. Fine-tuning (the shore at the top edge) is Porymap's.

    python3 tools/hack/add_kais_house.py
    python3 tools/hack/render_layout.py LAYOUT_CINNABAR_ISLAND out.png --map CinnabarIsland_Frlg --scale 2
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10

# The route house's blocks (metatile | collision | elevation), copied from LAYOUT_ROUTE2 (12-16, 20-22)
HOUSE = [
    [0x302C, 0x302D, 0x302D, 0x302D, 0x302F],  # the roof's top edge, drawn over grass
    [0x0434, 0x0436, 0x0436, 0x0435, 0x0437],  # the roof
    [0x043C, 0x0406, 0x0407, 0x043D, 0x043F],  # the wall: a window, then the door
]
LAYOUT, X, Y = 'LAYOUT_CINNABAR_ISLAND', 12, 0


def main():
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    l = layouts[LAYOUT]
    w, h = l['width'], l['height']
    path = os.path.join(ROOT, l['blockdata_filepath'])
    blocks = list(struct.unpack('<%dH' % (w * h), open(path, 'rb').read()))
    for dy, row in enumerate(HOUSE):
        for dx, block in enumerate(row):
            blocks[(Y + dy) * w + X + dx] = block
    open(path, 'wb').write(struct.pack('<%dH' % (w * h), *blocks))
    print('%s: KAI\'s house at (%d-%d, %d-%d)' % (LAYOUT, X, X + 4, Y, Y + 2))


if __name__ == '__main__':
    main()
