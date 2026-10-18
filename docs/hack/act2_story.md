# Act 2 story

Celadon University (the maps: [celadon_university.md](celadon_university.md)). Three courses, Fire, Water and Grass,
each a field trip with a research sheet to fill (see [The courses](#the-courses)), a TM made from the research, and
the professor's exam. Between courses,
the player's friends take them to a secret battle club at night. The third night is the exam for a real Trainer
Card; then comes the final project.

Progress is `VAR_ACT2_PROGRESS`, with stages `ACT2_*` in `include/constants/story.h`. The scripts are in
`data/scripts/act2_story.inc`, the C side in `src/act2_story.c`.

| Stage | Reached by | Then |
|---|---|---|
| `ACT2_START` | The end of the prologue: the player wakes in their dorm room | Stepping outside, KAI, MINA and ROWAN introduce themselves: they're in the Fire course too. |
| `ACT2_MET_FRIENDS` | Meeting them | PROF. ASHFORD (Lecture Hall A) introduces the course and the field trip leaves. On arrival, he explains the assignment, the Remote Desktop, and gives 5 Poké Balls (`Act2_EventScript_FireBriefing`, `ACT2_RESEARCH_BRIEFED`). |
| `ACT2_FIRE_TRIP` | The field trip: warped to Cinnabar Island (8,5) | The research sheet (the Pokémon Mansion), the TM (Flamethrower), the exam. |
| `ACT2_FIRE_PASSED` | Passing: warped to the dorm lobby | The friends: the secret club, the stadium, tonight. |
| `ACT2_FIRE_INVITED` | | Walking into the stadium: it's night; KAI battles the player. Then morning, in the dorm room. |
| `ACT2_CLUB_1_DONE` | | PROF. MARLOWE (the audiovisual classroom) introduces the Water course; on arrival he explains the assignment and gives 5 Poké Balls and the Good Rod. |
| `ACT2_WATER_TRIP` | Warped to Vermilion City (32,19) | The research sheet (fishing at the harbor, or Route 6's grass; the docks), the TM (BubbleBeam), the exam. |
| `ACT2_WATER_PASSED` | Warped to the dorm lobby | The club meets again tonight. |
| `ACT2_WATER_INVITED` | | Night: MINA battles the player. |
| `ACT2_CLUB_2_DONE` | | PROF. FERN (Home Economics) introduces the Grass course; on arrival she explains the assignment and gives 5 Poké Balls. |
| `ACT2_GRASS_TRIP` | Warped to Viridian City (4,17) | The research sheet (Route 22, west of the city), the TM (Giga Drain), the exam. |
| `ACT2_GRASS_PASSED` | Warped to the dorm lobby | One more night: battle for a real Trainer Card. |
| `ACT2_GRASS_INVITED` | | Night: the friends bring DALE, a Pokémon League examiner. Beating him: the Trainer Card. |
| `ACT2_GOT_TRAINER_CARD` | Winning | **Trainers battle the player again.** The director of the Science Research Institute has the final project. |
| `ACT2_FINAL_PROJECT` | Talking to the director (10 Poké Balls) | Catch a Pokémon that's two of Fire, Water and Grass. |
| `ACT2_PROJECT_DONE` | Showing the director one | "End of Act 2." Act 3 isn't built yet. |

## The courses

Each course opens on campus: the professor introduces it and the class leaves on the field trip. On arrival, the
professor explains the assignment and gives 5 Poké Balls (a frame script, once per course: `ACT2_RESEARCH_BRIEFED`).
After that, talking to the professor shows the **research sheet** (`Act2_BufferResearchSheet`):

1. Catch a Pokémon of the course's type (one in the party).
2. Use a move of that type in battle.
3. Record 3 different moves of that type in battle: the player's Pokémon's or wild ones'
   (`Act2_RecordMoveUse`, from `Cmd_printattackstring`; `VAR_ACT2_MOVES_SEEN`).
4. Solve the site's puzzle (below).

With all four done, the professor makes the course's **TM** with the player. The **exam** needs a Pokémon of the
course's type that knows the TM's move (`Act2_PartyCanUseCourseTM`), and only counts if it uses that move in the
battle (`ACT2_RESEARCH_USED_TM_MOVE`); a win without it is retaken. Progress is `VAR_ACT2_RESEARCH`
(`ACT2_RESEARCH_*`), reset when a course starts.

| Course | TM | Site | Puzzle (each part: a Pokémon of the type that knows a move of it, `Act2_FindCourseMoveUser`) | Its end |
|---|---|---|---|---|
| Fire | TM35 Flamethrower | Pokémon Mansion | Relight 3 cold furnaces on 1F (walls at (6,2), (31,2), (0,28)). Until then, the statue switch is cold. | The switch opens the sealed room; the old research journal there (23,13) |
| Water | TM51 BubbleBeam (new: `FOREACH_TM` in `include/constants/tms_hms.h`) | Vermilion City | Put out 3 fires on the docks ((26,28), (33,30), (37,24); `OBJ_EVENT_GFX_DOCK_FIRE`, a new placeholder sprite) | A young Lapras surfaces by the dock (28,31): talking to it. Then it asks to come along (see below) |
| Grass | TM19 Giga Drain | Route 22 | Regrow 3 withered patches ((24,13), (38,6), (44,10)); they grow into young trees | The field blooms |

**The young Lapras.** After it's recorded, the Lapras nuzzles the player and asks to come along; saying no just
makes it ask again. It joins at level 15 (`ACT2_LAPRAS_LEVEL`): into the party, or straight to the PC if the party
is full. `FLAG_ACT2_GOT_LAPRAS` is set. It doesn't count as catching a Water Pokémon for the sheet (it's a gift),
but it can take the BubbleBeam TM and the exam. **In Act 3, this is the Lapras in Silph Co.'s lab** that masters the
ultimate move and goes to Red (in FireRed, the Silph worker on 7F gives the player a Lapras "kept in our lab").

The site objects follow `Act2_SetupSiteObjects` (each site map's transition): `FLAG_TEMP_1` the professor and
friends, `FLAG_TEMP_2-4` the unsolved parts, `FLAG_TEMP_5-7` what replaces them, `FLAG_TEMP_8` the reward.

## The club's teams

Each friend brings their partner and what they caught on the field trips so far, picking species that aren't in
the player's party when the battle starts (`Act2_MakeClubParty`, from `DoTrainerBattle`; the `.party` entries are
placeholders of the same size).

| Night | Who | Partner | Fire course catch | Water course catch |
|---|---|---|---|---|
| 1 | KAI | Mankey 14 | Growlithe, Ponyta, Vulpix or Slugma, 15 | |
| 2 | MINA | Clefairy 17 | Vulpix, Ponyta, Growlithe or Slugma, 17 | Staryu, Psyduck, Poliwag, Horsea or Krabby, 18 |
| 3 | DALE (examiner) | Pidgeotto 23, Raticate 23, Kadabra 24 | | |

## The Act 2 playtest build

`make act2` builds `pokemon-unnamed-act2.gba` (objects in `build/firered-act2`, `-DACT2_PLAYTEST`). A new game there
starts at Act 2's beginning, in the dorm room, with Teddiursa and Tynamo at level 10, 5 Potions and the National
Pokédex (`Act2_StartPlaytest`). Oak's speech still plays first.

## The rules of Act 2

- **No Trainer Card, no trainer battles** (until `ACT2_GOT_TRAINER_CARD`, `Act2_TrainersLocked`): trainers don't
  notice the player (`CheckForTrainersWantingBattle`, `src/trainer_see.c`), and talking to one gets "Let me see your
  TRAINER CARD… Sorry, I only battle TRAINERS" (`ConfigureTrainerBattle`, `src/battle_setup.c`). Gym Leaders too.
  The story's own battles are the exception (`Act2_IsStoryTrainer`: the professors, KAI, MINA, DALE). Only wild
  Pokémon battle the player.
- **Wild levels follow the player**: every wild Pokémon in Act 2 is the party's average level, give or take 3
  (`ACT2_WILD_LEVEL_SPREAD`, `Act2_GetWildLevel`, from `ChooseWildMonLevel`). The tables below still pick the species;
  their levels only matter outside Act 2.
- **Act 2's own wild Pokémon**: some areas have an Act 2 table, used for all of Act 2 (`Act2_UsesAct2WildPokemon`).
  They're the `sAct2_*_FireRed` entries in `src/data/wild_encounters.json` (the name needs `FireRed`: a label
  without FireRed or LeafGreen is built for Emerald only). `GetCurrentMapWildMonHeaderId` tells them apart by their
  contents (`sAct2WildMonsInfos` in `src/wild_encounter.c`; add new ones there), since Porymap reorders the file.
  Kanto's tables stay as they were for Act 3.

  | Area | Pokémon | Levels |
  |---|---|---|
  | Pokémon Mansion, all floors | Growlithe (Flame Wheel from 12), Ponyta (Flame Charge at 15), Vulpix, Slugma, Magby and Charmander (rare); Rattata, Grimer, Koffing | 10-15 |
  | Vermilion City (fishing) | Magikarp, Goldeen, Poliwag, Horsea, Krabby, Staryu, Shellder, Psyduck | 13-17 |
  | Route 6 | Psyduck, Poliwag, Seel, Marill, Squirtle and Totodile (rare); Pidgey, Oddish, Abra | 13-16 |
  | Route 22 | Oddish, Bellsprout, Hoppip, Paras, Exeggcute, Bulbasaur and Chikorita (rare); Rattata, Spearow, Mankey | 16-19 |
  | Celadon City (fishing) | Lotad, Lombre (Water/Grass), Goldeen, Magikarp | 20-24 |
  | Route 7 | Capsakid (Grass; a Fire Stone from the Department Store makes Scovillain, Grass/Fire), Growlithe, Vulpix, Ponyta, Oddish, Meowth, Pidgey | 20-24 |

- **The start menu**: Act 2 adds POKéMON (`FLAG_SYS_POKEMON_GET`, which Act 1 never sets). Until the Trainer Card,
  the player's entry says "This is where a TRAINER CARD would go." (`Act2_HasTrainerCard`,
  `StartMenuPlayerNameCallback`). Both come with `Act2_GiveStartingKit`.
- **The Remote Desktop**: a key item that opens the Pokémon Storage System anywhere (the expansion's Pokémon Box
  Link, renamed: `ITEM_REMOTE_DESKTOP`). Given when Act 2 starts, kept in Act 3 (`Act2_GiveStartingKit`; saves
  already in Act 2 get it on campus).
- **The time skip**: at the end of the prologue, the two Pokémon the player brought go up to level 10
  (`ACT2_START_LEVEL`) with the moves they'd know there (`Act2_RaisePartyLevels`). Blacking out returns the player
  to Celadon's Pokémon Center; on a field trip, to that city's.
- **Night**: the club scenes set the time of day to night (`special Act2_SetNight`) until the next warp.

## The battles

| Who | Where | Team |
|---|---|---|
| PROF. ASHFORD (Fire) | Cinnabar Island | Oddish 13, Paras 13, Bellsprout 14: weak to Fire |
| KAI (night 1) | Stadium | See the club's teams |
| PROF. MARLOWE (Water) | Vermilion City | Sandshrew 15, Geodude 15, Vulpix 16: weak to Water |
| MINA (night 2) | Stadium | See the club's teams |
| PROF. FERN (Grass) | Viridian City | Psyduck 19, Diglett 19, Poliwhirl 20: weak to Grass |
| DALE, League examiner (night 3) | Stadium | Pidgeotto 23, Raticate 23, Kadabra 24 |

## Characters (names are placeholders)

| Who | Where | Sprite |
|---|---|---|
| KAI, MINA, ROWAN | Classmates: outside the dorm room at the start, on each field trip, the dorm lobby after each course, the stadium at night | Camper, Picnicker, Boy |
| PROF. ASHFORD | Lecture Hall A; Cinnabar | Scientist |
| PROF. MARLOWE | Audiovisual classroom; Vermilion, by the harbor | Old man |
| PROF. FERN | Home Economics; Viridian, at the west exit | Beauty |
| DALE | The stadium, night 3 | Cooltrainer |
| The director | Science Research Institute (7,7) | Gentleman |

## Base-game scenes stripped for Act 2

- **Cinnabar Island**: Bill's trip to One Island and the return from the Sevii Islands (Bill and the Seagallop).
- **Vermilion City**: the player walking off the S.S. Anne.
- **Viridian City**: the old man blocking the road and his catching tutorial (he stands by the road).
- **Route 22**: the rival.

## Not done yet

- Teams of up to 3 aren't enforced.
