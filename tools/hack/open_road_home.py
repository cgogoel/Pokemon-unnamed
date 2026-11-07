#!/usr/bin/env python3
"""Opens Kanto's maps for Act 2's road home on foot in both directions (docs/hack/act2_finale.md, "The road home").

The base game's maps assume the player walks Kanto in its own order; some one-way ledges then shut the way home from
the university. This patches only the listed blocks (metatile, collision, elevation), so re-running is harmless.

    python3 tools/hack/open_road_home.py
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10


def blk(metatile, solid=False, elevation=3):
    return metatile | (SOLID if solid else 0) | (elevation << 12)


def fixes():
    out = {}
    # Route 4, Cerulean City to Mt. Moon: past the grass, every way west was a ledge facing south (one way, toward
    # Cerulean). A gap at the west end of the ledge row under the trees at (86-89, 9-11), like the row's own gap at
    # (41-43, 9): grass at (90, 9), the row's left cap (0B0) moved to (91, 9).
    out['LAYOUT_ROUTE4'] = [
        ('set', 90, 9, blk(0x010)),
        ('set', 91, 9, blk(0x0B0, True, 0)),
    ]
    return out


def main():
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    for layout_id, changes in fixes().items():
        l = layouts[layout_id]
        w, h = l['width'], l['height']
        path = os.path.join(ROOT, l['blockdata_filepath'])
        blocks = list(struct.unpack('<%dH' % (w * h), open(path, 'rb').read()))
        for _, x, y, block in changes:
            blocks[y * w + x] = block
        open(path, 'wb').write(struct.pack('<%dH' % (w * h), *blocks))
        print('%s: %d blocks' % (layout_id, len(changes)))


if __name__ == '__main__':
    main()
