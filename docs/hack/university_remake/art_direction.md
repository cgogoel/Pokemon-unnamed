# Celadon University remake: art direction

Reference: the Tamamushi University diagram (the lore art). The campus should read as **white stone collegiate
buildings in a green forest**: a long classical central building with a clock tower, a Gyarados fountain, a great
glass vault behind it, a stadium, a glass science tower, a white hospital, and an avenue with a reflecting pool.

Mockups (drawn by `tools/hack/draw_university_tiles.py`, real GBA pixel art on the 16x16 metatile grid, every 8x8
tile in one 15-color palette):

- `mockup_campus_center.png`: the central building (Faculty Office, Lecture Hall A, Main Hall with its clock tower,
  Lecture Hall B, Cafeteria), the greenhouse (Home Economics) behind, the fountain square and the start of the avenue.
- `mockup_main_hall.png`, `mockup_lecture_hall.png`, `mockup_dorm_lobby.png`: each building beside its interior, at the
  same scale, the same size, the door in the same column (outlined in red).
- `plan_campus_blocks.png`: the proposed whole campus as blocks.
- `tiles_exterior.png`, `tiles_interior.png`: the unique 8x8 tiles the mockups use (what `tiles.png` would hold).

## One material kit, outside and inside

| Material | Outside | Inside |
|---|---|---|
| White ashlar stone (8-pixel courses, staggered joints) | Every facade; corner quoins; plinths; cornices with dentils | The plaster of every wall; columns in the Main Hall; stairs |
| Grey-blue slate (the school color) | Mansard roofs, spire, dormers, lamp posts | The wainscot of the Main Hall; banners; the dark wall tops |
| Window glass (3 blues, a white cross mullion) | Tall arched windows, keystones and sills | The same window art on the back walls |
| Gold | Pediment emblems, the clock, finials, lamp caps | Picture rail, clock, carpet borders, the emblem in the floor |
| Wood | Doors | Parquet and plank floors, wainscot in lecture halls and dorms, furniture |
| Gyarados blue | The fountain's Gyarados | The carpets (the Main Hall's runner, the dorm's rug) |
| Magikarp orange | - | The emblem medallion in the Main Hall floor |
| Greenhouse glass + leaves | Home Economics, the Science Institute's tower | Home Economics' garden inside |

The outline everywhere is a dark slate blue (`#283048`), never black, like FireRed's own buildings.

## Building language

- **Academic buildings** (Main Hall, lecture wings, end pavilions, Literature & Library): mansard slate roof with a light
  ridge, dormer windows, a white dentil cornice; two or three storeys of tall arched windows (one per 16-pixel column)
  with string courses between storeys; corner quoins; a plinth. Doors are arched stone portals with a fanlight over a
  wooden double door; the important ones get a pediment with the gold emblem. The Main Hall adds a four-column portico
  under a big pediment, and the clock tower stands on its roof (balustrade, clock at noon - the fountain's signal -,
  belfry, slate spire, gold finial).
- **Glass buildings** (Home Economics greenhouse, Science Institute): white steel ribs every 16 pixels, glass that
  lightens toward the top, rounded vault ends, leaves showing through (a 32x16 seamless leaf pattern, so the roof costs
  a handful of tiles). The Science Institute (not in the mockup yet) is a glass cylinder (vertical shading bands in
  the glass palette) beside a 4-wide steel lattice tower, on a white stone base.
- **Hospital** (not drawn yet): white stone, flat roof edged in slate, horizontal window bands, the Chansey-pink cross
  over the door (needs the accent palette).
- **Dorms**: the same stone and slate, homelier: square windows, iron balconies on the upper floors, a slate canopy
  over a glass door, wood wainscot and plank floors inside.
- **Grounds**: FireRed's own grass, forest, lone trees, paving and flower beds (the primary tileset, free); custom lamp
  posts (slate post, lantern, gold cap), benches, reflecting pools with white stone rims, the Gyarados fountain (an oval
  white basin, Gyarados rising from the pedestal, the jet falling in two arcs). Still to draw: the stadium's track,
  stands and floodlight towers, the pool deck, the ranch fence and barn, the gate's stone posts.

## Interiors

Rooms are the building's size (see `plan.md`). Back walls are 3 rows in the Main Hall (three storeys outside) and 2 rows
elsewhere: slate cornice, white plaster, a gold picture rail, then the wainscot and a dark skirting. The side and front
walls are dark slate wall tops, with the door mat in the door's column on the row above the bottom edge.

- **Main Hall**: marble checker floor, a Gyarados-blue runner from the door to the reception counter, the emblem
  medallion (Magikarp in a gold ring) in the floor, the big clock on the back wall under the tower, slate banners,
  white columns, benches, the staircase up to the clock gallery.
- **Lecture halls**: parquet, wood wainscot, a chalkboard with the Fire-Water-Grass triangle and a Magikarp sketch, a
  lectern, rows of desks with chairs, the aisle up the middle from the door.
- **Dorm lobby**: plank floor, wood wainscot, stairs up, pigeonhole mailboxes, notice board, bookshelf, vending machine,
  the warden's counter, a lounge with a blue rug, a sofa and a coffee table.

## Palettes (GBA 5-bit colors; secondary palettes 7-12)

| Palette | Used by | Colors (after transparent) |
|---|---|---|
| `stone` (both tilesets) | buildings, walls, windows, stairs, lamps, benches | outline `#283048`, white `#F8F8F0`, stone `#E0E0D0` `#C0C0B0` `#909080`, slate `#98A8C8` `#7080A8` `#485880`, glass `#C8E8F8` `#78B0E0` `#4070A8`, gold `#F8D058` `#C08828`, wood `#A86838` `#684028` |
| `glass` (exterior) | greenhouse, science tower | outline, white, stone, steel x3, glass x4, leaves x3, pink, red |
| `water` (exterior) | fountain, pools, Gyarados | outline, white, stone x3, water x3, Gyarados blue x2, cream, red, Magikarp orange, gold |
| `accent` (exterior, planned) | hospital cross, signs, stadium track and seats | outline, white, stone x3, reds x3, Chansey pink, green, track brown x2, seat blue x2 |
| `floor` (interior) | marble, parquet/planks, blue carpet, the emblem | outline, marble x3, wood x3, carpet blue x3, gold x2, orange, red |
| `furn` (interior) | furniture, chalkboard, books, plants, metal | outline, white, wood x3, board green x2, red, blue, yellow, leaves x3, metal x2 |

The exterior tileset uses `stone`, `glass`, `water`, `accent` (+ 2 spare); the interior tileset uses `stone`, `floor`,
`furn` (+ 3 spare, e.g. the lab and the hospital's whites and pinks). Sharing `stone` keeps outside and inside the
same white and slate.

## The tile budget

FRLG secondary tilesets (`include/fieldmap.h`): 1024 - 640 = **384 tiles**, 1024 - 640 = **384 metatiles**, palettes
**7-12 (6)**. The draft generator counts the mockups' art (flips deduplicated):

| | Unique 8x8 tiles | Off-palette tiles |
|---|---|---|
| Exterior: central building, tower, greenhouse, fountain, pools, lamps, benches, dorm | 424 | 0 |
| Interiors: Main Hall, Lecture Hall, Dorm lobby | 263 | 0 |

By piece (exterior): Main Hall 110, clock tower 55, a wing +13, a pavilion +21, greenhouse 94, fountain 93, pools 15,
lamp 6, bench 4, dorm +18. **The exterior is already over 384 with only the center drawn**, so the real tileset needs
an economy pass and probably two campus maps (see `plan.md`, "The budget"). The interiors fit with room for the other
rooms.
