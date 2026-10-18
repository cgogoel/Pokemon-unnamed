#!/usr/bin/env python3
"""Draws the Pokémon Mansion's furnaces (Act 2's Fire puzzle) into its secondary tileset: a brick furnace with an
iron hood, cold (an empty firebox with ash) and lit (flames inside).

Adds palette 09 (unused before), 6 tiles (a new row at the end of tiles.png) and two metatiles after the tileset's
own: METATILE_PokemonMansion_Furnace_Cold and _Lit, both drawn over the wall of the building tileset's metatile
0x061. Re-running redraws them in place.

    python3 tools/hack/draw_mansion_furnace.py
"""
import os
import struct
import zlib

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
TILESET = os.path.join(ROOT, 'data/tilesets/secondary/pokemon_mansion_frlg')
PRIMARY_TILES = 640
PRIMARY_METATILES = 640
BASE_METATILES = 258      # The tileset's own metatiles, before the furnaces
BASE_TILE_ROWS = 22       # tiles.png's rows of 16 tiles, before the furnaces
PAL = 9
WALL_METATILE = 0x061     # Building tileset: its bottom layer is the wall behind the furnace

PALETTE = [
    (0, 0, 0),        # 0 transparent
    (32, 32, 40),     # K outline
    (82, 82, 106),    # d dark iron
    (123, 123, 139),  # i iron
    (180, 180, 197),  # l iron, lit side
    (106, 41, 32),    # b dark brick
    (164, 74, 49),    # r brick
    (213, 123, 90),   # p brick, lit side
    (24, 16, 16),     # o the firebox
    (197, 32, 16),    # R flame, red
    (255, 123, 16),   # O flame, orange
    (255, 213, 49),   # Y flame, yellow
    (255, 246, 197),  # W flame, white-hot
    (90, 58, 49),     # m mortar
    (164, 164, 156),  # a ash
    (255, 255, 255),
]
KEYS = '.KdilbrpoROYWma'

COLD = [
    '.KKKKKKKKKKKKKK.',
    'KllllllllllllllK',
    'KiiiiiiiiiiiiiiK',
    'KddddddddddddddK',
    'KrrprrmrrprrmrrK',
    'KmmmmmmmmmmmmmmK',
    'KrmrrKKKKKKrrmrK',
    'KmrrKooooooKrrmK',
    'KrrKooooooooKrrK',
    'KmmKooooooooKmmK',
    'KrpKooooooooKprK',
    'KrrKooooooooKrrK',
    'KmmKoaoaaoaoKmmK',
    'KrrKdldldldlKrrK',
    'KbbbbbbbbbbbbbbK',
    'KKKKKKKKKKKKKKKK',
]
LIT = COLD[:7] + [
    'KmrrKoRooRoKrrmK',
    'KrrKoRORoRRoKrrK',
    'KmmKROOROORRKmmK',
    'KrpKROYOOYORKprK',
    'KrrKOYWYYWYOKrrK',
    'KmmKYWWWWWWYKmmK',
    'KrrKdOdOdOdOKrrK',
] + COLD[14:]


def read_png(path):
    data = open(path, 'rb').read()
    pos, idat, chunks = 8, b'', {}
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IDAT':
            idat += chunk
        else:
            chunks[kind] = chunk
    width, height, depth, color = struct.unpack('>IIBB', chunks[b'IHDR'][:10])
    assert depth == 4 and color == 3, 'expected a 4-bit indexed PNG'
    raw = zlib.decompress(idat)
    stride = width // 2
    rows, prev, pos = [], bytearray(stride), 0
    for _ in range(height):
        kind, line = raw[pos], bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for x in range(stride):
            a = line[x - 1] if x else 0
            b = prev[x]
            c = prev[x - 1] if x else 0
            if kind == 1:
                line[x] = (line[x] + a) & 0xFF
            elif kind == 2:
                line[x] = (line[x] + b) & 0xFF
            elif kind == 3:
                line[x] = (line[x] + (a + b) // 2) & 0xFF
            elif kind == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        rows.append([v for byte in line for v in (byte >> 4, byte & 0xF)])
        prev = line
    return width, rows, chunks.get(b'PLTE')


def write_png(path, width, rows, plte):
    def chunk(kind, body):
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', zlib.crc32(kind + body) & 0xFFFFFFFF)
    raw = b''.join(b'\0' + bytes((r[x] << 4) | r[x + 1] for x in range(0, width, 2)) for r in rows)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, len(rows), 4, 3, 0, 0, 0))
    if plte:
        png += chunk(b'PLTE', plte)
    open(path, 'wb').write(png + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def pixels(art):
    return [[KEYS.index(c) for c in row] for row in art]


def main():
    open(os.path.join(TILESET, 'palettes/%02d.pal' % PAL), 'w').write(
        'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in PALETTE))

    # Tiles: cold top-left, top-right, bottom-left, bottom-right, then lit bottom-left, bottom-right
    cold, lit = pixels(COLD), pixels(LIT)
    quads = [(cold, 0, 0), (cold, 8, 0), (cold, 0, 8), (cold, 8, 8), (lit, 0, 8), (lit, 8, 8)]
    png = os.path.join(TILESET, 'tiles.png')
    width, rows, plte = read_png(png)
    rows = rows[:BASE_TILE_ROWS * 8] + [[0] * width for _ in range(8)]
    for n, (art, x0, y0) in enumerate(quads):
        for y in range(8):
            for x in range(8):
                rows[BASE_TILE_ROWS * 8 + y][n * 8 + x] = art[y0 + y][x0 + x]
    write_png(png, width, rows, plte)
    first_tile = PRIMARY_TILES + BASE_TILE_ROWS * 16

    # Metatiles: the wall's bottom layer, the furnace on top
    primary = open(os.path.join(ROOT, 'data/tilesets/primary/building_frlg/metatiles.bin'), 'rb').read()
    wall = list(struct.unpack('<4H', primary[WALL_METATILE * 16:WALL_METATILE * 16 + 8]))
    attrs = open(os.path.join(ROOT, 'data/tilesets/primary/building_frlg/metatile_attributes.bin'), 'rb').read()
    wall_attr = attrs[WALL_METATILE * 4:WALL_METATILE * 4 + 4]

    def top(*tiles):
        return [(first_tile + t) | (PAL << 12) for t in tiles]
    metatiles_path = os.path.join(TILESET, 'metatiles.bin')
    attrs_path = os.path.join(TILESET, 'metatile_attributes.bin')
    metatiles = open(metatiles_path, 'rb').read()[:BASE_METATILES * 16]
    for layers in (wall + top(0, 1, 2, 3), wall + top(0, 1, 4, 5)):
        metatiles += struct.pack('<8H', *layers)
    open(metatiles_path, 'wb').write(metatiles)
    open(attrs_path, 'wb').write(open(attrs_path, 'rb').read()[:BASE_METATILES * 4] + wall_attr * 2)
    print('METATILE_PokemonMansion_Furnace_Cold 0x%03X, _Lit 0x%03X'
          % (PRIMARY_METATILES + BASE_METATILES, PRIMARY_METATILES + BASE_METATILES + 1))


if __name__ == '__main__':
    main()
