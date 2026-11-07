# Celadon University remake: survey of the current campus

Measured on `master` at 0888054b1d from `LAYOUT_CELADON_UNIVERSITY` (64x56, `gTileset_General_Frlg` +
`gTileset_CeladonCity`), rendered with `tools/hack/render_layout.py`. A building's footprint is the rectangle its tiles
cover on the map, roof and facade included (the door row is the bottom row). Interior sizes come from `layouts.json`.

Every building is a stamp of a FireRed building (`tools/hack/compose_university.py`), and every interior is a copy of a
FireRed room. The two were never sized for each other: the rooms are 2 to 11 times the building's area.

| Building on campus | Placeholder look | Footprint (tiles) | W x H | Door(s) | Interior map(s) | Interior size | Interior / exterior area |
|---|---|---|---|---|---|---|---|
| Main Hall (clock tower) | Celadon Dept. Store | x27-35, y18-27 | 9 x 10 | (29,27), (33,27) | `MAIN_HALL`, `MAIN_HALL_2F` | 19 x 17 (T-shaped, half of it black), 19 x 10 | 3.6x |
| Cafeteria (west end) | Game Corner end piece | x17-19, y23-27 | 3 x 5 | (18,26) | `CAFETERIA` | 15 x 11 | 11x |
| Lecture Hall A (west wing) | Game Corner | x20-26, y23-27 | 7 x 5 | (23,27) | `LECTURE_HALL_A` | 10 x 9 | 2.6x |
| Lecture Hall B (east wing) | Game Corner | x36-42, y23-27 | 7 x 5 | (39,27) | `LECTURE_HALL_B` | 13 x 11 | 4.1x |
| Faculty Office (east end) | Game Corner end piece | x43-45, y23-27 | 3 x 5 | (44,26) | `FACULTY_OFFICE` | 11 x 9 | 6.6x |
| Greenhouse (behind the west wing) | Restaurant's glass roof | x20-25, y19-21 | 6 x 3 | none | none | - | - |
| Literature | FireRed house | x37-40, y14-18 | 4 x 5 | (38,18) | `LITERATURE` | 11 x 17 | 9.4x |
| Library | FireRed house | x41-44, y14-18 | 4 x 5 | (42,18) | `LIBRARY` | 11 x 9 | 5x |
| Science Research Institute | Celadon Condominiums | x50-56, y8-15 | 7 x 8 | (53,15) | `SCIENCE_INSTITUTE` + `SCIENCE_LOUNGE`, `SCIENCE_RESEARCH_ROOM`, `SCIENCE_EXPERIMENT_ROOM` (doors off its hall) | 28 x 11 + three 15 x 11 | 5.5x (11x with the rooms) |
| University Hospital | Pokémon Center | x18-22, y38-42 | 5 x 5 | (20,42) | `HOSPITAL`, `HOSPITAL_2F` | 15 x 10, 15 x 10 | 6x a floor |
| Dorm (lobby tower) | Celadon Condominiums | x36-42, y37-44 | 7 x 8 | (39,44) | `DORM_LOBBY`, `DORM_2F`, `DORM_3F` | 15 x 20, 35 x 13, 35 x 13 | 5.4x lobby, 8x a floor |
| Dorm (the player's tower) | Celadon Condominiums | x43-49, y37-44 | 7 x 8 | (46,44) | `DORM_ROOM` (Act 2 start, (2,3)) | 11 x 9 | 1.8x |
| Home Economics | Celadon Gym | x50-57, y34-38 | 8 x 5 | (53,38) | `HOME_ECONOMICS` | 13 x 20 | 6.5x |
| Stadium | fence + Celadon paving track | x5-26, y5-17 | 22 x 13 | (gap at x15-16, y17) | outdoor | - | - |
| Research ranch | fence | x4-13, y19-27 | 10 x 9 | (gap at x8-9, y27) | outdoor | - | - |
| Gate | Saffron's south gate back | x29-34, y48-55 | 6 x 8 | (31,49), (32,49) | `CELADON_CITY_UNIVERSITY_GATE` | (shared Saffron gate layout) | - |

## What doesn't match the lore

The lore art (Tamamushi University) shows one long **white stone** central building with a clock tower in the middle,
the Gyarados fountain in front, a great arched glass roof behind it, a stadium with an oval track, stands and
floodlight towers, a tall cylindrical glass Science Research Institute with a lattice tower, a multi-storey white
hospital beside an outdoor pool, a long avenue with a reflecting pool, all wrapped in dark forest. The placeholders
are Celadon's buildings: a purple-and-yellow department store as the tower, orange-brick Game Corner wings, beige
condominiums, a Pokémon Center, a gym. Five different styles, none of them white stone.

## Interior details (for the remake's coordinate changes)

| Interior | Size | Warps | Objects | Coord events | Bg events |
|---|---|---|---|---|---|
| MainHall | 19x17 | (9,15) out; (9,2), (17,8) to 2F | 4: (4,9) (14,9) (12,13) (2,12) | - | 1 |
| MainHall2F | 19x10 | (11,8) down | 4: (10,5) (0,6) (15,8) (4,9) | - | 8 |
| LectureHallA | 10x9 | (3-5,7) out | 5: prof (6,2); student (4,5); KAI (6,5), MINA (7,5), ROWAN (2,5) | - | 5 |
| LectureHallB | 13x11 | (5,10) (6,9) (7,10) out | 4: (3,5); KAI (6,7), MINA (7,7), ROWAN (8,7) | - | 4 |
| Cafeteria | 15x11 | (5-7,9) out | 5: (12,7) (11,4) (9,4) (1,2) (3,7) | - | 0 |
| FacultyOffice | 11x9 | (3-5,7) out | 2: (1,2), Slowpoke (6,3) | - | 0 |
| Literature | 11x17 | (5,15) out | 4: (3,12) (3,13) (9,11) (9,12) | - | 0 |
| Library | 11x9 | (4,7) out | 2: (4,4) (6,2) | - | 2 |
| ScienceInstitute | 28x11 | (3-5,9) out; doors (13,5) (19,5) (25,5) to the rooms | 5: scientist (2,4); DIRECTOR (7,6); KAI (6,7), MINA (8,7), ROWAN (9,7) | (3-5,8) `Act2_EventScript_FinalBrief` | 4 |
| ScienceLounge | 15x11 | (7,9) | 3 | - | 0 |
| ScienceResearchRoom | 15x11 | (7,9) | 8: (5,4) (10,3); KAI (4,3) MINA (5,3) ROWAN (6,3); ASHFORD, MARLOWE, FERN at the door (7,9) | - | 2 |
| ScienceExperimentRoom | 15x11 | (7,9) | 2 | - | 0 |
| Hospital | 15x10 | (7,8) out; (1,6) to 2F | 5: nurse (7,2), Chansey (8,2), (4,4) (10,6) (12,5) | - | 0 |
| Hospital2F | 15x10 | (1,6) down | 3 | - | 0 |
| DormLobby | 15x20 | (11,19) (12,18) (13,19) out; (4,2) (12,2) up | 3: (2,9), Meowth (1,9), Clefairy (5,13) | - | 2 |
| Dorm2F / Dorm3F | 35x13 | (2,10), (32,10) stairs | 4 each (the friends' rooms: KAI (2,5), MINA (14,5), ROWAN (26,5)) | - | 7 / 4 |
| DormRoom | 11x9 | (4,7) out | 1: Lapras doll (9,6) | - | 8 |
| HomeEconomics | 13x20 | (5-7,18) out | 8: incl. KAI (4,6), MINA (8,6), ROWAN (9,6) | - | 2 |

Scripts outside the map folders that name a coordinate in these maps (found with
`grep -rn "MAP_CELADON_UNIVERSITY" data/scripts src include` and the movement labels they use):

| Where | What | Coordinates |
|---|---|---|
| `include/constants/story.h:78-80` | `ACT2_START_MAP/X/Y`: Act 2 starts in the dorm room | DormRoom (2,3) |
| `data/scripts/act2_story.inc:223` | `Act2_EventScript_ClubMorning`: back to bed after a club night | DormRoom (2,3) |
| `data/scripts/act2_story.inc:748` | `Act2_EventScript_BackToDorms`: after an exam | DormLobby (12,14) |
| `data/scripts/act2_story.inc:60-160` | `Act2_EventScript_MeetFriends` + `Act2_Movement_Kai/Mina/RowanToClass`: the friends walk from the dorm front to the Main Hall | campus: from (44-48,46) along the paths to the avenue (x 33) |
| `data/scripts/act2_story.inc` (club nights) | coord events at the stadium gate, the night friends and Dale on the field | campus (15-16,17), (16,10) (16,12) (14,13) (18,13) |
| `data/scripts/act2_finale.inc:13-95` | The director's brief: coord events inside the door, `Act2_Movement_BriefFrom3/4/5` (right, then up), `Act2_Movement_RowanToPlayer` (from (9,7) to (7,8)) | ScienceInstitute (3-5,8), (7,6), (6-9,7) |
| `data/scripts/act2_finale.inc:418` | Into the lab for Tri-Blast | ScienceResearchRoom (5,2) |
| `data/scripts/act2_finale.inc:462-560` | The lab scene: friends behind the player at the machine, `Act2_Movement_LabAshford/Marlowe/Fern` from the door (7,9) up to row 5 | ScienceResearchRoom (4-6,3), (5,2), (7,9) |
| `data/scripts/debug.inc:517` | `Debug_EventScript_SkipToFinalProject` | ScienceInstitute (4,9) |
| `src/act2_road.c:35` | The campus is on the route home (no coordinates) | - |

Other agents are editing university events right now, so re-run that grep (and the event dump in `plan.md`) when the
remake starts.
