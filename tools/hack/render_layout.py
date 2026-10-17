#!/usr/bin/env python3
"""Renders a map layout to a PNG the way the game draws it (both metatile layers, the tilesets' palettes).

For previewing layouts made or changed outside Porymap. Optionally marks the map's events.

    python3 tools/hack/render_layout.py LAYOUT_ID out.png [--map MapFolder] [--scale N] [--grid] [--rect x0,y0,x1,y1]
"""
import json
import os
import re
import struct
import sys
import zlib

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
PRIMARY_FRLG = 640


def read_png(path):
    data = open(path, 'rb').read()
    pos, idat = 8, b''
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            width, height, depth = struct.unpack('>IIB', chunk[:9])
        elif kind == b'IDAT':
            idat += chunk
    raw = zlib.decompress(idat)
    stride = (width * depth + 7) // 8
    rows, prev, pos = [], bytearray(stride), 0
    for _ in range(height):
        kind, line = raw[pos], bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for x in range(stride):
            a = line[x - 1] if x else 0
            b = prev[x]
            c = prev[x - 1] if x else 0
            if kind == 1:
                line[x] = (line[x] + a) & 255
            elif kind == 2:
                line[x] = (line[x] + b) & 255
            elif kind == 3:
                line[x] = (line[x] + (a + b) // 2) & 255
            elif kind == 4:
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                line[x] = (line[x] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
        prev = line
        if depth == 4:
            rows.append([(line[x // 2] >> (4 * (1 - x % 2))) & 15 for x in range(width)])
        else:
            rows.append(list(line[:width]))
    return rows


def write_png(path, image, scale=1):
    raw = b''
    for row in image:
        line = b'\x00' + b''.join(bytes(c) * scale for c in row)
        raw += line * scale

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n'
                + chunk(b'IHDR', struct.pack('>IIBBBBB', len(image[0]) * scale, len(image) * scale, 8, 2, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))


def tileset_folder(symbol):
    """data/tilesets/... folder of a gTileset_* symbol (via headers.h and metatiles.h)."""
    headers = open(os.path.join(ROOT, 'src/data/tilesets/headers.h')).read()
    block = headers[headers.index('const struct Tileset %s =' % symbol):]
    metatiles = re.search(r'\.metatiles = (\w+)', block).group(1)
    source = open(os.path.join(ROOT, 'src/data/tilesets/metatiles.h')).read()
    path = re.search(r'%s\[\] = INCBIN_U16\("([^"]+)/metatiles.bin"\)' % metatiles, source).group(1)
    return os.path.join(ROOT, path)


def load_tileset(folder):
    def pal(i):
        lines = open(os.path.join(folder, 'palettes/%02d.pal' % i)).read().split('\n')
        return [tuple(map(int, line.split())) for line in lines[3:19]]
    tiles = read_png(os.path.join(folder, 'tiles.png'))
    data = open(os.path.join(folder, 'metatiles.bin'), 'rb').read()
    metatiles = [struct.unpack('<8H', data[i:i + 16]) for i in range(0, len(data), 16)]
    return tiles, [pal(i) for i in range(16)], metatiles


def load_layout(layout_id):
    layouts = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    layout = layouts[layout_id]
    w, h = layout['width'], layout['height']
    blocks = struct.unpack('<%dH' % (w * h), open(os.path.join(ROOT, layout['blockdata_filepath']), 'rb').read())
    return layout, w, h, blocks


def render(layout_id, rect=None):
    layout, w, h, blocks = load_layout(layout_id)
    ptiles, ppals, pmeta = load_tileset(tileset_folder(layout['primary_tileset']))
    stiles, spals, smeta = load_tileset(tileset_folder(layout['secondary_tileset']))
    palettes = [ppals[i] if i < 7 else spals[i] for i in range(16)]
    x0, y0, x1, y1 = rect or (0, 0, w, h)

    def tile(t):
        rows, t = (ptiles, t) if t < PRIMARY_FRLG else (stiles, t - PRIMARY_FRLG)
        n = len(rows[0]) // 8
        if (t // n) * 8 >= len(rows):
            return None
        return [rows[(t // n) * 8 + y][(t % n) * 8:(t % n) * 8 + 8] for y in range(8)]

    image = [[palettes[0][0]] * ((x1 - x0) * 16) for _ in range((y1 - y0) * 16)]
    for my in range(y0, y1):
        for mx in range(x0, x1):
            mt = blocks[my * w + mx] & 0x3FF
            entries = pmeta[mt] if mt < PRIMARY_FRLG else smeta[mt - PRIMARY_FRLG]
            for layer in (0, 1):
                for k in range(4):
                    e = entries[layer * 4 + k]
                    pixels = tile(e & 0x3FF)
                    if pixels is None:
                        continue
                    hflip, vflip, p = e >> 10 & 1, e >> 11 & 1, e >> 12
                    for yy in range(8):
                        for xx in range(8):
                            c = pixels[7 - yy if vflip else yy][7 - xx if hflip else xx]
                            if c == 0 and layer == 1:
                                continue
                            image[(my - y0) * 16 + (k // 2) * 8 + yy][(mx - x0) * 16 + (k % 2) * 8 + xx] = \
                                palettes[0][0] if c == 0 else palettes[p][c]
    return image, (x0, y0)


def mark(image, origin, x, y, color):
    ox, oy = origin
    px, py = (x - ox) * 16, (y - oy) * 16
    if not (0 <= py < len(image) and 0 <= px < len(image[0])):
        return
    for i in range(16):
        for (a, b) in ((px + i, py), (px + i, py + 15), (px, py + i), (px + 15, py + i)):
            if 0 <= b < len(image) and 0 <= a < len(image[0]):
                image[b][a] = color


def main():
    layout_id, out = sys.argv[1], sys.argv[2]
    scale = int(sys.argv[sys.argv.index('--scale') + 1]) if '--scale' in sys.argv else 1
    rect = tuple(map(int, sys.argv[sys.argv.index('--rect') + 1].split(','))) if '--rect' in sys.argv else None
    image, origin = render(layout_id, rect)
    if '--grid' in sys.argv:
        for y in range(len(image)):
            for x in range(len(image[0])):
                if x % 160 == 0 or y % 160 == 0:
                    image[y][x] = (255, 0, 255)
    if '--map' in sys.argv:
        m = json.load(open(os.path.join(ROOT, 'data/maps', sys.argv[sys.argv.index('--map') + 1], 'map.json')))
        for o in m.get('object_events') or []:
            mark(image, origin, o['x'], o['y'], (255, 0, 255))
        for w in m.get('warp_events') or []:
            mark(image, origin, w['x'], w['y'], (255, 0, 0))
        for c in m.get('coord_events') or []:
            mark(image, origin, c['x'], c['y'], (255, 160, 0))
        for b in m.get('bg_events') or []:
            mark(image, origin, b['x'], b['y'], (0, 160, 255))
    write_png(out, image, scale)


if __name__ == '__main__':
    main()
