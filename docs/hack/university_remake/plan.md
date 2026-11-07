# Celadon University remake: implementation plan

Goal: remake the university with custom tiles that match the lore's look, without reusing FireRed's buildings, with
rooms that fit their buildings. See `survey.md` (the campus before the remake), `art_direction.md` (the look and
palettes) and the mockups in this folder.

## Decided

| Question | Decision |
|---|---|
| Room size | **Bigger than the building, in proportion** (the rule below), the door in the matching column |
| Tile budget | **Simplified art: one campus map, one campus tileset within 384 tiles** |
| Campus size | **76x64** |
| Home Economics | **Moves into the greenhouse** (the lore's glass roof) |
| Literature and Library | **One building, two doors** |
| Science Institute's rooms | **Floors of its glass tower** (1F hall, 2F Lounge, 3F Research Room, 4F Experiment Room) |

## The room size rule

For a building `w x h` (roof + facade) with its door at column `d`:

- room width `W = w + 4`, room height `H = h + 2`, kept between **11x9** and **21x15**;
- the room grows evenly on both sides, so **the door is at column `d + (W - w) / 2`** (`d + 2` unless capped), on row
  `H - 2` (the door mat; row `H - 1` is the front wall);
- every floor of a building is the same size;
- the one exception: the player's room (the dorm annex) stays **11x9**, a single room.

So a campus door `(x + d, y + h - 1)` warps to `(d + 2, H - 2)` inside, and leaving puts you back on the door.
Walls: 3 rows at the back for the three-storey Main Hall, 2 elsewhere; one column each side.

| Building | Outside (w x h, door) | Room(s) (W x H, door) | Before |
|---|---|---|---|
| Main Hall (+ clock tower) | 15x11, 7 | 1F, 2F gallery: 19x13, 9 | 19x17, 19x10 |
| Lecture Hall A / B | 11x9, 5 | 15x11, 7 | 10x9, 13x11 |
| Faculty Office, Cafeteria | 9x8, 4 | 13x10, 6 | 11x9, 15x11 |
| Home Economics (greenhouse) | 17x9, 8 | 21x11, 10 | 13x20 |
| Literature, Library (one 22x9 building) | doors 5, 16; each half 11x9 | 15x11 each, door 7 | 11x17, 11x9 |
| Science Institute | 13x13, 6 | 1F-4F: 17x15, 8 | 28x11 + three 15x11 |
| Hospital | 13x10, 6 | 1F, 2F: 17x12, 8 | 15x10, 15x10 |
| Dorm | 17x10, 8 | lobby, 2F, 3F: 21x12, 10 (2F/3F: three bedrooms over a corridor) | 15x20, 35x13 |
| Dorm annex (your room) | 11x8, 5 | 11x9, 5 (exception) | 11x9 |

## What's built (art step, this branch)

`tools/hack/draw_university_tiles.py` draws everything and writes:

| Output | What |
|---|---|
| `data/tilesets/secondary/celadon_university/` | `gTileset_CeladonUniversity` (over `gTileset_General_Frlg`): **357/384 tiles, 347/384 metatiles**, palettes 7-10 (stone, glass, water, accent) |
| `data/tilesets/secondary/university_interior/` | `gTileset_UniversityInterior` (over `gTileset_BuildingFrlg`): **310/384 tiles, 188/384 metatiles**, palettes 7-11 (stone, floor, furn, glass, accent) |
| `src/data/tilesets/{headers,graphics,metatiles}.h` | Both registered (not used by any layout yet). Both ROMs build. |
| `tools/hack/university_stamps.json` | Every building, room and the furniture kit as a grid of metatile ids (null = keep the ground), with door/warp cells: what the layout scripts will stamp |
| `docs/hack/university_remake/mockup_*.png` | Rendered from the written tilesets (not from the drawings), so they show what the game will |

Campus stamps: Main Hall with the clock tower (15x16: the tower's 5 rows rise over the path behind and are top-layer
so people walk behind it), lecture wing, pavilion, Literature & Library, greenhouse, Science Institute, hospital,
dorm, annex, stadium (stands, track, four floodlights), pool, barn, gate (its passage is the warp), fountain, reflecting
pool, lamp, bench. Ground, trees, paths, flower beds and the ranch fence are the primary's metatiles.

Room stamps: the Main Hall, Lecture Hall and Dorm lobby at the new size, and a kit sheet (beds, desk, kitchen,
dining table, stairs down, greenhouse garden beds and glass wall, lab benches, terminal, ward beds, lab floor) for the
other rooms.

Metatile behaviors: doors `MB_ANIMATED_DOOR` (no animation yet; the game skips it), the gate's passage and the room
door mats `MB_SOUTH_ARROW_WARP`, stairs the Celadon Condominiums' stairs behavior, the rest `MB_NORMAL` (collision
goes in the layouts).

Budget notes: the campus tileset has 27 tiles and 37 metatiles left. Every building is mirror-symmetric (its right
half is the left half flipped), with one window, one door, one dormer and one roof/wall kit shared by all, so new
buildings in the same style are cheap. The interior tileset has room for the rest of the furniture (74 tiles).

## What moves when the maps change (next step, after the other agents' event work lands)

Every interior gets a new layout, so **every object, warp, coord and bg event in the interiors moves**; the campus
grows to 76x64, so **every campus event moves**. Re-dump the events when starting (the list in `survey.md` is from
0888054b1d):

    python3 -c "import json,glob; [print(d, json.load(open(d+'/map.json')).get(k)) for d in sorted(glob.glob('data/maps/CeladonUniversity*')) for k in ('warp_events','object_events','coord_events','bg_events')]"
    grep -rn "MAP_CELADON_UNIVERSITY" data/scripts src include

Beyond moving events:

| Map | Script / data | Change |
|---|---|---|
| campus | 15 warps, ~22 objects, 15 signs | new positions; the gate map's warp ids stay |
| campus | `Act2_Movement_Kai/Mina/RowanToClass` (`act2_story.inc:85-160`) | redo: from the dorm door (56,47) to the Main Hall door (38,35) |
| campus | club nights: coord events at the stadium entrance, night friends and Dale on the field | the new stadium's entrance (y 16) and infield |
| DormRoom | `ACT2_START_X/Y` (`story.h`), `Act2_EventScript_ClubMorning` (`act2_story.inc:223`) | beside the bed in the new room |
| DormLobby | `Act2_EventScript_BackToDorms` (`act2_story.inc:748`) | a free tile in the 21x12 lobby |
| Dorm2F/3F | friends' rooms, name plates | three rooms per 21x12 floor |
| LectureHallA/B, HomeEconomics | the friends' seats | new seats |
| ScienceInstitute | the brief: coord events inside the door (now (7-9,12)), `Act2_Movement_BriefFrom3/4/5`, `Act2_Movement_RowanToPlayer`, the director and friends; `debug.inc:517` | redo for the 17x15 room |
| ScienceInstitute -> rooms | warps (13,5) (19,5) (25,5) become stairs/elevator to 2F-4F | new maps' warps |
| ScienceResearchRoom (3F) | `act2_finale.inc:418` warp to (5,2); the lab scene: friends, the machine, Ashford/Marlowe/Fern walking in (`Act2_Movement_LabAshford/Marlowe/Fern`) | they come up the stairs; redo the walks |
| MainHall / 2F, Hospital / 2F | stairs warps | the new stairs (Main Hall: (1-2,3) and (16-17,3)) |
| all rooms | door warps | `(d + 2, H - 2)` per the rule |

## Step order from here

1. ~~Decide~~ (done). ~~Tileset writer, all buildings, mockups~~ (done, this branch).
2. **Campus layout**: a script that lays out the 76x64 campus from `CAMPUS`/`CAMPUS_PATHS` in
   `draw_university_tiles.py` and the stamps (the mockup is exactly that), switches `LAYOUT_CELADON_UNIVERSITY` to
   `gTileset_CeladonUniversity` and 76x64, and writes collision (buildings, water, trees, fences solid; doors and the
   gate passage walkable).
3. **Rooms**: one function per room at the rule's size, from the room stamps and the kit, switching each
   `LAYOUT_CELADON_UNIVERSITY_*` to `gTileset_UniversityInterior`; new layouts/maps for the Science tower's floors.
4. **Events and scripts** per the table above (a script that rewrites the `map.json`s from a table, re-runnable).
5. **Strip leftovers** from the old vanilla rooms (`tools/hack/prune_dead_scripts.py`).
6. **Build both ROMs**, play the Act 2 start, a course day, a club night, the brief and the lab scene; screen-dump
   each room.
7. **Porymap pass** (you): fine-tune groves, paths and decoration. Re-running the layout scripts would overwrite it.
8. Later: door animations (`src/field_door.c`, 1x2 frames like FRLG's) for the wooden, glass and gate doors.
