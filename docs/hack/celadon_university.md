# Celadon University

Act 2's setting. The campus is its own map north of Celadon City, through a gate. Its look follows the Tamamushi
University lore art: white stone buildings with slate roofs in a forest (the remake: `docs/hack/university_remake/`).

| Map | Layout | Notes |
|---|---|---|
| `MAP_CELADON_CITY` | `LAYOUT_CELADON_CITY` | Route 6's gate front stamped on the north edge, doors at (38,5)-(39,5) (warps 13, 14) |
| `MAP_CELADON_CITY_UNIVERSITY_GATE` | `LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE` (shared with Saffron's gates) | A friendly guard |
| `MAP_CELADON_UNIVERSITY` | `LAYOUT_CELADON_UNIVERSITY` (76x64, `gTileset_General_Frlg` + `gTileset_CeladonUniversity`) | The gate's passage at (37,59)-(38,59) |

## How it's built

- `tools/hack/draw_university_tiles.py` draws the campus and writes `gTileset_CeladonUniversity`, then has
  `tools/hack/draw_university_interiors.py` compose the rooms in FireRed's interior look
  ([university_remake/interior_art.md](university_remake/interior_art.md)) and write their tilesets, all over
  `gTileset_BuildingFrlg`: `gTileset_UniversityClassroom` (Main Hall, lecture halls, Faculty Office, Literature,
  Library), `gTileset_UniversityDorm` (dorm lobby and floors, cafeteria), `gTileset_UniversityLab` (Science Institute,
  hospital, Study Hall, greenhouse); and `tools/hack/university_stamps.json` (every building and room as metatile ids
  with collision). Needs Pillow.
- `tools/hack/build_university_maps.py` lays out the campus and every room from the stamps and places every warp,
  person, sign and trigger (its tables are the source of truth). Re-running it overwrites the layouts, so after
  editing them in Porymap, don't; move events in Porymap or in its tables.

Rooms are bigger than their buildings in proportion: building `w x h` -> room `w + 4` by `h + 2` (11x9 to 21x15), the
door at the same place (column + 2).

## Buildings

| Building (campus door) | Room(s) | Notes |
|---|---|---|
| Main Hall + clock tower (38,35) | `MAIN_HALL` 19x13, `MAIN_HALL_2F` 19x13 | Stairs up on both sides ((1-2,3), (16-17,3)) to the clock gallery |
| Lecture Hall A (25,35) | `LECTURE_HALL_A` 15x11 | Fire course (Prof. Ashford) |
| Lecture Hall B, audiovisual (51,35) | `LECTURE_HALL_B` 15x11 | Water course (Prof. Marlowe) |
| Faculty Office (15,35) | `FACULTY_OFFICE` 13x10 | West end pavilion |
| Cafeteria (61,35) | `CAFETERIA` 13x10 | East end pavilion |
| Literature & Library, one building (9,25), (20,25) | `LITERATURE`, `LIBRARY` 15x11 | |
| Home Economics, the greenhouse (37,16) | `HOME_ECONOMICS` 21x11 | Grass course (Prof. Fern) |
| Science Research Institute, the glass tower (64,20) | 1F `SCIENCE_INSTITUTE`, 2F `SCIENCE_LOUNGE`, 3F `SCIENCE_RESEARCH_ROOM`, 4F `SCIENCE_EXPERIMENT_ROOM`, 17x15 each | Stairs alternate sides: up on the right from 1F and 3F, on the left from 2F |
| University Hospital (20,49) | `HOSPITAL`, `HOSPITAL_2F` 17x12 | The nurse heals; the ward upstairs |
| Dorm (56,47) | `DORM_LOBBY`, `DORM_2F`, `DORM_3F` 21x12 | Floors: a corridor with three bedrooms (KAI, MINA and the player on 2F; ROWAN on 3F). **Act 2 starts in the player's room**, at (15,6) beside the bed (`ACT2_START_*`); after a course or a club night the player wakes up there |
| Study Hall, by the dorms (56,57) | `STUDY_HALL` 15x10 | Study tables, old exam papers, a lounge corner |
| Stadium (the Sports Complex) | outdoor | The club meets on the infield; triggers across its entrance (row 15) |
| Ranch, pool, fountain, reflecting pools | outdoor | |

## Day and night

Time of day is the game's (docs/hack/time_of_day.md); club nights are night.

- **Music**: the campus plays `MUS_RG_SEVII_67` at night (its map.json `night_music`).
- **Locked buildings**: at night the academic buildings (Main Hall, Lecture Halls A and B, Faculty Office,
  Cafeteria, Literature, Library, the Home Economics greenhouse) are locked: their doors say "It's locked. Classes are
  over for the day." (`University_IsDoorLockedAtNight` in `src/act2_story.c`, checked in `TryDoorWarp`). Open all
  night: the dorms, the Study Hall, the hospital, and the Science Research Institute (research doesn't stop at night,
  and the TM MACHINE on 3F stays reachable in the final project).
- **People**: the campus's students (`FLAG_TEMP_4`) are out only by day; by night a guard stands in front of the Main
  Hall and a student heads for the Study Hall (`FLAG_TEMP_5`). The dorm corridors' students are in their rooms at
  night (`FLAG_TEMP_8`); three more students cram in the Study Hall (its `FLAG_TEMP_1`). The ranch keeper by the
  ranch's gate has a day line and a night one.
- **Names**: the buildings have plaques beside their doors (bg events on the wall); freestanding signposts only for
  the welcome by the gate, the stadium, the ranch and the pool, on the lawn beside the paths.

## Story spots

- Act 2's start: the friends wait in the dorm lobby at the foot of the stairs ((3,3), (4,3), (3,4)), then head out the
  lobby door for class.
- Club nights: `Act2_EventScript_ClubGate` (coord events at (5-24,15)) starts the night's arrival; the friends and
  DALE stand on the infield.
- The brief (`act2_finale.inc`): coord events across the institute's row 12; the player walks to (8,10) between KAI
  and MINA, facing the director (8,9).
- The TM Database ([tm_database.md](tm_database.md)): the TM MACHINE is 3F's right-hand terminal (11,1), the
  researcher beside it at (12,2).
- Tri-Blast: the lab is 3F; the player at the center terminal (8,2), the friends behind (7-9,3), the professors
  come up the stairs (2,2).
