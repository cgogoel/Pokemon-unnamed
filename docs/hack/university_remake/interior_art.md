# Celadon University remake: interior art (FireRed's look)

Feedback on the first interiors: they "look more like gen 2 tiles"; the palette should match the rest of the game in
tone and the furniture (desks etc.) is too low detail. The rooms are now redone in FireRed's own interior look (you
approved the three-room mockups and option (a), a few tilesets by look), with the campus's own windows.

`tools/hack/draw_university_interiors.py` composes every room (FireRed metatiles where one fits, new art by FireRed's
rules where not) and writes the three tilesets; `draw_university_tiles.py` runs it, and `build_university_maps.py`
lays the rooms out from `university_stamps.json` as before.

| Image | What |
|---|---|
| `interiors.png` | Every room, as the game draws it (rendered from the written tilesets), with its people |
| `interiors_before_after.png` | Every room: the first version (left) and now (right) |
| `interior_<room>.png` | One room each (scale 2) |
| `interior_v2_reference.png` | The FireRed rooms the look comes from: Viridian School, Cinnabar Lab lounge, the Copycat's bedroom, a house |
| `tiles_classroom.png`, `tiles_dorm.png`, `tiles_lab.png` | Every metatile of the three tilesets |

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

## Windows: the campus's own

The rooms' windows are the buildings' windows seen from inside: the campus art (`window()`, `sqwindow()` and
`glass_wall()` in `draw_university_tiles.py`), only recolored to the interiors' colors (the outline `#52526A`, the
Lab's glass blues `#BDE6FF` `#83BDDE` `#5294C5`, light greys for the sill), on the wall's last two rows. They stand
in the columns of the building's windows outside (room column = building column + 2, the room size rule), where
nothing else hangs there:

| Room | Windows (room columns) | Outside (building columns) |
|---|---|---|
| Main Hall 1F/2F | arched 5, 7, 11, 13 (the clock at 9 under the tower, notices at 3, 15) | every other column |
| Lecture Halls A/B | arched 3, 11 (the board or screen between) | every other column |
| Faculty Office | arched 4, 6, 8 | 2, 4, 6 |
| Cafeteria | arched 6, 8 (the kitchen along 1-5) | 2, 4, 6 |
| Literature | arched 4, 10 (notices at 6-8) | 2, 8 (and 5 over the door) |
| Library | arched 4, 7, 10 (shelves 1-3, 11-13) | 13, 16, 19 of the building |
| Study Hall | square 3, 5, 9, 11 | every other column |
| Dorm lobby | square 4, 13 | every third column |
| Dorm 2F/3F | square 4, 7, 13, 16 (notices at 9-10) | every third column |
| Hospital 1F | square 4, 6, 10, 12 (the healing machine at 8-9) | 2, 4, 6, 8, 10 |
| Hospital 2F | square 4, 6, 8, 10, 12 | 2, 4, 6, 8, 10 |
| Science Institute floors, Home Economics | the glass wall across the back | the glass tower, the greenhouse |

## What to reuse vs redraw

The university rooms already use `gTileset_BuildingFrlg` as their primary, and its furniture (palettes 0-5) works
under any secondary, so FireRed's whole house set is free. Other FireRed secondaries' pieces are copied into ours
(their tiles and colors), which costs tiles and palette slots but is authentic.

| Piece | Source (metatile ids) | Cost |
|---|---|---|
| Plank and parquet floors and their shaded columns, house plaster, notices, the little desk, drawers, bookshelf, palms, rug, kitchen, stairs (up and down), the door's void row | `gTileset_BuildingFrlg`: 0x0A9 / 0x0B1, 0x001 / 0x009, 0x0A8 / 0x0B0, 0x0D6, 0x030, 0x0BB, 0x0B6 / 0x0BE, 0x08C / 0x094, 0x0C3-0x0D5, 0x031-0x037, 0x076, 0x015 / 0x005-0x00E, 0x0EA / 0x0DA-0x0E3, 0x01A-0x01C | free |
| The player's PC desk, the bed's blanket and pillow | `gTileset_GenericBuilding1` 0x28F, 0x294, 0x287 | copied |
| Lab desks with PCs, PCs, the TM machine, consoles, picture | `gTileset_Lab_Frlg` 0x305-0x30F, 0x062 / 0x30B, 0x2A6 / 0x2B6, 0x320-0x32A, 0x324 / 0x325 | copied |
| Hospital: wall, counter, healing machine | `gTileset_PokemonCenterFrlg` 0x284 / 0x2AC, 0x2B9-0x2BD (one row), 0x2AA-0x2B3 | copied |
| White tile floor and shaded column | `gTileset_BuildingFrlg` 0x081 / 0x082 | free |
| Classroom: plaster, skirting, board + tray, bookcase, tables, desk books, stools, mat | `gTileset_School` 0x283, 0x28E, 0x296, 0x28B-0x28C / 0x293-0x294, 0x29B-0x29C / 0x2A3-0x2A4, 0x2AA-0x2AB, 0x299, 0x288, 0x2A5-0x2A7 | copied |
| Lab wall with its rail, wall bookshelves | `gTileset_BuildingFrlg` 0x069 / 0x071 / 0x079, 0x073-0x07C (used by the Lab) | free |
| Lounge: notices, rug, straw tables, orange stools, mat | `gTileset_Lab_Frlg` 0x28D / 0x295, 0x2EC-0x2F7, 0x2E0-0x2F2, 0x2CB, 0x290-0x292 | copied |
| Sofa | `gTileset_Condominiums` 0x2E0-0x2E2 (widened, moved into the Lab's blues) | copied |
| Vending machine | `gTileset_DepartmentStore` 0x325 / 0x32D (the roof's, in the Lab's colors) | copied |
| Widened pieces (a 7-cell board, 5-cell desks, a 1-cell bookcase or window, a 4-cell sofa) | FireRed's piece sliced: its ends kept, its middle columns repeated (`hslice`) | copied |
| **New art** (no FireRed equivalent) | the board's chalk (Fire-Water-Grass triangle, notes), the projection screen, the lectern, counters, benches, pillars, the clock and banners, the floor emblem, display cases, mailboxes, planters, bed head cells (blankets recolored per room; the ward's pink with steel frames), the standing mirror with the schedule, the photo of home, the wall-top bands, partitions, the solid edge columns | drawn in the palettes above |

Rule of thumb for the rest: use a FireRed piece when one exists, even if it needs widening; draw only what's
specific to the university, and draw it by the rules above.

### The rooms

| Room | Look | Tileset |
|---|---|---|
| Main Hall 1F | Lab plaster (three rows) over white tiles; a blue runner with the school emblem to the reception counter, four pillars, benches, palms, the clock between two banners | Classroom |
| Main Hall 2F | The same hall; a rug, display cases (moon stones; a model satellite), a bench | Classroom |
| Lecture Halls A, B | The School: cream plaster, plank floor, long desks with books, stools, the lectern; A's chalkboard, B's projection screen | Classroom |
| Faculty Office, Literature, Library | Lab plaster over parquet: bookshelves (the Library's also freestanding), counters, straw tables, the little desks, sofas | Classroom |
| Dorm lobby, 2F, 3F | The houses: grey-green plaster, planks, beds, desks, rugs; mailboxes, the warden's counter, a lounge | Dorm |
| Cafeteria | Lab plaster over white tiles: the house kitchen, a serving counter, tables with plates | Dorm |
| Science Institute 1F-4F | The glass wall over white tiles: the Lab's desks, PCs, consoles, the TM machine, rug, sofas | Lab |
| Hospital 1F, 2F | The Pokemon Center's wall and counter with its healing machine; a ward of six beds | Lab |
| Study Hall | The Lab lounge | Lab |
| Home Economics | The greenhouse's glass wall; planters, the tasting table | Lab |

## GBA budget

Secondary tilesets (FRLG, `include/fieldmap.h`): 384 tiles, 384 metatiles, palettes 7-12 (6 slots). All three are
over `gTileset_BuildingFrlg` (the pairing the rooms already had, which is what makes FireRed's house furniture, floors
and the door row free). The writer counts both layers' 8x8 tiles deduplicated with flips, references any tile
identical to a primary one, gives a tile a primary palette when one holds its colors, and packs the rest into
secondary palettes (near-identical whites and greys merged); it stops if a tileset goes over.

| Tileset | Rooms | Tiles | Metatiles | Secondary palettes |
|---|---|---|---|---|
| `gTileset_UniversityClassroom` | Main Hall 1F/2F, Lecture Halls A/B, Faculty Office, Literature, Library | 195 / 384 | 184 / 384 | 6 / 6 |
| `gTileset_UniversityDorm` | Dorm lobby, 2F, 3F, Cafeteria | 117 / 384 | 111 / 384 | 5 / 6 |
| `gTileset_UniversityLab` | Science Institute 1F-4F, Hospital 1F/2F, Study Hall, Home Economics | 108 / 384 | 117 / 384 | 5 / 6 |

Palettes are what fills up (copying from several FireRed tilesets brings several 15-color palettes): the Classroom
set has none left, so new pieces for its rooms should reuse its colors (`fit()` moves a piece into colors already
used) or go in the Dorm or Lab set. The first version's `gTileset_UniversityInterior` is gone.

## Layout compatibility

Every room keeps its size, collision, door mat, stairs and events: `COLLISION` in the tool is the layouts' collision
and marks from before the redo, and every piece stands on the cells the first version's did (bg events on
bookcases, boards, desks, beds and the TM machine still point at them). Rebuilt with `build_university_maps.py`, the
layouts differ from before only in their metatiles: the collision and elevation bits of every cell, every warp,
person, sign and trigger are unchanged, so the scripted walks and the brief, lab and club scenes are as before. The
tool also prints any cell where the picture and the collision disagree (furniture on a walkable cell, a solid cell
with nothing on it); none now. Visible differences: the last row is FireRed's black void instead of a front wall
(solid either way); the stairs are FireRed's house stairs (drawn over the wall's rows, the warp cells unchanged); the
door mat is FireRed's 3-cell mat (only its middle cell warps, as before); the Library's wall shelves are 1-3 and 11-13
(were 1-4, 10-13) to make room for its windows (its signs at 2, 3, 11, 12 are still on shelves).
