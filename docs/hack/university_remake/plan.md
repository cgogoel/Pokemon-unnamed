# Celadon University remake: implementation plan

Goal (the user's request): remake the university with custom tiles that match the lore's look, without reusing
FireRed's buildings, with **interiors the same size as the buildings outside**. Design only so far: nothing in
`data/maps` or `data/layouts` has changed. See `survey.md` (today's campus), `art_direction.md` (the look, palettes,
budget) and the mockups in this folder.

## Decisions for you

1. **Size rule** - (a) *recommended*: an interior is exactly the building's footprint, W x H (roof + facade), on every
   floor; (b) the interior is the footprint minus the roof's top rows (smaller rooms, more "realistic" depth).
2. **Campus size** - (a) *recommended*: grow the campus to 76x64 (`plan_campus_blocks.png`) so the bigger buildings
   fit with the lore's arrangement; (b) keep 64x56 and drop/merge buildings (Literature + Library into one room,
   the Faculty Office into the Main Hall).
3. **Tile budget** - (a) *recommended*: one exterior tileset after an economy pass, and if it still doesn't fit, split
   the campus into a front map (gate, avenue, fountain, central building, hospital, pool, dorms) and a back map
   (stadium, ranch, greenhouse, Literature & Library, Science Institute) joined along the back path; (b) split from
   the start; (c) simplify the art until one tileset fits (fewer unique details, a plainer fountain).
4. **Which building is which** - (a) *recommended*: Home Economics moves into the greenhouse (the lore's glass roof,
   Erika's flowers and tea), Literature and Library share one building with two doors, the Cafeteria and the Faculty
   Office are the central building's end pavilions; (b) keep today's assignments and only resize.
5. **Science Institute's rooms** - (a) *recommended*: the tower's floors (1F hall, 2F Lounge, 3F Research Room, 4F
   Experiment Room) by stairs or an elevator, each 13x13; (b) keep the rooms as doors off a 1F hall (the hall then
   needs a building at least 28 wide, which the tower isn't).

The rest of this plan assumes the (a)s.

## The size rule and door alignment

- An interior is **W x H of the building's footprint**: the outside's columns are the inside's columns, the outside's
  rows (roof + facade) the inside's rows. Every floor of a building has the same size. A clock tower or spire that
  rises over the roof is not part of the footprint (it's an upper floor: the Main Hall's 2F gallery).
- **Door column**: a door at footprint column `d` outside is at column `d` inside. Outside, the door is on the bottom
  row (`y + H - 1`); inside, the warp/door mat is on row `H - 2` and row `H - 1` is the front wall's edge (black
  below it), as FireRed's rooms do. So campus door `(x + d, y + H - 1)` <-> interior warp `(d, H - 2)`.
- Back walls: 3 rows when the building has 3 storeys (Main Hall), else 2. Side walls take column 0 and column W-1.
  So the walkable floor is `(W - 2) x (H - 2 - wall rows)`: Main Hall 13x6 (+ the door row), lecture halls 9x5,
  dorm lobby 15x6.
- A building smaller than 9x8 is too small for a room (7x4 of floor); the campus plan keeps every building with a door
  at 9x8 or more.

## The buildings (proposed)

| Building | Exterior (W x H, look) | Door col | Floors -> interiors (each W x H) | Today |
|---|---|---|---|---|
| Main Hall | 15x11, 3 storeys, portico, clock tower 5x6 on the roof | 7 | 1F `MAIN_HALL`, 2F `MAIN_HALL_2F` (clock gallery) | 19x17, 19x10 |
| Lecture Hall A / B | 11x9 wings, 2 storeys | 5 | `LECTURE_HALL_A`, `LECTURE_HALL_B` | 10x9, 13x11 |
| Faculty Office | 9x8 west pavilion | 4 | `FACULTY_OFFICE` | 11x9 |
| Cafeteria | 9x8 east pavilion | 4 | `CAFETERIA` | 15x11 |
| Home Economics | 17x9 glass greenhouse | 8 | `HOME_ECONOMICS` (garden under glass) | 13x20 |
| Literature & Library | 22x9, one building, two doors | 5, 16 | `LITERATURE`, `LIBRARY` (11x9 each, the two halves) | 11x17, 11x9 |
| Science Research Institute | 13x13 glass cylinder + lattice tower | 6 | `SCIENCE_INSTITUTE` 1F + `SCIENCE_LOUNGE`, `SCIENCE_RESEARCH_ROOM`, `SCIENCE_EXPERIMENT_ROOM` as 2F-4F | 28x11 + 3 x 15x11 |
| University Hospital | 13x10 white modern block | 6 | `HOSPITAL`, `HOSPITAL_2F` | 15x10, 15x10 |
| Dorm | 17x10, 3 storeys | 8 | `DORM_LOBBY`, `DORM_2F`, `DORM_3F` (3 bedrooms 5 wide over a corridor) | 15x20, 35x13, 35x13 |
| Dorm annex (the player's room) | 11x8 | 5 | `DORM_ROOM` | 11x9 |
| Stadium, ranch, pool, fountain, reflecting pools, gate | outdoor | - | - | - |

Placement: `plan_campus_blocks.png` (76x64: the central building across the middle, doors on row 35, the street on
36-37, the fountain square and avenue down to the gate at the bottom; stadium, Literature & Library, greenhouse, ranch
and Science Institute behind; hospital and pool to the south-west; dorm and annex to the south-east). The coordinates
are in `PLAN` in `tools/hack/draw_university_tiles.py`.

## Tilesets

| Tileset | Over primary | For | Palettes 7-12 |
|---|---|---|---|
| `gTileset_CeladonUniversity` (new, `data/tilesets/secondary/celadon_university`) | `gTileset_General_Frlg` | the campus: every building, the fountain, pools, lamps, benches, stadium, gate | stone, glass, water, accent |
| `gTileset_UniversityInterior` (new, `.../university_interior`) | `gTileset_BuildingFrlg` | every university room | stone (same as outside), floor, furn |
| (if the campus is split) `gTileset_CeladonUniversityBack` | `gTileset_General_Frlg` | the back map's buildings | stone, glass, accent |
| (if the interiors run out) `gTileset_UniversityLab` | `gTileset_BuildingFrlg` | the Science Institute's floors and the hospital | stone, floor, glass, furn |

Grass, trees, paving, flower beds and signs stay the primary's (free). Celadon's lamp posts and fence pieces are not
used any more (our own lamps; the stadium and ranch fences get drawn).

How to build them: the way `tools/hack/draw_station_grove.py` built `gTileset_RangerStationGrove` - draw every metatile
as a 16x16 picture, cut it into 8x8 tiles, deduplicate them with flips, fit each tile to one of the palettes, write
`tiles.png` (indexed), `palettes/07-12.pal`, `metatiles.bin` and `metatile_attributes.bin`, and name the metatile ids
for the layout scripts. `draw_university_tiles.py` already draws the art that way (everything on the 16-pixel grid,
every tile in one palette, the budget counted); what's left is the writer and the metatile naming. Registration:
`src/data/tilesets/headers.h`, `graphics.h`, `metatiles.h` (as the grove did). Metatile attributes: walls and roofs
`MB_NORMAL` + collision in the layout, doors with the behaviour the campus's doors have today (copy it from their
metatiles), stairs with the behaviour of the stairs they replace, the tower's and the roofs' back rows as top-layer metatiles so the player walks behind them.

**Door animations**: a door metatile only animates if `src/field_door.c` has it in `sDoorAnimGraphicsTable` with its
frames (`graphics/door_anims/*.png`). New doors (the wooden arched door, the greenhouse's glass door, the dorm's glass
door) each need a 3-frame animation drawn and an entry there; or use non-animated doors (warp on step) for the first
version.

### The budget

The exterior art in the mockup alone is 424 unique tiles (limit 384). Economy pass, in order of payoff:

1. Make the fountain left-right symmetric (Gyarados straight up the middle, the two jets mirrored): ~93 -> ~55.
2. Put the portico's columns and the tower's quoins on the 8-pixel grid and make both halves mirror images (the
   deduplication counts flips): Main Hall ~110 -> ~75, tower ~55 -> ~35.
3. One door design, one pediment size, one dormer, shared by every building.
4. The greenhouse's rounded ends drawn once, mirrored.

Estimate after the pass: the center ~300 tiles; the rest of the campus (hospital ~40, science tower ~70, stadium ~60,
Literature & Library ~20 new, pool and ranch ~25, gate ~20) ~235 more, so **~535 for the whole campus: two exterior
maps/tilesets** (front: central building, fountain, avenue, hospital, pool, dorms; back: stadium, ranch, greenhouse,
Literature & Library, Science Institute). The back map's tileset repeats the stone kit (~60 tiles). The interiors (263
for three rooms; the others mostly reuse the kit: walls, floors, windows, desks, shelves, plants) should fit one
tileset of 384, with the lab/hospital as the overflow tileset if not.

## What moves (every coordinate that changes)

Every interior gets a new layout, so **every object, warp, coord and bg event in the interiors moves**, and the campus
grows, so **every campus event moves**. The full list as of 0888054b1d is in `survey.md`; re-dump it when starting
(other agents are editing these maps now):

    python3 -c "import json,glob; [print(d, json.load(open(d+'/map.json')).get(k)) for d in sorted(glob.glob('data/maps/CeladonUniversity*')) for k in ('warp_events','object_events','coord_events','bg_events')]"
    grep -rn "MAP_CELADON_UNIVERSITY" data/scripts src include

What needs more than moving an event:

| Map | Script / data | Change |
|---|---|---|
| campus | all 15 warps (door positions), 22 objects, 15 signs | new positions from the plan; `CeladonCityUniversityGate`'s warp back to the campus doesn't change (warp ids stay) |
| campus | `Act2_Movement_Kai/Mina/RowanToClass` (`act2_story.inc:85-160`) | rewrite: from the new dorm front to the Main Hall door |
| campus | club nights: coord events at the stadium gate (15-16,17), night friends (16,12) (14,13) (18,13), Dale (16,10) | to the new stadium's gate and field |
| DormRoom | `ACT2_START_X/Y` (`story.h`), `Act2_EventScript_ClubMorning` (`act2_story.inc:223`) | beside the bed in the new room |
| DormLobby | `Act2_EventScript_BackToDorms` (`act2_story.inc:748`) | a free floor tile in the new lobby |
| Dorm2F/3F | the friends' rooms and their name plates (bg events) | three rooms per floor 5 wide: KAI, MINA, ROWAN's rooms keep their order |
| LectureHallA/B, HomeEconomics | the friends' seats (objects only, no movements) | new seats |
| ScienceInstitute | the brief: coord events inside the door (3-5,8), `Act2_Movement_BriefFrom3/4/5`, `Act2_Movement_RowanToPlayer`, director (7,6), friends (6-9,7); `debug.inc:517` | the door is at (6,11): the coord events go on (5-7,10) and the walks get redone for the new room; the debug warp too |
| ScienceInstitute -> rooms | warps (13,5) (19,5) (25,5) | become stairs/elevator warps to 2F-4F (decision 5) |
| ScienceResearchRoom | `act2_finale.inc:418` warp to (5,2); the lab scene: friends (4-6,3), the machine at (5,2), Ashford/Marlowe/Fern walking from the door (7,9) (`Act2_Movement_LabAshford/Marlowe/Fern`) | the "door" is the stairs now: the professors come up from the stairs; redo the three movements |
| MainHall / MainHall2F | stairs warps (9,2) (17,8) / (11,8) | the new stairs (the mockup puts them at (12-13,1-3)) |
| Hospital / Hospital2F | stairs warps (1,6) / (1,6), the nurse and Chansey | the new stairs and counter |
| all interiors | door warps | `(d, H - 2)` per the door rule; their `dest_warp_id`s on the campus stay |

Nothing in `src/` needs coordinates (`src/act2_road.c` only lists the campus map).

## Step order

1. **Decide** the five questions above.
2. **Tileset writer**: extend `draw_university_tiles.py` from mockups to tilesets (economy pass, metatile naming,
   `tiles.png`/palettes/`metatiles.bin`/attributes), register `gTileset_CeladonUniversity` and
   `gTileset_UniversityInterior`. Build; check the tilesets in Porymap.
3. **Doors**: draw the door animations and add them to `field_door.c` (or ship non-animated first).
4. **Campus layout script**: a new `compose_university.py` mode (or a new script) that lays out the 76x64 campus
   from the plan with the new metatiles + the primary's grass/trees/paths (keeps `fill_with_trees`). If split, two
   layouts and the connection between them. Render with `render_layout.py` and compare with the mockup.
5. **Interior layout script**: one function per room in the same style (`compose_dorm_floor.py` is the model),
   writing each `LAYOUT_CELADON_UNIVERSITY_*` at its new size and switching its tilesets in `layouts.json`.
6. **Events**: move every warp/object/sign per the tables (a script that rewrites `map.json`s from a table, so it can be
   re-run), then the scripts in "What moves": the brief, the lab scene, the friends' walk, the club nights, the start
   position, the dorm warps, the debug warp.
7. **Strip leftovers**: per the hack's rule, remove anything from the vanilla rooms that the new rooms no longer have
   (e.g. the Trainer Tower lobby's elevator), with `tools/hack/prune_dead_scripts.py`.
8. **Build both ROMs** (`make` and `make act2`), play the Act 2 start, a course day, a club night, the brief and the lab
   scene in mGBA; screen-dump each room.
9. **Porymap pass** (you): fine-tune the campus (tree groves, path curves, decoration) and the rooms.

## Porymap steps you'll need

- Open the new tilesets once Porymap shows them (step 2) and check the metatiles' layers/behaviours.
- After step 4/5, any hand-tuning of the campus or rooms happens there; re-running the layout scripts would overwrite
  it, as with `compose_university.py` today.
