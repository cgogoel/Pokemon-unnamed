"""Quest menu mockups (draft, not in the build): draws the PNGs in this folder at 3x.

    python3 docs/hack/quests_draft/make_mockups.py      (needs Pillow)

Uses the game's real fonts (graphics/fonts), window frame (graphics/text_window/1.png), keypad icons, the Kanto
region map and the fly map's red box (graphics/pokenav/region_map) and the campus layout (tools/hack/render_layout.py).
"""

import os
import subprocess
import sys
import tempfile

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gba_ui as g  # noqa: E402

OUT = os.path.dirname(os.path.abspath(__file__))
F_N, F_S = g.NORMAL, g.SMALL

BG_A = (176, 208, 240)
BG_B = (160, 192, 232)
HEADER = (48, 64, 104)
HEADER_SH = (24, 32, 56)
BAR_FILL = (88, 200, 120)
BAR_FILL_SH = (56, 160, 88)
BAR_EMPTY = (232, 232, 224)
BLINK = (248, 64, 64)

# The sample state for every mockup: the Water course, part way through.
QUEST = "WATER COURSE"
PLACE = "VERMILION CITY"
TASKS = [  # text, done, counter (shown in place of the box while under way)
    ("Catch 3 WATER POKéMON", False, "2/3"),
    ("Use a WATER move", True, None),
    ("Record 3 WATER moves", True, None),
    ("Put out 3 dock fires", False, "1/3"),
    ("Make the TM", False, None),
    ("Pass the exam", False, None),
]
QUEST_PCT = 53  # (2 + 1 + 3 + 1 + 0 + 0 + 0) of 13 steps (quests_draft.md)
ACT_PCT = 39  # (1 + 1 + 1 + 0.53) of 9 quests
NEXT = "Put out the fires on the docks."
NEXT_2 = "Any WATER move puts out a small one."
LIST = [  # name, state: now / done / locked
    ("WATER course", "now"),
    ("Club night 1", "done"),
    ("FIRE course", "done"),
    ("First day", "done"),
    ("? ? ?", "locked"),
]


def background(im, header_text=None):
    p = im.load()
    for y in range(g.H):
        for x in range(g.W):
            p[x, y] = BG_A if ((x // 8) + (y // 8)) % 2 == 0 else BG_B
    g.fill_rect(im, 0, 0, g.W, 16, HEADER)
    g.fill_rect(im, 0, 16, g.W, 1, HEADER_SH)
    if header_text:
        F_N.draw(im, 4, 1, header_text, g.WHITE, HEADER_SH)


def keypad(name):
    """A keypad icon (graphics/fonts/keypad_icons.png) as RGBA, as sKeypadIcons in src/text.c."""
    tiles = {"A": (0x00, 8), "B": (0x01, 8), "L": (0x02, 16), "R": (0x04, 16), "START": (0x06, 24),
             "SELECT": (0x09, 24), "UD": (0x20, 8), "LR": (0x21, 8)}
    t, w = tiles[name]
    src = Image.open(g.path("graphics", "fonts", "keypad_icons.png"))
    pal = src.getpalette()
    sp = src.load()
    out = Image.new("RGBA", (w, 12))
    op = out.load()
    x0, y0 = (t % 16) * 8, (t // 16) * 8
    for y in range(12):
        for x in range(w):
            v = sp[x0 + x, y0 + y]
            if v:
                op[x, y] = tuple(pal[v * 3:v * 3 + 3]) + (255,)
    return out


def hint(im, x, y, key, text, fg=g.WHITE, sh=HEADER_SH):
    icon = keypad(key)
    im.alpha_composite(icon, (x, y + 2))
    return F_N.draw(im, x + icon.width + 2, y, text, fg, sh)


def bar(im, x, y, w, pct, h=5):
    g.fill_rect(im, x, y, w, h, g.DARK)
    g.fill_rect(im, x + 1, y + 1, w - 2, h - 2, BAR_EMPTY)
    fill = (w - 2) * pct // 100
    if fill:
        g.fill_rect(im, x + 1, y + 1, fill, h - 2, BAR_FILL)
        g.fill_rect(im, x + 1, y + h - 2, fill, 1, BAR_FILL_SH)


def checkbox(im, x, y, done):
    g.fill_rect(im, x, y, 9, 9, g.DARK)
    g.fill_rect(im, x + 1, y + 1, 7, 7, g.WHITE)
    if done:
        p = im.load()
        for dx, dy in [(1, 4), (2, 5), (3, 6), (4, 5), (5, 4), (6, 3), (7, 2), (2, 4), (3, 5), (4, 4), (5, 3), (6, 2)]:
            p[x + dx, y + dy] = BAR_FILL_SH


def task_row(im, x, y, text, done, counter):
    """A task: its box (or its counter while under way), then the text, greyed once done."""
    if counter and not done:
        F_S.draw(im, x - 2, y, counter, g.RED, g.RED_SH)
    else:
        checkbox(im, x, y + 2, done)
    col = (g.GRAY, g.GRAY_SH) if done else (g.DARK, g.DARK_SH)
    F_S.draw(im, x + 15, y, text, *col)


def pin(im, x, y, color=BLINK):
    """A small map pin, 7x9."""
    shape = ["..XXX..", ".XoooX.", "XooWooX", "XoWWWoX", "XooWooX", ".XoooX.", "..XoX..", "...X...", "...X..."]
    p = im.load()
    for dy, row in enumerate(shape):
        for dx, c in enumerate(row):
            if c == "X":
                p[x + dx, y + dy] = (88, 16, 16, 255)
            elif c == "o":
                p[x + dx, y + dy] = color + (255,)
            elif c == "W":
                p[x + dx, y + dy] = (255, 224, 224, 255)


def save(im, name):
    g.scale(im.convert("RGB")).save(os.path.join(OUT, name))


def side_by_side(frames, name, captions):
    k, gap, cap = 3, 12, 24
    w = frames[0].width * k
    out = Image.new("RGB", (w * len(frames) + gap * (len(frames) - 1), frames[0].height * k + cap), (40, 40, 40))
    for i, f in enumerate(frames):
        out.paste(g.scale(f.convert("RGB"), k), (i * (w + gap), cap))
    for i, c in enumerate(captions):
        strip = Image.new("RGBA", (w // 2, cap // 2), (40, 40, 40, 255))
        F_N.draw(strip, 2, -2, c, g.WHITE, (90, 90, 90))
        out.paste(strip.resize((w, cap), Image.NEAREST).convert("RGB"), (i * (w + gap), 0))
    out.save(os.path.join(OUT, name))


# --- Layout A: list + detail --------------------------------------------------------------------------------------

def layout_a():
    im = Image.new("RGBA", (g.W, g.H))
    background(im, "QUESTS")
    x = hint(im, 150, 1, "A", "MAP")
    hint(im, x + 8, 1, "B", "BACK")

    # Quest list (tiles 1-9, rows 3-14): the main quest first, then the finished ones
    g.draw_window(im, 1, 3, 9, 12)
    F_S.draw(im, 8, 23, "ACT 2", g.DARK, g.DARK_SH)
    F_S.draw(im, 80 - F_S.width("%d%%" % ACT_PCT), 23, "%d%%" % ACT_PCT, g.DARK, g.DARK_SH)
    bar(im, 8, 35, 72, ACT_PCT, 4)
    g.fill_rect(im, 8, 42, 72, 1, g.DARK_SH)
    y = 45
    for name, state in LIST:
        if state == "now":
            F_S.draw(im, 8, y, "▶", g.DARK, g.DARK_SH)
        elif state == "done":
            checkbox(im, 8, y + 2, True)
        col = {"now": (g.BLUE, g.BLUE_SH), "done": (g.DARK, g.DARK_SH), "locked": (g.GRAY, g.GRAY_SH)}[state]
        F_S.draw(im, 20, y, name, *col)
        y += 13

    # Detail (tiles 11-28, rows 3-14)
    g.draw_window(im, 11, 3, 18, 12)
    F_N.draw(im, 88, 23, QUEST, g.BLUE, g.BLUE_SH)
    pct = "%d%%" % QUEST_PCT
    F_N.draw(im, 232 - F_N.width(pct), 23, pct, g.DARK, g.DARK_SH)
    pin(im, 88, 40)
    F_S.draw(im, 98, 38, PLACE, g.DARK, g.DARK_SH)
    bar(im, 88, 51, 144, QUEST_PCT, 5)
    y = 59
    for text, done, counter in TASKS:
        task_row(im, 90, y, text, done, counter)
        y += 10

    # Next step (rows 17-18)
    g.draw_window(im, 1, 17, 28, 2)
    F_N.draw(im, 8, 137, "NEXT:", g.RED, g.RED_SH)
    F_N.draw(im, 40, 137, NEXT, g.DARK, g.DARK_SH)
    save(im, "layout_a_list_detail.png")


# --- Layout B: one quest per page (journal) ----------------------------------------------------------------------

def layout_b():
    im = Image.new("RGBA", (g.W, g.H))
    background(im, "ACT 2: UNIVERSITY")
    bar(im, 150, 6, 52, ACT_PCT)
    F_N.draw(im, 206, 1, "%d%%" % ACT_PCT, g.WHITE, HEADER_SH)

    g.draw_window(im, 1, 3, 28, 10)
    # Page header: L / R flip between quests
    im.alpha_composite(keypad("L"), (8, 26))
    im.alpha_composite(keypad("R"), (216, 26))
    F_N.draw(im, 120 - F_N.width(QUEST) // 2, 23, QUEST, g.BLUE, g.BLUE_SH)
    sub = "Main quest 4 of 9"
    F_S.draw(im, 120 - F_S.width(sub) // 2, 37, sub, g.GRAY, g.GRAY_SH)
    pin(im, 10, 52)
    F_S.draw(im, 20, 50, PLACE, g.DARK, g.DARK_SH)
    bar(im, 100, 53, 104, QUEST_PCT, 6)
    F_S.draw(im, 208, 50, "%d%%" % QUEST_PCT, g.DARK, g.DARK_SH)
    g.fill_rect(im, 8, 63, 224, 1, g.DARK_SH)
    for i, (text, done, counter) in enumerate(TASKS):
        task_row(im, 8 if i < 3 else 128, 67 + (i % 3) * 12, text, done, counter)

    # Next step, in the dialogue box style
    g.draw_window(im, 1, 15, 28, 4)
    F_N.draw(im, 8, 121, "NEXT STEP", g.RED, g.RED_SH)
    hint(im, 190, 121, "A", "MAP", g.DARK, g.DARK_SH)
    F_N.draw(im, 8, 137, NEXT, g.DARK, g.DARK_SH)
    save(im, "layout_b_journal.png")


# --- Layout C: bag style, a mini map with the blinking target ----------------------------------------------------

def mini_map(target, size, zoom=2, blink_on=True):
    """A crop of the Kanto map around a map section (grid x, y), zoomed 2x like the Town Map's zoom."""
    full = g.region_map_kanto()
    rx, ry, rw, rh = g.mapsec_rect(*target)
    cw, ch = size[0] // zoom, size[1] // zoom
    cx, cy = rx + rw // 2 - cw // 2, ry + rh // 2 - ch // 2
    crop = full.crop((cx, cy, cx + cw, cy + ch)).resize(size, Image.NEAREST).convert("RGBA")
    if blink_on:
        bx, by = (rx - cx) * zoom, (ry - cy) * zoom
        p = crop.load()
        for i in range(-3, rw * zoom + 3):
            for t in (0, 1):
                for yy in (by - 3 + t, by + rh * zoom + 2 - t):
                    if 0 <= bx + i < size[0] and 0 <= yy < size[1]:
                        p[bx + i, yy] = BLINK + (255,)
                for xx in (bx - 3 + t, bx + rw * zoom + 2 - t):
                    if 0 <= xx < size[0] and 0 <= by + i < size[1]:
                        p[xx, by + i] = BLINK + (255,)
    return crop


def layout_c():
    im = Image.new("RGBA", (g.W, g.H))
    background(im, None)
    x = 4
    for name, on in [("MAIN", True), ("DONE", False)]:
        w = F_N.width(name) + 12
        g.fill_rect(im, x, 2, w, 14, (248, 248, 248) if on else (96, 112, 152))
        F_N.draw(im, x + 6, 1, name, g.DARK if on else g.WHITE, g.DARK_SH if on else HEADER)
        x += w + 2
    x = hint(im, 128, 1, "LR", "TAB")
    hint(im, x + 8, 1, "A", "MAP")

    # Mini map, left (tiles 1-10, rows 3-10)
    g.draw_window(im, 1, 3, 10, 8)
    im.alpha_composite(mini_map((14, 9), (80, 48)), (8, 24))
    F_S.draw(im, 8, 74, PLACE, g.DARK, g.DARK_SH)

    # Quest list, right (tiles 13-28, rows 3-12)
    g.draw_window(im, 13, 3, 16, 10)
    rows = [("WATER course", QUEST_PCT, "now"), ("Club night 2", 0, "next"), ("? ? ?", 0, "locked"), ("? ? ?", 0, "locked")]
    y = 23
    for name, pct, state in rows:
        if state == "now":
            F_N.draw(im, 104, y, "▶", g.DARK, g.DARK_SH)
            F_N.draw(im, 112, y, name, g.BLUE, g.BLUE_SH)
            bar(im, 112, y + 15, 84, pct)
            F_S.draw(im, 200, y + 11, "%d%%" % pct, g.DARK, g.DARK_SH)
            y += 24
        else:
            F_N.draw(im, 112, y, name, g.GRAY, g.GRAY_SH)
            y += 15
    g.fill_rect(im, 104, 89, 128, 1, g.DARK_SH)
    F_S.draw(im, 104, 92, "ACT 2", g.DARK, g.DARK_SH)
    bar(im, 134, 96, 62, ACT_PCT)
    F_S.draw(im, 200, 92, "%d%%" % ACT_PCT, g.DARK, g.DARK_SH)

    # The tasks and next step, as the bag's description box
    g.draw_window(im, 1, 14, 28, 5)
    for i, (text, done, counter) in enumerate(TASKS[:4]):
        task_row(im, 8 if i % 2 == 0 else 124, 111 + (i // 2) * 11, text, done, counter)
    F_N.draw(im, 8, 136, "NEXT:", g.RED, g.RED_SH)
    F_N.draw(im, 40, 136, NEXT, g.DARK, g.DARK_SH)
    save(im, "layout_c_bag_minimap.png")


# --- Start menu ---------------------------------------------------------------------------------------------------

def campus_background():
    tmp = os.path.join(tempfile.gettempdir(), "quest_mockup_campus.png")
    subprocess.check_call([sys.executable, g.path("tools", "hack", "render_layout.py"), "LAYOUT_CELADON_UNIVERSITY", tmp])
    full = Image.open(tmp).convert("RGBA")
    fx, fy = 31 * 16, 33 * 16  # the fountain
    bg = full.crop((fx - 112, fy - 56, fx - 112 + g.W, fy - 56 + g.H))
    red = g.sprite("graphics/object_events/pics/people/red.png", 0, (16, 32))
    bg.alpha_composite(red, (112, 64))
    return bg


def start_menu():
    im = campus_background()
    items = ["POKéDEX", "POKéMON", "BAG", "QUESTS", "RED", "SAVE", "OPTION", "EXIT"]
    g.draw_window(im, 22, 1, 7, len(items) * 2)
    for i, name in enumerate(items):
        y = 8 + i * 16
        F_N.draw(im, 184, y + 1, name, g.DARK, g.DARK_SH)
        if name == "QUESTS":
            F_N.draw(im, 176, y + 1, "▶", g.DARK, g.DARK_SH)
    save(im, "start_menu_quests.png")


# --- Region map ---------------------------------------------------------------------------------------------------

def map_frame(target, name_text, blink_on, player=(11, 6), route=None, passed=0, pin_style=False):
    im = g.region_map_kanto().convert("RGBA")
    p = im.load()
    if route:
        # The road home: dotted ahead, solid where it's been walked
        pts = [(g.mapsec_rect(x, y)[0] + 4, g.mapsec_rect(x, y)[1] + 4) for x, y in route]
        for i in range(len(pts) - 1):
            (x0, y0), (x1, y1) = pts[i], pts[i + 1]
            n = max(abs(x1 - x0), abs(y1 - y0))
            for s in range(n + 1):
                x = x0 + (x1 - x0) * s // n
                y = y0 + (y1 - y0) * s // n
                walked = i < passed
                if walked or s % 4 < 2:
                    col = (40, 72, 200, 255) if walked else (248, 248, 248, 255)
                    for dx in (0, 1):
                        for dy in (0, 1):
                            p[x + dx - 1, y + dy - 1] = col
    rx, ry, rw, rh = g.mapsec_rect(*target)
    if blink_on:
        if pin_style:
            for yy in range(ry, ry + rh):
                for xx in range(rx, rx + rw):
                    r, gg, b, a = p[xx, yy]
                    p[xx, yy] = ((r + 2 * 248) // 3, (gg + 2 * 64) // 3, (b + 2 * 64) // 3, 255)
            pin(im, rx, ry - 10)
        else:
            # The fly map's red outline box (FLYDESTICON_RED_OUTLINE), 16x16 around the square
            box = Image.open(g.path("graphics", "pokenav", "region_map", "fly_target_icons.png"))
            bp, bpal = box.load(), box.getpalette()
            for yy in range(16):
                for xx in range(16):
                    v = bp[xx, 40 + yy]
                    if v:
                        p[rx - 4 + xx, ry - 4 + yy] = tuple(bpal[v * 3:v * 3 + 3]) + (255,)
    px, py, _, _ = g.mapsec_rect(*player)
    im.alpha_composite(g.sprite("graphics/pokenav/region_map/red_icon.png", 0, (16, 16)), (px - 4, py - 4))
    # The Town Map's windows: the title top right, the quest line along the bottom
    g.draw_window(im, 22, 1, 7, 2)
    F_N.draw(im, 176, 9, "KANTO", g.DARK, g.DARK_SH)
    g.draw_window(im, 9, 17, 20, 2)
    F_N.draw(im, 72, 137, name_text, g.DARK, g.DARK_SH)
    return im


def maps():
    # A: the Town Map with the fly map's red box blinking on the quest's place (the player is in Vermilion's trip)
    frames = [map_frame((14, 9), "QUEST: VERMILION CITY", on, player=(14, 9)) for on in (True, False)]
    side_by_side(frames, "map_a_red_box_blink.png", ["Frame 1: box on", "Frame 2, 16 frames later: off"])
    # B: the road home: the route drawn, the walked part solid, the next stop tinted with a pin, blinking
    route = [(11, 6), (14, 6), (14, 9), (15, 9), (4, 5), (4, 8), (4, 11)]
    frames = [map_frame((15, 9), "ROAD HOME: DIGLETT'S CAVE", on, player=(14, 9), route=route, passed=2,
                        pin_style=True) for on in (True, False)]
    side_by_side(frames, "map_b_road_home_blink.png", ["Frame 1: next stop lit", "Frame 2: off"])


if __name__ == "__main__":
    layout_a()
    layout_b()
    layout_c()
    start_menu()
    maps()
