#!/usr/bin/env python3
"""Redraws the house stairs (gTileset_BuildingFrlg's, as in the Pallet Town house) for Celadon University's dorm
corridors (compose_dorm_floor.py), so they sit on the corridor's carpet and under its partition wall instead of on a
scrap of the house's wooden floor and wall.

Adds metatiles to gTileset_SafariZoneBuilding (the dorms' tileset), no new tiles: each is the house stair piece with
the house floor's tiles swapped, quadrant by quadrant, for the corridor carpet's (metatile 0x290) and the house wall's
for the partition wall's face (0x289). The stair art (rails, steps, the mat) is kept as it is. Re-running redraws them.

    python3 tools/hack/draw_dorm_stairs.py
"""
import os
import struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
PRIMARY = os.path.join(ROOT, 'data/tilesets/primary/building_frlg')
SAFARI = os.path.join(ROOT, 'data/tilesets/secondary/safari_zone_building_frlg')
SAFARI_METATILES = 128     # The tileset's own
PRIMARY_METATILES = 640

HOUSE_FLOOR = {0x003, 0x004, 0x005, 0x006, 0x013, 0x014, 0x015, 0x016}   # Tiles, palette 1
HOUSE_WALL = {0x002, 0x012}
CARPET, PARTITION = 0x290, 0x289

# The house stair pieces, in the order of the new metatiles (0x300 on)
PIECES = [
    0x015,  # 0x300 stair down: the wall over its top
    0x007,  # 0x301 stair down: its warp (the mat)
    0x00D,  # 0x302 stair down: the rail's foot, left
    0x00E,  # 0x303 stair down: the rail's foot, middle
    0x00F,  # 0x304 stair down: beside the rail's foot
    0x016,  # 0x305 stair up: the wall over its top, left
    0x017,  # 0x306 stair up: the wall over its top, right
    0x01D,  # 0x307 stair up: its warp (the mat)
    0x025,  # 0x308 stair up: below the warp
    0x026,  # 0x309 stair up: the rail's foot, left
    0x027,  # 0x30A stair up: the rail's foot, right
]
DORM_STAIRS = {piece: 0x280 + SAFARI_METATILES + i for i, piece in enumerate(PIECES)}


def read(folder, metatile_id, base):
    i = metatile_id - base
    layers = struct.unpack('<8H', open(os.path.join(folder, 'metatiles.bin'), 'rb').read()[i * 16:i * 16 + 16])
    attr = open(os.path.join(folder, 'metatile_attributes.bin'), 'rb').read()[i * 4:i * 4 + 4]
    return list(layers), attr


def main():
    carpet = read(SAFARI, CARPET, PRIMARY_METATILES)[0][:4]
    partition = read(SAFARI, PARTITION, PRIMARY_METATILES)[0][:4]
    data = open(os.path.join(SAFARI, 'metatiles.bin'), 'rb').read()[:SAFARI_METATILES * 16]
    attrs = open(os.path.join(SAFARI, 'metatile_attributes.bin'), 'rb').read()[:SAFARI_METATILES * 4]
    for piece in PIECES:
        layers, attr = read(PRIMARY, piece, 0)
        for k in range(4):  # The bottom layer's quadrants
            tile, pal = layers[k] & 0x3FF, layers[k] >> 12
            if pal == 1 and tile in HOUSE_FLOOR:
                layers[k] = carpet[k]
            elif pal == 1 and tile in HOUSE_WALL:
                layers[k] = partition[k]
        data += struct.pack('<8H', *layers)
        attrs += attr
    open(os.path.join(SAFARI, 'metatiles.bin'), 'wb').write(data)
    open(os.path.join(SAFARI, 'metatile_attributes.bin'), 'wb').write(attrs)
    print('Dorm stairs: ' + ', '.join('0x%03X->0x%03X' % kv for kv in DORM_STAIRS.items()))


if __name__ == '__main__':
    main()
