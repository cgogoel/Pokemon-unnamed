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
  - On the route home: trainers' Pokémon are the average (`ACT2_ROUTE_TRAINER_OFFSET`); wild Pokémon are 3 below
    (`ACT2_ROUTE_WILD_OFFSET`), with no spread.
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

**1F.** A gate building like those between towns and routes, abandoned. A switch on the wall opens a hidden stair
behind one of the counters.

**B1F: the clock.** 12 torches around a gated stair in the center, at the hours of a clock face. Two furnaces: one
with a moon on the wall above it, one with a sun. A Fire move lights a torch or furnace, a Water move puts it out.
Two torches start lit. Five explorers' notes hint at the time:

| Hint | Points to |
|---|---|
| "I couldn't see much when I went in, but…" | Night (the moon furnace) |
| Left before breakfast, the day not begun | Morning, not evening |
| The town's six o'clock bell hadn't rung | Before 6 |
| The long hand a quarter of the way round | Quarter past |
| The Miltank on the farm are milked at four | 4 o'clock |

4:15 a.m.: the torches at 4 (hour) and 3 (minute) lit, the moon furnace lit, everything else out. The gate opens.

**B2F: the flood.** Rocks in water; the player jumps from rock to rock across one square of water. Five dead ends,
each with a powerful Water Pokémon that battles the player. A medkit at the far end heals the party.

**B3F: the overgrowth.** A pattern on the floor before the overgrown room shows the path. In the tall grass, a step
off the path: "Grass Pokémon ambushed you!" and back to the path's start.

**B4F: the chamber.** Heatran, Suicune and Virizion (Lv. 50), a 3v3. The player acts once; then all three attack
together (Fire, Water and Grass, 120 power each, one animation). The battle cuts out: Mom and Dad are there, and
their Mew takes the blast. The trio calms. Mom and Dad: that was dangerous. "…" (how did you get Mew?) They befriended
it in these Woods when they were young; it comes to visit them when they need it.

## The ending

The player is back in their room at home. The friends come in: did you finish your exam? The player's epiphany: three
types at once. "Come to the lab with me!" In the lab, the player turns around: **Tri-Blast**, a 120-power TM move of
all three types. The professors come in, marvel, and congratulate the player. "Several years later" on black, then
Act 3: the player in a lab coat, in their lab at Silph Co.

**Tri-Blast:** 120 power, shown as the ??? type (no STAB). Against each target it's as effective as the best of
Fire, Water and Grass would be, and never less than 1x: anything weak to one of the three takes super-effective damage,
and nothing resists it.

## State

| What | Where |
|---|---|
| Story stage | `VAR_ACT2_PROGRESS` |
| The station's puzzles | `VAR_ACT2_FINALE` (`ACT2_FINALE_*` bits) |
| Torches and furnaces | `VAR_ACT2_CLOCK` (bit per torch 1-12; 13 moon, 14 sun) |
| The Woods' noise | `VAR_ACT2_WOODS_SOUND` |
| Route 2's trees gone, gates open | `FLAG_ACT2_ROUTE_OPEN` |
| B2F's dead-end Pokémon beaten | `FLAG_ACT2_DEAD_END_1`-`5` |
