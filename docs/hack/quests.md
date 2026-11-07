# Quests

The start menu's **QUESTS** (after BAG, from Act 2 on: `Quests_IsMenuAvailable`) guides the player through each
act's main story. Code: `src/quests.c` (the content, worked out from the story's vars) and `src/quest_menu.c` (the
screen). Tests: `test/quests.c`.

## The screen

- **Left:** the act's % and bar. Below them is the quest list: the quest under way first (blue), then the finished
  ones, newest first, each with a checkmark. A `? ? ?` row stands for the quests still to come. Up/Down choose a quest.
  Five rows show at a time; the list scrolls, with arrows under it when there's more above or below.
- **Right:** the chosen quest's title and %, the place of its next step (with a pin), a bar, and up to 6 tasks.
  While a task is under way, its counter (`2/3`) shows in place of its checkbox. Finished tasks are greyed out.
- **Bottom:** `NEXT:` and the next step. The narrow font is used when the normal one doesn't fit.
- **A: MAP** opens the Town Map with the cursor on the current quest's place (`FieldInitRegionMapFromQuests`).
  B goes back to the start menu.

## The map marker

The Town Map (`src/field_region_map.c`) is used by the TOWN MAP item, wall maps and the quest screen. On it, the
current quest's place (`Quests_GetTargetMapSec`) has the fly map's red outline box, blinking every 16 frames
(`CreateRegionMapQuestMarker` in `src/region_map.c`). With the cursor on that place, the name window reads
`QUEST: <place>`. The check is by the cursor's square, not by the layout's section, since Diglett's Cave shares its
square with Route 11 (`RegionMap_IsCursorOnMapSec`). The name window is 18 tiles wide for this, and the frame's tiles
moved to 0x40.

## Percentages

- **A quest's %** = steps done / its steps, floored.
- **The act's %** = the average of its quests' fractions.

## Act 2's quests

| # | Quest | Stages (`VAR_ACT2_PROGRESS`) | Steps |
|---|---|---|---|
| 1 | First day | `ACT2_START` | Meet the classmates (1) |
| 2 | FIRE course | `ACT2_MET_FRIENDS` – `ACT2_FIRE_TRIP` | 13 (below) |
| 3 | Club night 1 | `ACT2_FIRE_SEE_KAI` – `ACT2_FIRE_INVITED` | At the Sports Complex (`ACT2_RESEARCH_CLUB_ARRIVED`), the battle |
| 4 | WATER course | `ACT2_CLUB_1_DONE` – `ACT2_WATER_TRIP` | 13 |
| 5 | Club night 2 | `ACT2_WATER_SEE_MINA` – `ACT2_WATER_INVITED` | 2 |
| 6 | GRASS course | `ACT2_CLUB_2_DONE` – `ACT2_GRASS_TRIP` | 13 |
| 7 | Trainer Card | `ACT2_GRASS_SEE_ROWAN` – `ACT2_GRASS_INVITED` | At the Sports Complex, beat DALE |
| 8 | Road home | `ACT2_GOT_TRAINER_CARD` – `ACT2_FINAL_PROJECT` | The brief, 6 stops, Mom and Dad (8) |
| 9 | Eerie noise | `ACT2_PARENTS_TALKED` | 6 ranger station steps |

**A course's 13 steps:**

| Steps | What counts |
|---|---|
| 3 | Pokémon caught (`Act2_CountCaughtSpeciesOfType`) |
| 1 | A move of the type used |
| 3 | Moves recorded |
| 3 | Puzzle parts (`ACT2_RESEARCH_PUZZLE_1-3`) |
| 1 | The puzzle's end (`_PUZZLE_DONE`) |
| 1 | The TM (`_GOT_TM`) |
| 1 | The exam (passed: the stage moves on) |

The exam doesn't need the TM's move. The next step is the first unfinished one, in this order:

1. Before the trip: the course's room on campus.
2. Talk to the professor (`_BRIEFED`).
3. Catch, use a move, record moves.
4. The puzzle. On the Fire trip, after the furnaces: the statue switch.
5. The TM.
6. The exam.

The Water course follows `Act2_IsReadyForTM`: the small fires, then the TM, then the big fire with BubbleBeam, then
the young Lapras (`FLAG_ACT2_GOT_LAPRAS`), then the exam. The place that blinks is the site (Cinnabar, Vermilion,
Viridian Forest) or the professor's town (for the briefing).

The Grass course follows the night bloom: the glowcaps, the ODDISH, guarding the MOONVEIL until dawn, then the TM and
the exam, all at Viridian Forest (PROF. FERN is in its glade from dawn).

**The road home** takes either way: south (Vermilion, Route 11, Diglett's Cave) or north (Route 5, Cerulean, Mt. Moon,
Pewter). It has six stops by distance:

1. Saffron
2. Vermilion or Cerulean
3. Diglett's Cave or Pewter
4. Viridian
5. Pallet
6. The farm (Farmyard or Woods)

On entering a map at `ACT2_FINAL_PROJECT`, `Quests_OnMapEnter` (called from `LoadMapFromWarp` and
`LoadMapFromCameraTransition`) sets the stop's bit in `VAR_QUEST_LOG` by the map's section. Route 6, Route 11 and
Diglett's Cave mark the south way; Route 3, Route 4, Route 5, Mt. Moon, Cerulean and Pewter mark the north way
(`QUEST_LOG_ROAD_NORTH`). The next stop is the first one not reached, on the way the player last took. Until the
player takes one, it's the south way.

**The eerie noise:**

1. The station is entered (`QUEST_LOG_STATION_ENTERED`, from map entry).
2. `ACT2_FINALE_BASEMENT_OPEN`.
3. `ACT2_FINALE_CLOCK_SOLVED`.
4. B3F is reached (`QUEST_LOG_STATION_B3F`).
5. `ACT2_FINALE_GRASS_CROSSED`.
6. The trio is calmed.

The marker stays on Pallet Town, since the Woods and the station are in its map section.

## Act 3

There's a placeholder until Act 3's story has stages: one quest, SILPH CO., with Saffron blinking. To add Act 3's
quests, give `GetQuests` in `src/quests.c` a table like `sAct2Quests` and read its progress var in `GetActProgress`.

## State

| What | Where |
|---|---|
| Road home stops, the way taken, the station's floors | `VAR_QUEST_LOG` (`QUEST_LOG_*` in `include/constants/quests.h`) |
| Everything else | The story's own vars and flags |
