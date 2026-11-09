# Celadon University remake: interior art, v2 (draft)

Feedback on v1: the interiors "look more like gen 2 tiles"; the palette should match the rest of the game in tone and
the furniture (desks etc.) is too low detail. This is the design pass. Nothing in the game changes yet: the draft tool
`tools/hack/draw_university_interiors_v2.py` composes three rooms and writes mockups, no tileset.

| Mockup | What |
|---|---|
| `interior_v2_reference.png` | The FireRed rooms the look comes from: Viridian School, Cinnabar Lab lounge, the Copycat's bedroom, a house |
| `interior_v2_lecture_hall_a.png`, `..._compare.png` | Lecture Hall A, 15x11: the School's classroom, grown up |
| `interior_v2_dorm_2f.png`, `..._compare.png` | Dorm 2F, 21x12: the corridor and three bedrooms in FireRed's house set |
| `interior_v2_study_hall.png`, `..._compare.png` | Study Hall, 15x10: the Cinnabar Lab lounge's look |

The compare images put the current room (rendered from the layout, as the game draws it) next to the redo, same
people in both. All three rooms are at their current size, with every solid and walkable cell where it is now.

## What's off in v1

| | v1 | FireRed |
|---|---|---|
| Outline | Dark navy `#283048` on everything | Grey-violet `#52526A` in **every** interior tileset (primary and secondary) |
| Hue / saturation | Saturated, darker mid-tones: wood `#A86838` / `#684028`, parquet `#D8A868` `#B88048` `#885830`, carpet `#5880C8` `#3858A8` `#203870`, board `#407050` | Soft, light, slightly desaturated warm tones: floors are pale olive-gold (`#F6EE94` `#DECD6A` `#C5B44A` `#A49431` `#837310`) or near-white tiles; furniture wood is light orange (`#FFCD8B` `#E6A462` `#C58341` `#AC5A18`); blues are greyish (`#5A7BB4`, `#83BDDE`) |
| Shading | 1-2 flat shades per object, a dark rim | 3-4 shade ramps per material: a white or lightest-shade highlight along every top edge, mid body, dark underside, then the outline |
| Drop shadows | None: furniture floats on the floor | The floor one shade darker under and beside furniture, along the left wall, and under the back wall's foot |
| Furniture | Boxes: a desk is a brown rectangle with white squares; chairs are brown squares; the blackboard is a flat green panel | Built from parts: table tops with a woven/straw texture, front rails, legs with feet; stools with a seat highlight; bookcases with individual colored spines on two shelves; a board with a frame, chalk text, a tray with chalk and eraser |
| Walls | Slate cornice, white plaster with a gold rail, wainscot; **solid dark slate columns down both sides and a slate front wall** | Two rows of plaster (cream, grey-green or beige) with a rail and a skirting stripe; the wall's top seen from above as a thin white/light-grey band with an outline; **no side or front walls: the room ends in black** |
| Floors | Busy parquet with dark joints, saturated blue carpet | Low-contrast planks or tiles (joints one shade apart), rugs with a 2-tone border |
| Door | A blue mat on the floor | The red 3-cell mat over the black void (`gTileset_BuildingFrlg` 0x01A-0x01C under the secondary's mat) |

So the problem isn't one color: v1 is a different rendering style (the darker, flatter, outline-heavy look of GSC),
built in the campus's palettes. v2 drops it for FireRed's rules.

## Target: palettes

Every color below is taken from FireRed's own `.pal` files (the 8-bit values as the files store them).

| Material | Colors | From |
|---|---|---|
| Outline | `#52526A` | every interior tileset |
| Neutral greys / white | `#FFFFFF` `#D5D5D5` `#B4B4A4` `#7B7B7B` (and `#83838B`, `#8B8394` in some) | Condominiums 8-12, Lab 7, BuildingFrlg 0-5 |
| School plaster (lecture halls) | `#FFE6AC` `#D5BD83` `#AC945A` | School 7 |
| School skirting / glass | `#BDE6F6` `#9CCDDE` `#7BB4C5` `#629CAC` | School 7 |
| Chalkboard | `#94CD7B` `#629C4A` (chalk `#FFFFFF`) | School 7 |
| Plank floor (School) | `#F6EE94` `#DECD6A` `#C5B44A` `#A49431` `#837310` | BuildingFrlg 1 (free: primary) |
| House plaster (dorm) | grey-green `#ACC5B4` (`#C5DECD` lit), white top `#E6EEF6`, teal skirting `#5ABDAC` | BuildingFrlg (0x0A8 / 0x0B0; free) |
| Dorm planks | `#F6EE9C` `#CDBD83` `#BDA47B` | BuildingFrlg (0x0A9; free) |
| White tiles (study hall) | `#EEEEF6` `#CDCDD5` `#A4A4AC` | BuildingFrlg (0x081; free) |
| Lab plaster (study hall) | `#D5D5AC`, white top, the rail `#DECD6A` `#A49431` `#837310` | BuildingFrlg 0-1 (0x069 / 0x071; free) |
| Furniture wood | `#FFCD8B` `#E6A462` `#C58341` `#AC5A18` | Lab 7 |
| Straw / woven table tops | `#F6EE94` `#C5B44A` | Lab 7, School 8 |
| Upholstery / glass blue | `#BDE6FF` `#83BDDE` `#5294C5` | Lab 7 |
| School colors (slate + gold) for our own touches | `#739CC5` `#5A7BB4` `#41629C`, gold `#FFF683` `#E6B44A` | BuildingFrlg 3 (free: primary) |
| Reds (mats, blankets) | `#FF8373` `#CD5241` | BuildingFrlg 0 |

The university's slate blue and gold already exist in the primary's palette 3, so trims, banners and plaques drawn in
it need no palette slot. New art uses only these colors.

## Target: detail rules

1. Outline everything in `#52526A`; never black, never navy. Inner lines (drawers, panel insets, shelf boards) are
   the darkest shade of the material, not the outline.
2. Light comes from the top: the top edge of every object gets its lightest shade (often white), the front face the
   mid shades, the underside/lip the dark shade.
3. Every piece of furniture casts a shadow: the floor under it, 2-3 px below (and 2-3 px to the right for tall
   pieces), one step darker in the floor's own ramp. Rooms also shade the first floor column by the left wall (FRLG's
   `0x29D` / `0x0B1` / `0x082` variants).
4. Desks and tables, like the Lab's and the School's: a textured top (woven straw, `#F6EE94` with `#C5B44A` dither)
   with a 1 px lit rim, a front rail one shade darker, legs with feet showing below, the shadow under. Desk props
   (open books, notes, lamps) are 1-3 px details in white and light grey.
5. Chairs are separate objects: stools (School `0x288`), round orange stools (Lab `0x2CB`), blue chairs
   (BuildingFrlg `0x023` / `0x064`), with a seat highlight and their own shadow.
6. Shelves show individual spines in 4-5 colors on 2 shelves, a white top, glass where it's a cabinet.
7. Walls: 2 rows (plaster, then rail + wainscot/skirting at the bottom of row 1); the wall top across the top of row
   0 as outline/white/white/light grey/outline; windows and boards inset into the plaster with frames, not pasted on.
8. The room's edges: the edge columns (still solid) are black with the wall's top running down the room side; the
   last row is the black void with the red door mat's lower edge.
9. Shading ramps of 3-4 colors per material, pale and slightly desaturated; nothing at full saturation except tiny
   accents (book spines, the mat).

## What to reuse vs redraw

The university rooms already use `gTileset_BuildingFrlg` as their primary, and its furniture (palettes 0-5) works
under any secondary, so FireRed's whole house set is free. Other FireRed secondaries' pieces are copied into ours
(their tiles and colors), which costs tiles and palette slots but is authentic.

| Piece | Source (metatile ids) | Cost |
|---|---|---|
| Plank floor, shaded column, house plaster, windows, notices, desks, PC desk, drawers, bookshelf, palms, rug, stairs (up and down), the door's void row | `gTileset_BuildingFrlg`: 0x0A9 / 0x0B1, 0x0A8 / 0x0B0, 0x0B8 / 0x0C0, 0x0D6, 0x289 / 0x291 and 0x28F / 0x290 (GenericBuilding1's desk tiles are primary tiles), 0x0BB, 0x0B6 / 0x0BE, 0x0F5 / 0x0FD, 0x0C3-0x0D5, 0x015 / 0x005-0x00E, 0x0EA / 0x0DA-0x0E3, 0x01A-0x01C | free |
| White tile floor and shaded column | `gTileset_BuildingFrlg` 0x081 / 0x082 | free |
| Classroom: plaster, windows, skirting, board + tray, bookcase, tables, desk books, stools, palms, mat | `gTileset_School` 0x283, 0x285-0x28F, 0x293-0x297, 0x29B-0x29C / 0x2A3-0x2A4, 0x2AA-0x2AB, 0x299, 0x288, 0x281 / 0x289, 0x2A5-0x2A7 | copied |
| Lab wall with its rail, wall bookshelves | `gTileset_BuildingFrlg` 0x069 / 0x071 / 0x079, 0x073-0x07C (used by the Lab) | free |
| Lounge: small windows, notices, rug, straw tables, orange stools, mat | `gTileset_Lab_Frlg` 0x314 / 0x31C, 0x28D / 0x295, 0x2EC-0x2F7, 0x2E0-0x2F2, 0x2CB, 0x290-0x292 | copied |
| Sofa | `gTileset_Condominiums` 0x2E0-0x2E2 (widened) | copied |
| Vending machine | `gTileset_DepartmentStore` 0x325 / 0x32D (the roof's) | copied |
| Widened pieces (a 7-cell board, 5-cell desks, a 1-cell bookcase or window, a 4-cell sofa) | FireRed's piece sliced: its ends kept, its middle columns repeated (`hslice`) | copied |
| **New art** (no FireRed equivalent) | the board's chalk (Fire-Water-Grass triangle, notes), the lectern, bed head cells (FireRed's blanket 0x294 + pillow 0x287 under a new headboard; blankets recolored per room), the standing mirror with the schedule, the photo of home, the wall-top bands, partitions, the solid edge columns | drawn in the palettes above |

Rule of thumb for the rest: use a FireRed piece when one exists, even if it needs widening; draw only what's
specific to the university, and draw it by the rules above.

### Sources for the other rooms (phase 2)

| Room | FireRed source |
|---|---|
| Lecture Hall B (audiovisual) | School + Silph Co.'s projector screen / Condominiums' TV wall |
| Faculty Office | Silph Co. offices (desks with PCs, phones, plants; `gTileset_SilphCo` uses the Condominiums' gfx) |
| Cafeteria | Celadon Restaurant (`gTileset_RestaurantHotel`: chairs, round tables, the counter, kitchen) |
| Literature, Library | Lab and Museum bookcases, Pokémon Mansion shelves, the School's map |
| Main Hall 1F/2F | Museum (`gTileset_Museum`: columns, display cases, the reception) and the Department Store's marble |
| Dorm lobby | House set + the Celadon Hotel's counter |
| Home Economics | Primary kitchen (sink/stove 0x031-0x032, fridge) + our greenhouse glass |
| Science Institute floors, Hospital | `gTileset_Lab_Frlg` (research benches, machines, PCs) and the Pokémon Center (`gTileset_PokemonCenterFrlg`) |

## GBA budget

Secondary tilesets (FRLG, `include/fieldmap.h`): 384 tiles, 384 metatiles, palettes 7-12 (6 slots). The draft
tool's budget counts both layers' 8x8 tiles deduplicated with flips, with tiles identical to a
`gTileset_BuildingFrlg` tile free, and packs the rest into palettes (a primary palette when one holds the tile,
else 15-color secondary slots, near-identical whites/greys merged):

| Rooms | New tiles | Metatiles | Secondary palettes |
|---|---|---|---|
| Lecture Hall A | 58 | 45 | 2 |
| Dorm 2F | 57 | 55 | 3 |
| Study Hall | 26 | 53 | 2 |
| Lecture Hall A + Study Hall | 82 | 90 | 4 |
| Dorm 2F + 3F | 57 | 55 | 3 |
| **All three** | **137** | **140** | **6 of 6** |

(v1's `gTileset_UniversityInterior` holds 13 rooms in 310 tiles and 5 palettes.)

Findings:

- **Tiles and metatiles aren't the problem; palettes are.** Copying from several FireRed tilesets brings several
  15-color palettes, and three rooms already fill all six slots.
- **The pairing stays** `gTileset_BuildingFrlg` (primary) + our secondary: it's what the rooms use now, and it's what
  makes FireRed's house furniture, floors and the door row free.
- So split the interiors by look into a few secondaries, each drawing on one or two FireRed sources:
  - `gTileset_UniversityClassroom` (School + Lab lounge looks: both lecture halls, Study Hall, Literature, Library,
    Faculty Office): about 4 palettes for the first two rooms; the rest reuse them.
  - `gTileset_UniversityDorm` (the house set: lobby, 2F, 3F, maybe the Cafeteria): mostly free, 3 palettes.
  - `gTileset_UniversityLab` (Science floors and Hospital: `gTileset_Lab_Frlg` + Pokémon Center), as now.
  - The Main Hall (Museum look) and Home Economics (greenhouse) with whichever of these has room, or their own.
- Alternatives, for you to choose:
  - **(a) Split secondaries as above** (recommended): authentic pieces plus our own touches, a palette budget per look.
  - **(b) One secondary, palettes fixed first**: pick 6 palettes (e.g. School 7, Lab 7, Lab 8, Condominiums 9, our
    accent, a spare) and remap every copied piece's colors into them; one tileset for all academic rooms, at the cost
    of slight color shifts on copied pieces.
  - **(c) Use FireRed's secondaries directly** for the plainest rooms (e.g. the Study Hall on `gTileset_Lab_Frlg`):
    zero new tiles, but nothing custom (no board triangle, no lectern, no partitions) and the rooms look exactly like
    Cinnabar's.

## Layout compatibility

The redo keeps every cell's collision: walls and the edge columns solid, the door mat walkable with the warp in the
same cell, furniture in the same cells as v1 (bg events on bookcases, the board, desks, beds still point at them).
Differences: the last row is FireRed's black void instead of a front wall (solid either way); the Dorm's stairs are
FireRed's (drawn over rows 0-2, the warp cells (1-2,2) and (18-19,2) unchanged); the door mat is FireRed's 3-cell mat
(only its middle cell warps, as now).

## Next (phase 2, after you pick)

1. Pick the tileset split (a, b or c) and the source look for each remaining room.
2. Move the v2 pieces into the real writer: a FireRed-metatile copier (tiles + palettes into our secondary) next to
   v1's `Tileset`, a palette plan per secondary, and the rooms rebuilt on it in `draw_university_tiles.py` (the
   campus tileset doesn't change).
3. Redo the other rooms by the same rules; rewrite `university_stamps.json`; rebuild the layouts with
   `build_university_maps.py`; build both ROMs; screen-dump each room.
