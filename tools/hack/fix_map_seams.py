#!/usr/bin/env python3
"""Targeted fixes to hand-edited hack maps where a stamped or painted piece was cut off or didn't match its
surroundings. These maps were refined in Porymap after their composers ran, so re-running a composer would undo that
work; this patches only the listed blocks. Each fix sets exact blocks (metatile, collision, elevation), so re-running
is harmless.

    python3 tools/hack/fix_map_seams.py
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10


def blk(metatile, solid=False, elevation=3):
    return metatile | (SOLID if solid else 0) | (elevation << 12)


def campus_grass(x, y):
    """Celadon University's lawn, in compose_university.py's checkerboard."""
    return blk(((0x009, 0x008), (0x011, 0x010))[y % 2][x % 2])


def fixes():
    out = {}
    # The barn: the area around the room was a pale filler metatile (0x137); the void around a room is black
    out['LAYOUT_PALLET_TOWN_FARM_BARN'] = [('replace', 0x137, blk(0x008, True, 0))]
    # Campus: the dorm towers' stamps (Celadon Condominiums' top row) brought the bottoms of the trees that stand behind
    # the condominiums in Celadon, without the trees: lawn instead. The gym's stamp brought the bottom edge of
    # Celadon's paving above it, a pale strip leading nowhere: lawn instead.
    campus = []
    for x0 in (36, 43):
        for dx in (0, 1, 5, 6):
            campus.append(('set', x0 + dx, 37, campus_grass(x0 + dx, 37)))
    for x in range(50, 58):
        campus.append(('set', x, 33, campus_grass(x, 33)))
    out['LAYOUT_CELADON_UNIVERSITY'] = campus
    # The Woods: the ranger station's roof ran off the top of the map without its ridge; it ends on the ridge now
    # (Route 2's gate: 044 left, 045 middle, 046 right)
    woods = [('set', 0, 0, blk(0x044, True, 0)), ('set', 10, 0, blk(0x046, True, 0))]
    woods += [('set', x, 0, blk(0x045, True, 0)) for x in range(1, 10)]
    out['LAYOUT_WOODS'] = woods
    return out


def main():
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    for layout_id, changes in fixes().items():
        l = layouts[layout_id]
        w, h = l['width'], l['height']
        path = os.path.join(ROOT, l['blockdata_filepath'])
        blocks = list(struct.unpack('<%dH' % (w * h), open(path, 'rb').read()))
        n = 0
        for change in changes:
            if change[0] == 'replace':
                _, metatile, block = change
                for i, b in enumerate(blocks):
                    if (b & 0x3FF) == metatile:
                        blocks[i] = block
                        n += 1
            else:
                _, x, y, block = change
                if blocks[y * w + x] != block:
                    blocks[y * w + x] = block
                    n += 1
        open(path, 'wb').write(struct.pack('<%dH' % len(blocks), *blocks))
        print('%s: %d blocks changed' % (layout_id, n))


if __name__ == '__main__':
    main()
