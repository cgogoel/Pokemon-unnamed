#!/usr/bin/env python3
"""Draws a cartridge label for Pokémon Unnamed, the size of a Game Boy Advance cartridge label (the reference scan
was 1280x661 px; a printed label is about 36 x 18.6 mm).

An original design in the spirit of the FireRed label: a swirl of light streaks (in blue, for the hack's Lapras),
the title in yellow with a thick outline, and the game's own Lapras sprite. No Nintendo seal, rating box or official
logo artwork. Needs Pillow:

    python3 tools/hack/make_label.py [out.png]      (default: docs/hack/cartridge_label.png)
"""
import math
import os
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
W, H = 1280, 661
RADIUS = 46
FONT_TITLE = '/System/Library/Fonts/Supplemental/Arial Black.ttf'
FONT_SMALL = '/System/Library/Fonts/Supplemental/Arial Bold.ttf'

DEEP = (8, 40, 120)
MID = (24, 110, 205)
LIGHT = (120, 205, 255)
STREAK = (235, 250, 255)
YELLOW = (255, 214, 40)
NAVY = (22, 42, 120)
WHITE = (255, 255, 255)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def swirl():
    img = Image.new('RGB', (W, H))
    px = img.load()
    cx, cy = W * 0.47, H * 0.42
    for y in range(H):
        for x in range(W):
            dx, dy = (x - cx) / W, (y - cy) / W
            r = math.hypot(dx, dy) + 1e-6
            a = math.atan2(dy, dx)
            wave = math.sin(5 * a + 9 * math.log(r))          # The spiral's arms
            streak = math.sin(23 * a + 26 * math.log(r) + 1.3)  # Thin streaks along them
            t = 0.5 + 0.5 * wave
            col = lerp(DEEP, MID, t) if t < 0.6 else lerp(MID, LIGHT, (t - 0.6) / 0.4)
            if streak > 0.93 and wave > -0.2:
                col = lerp(col, STREAK, min(1, (streak - 0.93) / 0.05) * 0.85)
            glow = max(0, 1 - r * 2.2)                         # Brighter towards the middle
            col = lerp(col, LIGHT, glow * 0.35)
            px[x, y] = col
    return img.filter(ImageFilter.GaussianBlur(1.2))


def outlined_text(draw, xy, text, font, fill, outline, width, anchor='mm'):
    draw.text(xy, text, font=font, fill=fill, anchor=anchor, stroke_width=width, stroke_fill=outline)


def title(img):
    layer = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    big = ImageFont.truetype(FONT_TITLE, 150)
    sub = ImageFont.truetype(FONT_TITLE, 84)
    # Drop shadow, then the outlined letters
    shadow = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    sd.text((482, 244), 'POKÉMON', font=big, fill=(0, 0, 40, 170), anchor='mm', stroke_width=16, stroke_fill=(0, 0, 40, 170))
    sd.text((482, 414), 'UNNAMED', font=sub, fill=(0, 0, 40, 170), anchor='mm', stroke_width=12, stroke_fill=(0, 0, 40, 170))
    shadow = shadow.filter(ImageFilter.GaussianBlur(6))
    layer = Image.alpha_composite(layer, shadow)
    d = ImageDraw.Draw(layer)
    outlined_text(d, (470, 230), 'POKÉMON', big, YELLOW, NAVY, 14)
    # A highlight on the top half of the yellow letters
    hi = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(hi).text((470, 230), 'POKÉMON', font=big, fill=(255, 245, 170, 255), anchor='mm')
    mask = Image.new('L', (W, H), 0)
    ImageDraw.Draw(mask).rectangle((0, 0, W, 216), fill=255)
    hi.putalpha(ImageChops.multiply(hi.getchannel('A'), mask))
    layer = Image.alpha_composite(layer, hi)
    d = ImageDraw.Draw(layer)
    # The subtitle plate, like the "FIRERED VERSION" block
    outlined_text(d, (470, 400), 'UNNAMED', sub, WHITE, NAVY, 11)
    return Image.alpha_composite(img.convert('RGBA'), layer)


def lapras():
    sheet = Image.open(os.path.join(ROOT, 'graphics/pokemon/lapras/anim_front_gba.png'))
    frame = sheet.crop((0, 0, 64, 64)).convert('RGBA')
    bg = frame.getpixel((0, 0))
    frame.putdata([(0, 0, 0, 0) if p[:3] == bg[:3] else p for p in frame.getdata()])
    return frame.resize((64 * 6, 64 * 6), Image.NEAREST)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'docs/hack/cartridge_label.png')
    img = title(swirl())

    # Lapras, with a soft white glow behind it
    mon = lapras()
    glow = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    halo = Image.new('RGBA', mon.size, (255, 255, 255, 0))
    halo.putalpha(mon.getchannel('A').point(lambda a: 200 if a else 0))
    glow.paste(halo, (850, 150), halo)
    glow = glow.filter(ImageFilter.GaussianBlur(14))
    img = Image.alpha_composite(img, glow)
    img.paste(mon, (850, 150), mon)

    # Small print, bottom left and right
    d = ImageDraw.Draw(img)
    small = ImageFont.truetype(FONT_SMALL, 30)
    d.text((86, 600), 'A FireRed hack', font=small, fill=WHITE, anchor='lm', stroke_width=3, stroke_fill=NAVY)
    d.text((W - 86, 600), 'GBA', font=small, fill=WHITE, anchor='rm', stroke_width=3, stroke_fill=NAVY)

    # The label's rounded corners and a thin white edge
    mask = Image.new('L', (W, H), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, W - 1, H - 1), RADIUS, fill=255)
    ImageDraw.Draw(img).rounded_rectangle((3, 3, W - 4, H - 4), RADIUS - 3, outline=(255, 255, 255, 200), width=5)
    img.putalpha(mask)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    img.save(out, dpi=(900, 900))
    print(out, img.size)


if __name__ == '__main__':
    main()
