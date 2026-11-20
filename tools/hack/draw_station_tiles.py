#!/usr/bin/env python3
"""Draws the ranger station's custom metatiles (Act 2 finale, docs/hack/act2_finale.md).

Pokémon Mansion tileset (B1F, the clock), after the furnaces of draw_mansion_furnace.py (which this runs first, so
the tileset is rebuilt from the same base every time). A new row of 16 tiles at the end of tiles.png, palette 09
(the furnaces' iron, brick and flame colors):
    0x384 torch, unlit     0x385 torch, lit       (standing iron braziers on the basement floor; the lit flames
                                                   flicker: frames in anim/torch/)
    0x386 moon emblem      0x387 sun emblem       (painted on the wall, above the furnaces)
    0x388-0x3A0            the stone mosaic in the middle of the clock, 5x5 (draw_station_mosaic.py, palette 10)
    0x3A1, 0x3A2           its middle slab slid halfway, and slid away (the shaft and its ladder down, MB_LADDER)
    0x3A3-                 the lone ranger's furniture (FURNITURE: FireRed's building furniture on the basement
                           floor), then the cobwebbed copies of what B1F's cobwebs hang on (compose_ranger_station
                           B1_WEBS). New tiles go in the tileset's free (unused) slots.

Cerulean Cave tileset (B2F, the flood). A new row of tiles at the end of tiles.png, its palette 08 (cave greys):
    0x306 stepping stone   (a flat rock in the cave's pond water, walkable)

    0x307 the ladder up's top half on the chamber floor (no new tiles)

Mt. Ember tileset (B4F). No new tiles:
    0x362 the ladder up's top half on the altar room's floor

Generic building 2 tileset (1F). No new tiles:
    0x338 a cardboard box  (the building tileset's, on the gate's floor)

Re-running redraws them in place.

    python3 tools/hack/draw_station_tiles.py
"""
import math
import os
import struct

import draw_mansion_furnace as furnace
from draw_mansion_furnace import read_png, write_png

ROOT = furnace.ROOT
PRIMARY_TILES = 640
PRIMARY_METATILES = 640

# --- Pokémon Mansion ------------------------------------------------------------------------------------------------
MANSION = os.path.join(ROOT, 'data/tilesets/secondary/pokemon_mansion_frlg')
MANSION_METATILES = 260   # The tileset's own 258 and the two furnaces
MANSION_TILE_ROWS = 23    # The tileset's own 22 rows and the furnaces' row
MANSION_PAL = 9
KEYS = '.KdilbrpoROYWmax'  # Palette 09 (draw_mansion_furnace.PALETTE), x = white

# A standing iron brazier drawn like FireRed's objects, seen from above at an angle: a rim round a mouth full of coals
# and ash (its far inner wall in shadow), the lip's dark underside, a riveted band, a rounded bowl, a knob and a foot,
# all with a dark outline and lit from the upper left (white glints on the rim, a rivet and the knob). Its foot ends
# two rows short of the tile's bottom so its shadow (torch_shadow, on the floor layer) shows below it. Lit, flames
# rise out of the mouth over the far rim, the coals burn white-hot and the near rim takes the firelight; only the top
# two tiles change, and the flames flicker (TORCH_FLICKER_BOX).
TORCH_UNLIT = [
    '................',
    '....KKKKKKKK....',
    '..KKlliiiiddKK..',
    '.KlKddddddddKdK.',
    '.KxomoaoomaaodK.',
    '.KllaoomoamoidK.',
    '.KxllllllliiddK.',
    '..KddddddddddK..',
    '..KdxddlddiddK..',
    '...KlliiiiddK...',
    '....KKliidKK....',
    '...KKxliiddKK...',
    '..KllliiiidddK..',
    '..KKKKKKKKKKKK..',
    '................',
    '................',
]
TORCH_LIT = [
    '......R...R.....',
    '....KRORKROK....',
    '..KKROYORYORKK..',
    '.KlKOYWOYWYOKdK.',
    '.KxOYWWYWWYYOdK.',
    '.KlpOYWWWWYOpdK.',
    '.KxlpppppppiddK.',
] + TORCH_UNLIT[7:]
TORCH_FLICKER_BOX = (2, 0, 13, 4)


def torch_shadow(lit_floor, shaded_floor):
    """The floor layer's bottom two tiles under the torch: the floor darkened in a flat oval round the foot, a little
    to the right (the light comes from the upper left). Its rim takes the Mansion's own wall-shaded floor; its core is
    a step darker again (the floor palette's next shade down)."""
    darker = {lit: shaded for lit_row, shaded_row in zip(lit_floor, shaded_floor) for lit, shaded in zip(lit_row, shaded_row)}
    pic = [[lit_floor[y % 8][x % 8] for x in range(16)] for y in range(8, 16)]
    for y in range(8, 16):
        for x in range(16):
            d = ((x - 8.6) / 6.5) ** 2 + ((y - 13.6) / 1.3) ** 2
            if d < 1:
                c = darker[lit_floor[y % 8][x % 8]]
                pic[y - 8][x] = darker.get(c, c - 1) if d < 0.45 else c
    return [[row[x0:x0 + 8] for row in pic] for x0 in (0, 8)]
def plaque(art):
    """A round iron plaque on the wall, inside the tile with a pixel to spare: an outline, a bevel lit from the upper
    left (shaded lower right) and a dark face. Returns the face's radius."""
    for y in range(16):
        for x in range(16):
            dx, dy = x - 7.5, y - 7.5
            d = math.hypot(dx, dy)
            if d < 5.2:
                art[y][x] = 'd'
            elif d < 6.2:
                art[y][x] = 'l' if dx + dy < -2 else 'i' if dx + dy < 3 else 'K'
            elif d < 7.0:
                art[y][x] = 'K'
    return 5.2


def emblem_moon():
    """A round plaque with a crescent moon (lit on its outer edge) and two stars."""
    art = [['.'] * 16 for _ in range(16)]
    plaque(art)
    for y in range(16):
        for x in range(16):
            d, cut = math.hypot(x - 7.5, y - 7.5), math.hypot(x - 9.4, y - 6.4)
            if d < 4.0 and cut > 3.3:
                art[y][x] = 'Y' if cut < 4.1 or d > 3.2 and x > 7 else 'W'
    for x, y in ((11, 10), (10, 4)):
        art[y][x] = 'x'
    return [''.join(r) for r in art]


def emblem_sun():
    """A round plaque with a sun: a disc shaded from a white-hot upper left to an orange lower right, ringed red, and
    eight short rays."""
    art = [['.'] * 16 for _ in range(16)]
    plaque(art)
    for y in range(16):
        for x in range(16):
            dx, dy = x - 7.5, y - 7.5
            d = math.hypot(dx, dy)
            if d < 2.2:
                art[y][x] = 'W' if dx + dy < -1 else 'Y'
            elif d < 2.9:
                art[y][x] = 'O' if dx + dy > 1 else 'Y'
            elif d < 3.4:
                art[y][x] = 'R'
    for x, y in ((7, 3), (8, 3), (7, 12), (8, 12), (3, 7), (3, 8), (12, 7), (12, 8),
                 (4, 4), (11, 4), (4, 11), (11, 11)):
        art[y][x] = 'Y' if x + y < 15 else 'O'
    return [''.join(r) for r in art]


def quads(art, keys):
    """The four 8x8 tiles of a 16x16 picture: top-left, top-right, bottom-left, bottom-right."""
    px = [[keys.index(c) for c in row] for row in art]
    assert len(px) == 16 and all(len(r) == 16 for r in px), art
    return [[r[x0:x0 + 8] for r in px[y0:y0 + 8]] for y0, x0 in ((0, 0), (0, 8), (8, 0), (8, 8))]


def add_tile_row(folder, base_rows, tiles):
    png = os.path.join(folder, 'tiles.png')
    width, rows, plte = read_png(png)
    rows = rows[:base_rows * 8] + [[0] * width for _ in range(8)]
    for n, tile in enumerate(tiles):
        for y in range(8):
            rows[base_rows * 8 + y][n * 8:n * 8 + 8] = tile[y]
    write_png(png, width, rows, plte)
    return PRIMARY_TILES + base_rows * 16


def add_metatiles(folder, base_count, metatiles, attr):
    metatiles_path = os.path.join(folder, 'metatiles.bin')
    attrs_path = os.path.join(folder, 'metatile_attributes.bin')
    data = open(metatiles_path, 'rb').read()[:base_count * 16]
    attrs = open(attrs_path, 'rb').read()[:base_count * 4]
    for layers in metatiles:
        data += struct.pack('<8H', *layers)
        attrs += struct.pack('<I', attr)
    open(metatiles_path, 'wb').write(data)
    open(attrs_path, 'wb').write(attrs)
    assert os.path.getsize(attrs_path) * 4 == os.path.getsize(metatiles_path), 'metatile attributes out of step'


def layers_of(folder, metatile_id):
    data = open(os.path.join(folder, 'metatiles.bin'), 'rb').read()
    i = metatile_id - PRIMARY_METATILES
    return list(struct.unpack('<8H', data[i * 16:i * 16 + 16]))


LAYER_COVERED = 1 << 29  # Both layers under objects: the player walks in front of the torch and the stone
MB_NORMAL = 0x00


HFLIP, VFLIP = 1 << 10, 1 << 11
SHADOW_SLOTS = (131, 132)                            # Free tiles (blank, unused) for the torch's shadow
BLANK_TILE = 0                                       # The primary tileset's first tile is empty


def secondary_tile(folder, entry):
    """The 8x8 pixels (palette indices) of a metatile entry's tile in this secondary tileset (no flips)."""
    assert entry & 0xC00 == 0 and (entry & 0x3FF) >= PRIMARY_TILES, hex(entry)
    n = (entry & 0x3FF) - PRIMARY_TILES
    width, rows, _ = read_png(os.path.join(folder, 'tiles.png'))
    per_row = width // 8
    return [rows[(n // per_row) * 8 + y][(n % per_row) * 8:(n % per_row) * 8 + 8] for y in range(8)]


def put_tiles(folder, slots, tiles):
    png = os.path.join(folder, 'tiles.png')
    width, rows, plte = read_png(png)
    per_row = width // 8
    for slot, tile in zip(slots, tiles):
        for y in range(8):
            rows[(slot // per_row) * 8 + y][(slot % per_row) * 8:(slot % per_row) * 8 + 8] = tile[y]
    write_png(png, width, rows, plte)


# B1F's metatiles after the torches and emblems (0x384-0x387), in this order; compose_ranger_station.py places them
# by these same numbers (mosaic_id, furniture_id, webbed_id), so the two can't drift apart.
MOSAIC_FIRST = 0x388                     # The dial, 5x5, row by row; its middle (0x394) is the slab, closed
MOSAIC_SLAB = MOSAIC_FIRST + 12
MOSAIC_HALF, MOSAIC_OPEN = 0x3A1, 0x3A2  # The slab slid halfway; slid away, the shaft and its ladder (MB_LADDER)
MB_LADDER = 0x61

# The lone ranger's things: FireRed's building furniture (gTileset_BuildingFrlg, drawn on its top layer) stood on
# the basement's floor instead of the building's planks, on each of the floor pieces it may stand on
# Several of these are halves of a bigger thing, and the rest of it has to be stood next to them or they read as cut
# off: the building tileset lays its metatiles out eight to a row, so a piece's other half is the next one along
# (tv/tv_r) or the one below it (sink/sink_b). New names go on the end - furniture_id() numbers them in order, and
# the cobwebs are numbered after them.
FURNITURE = {
    'bed_l': 0x18, 'bed_r': 0x19, 'shelf_l': 0x73, 'shelf_r': 0x74, 'sink': 0x31, 'stove': 0x32, 'tv': 0x75,
    'crate_l': 0x99, 'crate_r': 0x9A, 'plant': 0x93, 'cabinet_l': 0x33, 'cabinet_r': 0x34,
    'tv_r': 0x76, 'plant_t': 0x8B, 'sink_b': 0x39, 'stove_b': 0x3A, 'cabinet_l_b': 0x3B, 'cabinet_r_b': 0x3C,
}
FURNITURE_FLOORS = (0x286, 0x287)  # The floor, and the floor shaded by a wall
FURNITURE_FIRST = 0x3A3


def mosaic_id(dx, dy):
    return MOSAIC_FIRST + dy * 5 + dx


def furniture_id(name, floor):
    return FURNITURE_FIRST + list(FURNITURE).index(name) * len(FURNITURE_FLOORS) + FURNITURE_FLOORS.index(floor)


# Cobwebs, baked into copies of the metatiles they hang on (furniture, wall corners): (metatile, corner) where
# corner is the web's anchor, 'TL' or 'TR'. Metatiles from gTileset_PokemonMansion or the furniture above.
WEBBED_FIRST = FURNITURE_FIRST + len(FURNITURE) * len(FURNITURE_FLOORS)
WEBBED = []   # Filled in from compose_ranger_station.B1_WEBS (webbed_targets), so the list lives in one place


def webbed_id(metatile, corner, targets):
    return WEBBED_FIRST + targets.index((metatile, corner))


# A cobweb strung across a top-left corner, drawn by hand: three spokes from the corner (along the top, down the
# middle, down the side), joined by threads that sag toward the corner; the outer one broken, a loose end hanging
WEB_SPOKES = [[(x, 0 if x < 7 else 1) for x in range(14)], [(i, i) for i in range(10)],
              [(0 if y < 7 else 1, y) for y in range(14)]]
WEB_THREADS = [
    [(4, 0), (3, 1), (3, 2), (3, 3), (2, 3), (1, 3), (0, 4)],
    [(8, 1), (7, 2), (7, 3), (6, 4), (6, 5), (6, 6), (5, 6), (4, 6), (3, 7), (2, 7), (1, 8)],
    [(12, 1), (11, 2), (11, 3), (10, 4), (10, 5), (10, 6), (9, 7), (9, 8), (9, 9), (8, 9), (7, 10),
     (3, 11), (2, 12), (1, 12), (9, 10), (10, 11), (10, 12)],
]


def web(corner):
    """The cobweb, 16x16: 1 the strands (bright: the spokes near the corner, the inner thread), 2 the finer threads
    (faint), 0 nothing. Mirrored for a top-right corner."""
    pic = [[0] * 16 for _ in range(16)]
    for spoke in WEB_SPOKES:
        for i, (x, y) in enumerate(spoke):
            pic[y][x] = 1 if i < 6 else 2
    for n, thread in enumerate(WEB_THREADS):
        for x, y in thread:
            if pic[y][x] == 0:
                pic[y][x] = 1 if n == 0 else 2
    if corner == 'TR':
        pic = [row[::-1] for row in pic]
    return pic


def read_pal(path):
    lines = open(path).read().split('\n')
    return [tuple(map(int, l.split())) for l in lines[3:19]]


def tile_pixels(entry, tiles):
    """The 8x8 palette indices an entry shows (its flips applied). tiles: (primary rows, secondary rows)."""
    t = entry & 0x3FF
    rows, n = (tiles[0], t) if t < PRIMARY_TILES else (tiles[1], t - PRIMARY_TILES)
    per = len(rows[0]) // 8
    px = [rows[(n // per) * 8 + y][(n % per) * 8:(n % per) * 8 + 8] for y in range(8)]
    if entry & HFLIP:
        px = [r[::-1] for r in px]
    if entry & VFLIP:
        px = px[::-1]
    return [list(r) for r in px]


class TileSink:
    """New 8x8 tiles for the Mansion tileset, put in its free slots; identical tiles (and mirror images) shared."""

    def __init__(self, slots):
        self.slots, self.tiles = list(slots), []

    def entry(self, px, pal):
        if not any(any(r) for r in px):
            return BLANK_TILE
        for n, t in enumerate(self.tiles):
            for flip, f in ((0, lambda p: p), (HFLIP, lambda p: [r[::-1] for r in p]), (VFLIP, lambda p: p[::-1]),
                            (HFLIP | VFLIP, lambda p: [r[::-1] for r in p[::-1]])):
                if t == f(px):
                    return (PRIMARY_TILES + self.slots[n]) | flip | (pal << 12)
        self.tiles.append(px)
        assert len(self.tiles) <= len(self.slots), 'Mansion tileset: out of free tiles'
        return (PRIMARY_TILES + self.slots[len(self.tiles) - 1]) | (pal << 12)


def nearest(pal, rgb):
    return min(range(1, 16), key=lambda i: sum((a - b) ** 2 for a, b in zip(pal[i], rgb)))


def bake_web(layers, corner, tiles, palettes, sink):
    """A metatile's eight entries with a cobweb drawn over them: on its top layer, in each tile's own palette (the
    web's two colors matched to that palette's nearest), or in the mosaic's palette where the top layer is empty."""
    pic = web(corner)
    out = list(layers)
    for q in range(4):
        part = [row[(q % 2) * 8:(q % 2) * 8 + 8] for row in pic[(q // 2) * 8:(q // 2) * 8 + 8]]
        if not any(any(r) for r in part):
            continue
        top = layers[4 + q]
        if top & 0x3FF:
            pal = top >> 12
            px = tile_pixels(top, tiles)
        else:
            pal, px = MOSAIC_PAL, [[0] * 8 for _ in range(8)]
        colors = {1: nearest(palettes[pal], (238, 238, 230)), 2: nearest(palettes[pal], (172, 172, 180))}
        for y in range(8):
            for x in range(8):
                if part[y][x]:
                    px[y][x] = colors[part[y][x]]
        out[4 + q] = sink.entry(px, pal)
    return out


MOSAIC_PAL = 10
LAYER_MASK = 3 << 29


def free_slots(folder, metatile_count, keep):
    """Tiles no metatile up to metatile_count uses (and not in keep): free for new art."""
    data = open(os.path.join(folder, 'metatiles.bin'), 'rb').read()[:metatile_count * 16]
    used = {(e & 0x3FF) - PRIMARY_TILES for i in range(metatile_count)
            for e in struct.unpack('<8H', data[i * 16:i * 16 + 16]) if (e & 0x3FF) >= PRIMARY_TILES}
    width, rows, _ = read_png(os.path.join(folder, 'tiles.png'))
    total = (width // 8) * (len(rows) // 8)
    return [n for n in range(1, total) if n not in used and n not in keep]


def draw_mansion(web_targets=()):
    import draw_station_mosaic as mosaic
    furnace.main()
    unlit, lit, moon, sun = (quads(a, KEYS) for a in (TORCH_UNLIT, TORCH_LIT, emblem_moon(), emblem_sun()))
    # Tiles: torch top (unlit) x2, torch bottom x2, torch top (lit) x2, moon x4, sun x4
    first = add_tile_row(MANSION, MANSION_TILE_ROWS, unlit + lit[:2] + moon + sun)
    # The torch's shadow on the floor (floor tile 0x2B9, its wall-shaded version 0x2A9; both palette 12)
    lit_floor, shaded_floor = (secondary_tile(MANSION, layers_of(MANSION, m)[0]) for m in (0x286, 0x287))
    put_tiles(MANSION, SHADOW_SLOTS, torch_shadow(lit_floor, shaded_floor))
    # The lit torch's flames flicker (its top two tiles, animated by InitTilesetAnim_PokemonMansion)
    flames = furnace.flicker_frames(TORCH_LIT, 'ROYW', '.', TORCH_FLICKER_BOX)
    furnace.write_anim_frames(os.path.join(MANSION, 'anim/torch'), [furnace.tiles_of(f, 0, 1) for f in flames],
                              KEYS, furnace.PALETTE)

    def top(*tiles):
        return [(first + t) | (MANSION_PAL << 12) for t in tiles]
    floor = layers_of(MANSION, 0x286)[:4]       # The basement floor
    shadowed = floor[:2] + [(PRIMARY_TILES + n) | (floor[2] & 0xF000) for n in SHADOW_SLOTS]
    wall = layers_of(MANSION, 0x2AA)[4:]        # The wall's face (drawn on its top layer)
    add_metatiles(MANSION, MANSION_METATILES, [
        shadowed + top(0, 1, 2, 3),              # 0x384 torch, unlit
        shadowed + top(4, 5, 2, 3),              # 0x385 torch, lit
        wall + top(6, 7, 8, 9),                  # 0x386 moon
        wall + top(10, 11, 12, 13),              # 0x387 sun
    ], MB_NORMAL | LAYER_COVERED)
    count = MANSION_METATILES + 4
    torch_row = [first - PRIMARY_TILES + n for n in range(16)]
    sink = TileSink(free_slots(MANSION, count, set(SHADOW_SLOTS) | set(torch_row)))

    # The mosaic, palette 10: the dial on the floor (both layers under the player), then the slab slid half away and
    # the open shaft
    open(os.path.join(MANSION, 'palettes/%02d.pal' % MOSAIC_PAL), 'w').write(
        'JASC-PAL\r\n0100\r\n16\r\n' + ''.join('%d %d %d\r\n' % c for c in mosaic.MOSAIC_PALETTE))
    floor_attr = (attr_of(MANSION, 0x286) & ~LAYER_MASK) | LAYER_COVERED

    def mosaic_square(pic, sx, sy):
        return floor + [sink.entry([row[sx + (q % 2) * 8:sx + (q % 2) * 8 + 8]
                                    for row in pic[sy + (q // 2) * 8:sy + (q // 2) * 8 + 8]], MOSAIC_PAL)
                        for q in range(4)]
    dial = mosaic.dial()
    add_metatiles(MANSION, count, [mosaic_square(dial, x * 16, y * 16) for y in range(5) for x in range(5)],
                  floor_attr)
    count += 25
    middle = mosaic.SLAB[0]
    add_metatiles(MANSION, count, [mosaic_square(mosaic.slab(mosaic.dial(), 'half'), middle, middle)], floor_attr)
    add_metatiles(MANSION, count + 1, [mosaic_square(mosaic.slab(mosaic.dial(), 'open'), middle, middle)],
                  (floor_attr & ~0x1FF) | MB_LADDER)
    count += 2
    assert count == FURNITURE_FIRST - PRIMARY_METATILES

    # The furniture on the basement floor
    building = os.path.join(ROOT, 'data/tilesets/primary/building_frlg')
    battrs = open(os.path.join(building, 'metatile_attributes.bin'), 'rb').read()
    bmeta = open(os.path.join(building, 'metatiles.bin'), 'rb').read()
    for name, m in FURNITURE.items():
        top_layer = list(struct.unpack('<8H', bmeta[m * 16:m * 16 + 16]))[4:]
        attr = struct.unpack('<I', battrs[m * 4:m * 4 + 4])[0]
        for f in FURNITURE_FLOORS:
            add_metatiles(MANSION, count, [layers_of(MANSION, f)[:4] + top_layer], attr)
            count += 1
    assert count == WEBBED_FIRST - PRIMARY_METATILES

    # Cobwebs on the furniture and in the corners
    _, prows, _ = read_png(os.path.join(building, 'tiles.png'))
    _, srows, _ = read_png(os.path.join(MANSION, 'tiles.png'))
    palettes = [read_pal(os.path.join(building if i < 7 else MANSION, 'palettes/%02d.pal' % i)) for i in range(16)]
    palettes[MOSAIC_PAL] = mosaic.MOSAIC_PALETTE
    for m, corner in web_targets:
        layers = layers_of(MANSION, m)
        add_metatiles(MANSION, count, [bake_web(layers, corner, (prows, srows), palettes, sink)], attr_of(MANSION, m))
        count += 1
    put_tiles(MANSION, sink.slots[:len(sink.tiles)], sink.tiles)
    assert count <= 384, 'Mansion tileset: %d metatiles' % count
    print('Mansion: torch 0x384/0x385, moon 0x386, sun 0x387, mosaic 0x%X-0x%X, furniture 0x%X-, %d cobwebs '
          '(%d new tiles, %d left)' % (MOSAIC_FIRST, MOSAIC_OPEN, FURNITURE_FIRST, len(web_targets), len(sink.tiles),
                                      len(sink.slots) - len(sink.tiles)))

# --- Cerulean Cave --------------------------------------------------------------------------------------------------
CAVE = os.path.join(ROOT, 'data/tilesets/secondary/cerulean_cave_frlg')
CAVE_METATILES = 134
CAVE_TILE_ROWS = 16
CAVE_PAL = 8
POND_WATER = (0x1A0, 0x1A1, 0x1A2, 0x1A3)  # The primary's pond water (metatile 0x12B's bottom layer), palette 4


def stepping_stone():
    """A flat stone seen from above, sitting in the water clear of the tile's edges: a dark outline, a top face lit
    from the upper left (a highlight, the stone, its shade) with a few pits, and a darker side under it at the
    waterline. Keys are palette 08's indices."""
    art = [['0'] * 16 for _ in range(16)]
    face = lambda x, y: ((x - 7.5) / 6.1) ** 2 + ((y - 6.4) / 4.6) ** 2 < 1
    body = lambda x, y: face(x, y) or ((x - 7.5) / 6.1) ** 2 + ((y - 8.4) / 4.6) ** 2 < 1
    for y in range(16):
        for x in range(16):
            if not body(x, y):
                continue
            edge = any(not body(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if edge:
                art[y][x] = 'f'
            elif not face(x, y):
                art[y][x] = '4'
            else:
                shade = (x - 7.5) * 0.6 + (y - 6.4)
                art[y][x] = '1' if shade < -4.5 else '2' if shade < 1.8 else '3'
    for y in range(1, 16):  # A highlight just inside the outline, on the lit upper left
        for x in range(1, 16):
            if art[y][x] == '2' and 'f' in (art[y - 1][x], art[y][x - 1]) and (x - 7.5) * 0.6 + (y - 6.4) < -1:
                art[y][x] = '1'
    for x, y in ((5, 6), (9, 4), (10, 8), (6, 9)):
        art[y][x] = '3' if art[y][x] == '2' else '4'
    return [''.join(r) for r in art]


GENERAL = os.path.join(ROOT, 'data/tilesets/primary/general_frlg')
POND_BANK_PAL = 1            # The primary's palette for the pond's banks (their tiles lie over the water)
POND_SHADE_PAL = 6           # The pond water's palette in the banks' shadow
VFLIP = 1 << 11


def bank_upside_down(metatile_id):
    """A pond bank piece of the primary turned upside down (its banks above become banks below): its bank tiles
    flipped and swapped top to bottom, the water's shaded quarters swapped with them. The water's own tiles stay
    the right way up (they're animated)."""
    data = open(os.path.join(GENERAL, 'metatiles.bin'), 'rb').read()
    layers = struct.unpack('<8H', data[metatile_id * 16:metatile_id * 16 + 16])
    order = (2, 3, 0, 1)
    water = [(POND_WATER[k] | (layers[order[k]] & 0xF000)) for k in range(4)]
    banks = [layers[4 + order[k]] ^ VFLIP if layers[4 + order[k]] & 0x3FF else 0 for k in range(4)]
    return water + banks


def primary_attr(metatile_id):
    data = open(os.path.join(GENERAL, 'metatile_attributes.bin'), 'rb').read()
    return struct.unpack('<I', data[metatile_id * 4:metatile_id * 4 + 4])[0]


def draw_cave():
    stone = quads(stepping_stone(), '012345' + '6789abcde' + 'f')
    first = add_tile_row(CAVE, CAVE_TILE_ROWS, stone)
    water = [t | (4 << 12) for t in POND_WATER]
    add_metatiles(CAVE, CAVE_METATILES, [
        water + [(first + t) | (CAVE_PAL << 12) for t in range(4)],   # 0x306 stepping stone
    ], MB_NORMAL | LAYER_COVERED)
    # The ladder up's top half standing on the chamber's floor, not on a scrap of the rock it leans on elsewhere
    add_metatiles(CAVE, CAVE_METATILES + 1, [layers_of(CAVE, 0x281)[:4] + layers_of(CAVE, 0x28E)[4:]],
                  attr_of(CAVE, 0x28E))
    # The pond's banks where the water has floor below it: FireRed's banks above, turned upside down (0x123 along
    # the top, 0x120/0x121 its corners, 0x130/0x131 the nubs where two banks meet round a diagonal)
    add_metatiles(CAVE, CAVE_METATILES + 2, [bank_upside_down(m) for m in (0x123, 0x120, 0x121, 0x131, 0x130)],
                  primary_attr(0x12B))
    print('Cerulean Cave: stepping stone 0x306, ladder top on the floor 0x307, pond banks below 0x308 (along), '
          '0x309/0x30A (corners: left, right), 0x30B/0x30C (nubs: bottom left, bottom right)')


def attr_of(folder, metatile_id):
    i = metatile_id - PRIMARY_METATILES
    return struct.unpack('<I', open(os.path.join(folder, 'metatile_attributes.bin'), 'rb').read()[i * 4:i * 4 + 4])[0]


# --- Mt. Ember (B4F) ------------------------------------------------------------------------------------------------
EMBER = os.path.join(ROOT, 'data/tilesets/secondary/mt_ember_frlg')
EMBER_METATILES = 226


def draw_ember():
    """No new tiles: the ladder up's top half on the altar room's floor (0x281) instead of a scrap of rock."""
    add_metatiles(EMBER, EMBER_METATILES, [layers_of(EMBER, 0x281)[:4] + layers_of(EMBER, 0x28E)[4:]],
                  attr_of(EMBER, 0x28E))
    print('Mt. Ember: ladder top on the floor 0x362')


# --- Generic building 2 (1F) ----------------------------------------------------------------------------------------
GATE = os.path.join(ROOT, 'data/tilesets/secondary/generic_building_2_frlg')
GATE_METATILES = 184
BOX = (0x0DD, 0x0DE, 0x0ED, 0x0EE)  # The building tileset's cardboard box (metatile 0x181), palette 1


SWITCH_SLOTS = range(152, 160)  # The building tileset's last eight tiles, empty and unused
GATE_WALL = 0x28F               # The wall above the dado rail, where the switch is mounted
SWITCH_KEYS = '.Kgsw' + 'xxxxxxxx' + 'Rr' + 'h'  # The primary's palette 0: outline, greys, white, red, wall shade


def wall_switch(pulled):
    """A small breaker box mounted on the wall above the dado rail, its red lever up (or pulled down), with a drop
    shadow on the wall to the right and below. All inside the one square, nothing over the next."""
    art = [['.'] * 16 for _ in range(16)]
    for y in range(2, 13):
        for x in range(4, 14):
            art[y][x] = 'h' if (x == 13 or y == 12) else '.'
    for y in range(1, 12):
        for x in range(3, 13):
            edge = x in (3, 12) or y in (1, 11)
            art[y][x] = 'K' if edge else 'w' if (x == 4 or y == 2) else 'h' if (x == 11 or y == 10) else 's'
    for x, y in ((5, 3), (10, 3), (5, 9), (10, 9)):
        art[y][x] = 'g'                                            # Screws
    for y in range(4, 9):
        art[y][7], art[y][8] = 'K', 'K'                           # The lever's slot
    arm, knob = ((range(3, 6), 1) if not pulled else (range(7, 10), 9))
    for y in arm:
        art[y][7], art[y][8] = 'g', 'g'
    for dy, row in enumerate(('.KK.', 'KrRK', 'KRRK') if not pulled else ('KrRK', 'KRRK', '.KK.')):
        for dx, c in enumerate(row):
            if c != '.':
                art[knob + dy][6 + dx] = c
    return [''.join(r) for r in art]


def draw_gate():
    """The building tileset's box, standing on the gate's floor (0x281); the wall switch, up and pulled."""
    floor = layers_of(GATE, 0x281)[:4]
    add_metatiles(GATE, GATE_METATILES, [floor + [t | (1 << 12) for t in BOX]], MB_NORMAL | LAYER_COVERED)
    up, down = (quads(wall_switch(p), SWITCH_KEYS) for p in (False, True))
    put_tiles(GATE, list(SWITCH_SLOTS), up + down)
    wall = layers_of(GATE, GATE_WALL)[:4]
    switch = lambda first: wall + [(PRIMARY_TILES + SWITCH_SLOTS[first + n]) for n in range(4)]
    add_metatiles(GATE, GATE_METATILES + 1, [switch(0), switch(4)], attr_of(GATE, GATE_WALL))
    print('Generic building 2: box on the floor 0x338, wall switch 0x339 (up) / 0x33A (pulled)')


if __name__ == '__main__':
    import compose_ranger_station
    draw_mansion(compose_ranger_station.web_targets())
    draw_cave()
    draw_gate()
    draw_ember()
