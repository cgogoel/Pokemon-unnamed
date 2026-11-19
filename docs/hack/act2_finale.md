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
an open brief: make a TM with at least 120 power, the capstone of their studies. Remember the studies, and the passion
that got them into university. It's an unguided exam: each gets a TOWN MAP and ¥10,000 (spend it wisely: healing
items, towns are far apart). Then the DOOM SCROLL: as PROF. FERN taught, some Pokémon only appear at night or by day;
the director once saw a freshman stand mesmerized at the fountain for 12 hours. "Learn the art of the DOOM SCROLL"
(a key item: wait until night or morning, [time_of_day.md](time_of_day.md)). KAI: "So… open social media."
Last, the director sends the player to the researcher on 3F about the TM MACHINE: the TM Database
([tm_database.md](tm_database.md)).

Then the friends come over, each with a TM in their favorite type, matching the night the player battled them:

| Friend | Type | Their TM (120 power) |
|---|---|---|
| KAI | Fire | Flare Blitz |
| MINA | Water | Wave Crash |
| ROWAN | Grass | Solar Beam |

They ask the player's plan; "…" — the player doesn't love one type, they want to discover something new. The friends
echo the director: the player grew up on a farm near Pallet Town, maybe go back there. Last, the road: town by town,
with trainers (they all have Trainer Cards now) and wild Pokémon; south through Vermilion and Diglett's Cave or north
through Cerulean, Mt. Moon and Pewter. KAI is heading home first, and invites the player to stop by his family's
apartment (Celadon Condominiums, 2F, by the university gate) for snacks: there, during the final project, his dads give
5 MOOMOO MILK and 5 POKé WRAPS (a new item, homemade: restores 75 HP, `ITEM_POKE_WRAP`, icon by `tools/hack/draw_poke_wrap.py`) once (`FriendHomes_EventScript_KaiDadsSnacks`, `FLAG_ACT2_KAI_DADS_SNACKS`).

## The road home

Two ways home, on foot and without HMs. Both start the same:

Celadon City → Route 7 → (gate) → Saffron City, then

- **South**: (gate) → Route 6 → Vermilion City → Route 11 (its west end) → Diglett's Cave → Route 2 → Viridian City
- **North**: (gate) → Route 5 → Cerulean City → Route 4 → Mt. Moon (B1F → B2F → B1F → 1F) → Route 4 → Route 3 →
  Pewter City → Route 2 (through Viridian Forest, or down its east side) → Viridian City

and then Route 1 → Pallet Town → the farm. The Underground Path joins them (Route 5 to Route 6, under Saffron).

The code is `src/act2_road.c`.

- **Open** (`Act2Road_IsRouteOpen`: `FLAG_ACT2_ROUTE_OPEN`, or `VAR_ACT2_PROGRESS` at `ACT2_FINAL_PROJECT` or later in
  Act 2):
  - All four Saffron gates (`Route5_SouthEntrance`, `Route6_NorthEntrance`, `Route7_EastEntrance`,
    `Route8_WestEntrance`): every trigger tile, either way, checks first; the guard says "Oh, you've got a TRAINER
    CARD. Go right on through!" instead of the thirsty line (once per visit, `VAR_TEMP_2`; their TEA scene and
    `VAR_MAP_SCENE_ROUTE5_ROUTE6_ROUTE7_ROUTE8_GATES` are untouched).
  - Route 2's Cut trees are gone (`Route2_OnTransition` sets their `FLAG_TEMP_*`): (18,26), (16,62) and (15,69)
    between Diglett's Cave and Viridian, and (11,13) between Pewter and Diglett's Cave.
  - Route 4: its ledges all faced south (toward Cerulean), so the way west from Cerulean to Mt. Moon was shut. A gap
    at the west end of the ledge row under the trees, at (90, 9), like the row's own gap at (41-43, 9). It's a map
    edit: `python3 tools/hack/open_road_home.py` (re-running is harmless).
  - Mt. Moon B2F: as in FireRed, the way between the two halves runs through the fossils' gap at (13-14, 7); the
    player takes one fossil (the SUPER NERD takes the other) to pass.
  - Nothing else on either way needs an HM: the other ledges (Routes 1, 2, 3, 4's west part, 6, 7) face the way home
    or have a way round. Checked with a walk of the maps (ledges, one-way tiles, NPCs and trainers as walls).
- **Levels** (`Act2Road_IsActive`: `ACT2_FINAL_PROJECT` or later while in Act 2), from the area's level (below),
  between 1 and 100:
  - On the route home: trainers' Pokémon are the area's level (`ACT2_ROUTE_TRAINER_OFFSET`); wild Pokémon are
    anywhere from 3 below (`ACT2_ROUTE_WILD_OFFSET`) up to it.
  - The area's final trainer on the route (below) is the party's average level when the battle starts.
  - Everywhere else: both are 15 above the area's level (`ACT2_OFF_ROUTE_OFFSET`).
  - Every base-game trainer is scaled, however the battle starts (`Act2Road_IsScaledTrainer`; a trainer spotting the
    player, a script's `trainerbattle_no_intro`, doubles, rematches: they all make the party
    in `DoTrainerBattle`). The story's trainers keep their parties (`Act2_IsStoryTrainer`, and every trainer from
    `TRAINER_ACT2_PROF_FIRE` on, so ones the finale adds too). The club's battles still follow the battling team
    (`Act2_MakeClubParty`).
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
  generated: `python3 tools/hack/gen_other_region_pools.py` writes `src/data/act2_other_region_pools.h` (707
  Pokémon). Left out: legendaries, mythicals, Ultra Beasts, paradox Pokémon, regional forms, Megas and other
  battle forms, any form but a species' first, babies, species defined through a macro (Unown, Vivillon's line,
  Furfrou, Minior, Alcremie, Mothim), and the lines the story gives in one place only - the glade's Morelull and
  Act 1's five (`EXCLUDE_LINES`; docs/hack/befriending.md).

The route home (`sRouteHomeMaps`; a city's buildings aren't on it):

| Stretch | Maps |
|---|---|
| Celadon | `CELADON_UNIVERSITY`, `CELADON_CITY_UNIVERSITY_GATE`, `CELADON_CITY` |
| To Saffron | `ROUTE7`, `ROUTE7_EAST_ENTRANCE`, `SAFFRON_CITY_CONNECTION`, `SAFFRON_CITY` |
| South: to Vermilion | `ROUTE6_NORTH_ENTRANCE`, `ROUTE6`, `VERMILION_CITY` |
| South: Diglett's Cave | `ROUTE11`, `DIGLETTS_CAVE_SOUTH_ENTRANCE`, `DIGLETTS_CAVE_B1F`, `DIGLETTS_CAVE_NORTH_ENTRANCE` |
| North: to Cerulean | `ROUTE5_SOUTH_ENTRANCE`, `ROUTE5`, `CERULEAN_CITY` |
| North: Mt. Moon | `ROUTE4`, `MT_MOON_1F`, `MT_MOON_B1F`, `MT_MOON_B2F` |
| North: to Pewter | `ROUTE3`, `PEWTER_CITY` |
| North: Viridian Forest | `ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE`, `VIRIDIAN_FOREST`, `ROUTE2_VIRIDIAN_FOREST_SOUTH_ENTRANCE` |
| Under Saffron | `UNDERGROUND_PATH_NORTH_ENTRANCE`, `UNDERGROUND_PATH_NORTH_SOUTH_TUNNEL`, `UNDERGROUND_PATH_SOUTH_ENTRANCE` |
| To Pallet | `ROUTE2`, `ROUTE2_EAST_BUILDING`, `VIRIDIAN_CITY`, `ROUTE1`, `PALLET_TOWN` |
| Home | `FARMYARD`, `WOODS` |

### Area levels

All of Act 2's levels (the courses' wild Pokémon, `Act2_GetWildLevel`, and the final project's wild and trainers'
Pokémon) start from the **area's level** (`Act2Road_GetAreaLevel`): the party's average level when the player first
came to the area. Coming back later, or leveling up, doesn't change it.

- An **area** is a region map section (`MAPSEC_*`): a route with its gates and houses, a city with its buildings, a
  cave's floors (Mt. Moon's three, Diglett's Cave's), Celadon with the university, Pallet with the farm and the Woods.
  On and off the route share it: a Pokémon Center on Route 4 is 15 above Route 4's level.
- **Set** on arrival (`Act2Road_OnMapEntered`, from `LoadMapFromWarp` and `LoadMapFromCameraTransition`), or at the
  first battle if that comes first (the final project starts inside the institute).
- **Periods**: each course (field trip and its night: `ACT2_START`-`ACT2_FIRE_INVITED`,
  `ACT2_CLUB_1_DONE`-`ACT2_WATER_INVITED`, `ACT2_CLUB_2_DONE`-`ACT2_GOT_TRAINER_CARD`) and the final project
  (`ACT2_FINAL_PROJECT` on) set their areas afresh: Vermilion on the Water trip and Vermilion on the road home are
  set separately.
- **Stored** in `SaveBlock3` (`include/global.h`): `act2AreaLevels`, a byte per Kanto map section
  (`KANTO_MAPSEC_COUNT`, indexed from `KANTO_MAPSEC_START`; 0 = not set yet), and `act2AreaLevelsPeriod`, the period
  they belong to (a new period clears them). Map sections outside Kanto aren't stored: there, levels follow the
  party.
- **Final trainers** (`sAreaFinalTrainers`): in each area of the route home with trainers on the way, the one nearest
  the area's exit toward home is the party's average level when the battle starts, not the area's:

  | Area | Trainer | Where |
  |---|---|---|
  | Route 6 | `TRAINER_CAMPER_JEFF` | (13, 33), by the exit to Vermilion |
  | Route 11 | `TRAINER_YOUNGSTER_EDDIE` | (19, 8), nearest Diglett's Cave |
  | Route 4 | `TRAINER_LASS_CRISSY` | (75, 3), Route 4's only trainer |
  | Mt. Moon | `TRAINER_LASS_IRIS` | 1F (20, 26), nearest the exit to Route 4 |
  | Route 3 | `TRAINER_BUG_CATCHER_COLTON` | (12, 6), nearest Pewter |
  | Viridian Forest | `TRAINER_BUG_CATCHER_RICK` | (47, 45), nearest the exit to Route 2 |

  The other areas have no trainers on the way.
  Rematches use other trainer IDs and get the area's level.

## Home

Mom and Dad are in the farmhouse. The Woods' edge sends the player to them first. They suggest meditating back in the
Woods, where the university journey started (`ACT2_PARENTS_TALKED`).

In the Woods: "An eerie noise is faintly heard." Every 20 tiles closer to the ranger station (top left), "The noise
is getting louder", and the first step within 12 tiles says it's coming from the station; every 10 farther, "quieter" (`VAR_ACT2_WOODS_SOUND` keeps the distance last announced).

At the station's door: "Am I ready to go in?" Yes: inside.

## The abandoned ranger station

Maps `RangerStation_1F` and `_B1F`-`_B4F` (map group `gMapGroup_Dungeons_Frlg`). Layouts:
`tools/hack/compose_ranger_station.py` (re-running overwrites all five, and writes `src/data/act2_station_layout.h`,
the positions the C code uses); custom metatiles: `tools/hack/draw_station_tiles.py` (runs
`draw_mansion_furnace.py` first) and B3F's tileset, `tools/hack/draw_station_grove.py` (run before the composer). Code: `src/act2_station.c`; each floor's events in its `scripts.inc`. No wild
Pokémon on any floor but B1F's rooms and corridor (below). Floors are joined by stairs (1F-B1F-B2F) and ladders (B2F-B3F-B4F). Every ladder sits on its
floor's own tiles: B2F's and B4F's ladder tops are redrawn on the chamber floor (`draw_station_tiles.py`), B3F's
are drawn for the planks.

**1F** (`MUS_RG_POKE_MANSION`). Saffron's north/south gate, abandoned: no guards, boxes left behind, the north door
walled up, the left aisle widened for a stair down behind the left counter. Entered from the Woods by the story:
`warp MAP_RANGER_STATION_1F, 6, 9`; its door mat (warps 0-2 at (5-7, 9)) leads to `MAP_WOODS` warp 0 (at (5, 10),
below the station's door). The wall switch at (6, 1), a breaker box drawn into the wall above the dado rail (`draw_station_tiles.py`; its
lever is drawn pulled once the basement is open): "Pull the switch?" Yes: the bottom of the left
counter (4, 8-9) slides away (`ACT2_FINALE_BASEMENT_OPEN`, redrawn on load). The stair down is warp 3 at (2, 3).

**B1F: the clock** (`MUS_RG_POKE_MANSION`), 33x54. A Pokémon Mansion basement floor: the clock room (21x23, its
top left corner at (6, 15)) stands in the middle, a box of walls with its door at the top, between the furnaces
(15-17, 15). Round it, a corridor and rooms: the office (top left), a hall with two statues (top middle), the lab (top
right), the greenhouse (bottom left), the study (bottom middle) and the stair hall (bottom right), where the stair up
is (warp 0 at (29, 48)). Walls are the Mansion's pieces (`Walls` in the composer). 12 standing torches on a clock face
of radius 6 round a stone mosaic (5x5, (14-18, 23-27), `draw_station_mosaic.py`, Mansion palette 10): a raised stone
rim, a terracotta band with gold stones at the hours, night (a crescent moon, stars) on the left and day (a sun) on
the right, parted by a gold line from 12 to 6. Its middle (16, 25) is a stone slab with a carved ring; set the clock
and it slides away from a shaft with a ladder down (warp 1, `MB_LADDER`).

**The lone ranger.** B1F was one ranger's post, alone, a guard post to nowhere: nobody told them what they guarded.
They lived down here (`B1_FURNITURE` in the composer: FireRed's building furniture on the basement floor): a shelf,
a TV, a cabinet and crates in the office; their bed between the statues; a kitchen nook (sink, stove, cupboard) in
the corridor under the office; crates in the lab and the stair hall; a bookshelf in the study. Everything's under
cobwebs (`B1_WEBS`: baked into copies of the furniture and wall corners they hang on, in each tile's own palette).
The hum under the floor drove them mad trying to read the torches and furnaces. They never left: their remains
(`OBJ_EVENT_GFX_RANGER_REMAINS`, 16x32) sit slumped against the study's back wall at (19, 44), the last note in
their hands.

**The lore of 4:15.** The station's founders sealed three Pokémon below (the chamber's trio, B4F) at a quarter past
four one morning, the night the ground split, and built the clock to that hour. The trio stirs at that hour every
night: the hum. The ranger pieces it together over five notes; the fourth finds the founders' log, the fifth (in
their hands) says why the clock is set to four.

Wild Pokémon in the rooms and corridor (`sAct2_RangerStationB1F_FireRed`: Rattata, Zubat, Gastly, Grimer, Koffing,
Voltorb, Magnemite...; Act 2's levels), none in the clock room (`Act2Station_IsEncounterFree`, the room's square
`STATION_CLOCK_*`). Two Pokémon hide in the doorways of the lab (Magneton) and the study (Haunter): stepping into the
doorway ((26, 10), (16, 42); coord events) makes it jump out from the room's side ((26, 9), (16, 43)) for a battle 3
levels above the station's level. Beaten or caught, it's gone for good (`ACT2_FINALE_LAB_GUARD`, `_STUDY_GUARD`); run
from, it stays out, blocking the way, and talking to it battles again (their objects' flags `FLAG_TEMP_1`/`2`, set
on transition).

**Levels.** The station is an area of its own (`Act2Road_GetAreaLevel`; its floors keep Pallet's map section for the
region map, so `act2_road.c` keeps its level in the unused `MAPSEC_SEVII_ISLE_22` slot): the party's average level
when the player first goes in. B1F's wild Pokémon, the doorway Pokémon (+3) and B2F's dead ends (+2) start from it.

| Piece | Position | Piece | Position | Piece | Position |
|---|---|---|---|---|---|
| 12 | (16, 19) | 4 | (21, 28) | 8 | (11, 28) |
| 1 | (19, 20) | 5 | (19, 30) | 9 | (10, 25) |
| 2 | (21, 22) | 6 | (16, 31) | 10 | (11, 22) |
| 3 | (22, 25) | 7 | (13, 30) | 11 | (13, 20) |
| Moon furnace | (12, 17) | Sun furnace | (20, 17) | | |

Every torch and furnace is a bg_event running `RangerStation_B1F_EventScript_ClockPiece`, which finds the piece the
player faces. Unlit: if a party Pokémon knows any FIRE-type move, "Have X use Y to light it?"; lit: the same with a
WATER-type move, to put it out; without one, a hint. State: `VAR_ACT2_CLOCK` (bit n-1 for n o'clock, 12 moon, 13
sun), set to `ACT2_CLOCK_START` (10 and 2 lit) on the first visit (`ACT2_FINALE_CLOCK_SET`). The ranger's log, five
notes (pages: `OBJ_EVENT_GFX_STATION_NOTE`, `draw_station_note.py`; the last in the remains' hands), one from each
stage of their stay, gives one hint each. Reading one counts it (`ACT2_FINALE_NOTE_1`-`5`) and shows "Notes found:
N/5"; the terminal on the clock room's top wall ((9, 17), read from below) shows every note found so far. Text for
editing: `docs/hack/station_notes.md`.

| Note | Where | Stage | Hint | Points to |
|---|---|---|---|---|
| 1, DAY 3 | Office, on the desk (6, 6) | Posted alone, puzzled | "Only when it's dark, when the stars are out" | Night (the moon furnace) |
| 2, DAY 40 | Greenhouse, among the plants (3, 46) | Lonely: talking to the plants | "It worms into my conscious before I wake" | Morning, not evening |
| 3, DAY 112 | Lab, at the machines (27, 3); guarded | Obsessed: torches, furnaces | "It starts before the SPEAROW wake" | Before dawn (before 6) |
| 4, DAY ??? | Statue hall, by the bed (16, 4) | Unravelling; finds the founders' log | "a clock's long hand lay flat, not angled" | Quarter past or to |
| 5, HOLT | The study, in the remains' hands (19, 44); guarded | The end | "hands reaching to the east"; sleep cycles of 2 hours from 10, "3 or 4 of them" | :15, and 4 (6 is after dawn) |

**Solution**: 4:15 a.m. From the start: put out 10 and 2, light 4 (hour), 3 (minute) and the moon furnace
(`ACT2_CLOCK_SOLUTION`, exactly). The camera shakes, and the mosaic's slab grinds aside, halfway, then all the way
(`Act2Station_SlideMosaic`; `METATILE_RangerStation_MosaicHalf`, `_MosaicOpen`), for good (`ACT2_FINALE_CLOCK_SOLVED`).
Step on the ladder to go down.

**B2F: the flood** (`MUS_RG_SEVII_CAVE`). A Cerulean Cave chamber flooded with water too deep to wade (and blocked
for Surf: the water squares are impassable). Flat stepping stones two squares apart: walking at one square of water
with a stone (or floor) right behind it jumps it, like a ledge (`Act2Station_CanJumpGap`, this map only; not over or
onto an object; a follower jumps the same gaps behind the player, `Act2Station_IsFollowerGap`). The floor is dark
but for a small circle round the player (flash level 5, radius 40 px: the stones one jump away and no further,
`Act2Station_GetFloodFlashLevel`), so the way has to be found stone by stone. Ladder up (warp 0 at (1, 15)), ladder
down at the far end (warp 1 at (19, 1)), the MEDKIT (a red first-aid case, `OBJ_EVENT_GFX_MEDKIT`,
`tools/hack/draw_medkit.py`, at (19, 5); heals the party, any number of times). The stones form a tree with exactly
five dead ends (the composer checks); at each, a Water Pokémon hidden under the water beyond (`FLAG_TEMP_1`-`5`)
leaps out and battles the player on arrival (coord event on the stone), at the station's level + 2. Beaten or
caught: gone for good.

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
`_VIRIZION`); hidden for the scene: MEW (7, 7) (`LOCALID_CHAMBER_MEW`, `FLAG_TEMP_2`), DAD (6, 8)
(`LOCALID_CHAMBER_DAD`, `FLAG_TEMP_1`), MOM (8, 8) (`LOCALID_CHAMBER_MOM`, `FLAG_TEMP_3`). Coord events on row
y = 10, x 3-11 (`VAR_ACT2_PROGRESS` == 15, `ACT2_PARENTS_TALKED`; close enough that the trio is on screen) run
`RangerStation_B4F_EventScript_Encounter` (`Act2_EventScript_ChamberScene`): the player lines up at x 7 and steps
up to (7, 9), just below where Mew and the parents appear after the battle.

Heatran, Suicune and Virizion (Lv. 50), a 3v3. The player acts once; then all three attack
together (Fire, Water and Grass, 120 power each, one animation). The battle cuts out: Mom and Dad are there, and
their Mew takes the blast. The trio calms. Mom and Dad: that was dangerous. "…" (how did you get Mew?) They befriended
it in these Woods when they were young; it comes to visit them when they need it.

**Resetting the station** (debug): `VAR_ACT2_FINALE` = 0 (switch, clock, mosaic, grass, guards, notes), `VAR_ACT2_CLOCK` = 0 (set to
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

## Testing it

The overworld debug menu (hold R, press START) jumps straight into each part of the finale
(`data/scripts/debug.inc`, listed in `src/debug.c`):

| Entry | Where it puts you |
|---|---|
| Skip to final project | The director's brief at the Science Institute (`ACT2_GOT_TRAINER_CARD`) |
| Skip to Pallet Town | `ACT2_FINAL_PROJECT` with the road home open and its earlier stops behind you, two steps east of the farmyard |
| Skip to ranger station | The Woods with the station open (`ACT2_PARENTS_TALKED`) |
| Skip to Act 3 | Runs `Act2_EndAct2`, so the act's ending plays |

Each one closes Act 1 as well (`ACT1_DONE`), since Act 1's last stage would otherwise keep its final scene armed in
the farmyard. The menu is in every build but `make release` (`DEBUG_OVERWORLD_MENU`, `include/config/debug.h`), the
Act 2 playtest ROM included.

## State

| What | Where |
|---|---|
| Story stage | `VAR_ACT2_PROGRESS` |
| The station's puzzles | `VAR_ACT2_FINALE` (`ACT2_FINALE_*` bits) |
| Torches and furnaces | `VAR_ACT2_CLOCK` (bit per torch 1-12; 13 moon, 14 sun) |
| The Woods' noise | `VAR_ACT2_WOODS_SOUND` |
| Route 2's trees gone, gates open | `FLAG_ACT2_ROUTE_OPEN` |
| Each area's level, and its period | `gSaveBlock3Ptr->act2AreaLevels`, `act2AreaLevelsPeriod` |
| B2F's dead-end Pokémon beaten | `FLAG_ACT2_DEAD_END_1`-`5` |
