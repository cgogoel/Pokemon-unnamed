# Quest menu: draft (mockups and content model)

Draft for choosing a layout. Nothing here is in the build yet. The mockups are drawn by `make_mockups.py` (Pillow)
with the game's real fonts (`graphics/fonts/latin_normal.png`, `latin_small.png`), window frame
(`graphics/text_window/1.png`), keypad icons, and the Kanto region map with its frame and the fly map's red box
(`graphics/pokenav/region_map/`). They're 240x160 shown at 3x. Every mockup shows the same moment: the Water course
on the Vermilion trip, with 2 of 3 Water Pokémon caught, a Water move used, 3 moves recorded and 1 of 3 dock fires out.

| File | What |
|---|---|
| `start_menu_quests.png` | The pause menu with QUESTS added after BAG (Act 2 and Act 3 only) |
| `layout_a_list_detail.png` | **A: List + detail.** Quests on the left (current quest first, then finished ones, then `? ? ?`), and the act's %. The selected quest's tasks and % on the right; the next step along the bottom. A: map, B: back. |
| `layout_b_journal.png` | **B: Journal page.** One quest per page; L/R turns pages. The act's % in the header; the quest's place, %, and tasks in two columns; a NEXT STEP box like the dialogue box. |
| `layout_c_bag_minimap.png` | **C: Bag style with a mini map.** MAIN/DONE tabs. A zoomed map crop at top left with the target blinking (no extra screen needed); the quest list with % bars at right; the tasks and next step in the bottom box, like the bag's description. |
| `map_a_red_box_blink.png` | **Map A:** the Town Map (and fly map) with the fly map's existing red outline box (`FLYDESTICON_RED_OUTLINE`) blinking on the quest's map section. It shows on and off every 16 frames, like `SpriteCB_FlyDestIcon`. The bottom window shows `QUEST: <place>`. |
| `map_b_road_home_blink.png` | **Map B:** the final project's road home drawn on the map. The walked part is a solid line and the rest is dotted. The next stop is tinted red with a pin and blinks. |

The map style is a separate choice from the quest screen. Map A works anywhere. Map B adds the route line for the
road home, and every other quest would use only its blinking marker (pin and tint).

## Where it hooks in

- **Start menu** (`src/start_menu.c`): `MENU_ACTION_QUESTS` after BAG when `VAR_STORY_ACT` ≥ `STORY_ACT_UNIVERSITY`.
  Act 1 has no quest menu.
- **Quest screen**: a new `src/quest_menu.c` (UI) and `src/quests.c` (the content model below: pure functions over
  the story vars, so the menu, the Town Map and tests share it).
- **Region map**: from the quest screen (A: MAP), and on the Town Map and fly map. The target's map section gets the
  blinking marker (`Quest_GetTargetMapSec()`; `MAPSEC_NONE` means none).

## Percentages

- **A quest's %** = steps done / its steps. Steps are counted from units that already exist, so the bar moves with
  each catch, move, fire or stop.
- **The act's %** = the sum of each quest's fraction / the number of quests (9 in Act 2). Example: First day,
  Fire course and Club night 1 are done and the Water course is at 53%, so (1 + 1 + 1 + 0.53) / 9 = 39%.
- Shown floored, so a quest reads 100% only when it's done.

## Act 2 quests

Nine main quests, in order. A quest is *current* while `VAR_ACT2_PROGRESS` is in its stage range, *done* past
it, and `? ? ?` before it. "Blinks" is the map section that gets the marker.

| # | Quest | Stages | Steps (units) |
|---|---|---|---|
| 1 | FIRST DAY | `ACT2_START` | Meet the friends (1) |
| 2 | FIRE COURSE | `ACT2_MET_FRIENDS` – `ACT2_FIRE_TRIP` | The course's 13 (below) |
| 3 | CLUB NIGHT 1 | `ACT2_FIRE_SEE_KAI` – `ACT2_FIRE_INVITED` | Visit KAI (1), win the battle (1) |
| 4 | WATER COURSE | `ACT2_CLUB_1_DONE` – `ACT2_WATER_TRIP` | 13 |
| 5 | CLUB NIGHT 2 | `ACT2_WATER_SEE_MINA` – `ACT2_WATER_INVITED` | Visit MINA (1), win (1) |
| 6 | GRASS COURSE | `ACT2_CLUB_2_DONE` – `ACT2_GRASS_TRIP` | 13 |
| 7 | TRAINER CARD | `ACT2_GRASS_SEE_ROWAN` – `ACT2_GRASS_INVITED` | Visit ROWAN (1), beat DALE (1) |
| 8 | THE ROAD HOME | `ACT2_GOT_TRAINER_CARD` – `ACT2_FINAL_PROJECT` | The brief (1), 6 stops (6), Mom and Dad (1) = 8 |
| 9 | THE NOISE IN THE WOODS | `ACT2_PARENTS_TALKED` | 6 (below) |

`ACT2_LEGENDS_CALMED` → `ACT2_DONE` is one cutscene (the friends' visit, the lab, Tri-Blast), so the screen just shows
all nine done.

### A course (2, 4, 6)

**Steps (13)**: Water Pokémon caught (0-3, `Act2_CountCaughtSpeciesOfType`) + used a move of the type (0-1) + moves
recorded (0-3, `CountMovesRecorded`) + the puzzle's parts (0-3, `ACT2_RESEARCH_PUZZLE_1-3`) + its end (0-1,
`ACT2_RESEARCH_PUZZLE_DONE`) + the TM (0-1, `ACT2_RESEARCH_GOT_TM`) + the exam (0-1, passed = the stage moves on).
Before the trip (`ACT2_MET_FRIENDS` / `CLUB_1_DONE` / `CLUB_2_DONE`) it's 0%. This reuses `GetResearchDone` and
`Act2_BufferResearchSheet`'s counts, so the quest screen and the professor's research sheet always agree.

**Tasks shown** (the research sheet's four, then the TM and the exam). While a task is under way its counter shows in
place of the checkbox (`2/3`):

| Task | Fire | Water | Grass |
|---|---|---|---|
| Catch 3 … POKéMON | FIRE | WATER | GRASS |
| Use a … move | | | |
| Record 3 … moves | | | |
| The puzzle | Relight 3 furnaces | Put out 3 dock fires | Regrow 3 plants |
| Make the TM | FLAMETHROWER | BUBBLEBEAM | GIGA DRAIN |
| Pass the exam | PROF. ASHFORD | PROF. MARLOWE | PROF. FERN |

**Next step**: the first rule that matches.

| When | Fire | Water | Grass | Blinks |
|---|---|---|---|---|
| Before the trip | Go to LECTURE HALL A on campus. | Go to the AUDIOVISUAL room on campus. | Go to HOME ECONOMICS on campus. | Celadon City |
| Trip, not briefed | Talk to PROF. ASHFORD. | Talk to PROF. MARLOWE. | Talk to PROF. FERN. | The trip's town |
| Caught < 3 | Catch FIRE POKéMON in the MANSION. | Fish at the harbor, or try ROUTE 6. | Catch GRASS POKéMON on ROUTE 22. | Fire: Cinnabar; Water: Vermilion; Grass: Route 22 |
| No move used | Use a FIRE move in battle. | Use a WATER move in battle. | Use a GRASS move in battle. | The site |
| Moves < 3 | Record FIRE moves in battle: n/3. | (same, WATER) | (same, GRASS) | The site |
| Puzzle under way | Relight the MANSION's cold furnaces. | Put out the fires on the docks. | Regrow the withered plants. | Cinnabar / Vermilion / Route 22 |
| Parts done, end not | Press the statue switch. Then the sealed room. | — | — | Cinnabar |
| Water: small fires out, no TM | — | Make BUBBLEBEAM with PROF. MARLOWE. | — | Vermilion |
| Water: TM, big fire burning | — | Put out the big fire with BUBBLEBEAM. | — | Vermilion |
| Water: big fire out, no Lapras | — | Someone is waiting by the pier… | — | Vermilion |
| Research done, no TM | Make the TM with PROF. ASHFORD. | (MARLOWE) | (FERN) | The trip's town |
| TM, no Pokémon of the type knows it | Teach FLAMETHROWER to a FIRE POKéMON. | (BUBBLEBEAM, WATER) | (GIGA DRAIN, GRASS) | — |
| Ready | Beat PROF. ASHFORD using FLAMETHROWER. | (…) | (…) | The trip's town |

Water's order follows `Act2_IsReadyForTM`: there the TM comes before the big fire. Before the docks catch fire
(`ACT2_RESEARCH_DOCK_FIRE` unset), the puzzle row reads "Catch a WATER POKéMON first." instead.

### A club night (3, 5, 7)

| When | Next step | Blinks |
|---|---|---|
| `…_SEE_KAI` / `_MINA` / `_ROWAN` | Visit KAI's room, DORMS 2F. / MINA's room, DORMS 2F. / ROWAN's room, DORMS 3F. | Celadon City |
| `…_INVITED`, not arrived | Meet the club at the SPORTS COMPLEX. | Celadon City |
| Arrived (`ACT2_RESEARCH_CLUB_ARRIVED`) | Line up your team, then talk to a friend. (Night 3: Talk to DALE when you're ready.) | Celadon City |

Steps: visited (the stage is `_INVITED`) + won (the stage moves on).

### 8: The road home

| When | Next step | Blinks |
|---|---|---|
| `ACT2_GOT_TRAINER_CARD` | Go to the SCIENCE RESEARCH INSTITUTE. | Celadon City |
| `ACT2_FINAL_PROJECT`, next stop … | | |
| Saffron | Head east on ROUTE 7 to SAFFRON CITY. | Saffron City |
| Vermilion | Go south via ROUTE 6 to VERMILION. | Vermilion City |
| Diglett's Cave | Go east to ROUTE 11 and DIGLETT'S CAVE. | Diglett's Cave |
| Viridian | Through the cave, then south to VIRIDIAN. | Viridian City |
| Pallet | Go south down ROUTE 1 to PALLET TOWN. | Pallet Town |
| Home | Go home to the farm. | Pallet Town |
| Home reached | Talk to Mom and Dad in the farmhouse. | Pallet Town |

**Stops**: Saffron City, Vermilion City, Diglett's Cave (any of its maps), Viridian City, Pallet Town, the Farmyard,
from `sRouteHomeMaps` in `src/act2_road.c`. **New state:** `VAR_ACT2_ROAD_STOPS`, one bit per stop, set on entering
the stop's map in Act 2 at `ACT2_FINAL_PROJECT` (from the same map-load hook `act2_road.c` uses). Bits rather than
"furthest" so the count stays right whatever order the player walks. The next stop is the first one not reached.
With Map B, the route line is solid up to the last stop reached.

The % = (brief + stops reached + parents) / 8. Example: briefed, Saffron and Vermilion reached = 3/8 = 37%.

### 9: The noise in the Woods

| Step | Done when | Next step text | Blinks |
|---|---|---|---|
| 1 | The station entered (**new** bit `ACT2_FINALE_ENTERED` in `VAR_ACT2_FINALE`) | Follow the eerie noise in the WOODS. | Pallet Town |
| 2 | `ACT2_FINALE_BASEMENT_OPEN` | Search the ranger station. | Pallet Town |
| 3 | `ACT2_FINALE_CLOCK_SOLVED` | Set the clock. The explorers' notes… | Pallet Town |
| 4 | B3F reached (**new** bit `ACT2_FINALE_REACHED_B3F`) | Cross the flooded floor. | Pallet Town |
| 5 | `ACT2_FINALE_GRASS_CROSSED` | Find the hidden path. The tablet… | Pallet Town |
| 6 | `ACT2_LEGENDS_CALMED` | Go down to what's making the noise. | Pallet Town |

The station and the Woods are both in `MAPSEC_PALLET_TOWN`, so the marker stays on Pallet. The two new bits fit in
`VAR_ACT2_FINALE`'s free bits (5 on).

### Possible side quest (optional, for the MAIN/DONE or a SIDE tab)

- **THE FLOODED FLOOR**: the five Water Pokémon at B2F's dead ends, 0-5 (`FLAG_ACT2_DEAD_END_1-5`). Shown once the
  station is entered. Act 2 has no other optional content yet, so layout C's tabs are MAIN/DONE rather than
  MAIN/SIDE.

## Act 3 (placeholder hook)

The content model is a table of quests per act: `{ title, first stage, last stage, StepsFunc, NextStepFunc,
TargetMapSecFunc }` against that act's progress var. Act 3 gets its own table once `VAR_ACT3_PROGRESS` exists. Until
then, in Act 3 the screen shows one quest, **SILPH CO.**: "Your work at SILPH CO. starts here." It's at 0%, and
Saffron City blinks. The header reads `ACT 3: WORKFORCE`.

## Questions for the user

1. Quest screen: **A** (list + detail), **B** (journal page), or **C** (bag style with a mini map)?
2. Map marker: **Map A** (the fly map's red box blinking) or **Map B** (a blinking pin and tint, plus the road home
   drawn as a route)?
3. Where the marker shows: only from the quest screen, or also on the Town Map and fly map?
4. In the start menu, QUESTS goes after BAG. Is that the right place?
