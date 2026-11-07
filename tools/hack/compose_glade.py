#!/usr/bin/env python3
"""Lays out the night bloom's glade (Act 2's Grass course, docs/hack/act2_story.md), off Viridian Forest.

A moonlit clearing in the forest's own trees (gTileset_General_Frlg + gTileset_ViridianForest): columns of trees three
squares wide, as in Viridian Forest, around a clearing with the MOONVEIL in the middle. Three gaps in the trees, west,
north and east, are where the BUG POKéMON come in (a wave each); the south gap is the way back to the forest.

Also writes src/data/night_bloom_layout.h, the squares the C side needs (src/act2_night_bloom.c):
    - the hedge each wave's gap gets when the player plants one (round bushes, 0x005);
    - Viridian Forest's hidden opening: a tree column of its north edge that becomes a path once the third ODDISH
      shows the way (x 39-41, rows 0-4; LAYOUT_VIRIDIAN_FOREST itself isn't changed).

    python3 tools/hack/compose_glade.py
"""
import json
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOLID = 1 << 10

W, H = 21, 16
GAP_COLUMN = 3                          # x 9-11: the north gap and the south way out
SIDE_GAP_ROWS = (7, 8)                  # The west and east gaps, in the outer columns
BAND_BOTTOM = 4                         # The top band of trees ends on this row (its trunks)
BOTTOM_TOP = 12                         # The bottom band of trees starts on this row
MOONVEIL = (10, 8)
HEDGES = {                              # A wave's gap: its hedge squares (inner mouth of the gap)
    1: [(2, 7), (2, 8)],                # West
    2: [(9, 4), (10, 4), (11, 4)],      # North
    3: [(18, 7), (18, 8)],              # East
}
HEDGE = 0x005
FOREST_OPENING = (39, 0, 3, 5)          # x, y, width, height in Viridian Forest

TOP = [0x288, 0x299, 0x28A]
CANOPY_A = [0x298, 0x299, 0x29A]
CANOPY_B = [0x290, 0x291, 0x292]
TRUNK_1 = [0x2A0, 0x2A1, 0x2A2]
TRUNK_2 = [0x2A3, 0x2A4, 0x2A5]
GRASS = [[0x008, 0x009], [0x010, 0x011]]
FLOWERS = 0x282
TUFT = 0x281


def blk(metatile, solid=False, elevation=3):
    return metatile | (SOLID if solid else 0) | (elevation << 12)


def tree(row, x):
    return blk(row[x % 3], True, 0)


def grass(x, y):
    return blk(GRASS[y % 2][x % 2])


def canopy_down(y, end):
    """Canopy rows of a column that ends (trunks) at row end: A on rows of end's parity minus 2, B between."""
    return CANOPY_A if (end - 2 - y) % 2 == 0 else CANOPY_B


def column(x, y):
    """The block at (x, y): trees, or None for the clearing's grass."""
    c = x // 3
    if c == GAP_COLUMN:
        return None
    if c in (0, 6):
        top, bottom = SIDE_GAP_ROWS
        if y < top:
            # Down from the forest above, ending in trunks just above the gap
            if y == top - 2:
                return tree(TRUNK_1, x)
            if y == top - 1:
                return tree(TRUNK_2, x)
            return tree(canopy_down(y, top - 2), x)
        if y <= bottom:
            return None
        if y == bottom + 1:
            return tree(TOP, x)
        return tree(CANOPY_B if (y - bottom) % 2 == 0 else CANOPY_A, x)
    if y <= BAND_BOTTOM:
        if y == BAND_BOTTOM - 1:
            return tree(TRUNK_1, x)
        if y == BAND_BOTTOM:
            return tree(TRUNK_2, x)
        return tree(canopy_down(y, BAND_BOTTOM - 1), x)
    if y >= BOTTOM_TOP:
        if y == BOTTOM_TOP:
            return tree(TOP, x)
        return tree(CANOPY_B if (y - BOTTOM_TOP) % 2 == 1 else CANOPY_A, x)
    return None


def compose():
    cells = []
    for y in range(H):
        for x in range(W):
            b = column(x, y)
            if b is None:
                b = grass(x, y)
                # A ring of flowers around the MOONVEIL, a few tufts elsewhere
                d = abs(x - MOONVEIL[0]) + abs(y - MOONVEIL[1])
                if d == 3:
                    b = blk(FLOWERS)
                elif (x * 7 + y * 3) % 13 == 0 and 3 <= x <= 17 and 5 <= y <= 11 and d > 3:
                    b = blk(TUFT)
            cells.append(b)
    return cells


def forest_opening():
    x0, y0, w, h = FOREST_OPENING
    return [(x0 + i, y0 + j, grass(x0 + i, y0 + j)) for j in range(h) for i in range(w)]


def write_layout(cells):
    path = os.path.join(ROOT, 'data/layouts/layouts.json')
    data = json.load(open(path))
    folder = 'ViridianForest_Glade'
    entry = {
        'id': 'LAYOUT_VIRIDIAN_FOREST_GLADE', 'name': folder + '_Layout', 'width': W, 'height': H,
        'border_width': 3, 'border_height': 2, 'primary_tileset': 'gTileset_General_Frlg',
        'secondary_tileset': 'gTileset_ViridianForest',
        'border_filepath': 'data/layouts/%s/border.bin' % folder,
        'blockdata_filepath': 'data/layouts/%s/map.bin' % folder, 'layout_version': 'frlg',
    }
    for i, l in enumerate(data['layouts']):
        if l.get('id') == entry['id']:
            data['layouts'][i] = entry
            break
    else:
        data['layouts'].append(entry)
    open(path, 'w').write(json.dumps(data, indent=2) + '\n')
    os.makedirs(os.path.join(ROOT, 'data/layouts', folder), exist_ok=True)
    open(os.path.join(ROOT, entry['blockdata_filepath']), 'wb').write(struct.pack('<%dH' % len(cells), *cells))
    border = [tree(CANOPY_A, x) for x in range(3)] + [tree(CANOPY_B, x) for x in range(3)]
    open(os.path.join(ROOT, entry['border_filepath']), 'wb').write(struct.pack('<6H', *border))


def write_header():
    lines = [
        '// Generated by tools/hack/compose_glade.py: the night bloom\'s squares (docs/hack/act2_story.md)',
        '',
        'struct NightBloomSquare',
        '{',
        '    u8 x, y;',
        '    u16 block;',
        '};',
        '',
        '// The glade: each wave\'s hedge (waves 1-3: west, north, east)',
        'static const struct NightBloomSquare sGladeHedges[][3] = {',
    ]
    for wave in sorted(HEDGES):
        squares = HEDGES[wave] + [HEDGES[wave][-1]] * (3 - len(HEDGES[wave]))
        lines.append('    {%s},' % ', '.join('{%d, %d, 0x%04X}' % (x, y, blk(HEDGE, True)) for x, y in squares))
    lines += [
        '};',
        '',
        '// Viridian Forest: the hidden opening into the glade, once the third ODDISH shows the way',
        'static const struct NightBloomSquare sForestOpening[] = {',
    ] + ['    {%d, %d, 0x%04X},' % s for s in forest_opening()] + ['};', '']
    open(os.path.join(ROOT, 'src/data/night_bloom_layout.h'), 'w').write('\n'.join(lines))


def main():
    write_layout(compose())
    write_header()


if __name__ == '__main__':
    main()
