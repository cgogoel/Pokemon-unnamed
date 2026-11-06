# Act 2 finale: the final project

What happens after the Trainer Card (the story so far: [act2_story.md](act2_story.md)). Stages are `ACT2_*` in
`include/constants/story.h`, from `ACT2_GOT_TRAINER_CARD`.

| Stage | Reached by | Then |
|---|---|---|
| `ACT2_GOT_TRAINER_CARD` | Winning the Trainer Card | Morning. The friends wait in the Science Research Institute with the director. |
| `ACT2_FINAL_PROJECT` | The director's brief | Free to explore Kanto. Trainers and wild Pokémon are scaled (below). |
| `ACT2_PARENTS_TALKED` | Mom and Dad, at home | The Woods open: the eerie noise, the ranger station. |
| `ACT2_LEGENDS_CALMED` | Mom, Dad and Mew stop the legendary trio | The player wakes in their room at home; the friends arrive. |
| `ACT2_DONE` | Tri-Blast, in the lab | "Several years later", then Act 3 in Silph Co. |

## The brief

The director of the Science Research Institute (the student's own professor for the project) gives the four students
an open brief: make a TM with at least 120 power. Follow your passion; remember what got you interested in the first
place. It's an unguided exam: each gets a TOWN MAP and ¥10,000, and should stock up at the Department Store before
leaving.

Then the friends come over, each with a TM in their favorite type, matching the night the player battled them:

| Friend | Type | Their TM (120 power) |
|---|---|---|
| KAI | Fire | Flare Blitz |
| MINA | Water | Wave Crash |
| ROWAN | Grass | Solar Beam |

They ask the player's plan; "…" — the player doesn't love one type, they want to discover something new. The friends
echo the director: maybe go back to the Woods where it all started? Last, a warning: with a Trainer Card, trainers
on the road will challenge them.

## The road home

The most direct route home, which needs no HMs:

Celadon City → Route 7 → (gate) → Saffron City → (gate) → Route 6 → Vermilion City → Route 11 → Diglett's Cave →
Route 2 → Viridian City → Route 1 → Pallet Town → the farm.

The code is `src/act2_road.c`.

- **Open** (`Act2Road_IsRouteOpen`: `FLAG_ACT2_ROUTE_OPEN`, or `VAR_ACT2_PROGRESS` at `ACT2_FINAL_PROJECT` or later in
  Act 2): the thirsty guards in the Route 7 and Route 6 gates (`Route7_EastEntrance`, `Route6_NorthEntrance`) say
  "Oh, you've got a TRAINER CARD. Go right on through!" instead of turning the player back (once per visit,
  `VAR_TEMP_2`; their TEA scene and `VAR_MAP_SCENE_ROUTE5_ROUTE6_ROUTE7_ROUTE8_GATES` are untouched). Route 2's Cut
  trees at (18,26), (16,62) and (15,69) are gone (`Route2_OnTransition` sets their `FLAG_TEMP_*`). Nothing else on
  the way needs an HM: the ledges on Routes 6, 7, 1 and 2 face south, toward home.
- **Levels** (`Act2Road_IsActive`: `ACT2_FINAL_PROJECT` or later while in Act 2), against the party's average level,
  between 1 and 100:
  - On the route home: trainers' Pokémon are the average (`ACT2_ROUTE_TRAINER_OFFSET`); wild Pokémon are anywhere from 3
    below (`ACT2_ROUTE_WILD_OFFSET`) up to the average.
  - Everywhere else: both are 15 above (`ACT2_OFF_ROUTE_OFFSET`).
  - The story's trainers keep their parties (`Act2_IsStoryTrainer`, and every trainer from `TRAINER_ACT2_PROF_FIRE`
    on, so ones the finale adds too).
  - A scaled Pokémon evolves the way it would have by that level (`Act2Road_EvolveSpeciesToLevel`): level-up
    evolutions, including gendered and personality ones (Combee, Wurmple), and Kanto's where a region decides; not
    friendship, time, item or trade ones.
  - Trainers' Pokémon are rebuilt in place (`Act2Road_ScaleTrainerParty`, from `DoTrainerBattle`, for both
    opponents): the new species and level, the moves they'd know there, full HP; personality, IVs, EVs, held item,
    ball and the no-Dynamax/no-Tera markers stay. Wild ones the same way after they're made
    (`Act2Road_OnWildMonCreated`; the level from `Act2Road_GetWildLevel` in `ChooseWildMonLevel`).
- **Pokémon from other regions**: 30% of scaled Pokémon (`ACT2_ROAD_OTHER_REGION_CHANCE`), wild and trainers', are
  one from beyond Kanto (National Dex 152 on) with the same primary type (or else its second type) and evolution stage
  (basic, stage 1, stage 2; babies don't count, so Pikachu is basic), then evolved for the level. The pools are
  generated: `python3 tools/hack/gen_other_region_pools.py` writes `src/data/act2_other_region_pools.h` (724
  Pokémon). Left out: legendaries, mythicals, Ultra Beasts, paradox Pokémon, regional forms, Megas and other
  battle forms, any form but a species' first, babies, and species defined through a macro (Unown, Vivillon's line,
  Furfrou, Minior, Alcremie, Mothim).

The route home (`sRouteHomeMaps`; a city's buildings aren't on it):

| Stretch | Maps |
|---|---|
| Celadon | `CELADON_UNIVERSITY`, `CELADON_CITY_UNIVERSITY_GATE`, `CELADON_CITY` |
| To Saffron | `ROUTE7`, `ROUTE7_EAST_ENTRANCE`, `SAFFRON_CITY_CONNECTION`, `SAFFRON_CITY` |
| To Vermilion | `ROUTE6_NORTH_ENTRANCE`, `ROUTE6`, `VERMILION_CITY` |
| Diglett's Cave | `ROUTE11`, `DIGLETTS_CAVE_SOUTH_ENTRANCE`, `DIGLETTS_CAVE_B1F`, `DIGLETTS_CAVE_NORTH_ENTRANCE` |
| To Pallet | `ROUTE2`, `ROUTE2_EAST_BUILDING`, `VIRIDIAN_CITY`, `ROUTE1`, `PALLET_TOWN` |
| Home | `FARMYARD`, `WOODS` |

## Home

Mom and Dad are in the farmhouse. The Woods' edge sends the player to them first. They suggest meditating back in the
Woods, where the university journey started (`ACT2_PARENTS_TALKED`).

In the Woods: "An eerie noise is faintly heard." Every 10 tiles closer to the ranger station (top left), "The noise
is getting louder"; every 10 farther, "quieter" (`VAR_ACT2_WOODS_SOUND` keeps the distance last announced).

At the station's door: "Am I ready to go in?" Yes: inside.

## The abandoned ranger station

Maps `RangerStation_1F` and `_B1F`-`_B4F` (map group `gMapGroup_Dungeons_Frlg`). Layouts:
`tools/hack/compose_ranger_station.py` (re-running overwrites all five, and writes `src/data/act2_station_layout.h`,
the positions the C code uses); custom metatiles: `tools/hack/draw_station_tiles.py` (runs
`draw_mansion_furnace.py` first) and B3F's tileset, `tools/hack/draw_station_grove.py` (run before the composer). Code: `src/act2_station.c`; each floor's events in its `scripts.inc`. No wild
Pokémon on any floor. Floors are joined by stairs (1F-B1F-B2F) and ladders (B2F-B3F-B4F). Every ladder sits on its
floor's own tiles: B2F's and B4F's ladder tops are redrawn on the chamber floor (`draw_station_tiles.py`), B3F's
are drawn for the planks.

**1F** (`MUS_RG_POKE_MANSION`). Saffron's north/south gate, abandoned: no guards, boxes left behind, the north door
walled up, the left aisle widened for a stair down behind the left counter. Entered from the Woods by the story:
`warp MAP_RANGER_STATION_1F, 6, 9`; its door mat (warps 0-2 at (5-7, 9)) leads to `MAP_WOODS` warp 0 (at (5, 10),
below the station's door). The switch panel on the wall at (6, 1): "Pull the switch?" Yes: the bottom of the left
counter (4, 8-9) slides away (`ACT2_FINALE_BASEMENT_OPEN`, redrawn on load). The stair down is warp 3 at (2, 3).

**B1F: the clock** (`MUS_RG_POKE_MANSION`). A Pokémon Mansion basement room. 12 standing torches on a clock face of
radius 6 around a cage of iron bars at (8-12, 8-12) with the stair down inside (warp 1 at (11, 9)); the stair up is
in the bottom right alcove (warp 0 at (17, 17)).

| Piece | Position | Piece | Position | Piece | Position |
|---|---|---|---|---|---|
| 12 | (10, 4) | 4 | (15, 13) | 8 | (5, 13) |
| 1 | (13, 5) | 5 | (13, 15) | 9 | (4, 10) |
| 2 | (15, 7) | 6 | (10, 16) | 10 | (5, 7) |
| 3 | (16, 10) | 7 | (7, 15) | 11 | (7, 5) |
| Moon furnace | (6, 2) | Sun furnace | (14, 2) | | |

Every torch and furnace is a bg_event running `RangerStation_B1F_EventScript_ClockPiece`, which finds the piece the
player faces. Unlit: if a party Pokémon knows any FIRE-type move, "Have X use Y to light it?"; lit: the same with a
WATER-type move, to put it out; without one, a hint. State: `VAR_ACT2_CLOCK` (bit n-1 for n o'clock, 12 moon, 13
sun), set to `ACT2_CLOCK_START` (10 and 2 lit) on the first visit (`ACT2_FINALE_CLOCK_SET`). Five explorers' notes
(clipboards) at (3, 4), (17, 4), (2, 11), (5, 18), (13, 18) give one hint each:

| Note | Hint | Points to |
|---|---|---|
| 1, TOM | "I couldn't see much when I went in. Only the stars lit the trail" | Night (the moon furnace) |
| 2, PIP | "I set out before breakfast. The day hadn't even begun." | Morning, not evening |
| 3, DANA | "the town's six o'clock bell hadn't rung yet" | Before 6 |
| 4, LEX | "The long hand had gone a quarter of the way round" | Quarter past |
| 5, OLD BRAM | "the MILTANK get milked at four. I heard them as I left" | 4 o'clock |

**Solution**: 4:15 a.m. From the start: put out 10 and 2, light 4 (hour), 3 (minute) and the moon furnace
(`ACT2_CLOCK_SOLUTION`, exactly). The camera shakes and the cage's front bars (9-11, 12) drop for good
(`ACT2_FINALE_CLOCK_SOLVED`).

**B2F: the flood** (`MUS_RG_SEVII_CAVE`). A Cerulean Cave chamber flooded with water too deep to wade (and blocked
for Surf: the water squares are impassable). Flat stepping stones two squares apart: walking at one square of water
with a stone (or floor) right behind it jumps it, like a ledge (`Act2Station_CanJumpGap`, this map only; not over or
onto an object). Ladder up (warp 0 at (1, 15)), ladder down at the far end (warp 1 at (19, 1)), the MEDKIT (an item
ball at (19, 5); heals the party, any number of times). The stones form a tree with exactly five dead ends (the
composer checks); at each, a Water Pokémon in the water beyond battles the player on arrival (coord event on the
stone) or when talked to, at the party's average level + 2. Beaten or caught: gone for good.

| Dead end | Stone | Pokémon at | Species | Flag |
|---|---|---|---|---|
| 1 | (9, 3) | (10, 3) | Kingdra | `FLAG_ACT2_DEAD_END_1` |
| 2 | (3, 7) | (3, 8) | Tentacruel | `FLAG_ACT2_DEAD_END_2` |
| 3 | (17, 7) | (17, 6) | Gyarados | `FLAG_ACT2_DEAD_END_3` |
| 4 | (9, 13) | (9, 14) | Cloyster | `FLAG_ACT2_DEAD_END_4` |
| 5 | (15, 13) | (15, 14) | Starmie | `FLAG_ACT2_DEAD_END_5` |

**B3F: the overgrowth** (`MUS_RG_VIRIDIAN_FOREST`). An old basement room (the Mansion's walls) with a wooden plank
floor that trees and tall grass have grown up through: its own tileset, `gTileset_RangerStationGrove`
(`tools/hack/draw_station_grove.py`: plank variants, whole overworld trees (crown to trunk) and tall grass redrawn
on the planks, a wooden ladder up and a hatch cut through the boards for the ladder down, the Mansion's walls). Near the entrance (ladder up, warp 0 at (12, 18)) a stone tablet at
(5-7, 16-18) shows the hidden path at a small scale: a 9x9 grid, the path's squares light and joined, the start red,
the way out yellow. It maps square for square onto the 9x9 field of tall grass at (3-11, 5-13). The path (33
squares) starts at the field's bottom left, entered from (3, 14), and leaves at the top towards the ladder down (warp
1 at (9, 3)). A step onto a field square off the path (`Act2Station_TryOvergrowthStep`, against `sOvergrowthPath`,
generated with the tablet from `B3_PATH` in the composer): "GRASS POKéMON ambushed you! You fled back to the
start." and a warp to (3, 14). Reaching the row above the field sets `ACT2_FINALE_GRASS_CROSSED`.

**B4F: the chamber** (`MUS_RG_SEVII_DUNGEON`). Arriving heals the party, and a party of fewer than three is filled up from the PC (the farm box last; `Act2_FillChamberParty`), so the battle is always a 3v3. Mt. Ember's Ruby Path altar room, stretched, in darkness (no
mountainside around it). Ladder up: warp 0 at
(11, 13). Objects: HEATRAN (5, 6), SUICUNE (7, 6), VIRIZION (9, 6), facing down (`LOCALID_CHAMBER_HEATRAN` / `_SUICUNE` /
`_VIRIZION`); hidden for the scene: MEW (7, 9) (`LOCALID_CHAMBER_MEW`, `FLAG_TEMP_2`), DAD (6, 10)
(`LOCALID_CHAMBER_DAD`, `FLAG_TEMP_1`), MOM (8, 10) (`LOCALID_CHAMBER_MOM`, `FLAG_TEMP_3`), all facing up. Coord
events on row y = 12, x 3-10 (`VAR_ACT2_PROGRESS` == 15, `ACT2_PARENTS_TALKED`) run
`RangerStation_B4F_EventScript_Encounter` (a stub, for the story to fill in).

Heatran, Suicune and Virizion (Lv. 50), a 3v3. The player acts once; then all three attack
together (Fire, Water and Grass, 120 power each, one animation). The battle cuts out: Mom and Dad are there, and
their Mew takes the blast. The trio calms. Mom and Dad: that was dangerous. "…" (how did you get Mew?) They befriended
it in these Woods when they were young; it comes to visit them when they need it.

**Resetting the station** (debug): `VAR_ACT2_FINALE` = 0 (switch, clock, cage, grass), `VAR_ACT2_CLOCK` = 0 (set to
the start again on the next B1F visit), clear `FLAG_ACT2_DEAD_END_1`-`5`.

## The ending

The player is back in their room at home. The friends come in: did you finish your exam? The player's epiphany: three
types at once. "Come to the lab with me!" In the lab, the player turns around: **Tri-Blast**, a 120-power TM move of
all three types. The professors come in, marvel, and congratulate the player. "Several years later" on black, then
Act 3: the player in a lab coat, in their lab at Silph Co.

**Tri-Blast:** 120 power, shown as the ??? type (no STAB). Against each target it's as effective as the best of
Fire, Water and Grass would be, and never less than 1x: anything weak to one of the three takes super-effective damage,
and nothing resists it. Any species can learn it from the TM, but only a Pokémon at max friendship
(`CanMonLearnTriBlast`): otherwise "isn't close enough to you yet".

## State

| What | Where |
|---|---|
| Story stage | `VAR_ACT2_PROGRESS` |
| The station's puzzles | `VAR_ACT2_FINALE` (`ACT2_FINALE_*` bits) |
| Torches and furnaces | `VAR_ACT2_CLOCK` (bit per torch 1-12; 13 moon, 14 sun) |
| The Woods' noise | `VAR_ACT2_WOODS_SOUND` |
| Route 2's trees gone, gates open | `FLAG_ACT2_ROUTE_OPEN` |
| B2F's dead-end Pokémon beaten | `FLAG_ACT2_DEAD_END_1`-`5` |
