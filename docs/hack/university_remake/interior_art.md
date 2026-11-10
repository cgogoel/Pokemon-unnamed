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
| `interior_<room>.png` | One room each (scale 3) |
| `interior_v2_reference.png` | The FireRed rooms the look comes from: Viridian School, Cinnabar Lab lounge, the Copycat's bedroom, a house |
| `tiles_classroom.png`, `tiles_dorm.png`, `tiles_lab.png` | Every metatile of the three tilesets |
| `audit/*.png` | The seam audit's fixes, before and after (scale 5) |

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
| `gTileset_UniversityClassroom` | Main Hall 1F/2F, Lecture Halls A/B, Faculty Office, Literature, Library | 243 / 384 | 219 / 384 | 6 / 6 |
| `gTileset_UniversityDorm` | Dorm lobby, 2F, 3F, Cafeteria | 210 / 384 | 144 / 384 | 6 / 6 |
| `gTileset_UniversityLab` | Science Institute 1F-4F, Hospital 1F/2F, Study Hall, Home Economics | 165 / 384 | 139 / 384 | 5 / 6 |

Palettes are what fills up (copying from several FireRed tilesets brings several 15-color palettes): the Classroom and
Dorm sets have none left, so new pieces there must reuse their colors (`fit()` moves a piece into colors already used)
or go in the Lab set. The first version's `gTileset_UniversityInterior` is gone.

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

## Seam audit

`draw_university_interiors.py --audit` checks every room for pieces that don't meet cleanly (`audit()`):
- **floor**: floor pixels (not under the middle layer) that aren't the room's floor in its own phase, its shaded
  column, the wall's foot, or one of those in the floor's shadow ramp: another floor showing through a FireRed
  piece's bottom layer, a pattern restarting mid-floor, a piece hanging into the floor row;
- **jump**: a cell edge where the picture's lines run on 1-2 rows higher or lower (a rail, skirting or outline a
  pixel off);
- **step**: a row edge in the floor where the floor doesn't continue (a shadow band or a wall foot that stops);
- **wall**: in a staircase's wall cells, another wall showing where the room's own wall is (the houses' grey
  stairwell, their plaster or skirting), reached from the top of the steps' row;
- **blank**: plain wall left between a staircase and the wall's top band (a flight ending mid-wall);
- **down**: a flight down whose top tread isn't at the floor line;
- every widened FireRed piece (`hslice`) whose joins don't continue the piece as drawn.

Each room was also checked by eye at scale 5-6 (walls and corners, windows on the trims, stairs, mats and the void
row, rugs, desks, shelves, counters, sofas, beds, partitions). Fixed (`audit/<n>.png`, before | after):

| Room | Issue | Fix | Crop |
|---|---|---|---|
| All | The wall's top band stopped at the edge columns, whose inner outline ran up through it | The band runs on over the edge columns' wall tops, closed by their outline | `01_corners` |
| Lecture halls | The shaded first column replaced the skirting's foot at (1,2) | The column's shadow is laid over the foot (`shade_cell`) | `01_corners` |
| All stone rooms | The arched windows cut through the wall's rail and wainscot | Windows shortened (rows of the lower pane taken out, the arch, cross and sill kept) so the sill rests on the rail (`campus_window`, `RAIL`) | `02_windows_rail` |
| Hospital, Study Hall | The square windows ran over the Pokemon Center wall's band, and 1 px into the Lab's rail | Same, for each wall | `03_square_windows` |
| Every room with stairs | The stairs' landings showed the floor FireRed drew them on (the Copycat's planks, the Pallet parquet) | Those pixels become the room's floor (`refloor`) | `04_stairs_floor`, `05_stairs_down` |
| Every room with stairs (16 staircases) | The house stairs brought the houses' grey stairwell behind the steps: a foreign block over the room's rail, wainscot and trim | Built into the room's wall (`stairwell`): wherever the grey stairwell showed above the steps (reached from their top edge), the room's own plaster, rail and skirting show; the steps climb against it, the white rail in front, nothing above the wall's top band; the shaded faces inside the steps stay | `19_stairs_all` (every staircase) |
| Every room with stairs | Every flight stopped 14 px above the floor line, leaving plain wall up to the top band (13 px under 2-row walls, 29 px in the Main Hall), and the 8 flights down drew the same climbing steps as the 8 up, so they read as going up into the wall | Each staircase is an opening in the wall (`stairwell`): a dark doorway, outlined with the wall's white edge either side, from right under the top band down to the floor line. Up: the house steps climb into it (lit). Down: the flight starts at the floor line and descends into it, treads seen from above, the top one level with the floor and each lower one a shade darker and narrower (FireRed's Department Store stairs down), the rail's post down its side; the landing (the warp cells) is plain floor | `20_stairs_openings` (all 16) |
| Lecture halls, Literature, Library, Faculty Office, Study Hall, lobby, Science floors, Cafeteria, Hospital | Pieces against the back wall (bookcases, shelves, PCs, consoles, the TM machine, the kitchen, the healing machine) covered the wall's top band | They stand with their feet 2 px onto the floor row, as FireRed's do, or hang just under the band if taller (`against_wall`); one-cell bookcases are narrowed where a shelf edge runs (`narrow`) | `06_wall_pieces`, `07_consoles`, `18_shelves` |
| Lecture Hall A | The board's eraser and chalk repeated at every slice of the widened board and tray | Once, at its end | `08_chalkboard` |
| Main Hall 1F/2F | The banners hung into the floor row; a light line across the plaster's middle row (a stretched crop) | Shorter banners; the middle row plain plaster | `09_main_hall_wall` |
| Main Hall, Science Institute | The runners stopped without their border | Bordered ends | `10_runner_end`, `11_institute_runner` |
| Cafeteria | Sink, stove, cupboard and fridge cut off at their middles | The houses' whole pieces (0x140-0x149, 0x114-0x11D, 0x117 / 0x11F), standing on the floor | `12_kitchen` |
| Hospital | A strip of the healing machine's counter showed in the wall | Cropped (the counter in front is the one used) | `13_healing_machine` |
| Dorm 2F/3F | The bedroom walls stopped under the corridor wall's face; the corridor wall's top didn't meet the edge columns | The bedroom walls run up through the face to its top; its top runs on over the edges | `14_partitions`, `15_partition_edges` |
| Lecture Hall B | The screen's case covered the wall's top band | Hung below it | `16_screen` |
| Dorm lobby | A tall palm stood in front of the window | A low palm (its pot cell) under it | `17_lobby_window` |

What the audit still lists is by design: curves crossing a cell edge (the clock, the emblem, the satellite model, the
chalk notes and the screen's slide) read as 1-2 row line jumps; the stairs' landings carry FireRed's own rail feet
and shadow; the chalk tray's floor is the board's shadow, as in the School (0x293-0x294); a few table legs and the
narrowed bookcases differ from the source at their joins without a visible seam. Collision, elevation and every event
were checked unchanged cell by cell after the rebuild.

## Stairs and whole pieces (v3)

**Stairs.** FireRed's house staircase rises sideways into the corner with its rail drawn across the front of the steps;
in our rooms that read as steps blocked by the rail and climbing into the outer wall. Every staircase is now a straight
flight in an opening in the back wall, two cells wide (`stairwell`), the way FireRed's Pokemon Center and Department
Store stairs go up into the wall:
- up: risers facing the room and lit treads, each step a little narrower further up, the bottom riser on the floor
  line (the player steps onto it from the warp cells, which are unchanged), the top tread arriving at the upper floor
  (a strip of the room's own floor in shadow across the top of the opening);
- down: the top tread level with the floor line, clear floor in front of it (the warp cells), the flight descending
  away into the dark, each lower tread a shade darker and narrower;
- the rail is a banister down the open side only (away from the room's outer wall; both sides if the stairs stood
  free), beside the treads, never over them; the treads keep 2 px off the outer wall.
The warps and collision didn't need to move. `audit()` flags a rail over a tread (`rail`) and a top step against the
outer side wall (`wallside`). `audit/21_stairs_straight.png`: all 16, before | after.

**Whole pieces.** Several FireRed pieces were placed with one of their cells: the dorms' and offices' desks (0x030
is a cabinet whose feet are the cell below), the player's PC desk (0x28F without its chair half 0x290), the photo's
drawers (0x0BB without 0x0B3), the lobby's palm (0x094 without 0x08C), the Science floors' PCs (0x062 and 0x30B
without the top 0x304) and consoles (without their top row), the healing machine (its foot cropped). Where a room has
one cell for them they are now whole one-cell pieces drawn to match (`desk_1x1`: a desk with a book or a PC, low
drawers; `small_plant`), the PCs and consoles FireRed's whole two-cell machines from Oak's lab (0x2AD/0x2B5,
0x2A6/0x2B6, 0x2A7/0x2B7), the healing machine whole. `PIECES` lists every multi-cell FireRed piece used; `audit()`
flags any placed without all its cells (`partial`). `audit/22-29_*.png`: before | after.

**The campus's trees.** The north forest ended on its lower row (crowns' bottoms with the next row's tips), so a
row of tree tips without trees showed along its edge above the stadium, the greenhouse's grove and the ranch; it now
ends on the lone tree's trunk row. The south forest began on crowns without tips: the lone tree's tips now stand on
the lawn above it (the dorm and annex path stops a row short for them; the night student who stood on that row moved
up one, to (59,58)). `tree_problems()` checks every tree on the campus (all four edges and every grove: halves paired,
crown, tip and trunk in order, the map's top and bottom rows continued by the border) and finds none.
`audit/30_campus_top.png`, `audit/31_campus_bottom.png`.
