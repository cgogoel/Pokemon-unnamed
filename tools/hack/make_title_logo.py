#!/usr/bin/env python3
"""Builds the title screen logo: FireRed's "POKéMON" with "Unnamed" in place of "FireRed Version".

Reads graphics/title_screen_frlg/firered/game_title_logo.{png,bin}, renders the subtitle with a system font
(tools/hack/render_text.swift, macOS), styles it like the original (white, grey edges, black outline) and writes
graphics/title_screen_frlg/unnamed/game_title_logo.{png,bin}. The palette is FireRed's, unchanged.

    python3 tools/hack/make_title_logo.py [--preview out.png]
"""
import os
import struct
import subprocess
import sys
import tempfile
import zlib

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'graphics/title_screen_frlg/firered')
OUT = os.path.join(ROOT, 'graphics/title_screen_frlg/unnamed')

SUBTITLE = 'Unnamed'
FONT = 'AvenirNextCondensed-HeavyItalic'
SUPERSAMPLE = 4
TEXT_HEIGHT = 15        # Cap height in pixels, about the original's letters
CENTER = (92, 76)       # Where "FireRed Version" was
OLD_SUBTITLE = (46, 57, 130, 96)  # x0, y0, x1, y1 to clear
MAP_W, MAP_H = 32, 20
OUTLINE = 206           # Black
GREYS = range(192, 208)  # The original's white-to-black ramp


def read_png(path):
    data = open(path, 'rb').read()
    pos, idat, plte = 8, b'', None
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            width, height, depth = struct.unpack('>IIB', chunk[:9])
        elif kind == b'PLTE':
            plte = chunk
        elif kind == b'IDAT':
            idat += chunk
    raw = zlib.decompress(idat)
    stride = width * depth // 8
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
        rows.append(list(line))
    return rows, plte


def write_png(path, rows, plte):
    raw = b''.join(b'\x00' + bytes(row) for row in rows)

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n'
                + chunk(b'IHDR', struct.pack('>IIBBBBB', len(rows[0]), len(rows), 8, 3, 0, 0, 0))
                + chunk(b'PLTE', plte) + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def read_pal(path):
    lines = open(path).read().split('\n')
    return [tuple(map(int, line.split())) for line in lines[3:3 + int(lines[2])]]


def compose_logo(tiles):
    entries = struct.unpack('<%dH' % (MAP_W * MAP_H), open(os.path.join(SRC, 'game_title_logo.bin'), 'rb').read())
    image = [[0] * (MAP_W * 8) for _ in range(MAP_H * 8)]
    for ty in range(MAP_H):
        for tx in range(MAP_W):
            e = entries[ty * MAP_W + tx]
            t, hflip, vflip = e & 0x3FF, e >> 10 & 1, e >> 11 & 1
            sx, sy = (t % 32) * 8, (t // 32) * 8
            for y in range(8):
                for x in range(8):
                    image[ty * 8 + y][tx * 8 + x] = tiles[sy + (7 - y if vflip else y)][sx + (7 - x if hflip else x)]
    return image


def render_coverage():
    with tempfile.TemporaryDirectory() as tmp:
        pgm = os.path.join(tmp, 'text.pgm')
        size = TEXT_HEIGHT * SUPERSAMPLE * 1.45
        subprocess.run(['swift', os.path.join(os.path.dirname(__file__), 'render_text.swift'),
                        SUBTITLE, FONT, str(size), pgm], check=True)
        data = open(pgm, 'rb').read()
    header, rest = data.split(b'\n', 1)
    dims, rest = rest.split(b'\n', 1)
    _, rest = rest.split(b'\n', 1)
    w, h = map(int, dims.split())
    big = [list(rest[y * w:(y + 1) * w]) for y in range(h)]
    # Average SUPERSAMPLE x SUPERSAMPLE blocks
    sw, sh = w // SUPERSAMPLE, h // SUPERSAMPLE
    small = [[0.0] * sw for _ in range(sh)]
    for y in range(sh):
        for x in range(sw):
            total = sum(big[y * SUPERSAMPLE + dy][x * SUPERSAMPLE + dx]
                        for dy in range(SUPERSAMPLE) for dx in range(SUPERSAMPLE))
            small[y][x] = total / (255.0 * SUPERSAMPLE * SUPERSAMPLE)
    return small


def nearest_grey(level, palette):
    return min(GREYS, key=lambda i: abs(palette[i][0] - level))


def main():
    tiles, plte = read_png(os.path.join(SRC, 'game_title_logo.png'))
    palette = read_pal(os.path.join(SRC, 'game_title_logo.pal'))
    image = compose_logo(tiles)

    x0, y0, x1, y1 = OLD_SUBTITLE
    for y in range(y0, y1):
        for x in range(x0, x1):
            image[y][x] = 0

    cover = render_coverage()
    h, w = len(cover), len(cover[0])
    ox, oy = CENTER[0] - w // 2, CENTER[1] - h // 2
    inside = [[cover[y][x] > 0.15 for x in range(w)] for y in range(h)]
    for y in range(-1, h + 1):
        for x in range(-1, w + 1):
            px, py = ox + x, oy + y
            c = cover[y][x] if 0 <= y < h and 0 <= x < w else 0.0
            if c > 0.15:
                # White inside, greys at the edges like the original's anti-aliasing
                image[py][px] = nearest_grey(int(255 * min(1.0, c * 1.25)), palette)
            elif any(0 <= y + dy < h and 0 <= x + dx < w and inside[y + dy][x + dx]
                     for dy in (-1, 0, 1) for dx in (-1, 0, 1)):
                image[py][px] = OUTLINE

    # Back to unique 8x8 tiles and a tilemap
    unique, entries = {}, []
    for ty in range(MAP_H):
        for tx in range(MAP_W):
            tile = tuple(tuple(image[ty * 8 + y][tx * 8 + x] for x in range(8)) for y in range(8))
            if tile not in unique:
                unique[tile] = len(unique)
            entries.append(unique[tile])
    if len(unique) > 256:
        sys.exit('too many tiles: %d' % len(unique))
    sheet_rows = (len(unique) + 31) // 32 * 8
    sheet = [[0] * 256 for _ in range(sheet_rows)]
    for tile, n in unique.items():
        for y in range(8):
            for x in range(8):
                sheet[(n // 32) * 8 + y][(n % 32) * 8 + x] = tile[y][x]
    os.makedirs(OUT, exist_ok=True)
    write_png(os.path.join(OUT, 'game_title_logo.png'), sheet, plte)
    with open(os.path.join(OUT, 'game_title_logo.bin'), 'wb') as f:
        f.write(struct.pack('<%dH' % len(entries), *entries))
    print('%d tiles' % len(unique))

    if '--preview' in sys.argv:
        out = sys.argv[sys.argv.index('--preview') + 1]
        rgb = [[palette[v] if v else (120, 160, 220) for v in row] for row in image]
        raw = b''
        for row in rgb:
            line = b'\x00' + b''.join(bytes(c) * 3 for c in row)
            raw += line * 3

        def chunk(kind, data):
            return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))
        with open(out, 'wb') as f:
            f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', len(rgb[0]) * 3, len(rgb) * 3, 8, 2, 0, 0, 0))
                    + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))


if __name__ == '__main__':
    main()
