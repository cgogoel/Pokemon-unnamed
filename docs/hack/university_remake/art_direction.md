# Celadon University remake: art direction

Reference: the Tamamushi University diagram (the lore art). The campus reads as **white stone collegiate buildings in a
green forest**: a long classical central building with a clock tower, the Gyarados fountain, a great glass vault, a
stadium with a track and floodlights, a glass science tower, a white hospital by a pool, and an avenue with
reflecting pools.

Mockups, rendered from the written tilesets by `tools/hack/draw_university_tiles.py`:

- `mockup_campus.png`: the whole 76x64 campus (scale 2).
- `mockup_main_hall.png`, `mockup_lecture_hall.png`, `mockup_dorm_lobby.png`: each building beside its room at the
  same scale, the room's top level with the building's footprint, the doors outlined in red (scale 3).
- `mockup_room_kit.png`: the furniture kit for the other rooms (bedroom, kitchen, dining, greenhouse garden, lab, ward).
- `tiles_exterior.png`: every metatile of the campus tileset. The rooms: `interior_art.md` (their look, mockups and
  tilesets).

## One material kit, outside and inside

| Material | Outside | Inside |
|---|---|---|
| White stone (8-pixel courses) | Every facade, plinths, cornices with dentils, the gate's pillars | The plaster of every wall; columns; stairs |
| Grey-blue slate (the school color) | Mansard roofs, spire, dormers, canopies, lamp posts | The Main Hall's wainscot; banners; the dark wall tops |
| Window glass (3 blues, a white cross) | Tall arched windows (academic), square windows (dorms, hospital) | The same arched window on the back walls |
| Gold | Pediment emblems, the clock, finials, lamp caps | Picture rail, clock, carpet borders, the floor emblem |
| Wood | Doors, the barn | Parquet and planks, wainscot in lecture halls and dorms, furniture |
| Gyarados blue | The fountain's Gyarados | The Main Hall's runner, the dorm's rug |
| Magikarp orange | - | The emblem medallion in the Main Hall floor |
| Greenhouse glass + leaves | Home Economics, the Science tower | Home Economics' glass wall and garden beds; lab glassware |
| Accent (track red, seat blue, hospital red and pink) | Stadium, the hospital's crosses | Ward beds |

The outline everywhere is a dark slate blue (`#283048`), never black, like FireRed's own buildings.

## Building language (simplified to fit one tileset)

Every building is drawn on the 16x16 grid and is **mirror-symmetric**: the right half is the left half flipped, which
the tile deduplication gets for free. One window, one door, one dormer, one roof and wall kit serve every building.

- **Academic** (Main Hall, lecture wings, end pavilions, Literature & Library): mansard slate roof with a light ridge
  and dormers, a white dentil cornice, storeys of tall arched windows, a plinth. Doors are arched stone portals with a
  fanlight over a wooden double door. The pavilions get a small pediment; the Main Hall a four-column portico under
  a big pediment with the gold emblem, and the clock tower on its roof (balustrade, clock at noon, belfry, slate
  spire, gold finial).
- **Glass**: the greenhouse is a vault of glass with rounded ends and leaves through the lower panes (a 16x16 seamless
  leaf pattern), a glass front on a low white wall. The Science Institute is a glass cylinder (shaded by column, a
  floor band every metatile, a white disc roof) beside a steel lattice tower with a red beacon, on a white podium.
- **Hospital**: a modern white block, a flat gravel roof with a big red cross, three storeys of square windows, a
  glass door under a slate canopy and the cross sign above it.
- **Dorms**: the academic kit with square windows and a glass door under a canopy; the annex smaller, a wooden door.
- **Grounds**: stadium (blue-seated stands along the top, an oval red track with lane lines around a grass infield,
  four floodlight towers), the outdoor pool (tiled deck, lane ropes), the ranch's barn (inside the primary's fence),
  the gate (stone pillars with gold lamps either side of the passage, the gatehouse roof), the Gyarados fountain on the
  square's paving, reflecting pools, lamp posts, benches. Grass, forest, lone trees, paving, flower beds, fences and
  signs are FireRed's own (the primary tileset).

## Interiors

Redone in FireRed's own interior look, with the campus's windows: see `interior_art.md`. (The first version's rooms,
drawn in the campus palettes below, are gone.)

## Palettes (GBA 5-bit colors; secondary palettes from 7)

| Palette | In | Colors (after transparent) |
|---|---|---|
| `stone` | campus (slot 7) | outline `#283048`, white `#F8F8F0`, stone `#E0E0D0` `#C0C0B0` `#909080`, slate `#98A8C8` `#7080A8` `#485880`, glass `#C8E8F8` `#78B0E0` `#4070A8`, gold `#F8D058` `#C08828`, wood `#A86838` `#684028` |
| `glass` | campus (8) | outline, white, stone, steel x3, glass x4, leaves x3, pink, red |
| `water` | campus (9) | outline, white, stone x3, water x3, Gyarados blue x2, cream, red, Magikarp orange, gold |
| `accent` | campus (10) | outline, white, stone x3, track red x2, seat blue x2, reds x2, pink, green, steel, yellow |

## The budget

FRLG secondary tilesets (`include/fieldmap.h`): **384 tiles, 384 metatiles, palettes 7-12**.

| Tileset | Tiles | Metatiles | Palettes |
|---|---|---|---|
| `gTileset_CeladonUniversity` (every campus building and decoration) | 357 / 384 | 347 / 384 | 4 / 6 |
| The rooms' three tilesets | see `interior_art.md` | | |

The script stops if a tileset goes over, and reports any tile that doesn't fit one palette (none now).
