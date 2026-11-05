#!/usr/bin/env python3
"""Draws the 3v3 move badges (src/triple_battle_ui.c): "DIRECT", "INDIRECT" and "ALL" in white on a red, blue and
dark pill, lettered with the game's own small font (graphics/fonts/latin_small.png, widths from src/fonts.c).

Writes graphics/battle_interface/triple_badges.png: three 64x32 frames stacked (a 64x32 sprite each; the pill in the
top 16 rows), and its palette. Prints each pill's width for TRIPLE_BADGE_WIDTHS. Needs Pillow:

    python3 tools/hack/draw_triple_badges.py
"""
import os
import re
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
OUT = os.path.join(ROOT, 'graphics/battle_interface/triple_badges.png')
BADGES = [('DIRECT', 1), ('INDIRECT', 2), ('ALL', 3)]  # Text, color pair
PALETTE = [(0, 0, 0), (255, 255, 255),
           (208, 48, 40), (120, 24, 16),   # Red: fill, letter shadow
           (48, 96, 200), (24, 48, 112),   # Blue
           (72, 72, 80), (32, 32, 40)]     # Dark
PALETTE += [(0, 0, 0)] * (16 - len(PALETTE))
PILL_H = 11
GLYPH_TOP = 4  # The small font's letters start 4 rows into their 16x16 cell


def charmap():
    codes = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"'(.)'\s*=\s*([0-9A-F]{2})\b", line)
        if m:
            codes[m.group(1)] = int(m.group(2), 16)
    return codes


def widths():
    src = open(os.path.join(ROOT, 'src/fonts.c')).read()
    body = src[src.index('gFontSmallLatinGlyphWidths[] = {'):]
    body = body[body.index('{') + 1:body.index('};')]
    return [int(n) for n in re.findall(r'\d+', body)]


def main():
    font = Image.open(os.path.join(ROOT, 'graphics/fonts/latin_small.png'))
    codes, glyph_widths = charmap(), widths()
    sheet = Image.new('P', (64, 32 * len(BADGES)), 0)
    sheet.putpalette([c for rgb in PALETTE for c in rgb])
    for n, (text, pair) in enumerate(BADGES):
        fill, shadow = 2 * pair, 2 * pair + 1
        text_w = sum(glyph_widths[codes[ch]] for ch in text)
        w = text_w + 6
        top = 32 * n
        for y in range(PILL_H):
            for x in range(w):
                corner = (x in (0, w - 1) and y in (0, PILL_H - 1))
                if not corner:
                    sheet.putpixel((x, top + y), fill)
        x0 = 3
        for ch in text:
            code = codes[ch]
            cx, cy = (code % 16) * 16, (code // 16) * 16
            for gy in range(8):
                for gx in range(glyph_widths[code]):
                    v = font.getpixel((cx + gx, cy + GLYPH_TOP + gy))
                    if v == 1:
                        sheet.putpixel((x0 + gx, top + 2 + gy), 1)
                    elif v == 2 and 2 + gy < PILL_H - 1:
                        sheet.putpixel((x0 + gx, top + 2 + gy), shadow)
            x0 += glyph_widths[code]
        print(text, w)
    sheet.save(OUT)
    with open(OUT.replace('.png', '.pal'), 'w') as f:
        f.write('JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in PALETTE))


if __name__ == '__main__':
    main()
