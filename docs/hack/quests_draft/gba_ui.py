"""Tiny GBA UI kit for mockups: the game's real fonts, window frames and region map, drawn with Pillow.

Used by make_mockups.py. Reads graphics/ from the repo root (two levels up from this file).
"""

import os
import re
import struct

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
W, H = 240, 160


def path(*p):
    return os.path.join(ROOT, *p)


# --- Charmap and fonts -------------------------------------------------------------------------------------------

def load_charmap():
    cmap = {}
    with open(path("charmap.txt"), encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^'(.+?)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line.rstrip())
            if m and len(m.group(1)) <= 2:
                ch = m.group(1).replace("\\'", "'")
                cmap.setdefault(ch, int(m.group(2), 16))
    return cmap


CHARMAP = load_charmap()


def load_widths(name):
    src = open(path("src", "fonts.c")).read()
    m = re.search(r"g%sGlyphWidths\[\] = \{(.*?)\};" % name, src, re.S)
    return [int(x) for x in re.findall(r"\d+", m.group(1))]


class Font:
    def __init__(self, png, widths_name, cell_h=16):
        self.img = Image.open(path("graphics", "fonts", png))
        self.px = self.img.load()
        self.widths = load_widths(widths_name)
        self.cell_h = cell_h

    def glyph(self, code):
        gx, gy = (code % 16) * 16, (code // 16) * self.cell_h
        return gx, gy

    def width(self, text):
        return sum(self.widths[self.code(c)] for c in text)

    def code(self, c):
        if c in CHARMAP:
            return CHARMAP[c]
        return CHARMAP.get(c.upper(), 0)

    def draw(self, im, x, y, text, fg, shadow=None, bg=None):
        """Draw text with the glyphs' own pixels: index 1 = fg, 2 = shadow (3 = bg, left transparent)."""
        p = im.load()
        cx = x
        for c in text:
            if c == "\n":
                cx = x
                y += 16
                continue
            code = self.code(c)
            gx, gy = self.glyph(code)
            w = self.widths[code]
            for yy in range(self.cell_h):
                for xx in range(w):
                    v = self.px[gx + xx, gy + yy]
                    tx, ty = cx + xx, y + yy
                    if not (0 <= tx < im.width and 0 <= ty < im.height):
                        continue
                    if v == 1:
                        p[tx, ty] = fg
                    elif v == 2 and shadow is not None:
                        p[tx, ty] = shadow
            cx += w
        return cx


NORMAL = Font("latin_normal.png", "FontNormalLatin")
SMALL = Font("latin_small.png", "FontSmallLatin")
NARROW = Font("latin_narrow.png", "FontNarrowLatin")
SHORT = Font("latin_short.png", "FontShortLatin")

# FRLG-ish text colours (fg, shadow)
WHITE = (255, 255, 255)
DARK = (98, 98, 98)
DARK_SH = (213, 213, 205)
BLUE = (48, 80, 200)
BLUE_SH = (160, 192, 240)
RED = (224, 8, 8)
RED_SH = (248, 184, 112)
GREEN = (24, 128, 48)
GREEN_SH = (160, 224, 160)
GRAY = (160, 160, 160)
GRAY_SH = (224, 224, 224)
LIGHT = (255, 255, 255)
LIGHT_SH = (98, 98, 98)


# --- Window frames -----------------------------------------------------------------------------------------------

def frame_tiles(name="1.png"):
    im = Image.open(path("graphics", "text_window", name)).convert("RGBA")
    pal = Image.open(path("graphics", "text_window", name))
    tiles = [im.crop((tx * 8, ty * 8, tx * 8 + 8, ty * 8 + 8)) for ty in range(3) for tx in range(3)]
    # Colour 0 is transparent
    idx = pal.load()
    for t, (tx, ty) in zip(tiles, [(x, y) for y in range(3) for x in range(3)]):
        tp = t.load()
        for yy in range(8):
            for xx in range(8):
                if idx[tx * 8 + xx, ty * 8 + yy] == 0:
                    tp[xx, yy] = (0, 0, 0, 0)
    return tiles


def draw_window(im, tx, ty, tw, th, frame="1.png", fill=WHITE):
    """A std window whose inside is tiles (tx, ty)-(tx+tw-1, ty+th-1), framed one tile outside, as DrawStdWindowFrame."""
    t = frame_tiles(frame)
    x0, y0 = (tx - 1) * 8, (ty - 1) * 8
    for j in range(th + 2):
        for i in range(tw + 2):
            col = 0 if i == 0 else 2 if i == tw + 1 else 1
            row = 0 if j == 0 else 2 if j == th + 1 else 1
            if col == 1 and row == 1:
                im.paste(fill, (x0 + i * 8, y0 + j * 8, x0 + i * 8 + 8, y0 + j * 8 + 8))
            else:
                tile = t[row * 3 + col]
                im.alpha_composite(tile, (x0 + i * 8, y0 + j * 8)) if im.mode == "RGBA" else im.paste(tile, (x0 + i * 8, y0 + j * 8), tile)


def fill_rect(im, x, y, w, h, color):
    im.paste(color, (x, y, x + w, y + h))


# --- Region map --------------------------------------------------------------------------------------------------

def read_jasc(p):
    lines = open(p).read().split()
    n = int(lines[2])
    vals = list(map(int, lines[3:3 + 3 * n]))
    return [tuple(vals[i:i + 3]) for i in range(0, 3 * n, 3)]


def region_map_kanto():
    """The Kanto map BG (affine 8bpp, 64x64 u8 tilemap) as drawn at scroll 0, plus the frame BG on top."""
    gfx = Image.open(path("graphics", "pokenav", "region_map", "map_kanto.png"))
    gp = gfx.load()
    raw = gfx.getpalette()
    pal = [tuple(raw[i:i + 3]) for i in range(0, len(raw), 3)]
    tm = open(path("graphics", "pokenav", "region_map", "map_kanto.bin"), "rb").read()
    cols = gfx.width // 8
    out = Image.new("RGB", (W, H))
    op = out.load()
    for ty in range(H // 8):
        for tx in range(W // 8):
            t = tm[ty * 64 + tx]
            sx, sy = (t % cols) * 8, (t // cols) * 8
            for yy in range(8):
                for xx in range(8):
                    v = gp[sx + xx, sy + yy]
                    op[tx * 8 + xx, ty * 8 + yy] = pal[v] if v < len(pal) else (0, 0, 0)
    # Frame BG (4bpp text BG, 32x20 u16 tilemap) on top, colour 0 transparent
    fr = Image.open(path("graphics", "pokenav", "region_map", "frame.png"))
    fpal = fr.getpalette()
    fp = fr.load()
    fcols = fr.width // 8
    ftm = open(path("graphics", "pokenav", "region_map", "frame.bin"), "rb").read()
    for ty in range(20):
        for tx in range(30):
            (e,) = struct.unpack_from("<H", ftm, (ty * 32 + tx) * 2)
            t, hf, vf = e & 0x3FF, e & 0x400, e & 0x800
            sx, sy = (t % fcols) * 8, (t // fcols) * 8
            if sy >= fr.height:
                continue
            for yy in range(8):
                for xx in range(8):
                    rx = 7 - xx if hf else xx
                    ry = 7 - yy if vf else yy
                    v = fp[sx + rx, sy + ry]
                    if v:
                        op[tx * 8 + xx, ty * 8 + yy] = tuple(fpal[v * 3:v * 3 + 3])
    return out


def mapsec_rect(x, y, w=1, h=1):
    """Screen pixels of a map section at grid (x, y) (region_map_sections.json), unzoomed."""
    return ((x + 1) * 8, (y + 2) * 8, w * 8, h * 8)


def sprite(png, frame=0, size=None):
    im = Image.open(path(*png.split("/")))
    pal = im.getpalette()
    w, h = size or im.size
    rgba = Image.new("RGBA", (w, h))
    src = im.load()
    rp = rgba.load()
    fx, fy = (frame * w) % im.width, ((frame * w) // im.width) * h
    for yy in range(h):
        for xx in range(w):
            v = src[fx + xx, fy + yy]
            if v:
                rp[xx, yy] = tuple(pal[v * 3:v * 3 + 3]) + (255,)
    return rgba


def scale(im, k=3):
    return im.resize((im.width * k, im.height * k), Image.NEAREST)
