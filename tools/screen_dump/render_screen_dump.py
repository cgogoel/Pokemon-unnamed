#!/usr/bin/env python3
"""Renders a SCREEN_DUMP from a `make check` log into PNG files.

Usage: python3 tools/screen_dump/render_screen_dump.py <check log> <output dir> [scale]

Development aid for battle layout work. Approximates the GBA's mode 0: four text backgrounds and sprites
with priorities and flips. Blending, windows, mosaic and affine scaling are ignored (affine sprites are drawn
unscaled at their centre).
"""

import os
import re
import struct
import sys
import zlib

WIDTH, HEIGHT = 240, 160

OBJ_SIZES = {
    (0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64),
    (1, 0): (16, 8), (1, 1): (32, 8), (1, 2): (32, 16), (1, 3): (64, 32),
    (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64),
}


def read_dumps(path):
    dumps = []
    current = None
    with open(path, "r", errors="replace") as f:
        for line in f:
            m = re.search(r"SCRDUMP (\S+)(.*)", line)
            if not m:
                continue
            kind, rest = m.group(1), m.group(2).strip()
            if kind == "BEGIN":
                current = {"name": rest, "V": bytearray(0x18000), "O": bytearray(0x400), "P": bytearray(0x400)}
            elif current is None:
                continue
            elif kind == "R":
                current["regs"] = [int(x, 16) for x in rest.split()]
            elif kind == "S":
                current["scroll"] = [int(x, 16) for x in rest.split()]
            elif kind in "VOP":
                offset, data = rest.split()
                offset = int(offset, 16)
                current[kind][offset:offset + len(data) // 2] = bytes.fromhex(data)
            elif kind == "END":
                dumps.append(current)
                current = None
    return dumps


def color(pal, index):
    value = pal[2 * index] | (pal[2 * index + 1] << 8)
    r, g, b = value & 31, (value >> 5) & 31, (value >> 10) & 31
    return (r << 3 | r >> 2, g << 3 | g >> 2, b << 3 | b >> 2)


def tile_pixel(vram, base, tile, x, y, bpp8):
    if bpp8:
        return vram[(base + tile * 64 + y * 8 + x) % len(vram)]
    byte = vram[(base + tile * 32 + y * 4 + x // 2) % len(vram)]
    return (byte >> 4) if x & 1 else (byte & 0xF)


def render_bg(dump, bg, layers):
    cnt = dump["regs"][1 + bg]
    priority = cnt & 3
    char_base = ((cnt >> 2) & 3) * 0x4000
    bpp8 = bool(cnt & 0x80)
    screen_base = ((cnt >> 8) & 0x1F) * 0x800
    size = (cnt >> 14) & 3
    width = 512 if size in (1, 3) else 256
    height = 512 if size in (2, 3) else 256
    hofs, vofs = dump["scroll"][2 * bg] & 0x1FF, dump["scroll"][2 * bg + 1] & 0x1FF
    vram, pal = dump["V"], dump["P"]

    for sy in range(HEIGHT):
        for sx in range(WIDTH):
            x, y = (sx + hofs) % width, (sy + vofs) % height
            block = (x // 256) + (y // 256) * (2 if width == 512 else 1)
            entry_addr = screen_base + block * 0x800 + ((y % 256) // 8) * 64 + ((x % 256) // 8) * 2
            entry = vram[entry_addr] | (vram[entry_addr + 1] << 8)
            tile, hflip, vflip, palnum = entry & 0x3FF, entry & 0x400, entry & 0x800, entry >> 12
            px, py = x % 8, y % 8
            if hflip:
                px = 7 - px
            if vflip:
                py = 7 - py
            index = tile_pixel(vram, char_base, tile, px, py, bpp8)
            if index == 0:
                continue
            rgb = color(pal, index if bpp8 else palnum * 16 + index)
            layers.append((priority, 1, bg, sx, sy, rgb))


def render_objs(dump, layers):
    oam, vram, pal = dump["O"], dump["V"], dump["P"]
    one_d = bool(dump["regs"][0] & 0x40)
    for i in range(128):
        a0, a1, a2 = struct.unpack_from("<HHH", oam, i * 8)
        affine = a0 & 0x100
        if not affine and a0 & 0x200:
            continue  # Hidden
        mode = (a0 >> 10) & 3
        if mode == 2:
            continue  # OBJ window
        shape, size = a0 >> 14, a1 >> 14
        if (shape, size) not in OBJ_SIZES:
            continue
        w, h = OBJ_SIZES[(shape, size)]
        y, x = a0 & 0xFF, a1 & 0x1FF
        if y >= 160:
            y -= 256
        if x >= 240:
            x -= 512
        double = affine and a0 & 0x200
        if double:
            x += w // 2
            y += h // 2
        bpp8 = bool(a0 & 0x2000)
        hflip = not affine and a1 & 0x1000
        vflip = not affine and a1 & 0x2000
        tile, priority, palnum = a2 & 0x3FF, (a2 >> 10) & 3, a2 >> 12
        tiles_per_row = (w // 8) * (2 if bpp8 else 1) if one_d else 32
        for py in range(h):
            for px in range(w):
                sx, sy = x + px, y + py
                if not (0 <= sx < WIDTH and 0 <= sy < HEIGHT):
                    continue
                tx, ty = (w - 1 - px) if hflip else px, (h - 1 - py) if vflip else py
                t = tile + (ty // 8) * tiles_per_row + (tx // 8) * (2 if bpp8 else 1)
                index = tile_pixel(vram, 0x10000, t, tx % 8, ty % 8, bpp8)
                if index == 0:
                    continue
                rgb = color(pal, 256 + (index if bpp8 else palnum * 16 + index))
                layers.append((priority, 0, i, sx, sy, rgb))


def render(dump, scale):
    dispcnt = dump["regs"][0]
    layers = []
    for bg in range(4):
        if dispcnt & (0x100 << bg):
            render_bg(dump, bg, layers)
    if dispcnt & 0x1000:
        render_objs(dump, layers)

    image = [[color(dump["P"], 0)] * WIDTH for _ in range(HEIGHT)]
    # Draw back to front: higher priority value first; BGs behind OBJs at equal priority; lower OBJ index on top.
    layers.sort(key=lambda l: (-l[0], -l[1], -l[2]))
    for _, _, _, sx, sy, rgb in layers:
        image[sy][sx] = rgb

    rows = []
    for y in range(HEIGHT):
        row = bytearray([0])
        for x in range(WIDTH):
            row += bytes(image[y][x]) * scale
        rows.extend([bytes(row)] * scale)
    return rows


def write_png(path, rows, width, height):
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(b"".join(rows), 9)))
        f.write(chunk(b"IEND", b""))


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    log_path, out_dir = sys.argv[1], sys.argv[2]
    scale = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    os.makedirs(out_dir, exist_ok=True)
    dumps = read_dumps(log_path)
    if not dumps:
        print("No SCREEN_DUMP found in the log.")
        sys.exit(1)
    for n, dump in enumerate(dumps):
        name = re.sub(r"[^A-Za-z0-9]+", "_", dump["name"]).strip("_")[:60] or f"dump{n}"
        path = os.path.join(out_dir, f"{n:02d}_{name}.png")
        write_png(path, render(dump, scale), WIDTH * scale, HEIGHT * scale)
        print(path)


if __name__ == "__main__":
    main()
