#!/usr/bin/env python3
"""Builds the player boy's overworld sprites: the male Silph worker's head on Red's body.

Reads Red's original sheets (graphics/object_events/pics/people/red/) and the worker
(graphics/object_events/pics/people/worker_m.png) and writes the composites to
graphics/object_events/pics/people/player_boy/, which the game uses for the player
(see docs/hack/opening.md). Re-run after changing either source.

It also gives the player's portraits the worker's hair color (Red's own portraits stay as they are):
- graphics/oak_speech/player_boy.pal: the intro picture's palette, hair slots recolored
- graphics/trainers/back_pics/player_boy.png: the battle back sprite, hair repainted

    python3 tools/hack/compose_player_head.py            # write the sheets
    python3 tools/hack/compose_player_head.py --usage    # palette slots Red's bodies still use
"""
import os
import struct
import sys
import zlib

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')
PEOPLE = os.path.join(ROOT, 'graphics/object_events/pics/people')
PALETTES = os.path.join(ROOT, 'graphics/object_events/palettes')
OUT_DIR = os.path.join(PEOPLE, 'player_boy')

# (sheet, frame width)
SHEETS = [
    ('red_normal', 16),
    ('red_surf_run', 16),
    ('red_item', 16),
    ('red_bike', 32),
    ('red_surf', 32),
    ('red_fish', 32),
    ('red_vs_seeker_bike', 32),
]

HEAD_HEIGHT = 11            # rows, the same for Red (cap to chin) and the worker (hair to chin)
RED_CAP = {0x8, 0xB, 0xC}   # cap outline and reds
RED_CAP_WHITE = {0x9, 0xA}  # logo and brim, which tell which way the head faces
WORKER_HAIR = {0x5, 0x6, 0x7}
# Worker (npc_white palette) slot -> player_boy palette slot
WORKER_TO_PLAYER = {
    0x0: 0x0,
    0x1: 0x2,   # skin
    0x2: 0x2,   # skin
    0x3: 0x3,   # skin shade
    0x4: 0x4,   # skin outline
    0x5: 0x1,   # hair, light
    0x6: 0x5,   # hair, mid
    0x7: 0x8,   # hair, dark
    0xE: 0x9,   # white
    0xF: 0xF,   # outline
}
# Slots 1, 5 and 8 are the worker's hair now (graphics/object_events/palettes/player_boy.pal). On Red's body,
# 1 (a few stray pixels) and 8 (dark red outline) become the near-identical skin outline, slot 4, and 5 (the
# fishing line and an item glint) becomes light blue, slot 7.
RED_REMAP = {0x1: 0x4, 0x5: 0x7, 0x8: 0x4}
CAP_MIN_WIDTH = 4           # pixels of cap colors in a row, so the fishing bobber isn't taken for the cap
WORKER_FRAME = {'down': 0, 'up': 1, 'left': 2}


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


def write_png(path, rows, palette):
    height, width = len(rows), len(rows[0])
    raw = b''
    for row in rows:
        raw += b'\x00' + bytes((row[x] << 4) | row[x + 1] for x in range(0, width, 2))

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

    plte = b''.join(bytes(c) for c in palette)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 4, 3, 0, 0, 0))
                + chunk(b'PLTE', plte) + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def read_pal(path):
    lines = open(path).read().split('\n')[3:19]
    return [tuple(map(int, line.split())) for line in lines]


def worker_heads():
    rows = read_png(os.path.join(PEOPLE, 'worker_m.png'))
    heads = {}
    for facing, frame in WORKER_FRAME.items():
        cols = range(frame * 16, frame * 16 + 16)
        top = next(y for y in range(32) if any(rows[y][x] for x in cols))
        head = [[rows[top + dy][x] for x in cols] for dy in range(HEAD_HEIGHT)]
        hair = [x for dy in range(7) for x in range(16) if head[dy][x] in WORKER_HAIR]
        heads[facing] = (head, min(hair), max(hair))
    # Facing right is facing left, flipped
    head, left, right = heads['left']
    heads['right'] = ([row[::-1] for row in head], 15 - right, 15 - left)
    return heads


def find_head(frame):
    """Red's head in a frame: its top row, cap columns and which way it faces (None if there's no cap)."""
    width = len(frame[0])
    cap_rows = [y for y, row in enumerate(frame) if sum(v in RED_CAP for v in row) >= CAP_MIN_WIDTH]
    if not cap_rows:
        return None
    top = cap_rows[0]
    cap = [x for y in cap_rows if y < top + 7 for x in range(width) if frame[y][x] in RED_CAP]
    left, right = min(cap), max(cap)
    white = [x for y in range(top + 2, min(top + 9, 32)) for x in range(left, right + 1)
             if frame[y][x] in RED_CAP_WHITE]
    middle = (left + right) / 2
    if not white:
        facing = 'up'
    else:
        mean = sum(white) / len(white)
        facing = 'left' if mean < middle - 1.5 else 'right' if mean > middle + 1.5 else 'down'
    return top, left, right, facing


def compose_frame(frame, heads):
    found = find_head(frame)
    if found is None:
        return [[RED_REMAP.get(v, v) for v in row] for row in frame], None
    top, left, right, facing = found
    head, hair_left, hair_right = heads[facing]
    dx = round(((left + right) - (hair_left + hair_right)) / 2)
    width = len(frame[0])
    out = [[RED_REMAP.get(v, v) for v in row] for row in frame]
    x0 = max(0, min(left, hair_left + dx) - 1)
    x1 = min(width - 1, max(right, hair_right + dx) + 1)
    for dy in range(HEAD_HEIGHT):
        y = top + dy
        if y >= len(frame):
            break
        for x in range(x0, x1 + 1):
            hx = x - dx
            worker = head[dy][hx] if 0 <= hx < 16 else 0
            if worker:
                out[y][x] = WORKER_TO_PLAYER[worker]
            elif left - 1 <= x <= right + 1:
                out[y][x] = 0   # Red's cap and face
    return out, facing


def compose_sheet(rows, frame_width, heads):
    out = [row[:] for row in rows]
    facings = []
    for start in range(0, len(rows[0]), frame_width):
        frame = [row[start:start + frame_width] for row in rows]
        frame, facing = compose_frame(frame, heads)
        facings.append(facing)
        for y, row in enumerate(frame):
            out[y][start:start + frame_width] = row
    return out, facings


HAIR_MID = (123, 115, 65)
HAIR_DARK = (57, 57, 24)

# Intro picture (graphics/oak_speech/red): slots 8 and 9 are only used by the hair
INTRO_HAIR = {8: HAIR_MID, 9: HAIR_DARK}

# Battle back sprite: the hair is the biggest patch of slots 4 and 8, which also outline the skin and cap.
# Its hair goes to slot 1 (unused there) and slot 8 turns dark olive.
BACK_HAIR_SLOTS = {0x4, 0x8}
BACK_SKIN = {0x2, 0x3}
BACK_HAIR_MID_SLOT = 0x1
BACK_PALETTE = {0x1: HAIR_MID, 0x8: HAIR_DARK}


def write_jasc(path, colors, newline):
    lines = ['JASC-PAL', '0100', str(len(colors))] + ['%d %d %d' % c for c in colors]
    with open(path, 'w', newline='') as f:
        f.write(newline.join(lines) + newline)


def intro_palette():
    path = os.path.join(ROOT, 'graphics/oak_speech/red/pal.pal')
    text = open(path, newline='').read()
    newline = '\r\n' if '\r\n' in text else '\n'
    lines = text.split(newline)
    colors = [tuple(map(int, line.split())) for line in lines[3:3 + int(lines[2])]]
    for slot, color in INTRO_HAIR.items():
        colors[slot] = color
    write_jasc(os.path.join(ROOT, 'graphics/oak_speech/player_boy.pal'), colors, newline)


def read_png_palette(path):
    data = open(path, 'rb').read()
    pos = 8
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        if kind == b'PLTE':
            chunk = data[pos + 8:pos + 8 + length]
            return [tuple(chunk[i:i + 3]) for i in range(0, len(chunk), 3)]
        pos += 12 + length


def back_pic():
    path = os.path.join(ROOT, 'graphics/trainers/back_pics/red.png')
    rows = read_png(path)
    palette = read_png_palette(path)[:16]
    for slot, color in BACK_PALETTE.items():
        palette[slot] = color
    height, width = len(rows), len(rows[0])
    for top in range(0, height, 64):   # one 64x64 frame per throw step
        seen = set()
        patches = []
        for y in range(top, top + 64):
            for x in range(width):
                if rows[y][x] in BACK_HAIR_SLOTS and (x, y) not in seen:
                    patch, todo = [], [(x, y)]
                    seen.add((x, y))
                    while todo:
                        px, py = todo.pop()
                        patch.append((px, py))
                        for nx, ny in ((px + 1, py), (px - 1, py), (px, py + 1), (px, py - 1)):
                            if (top <= ny < top + 64 and 0 <= nx < width and (nx, ny) not in seen
                                    and rows[ny][nx] in BACK_HAIR_SLOTS):
                                seen.add((nx, ny))
                                todo.append((nx, ny))
                    patches.append(patch)
        if not patches:
            continue
        hair = max(patches, key=len)
        for x, y in hair:
            touches_skin = any(0 <= nx < width and top <= ny < top + 64 and rows[ny][nx] in BACK_SKIN
                               for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))
            if rows[y][x] == 0x4 and not touches_skin:
                rows[y][x] = BACK_HAIR_MID_SLOT
    write_png(os.path.join(ROOT, 'graphics/trainers/back_pics/player_boy.png'), rows, palette)


def main():
    heads = worker_heads()
    usage = set()
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, frame_width in SHEETS:
        red = read_png(os.path.join(PEOPLE, 'red', name + '.png'))
        out, facings = compose_sheet(red, frame_width, heads)
        print(name, ' '.join(f or '-' for f in facings))
        if '--usage' in sys.argv:
            for start in range(0, len(red[0]), frame_width):
                frame = [row[start:start + frame_width] for row in red]
                found = find_head(frame)
                skip_top = found[0] if found else 99
                for y, row in enumerate(frame):
                    if not skip_top <= y < skip_top + HEAD_HEIGHT:
                        usage.update(row)
            continue
        palette = read_pal(os.path.join(PALETTES, 'player_boy.pal'))
        write_png(os.path.join(OUT_DIR, name.replace('red_', '') + '.png'), out, palette)
    if '--usage' in sys.argv:
        print('slots used outside the head:', sorted(usage))
        return
    intro_palette()
    back_pic()


if __name__ == '__main__':
    main()
