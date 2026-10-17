# Act 2 story

Celadon University (the maps: [celadon_university.md](celadon_university.md)). Three courses, Fire, Water and Grass,
each a field trip where the player catches a Pokémon of that type and passes the professor's exam. Between courses,
the player's friends take them to a secret battle club at night. The third night is the exam for a real Trainer
Card; then comes the final project.

Progress is `VAR_ACT2_PROGRESS`, with stages `ACT2_*` in `include/constants/story.h`. The scripts are in
`data/scripts/act2_story.inc`, the C side in `src/act2_story.c`.

| Stage | Reached by | Then |
|---|---|---|
| `ACT2_START` | The end of the prologue: the player wakes in their dorm room | Stepping outside, KAI, MINA and ROWAN introduce themselves: they're in the Fire course too. |
| `ACT2_MET_FRIENDS` | Meeting them | PROF. ASHFORD (Lecture Hall A) starts the Fire course: 5 Poké Balls, then the field trip. |
| `ACT2_FIRE_TRIP` | The field trip: warped to Cinnabar Island (8,5) | Fire Pokémon in the Pokémon Mansion. With one in the party, the professor's exam (a battle). |
| `ACT2_FIRE_PASSED` | Passing: warped to the dorm lobby | The friends: the secret club, the stadium, tonight. |
| `ACT2_FIRE_INVITED` | | Walking into the stadium: it's night; KAI battles the player. Then morning, in the dorm room. |
| `ACT2_CLUB_1_DONE` | | PROF. MARLOWE (the audiovisual classroom) starts the Water course: 5 Poké Balls and the Good Rod. |
| `ACT2_WATER_TRIP` | Warped to Cerulean City (23,10) | Water Pokémon across the Nugget Bridge (Route 24): fishing, or the grass on the river bank. Exam. |
| `ACT2_WATER_PASSED` | Warped to the dorm lobby | The club meets again tonight. |
| `ACT2_WATER_INVITED` | | Night: MINA battles the player. |
| `ACT2_CLUB_2_DONE` | | PROF. FERN (Home Economics) starts the Grass course: 5 Poké Balls. |
| `ACT2_GRASS_TRIP` | Warped to Viridian City (4,17) | Grass Pokémon on Route 22, west of the city. Exam. |
| `ACT2_GRASS_PASSED` | Warped to the dorm lobby | One more night: battle for a real Trainer Card. |
| `ACT2_GRASS_INVITED` | | Night: the friends bring DALE, a Pokémon League examiner. Beating him: the Trainer Card. |
| `ACT2_GOT_TRAINER_CARD` | Winning | **Trainers battle the player again.** The director of the Science Research Institute has the final project. |
| `ACT2_FINAL_PROJECT` | Talking to the director (10 Poké Balls) | Catch a Pokémon that's two of Fire, Water and Grass. |
| `ACT2_PROJECT_DONE` | Showing the director one | "End of Act 2." Act 3 isn't built yet. |

## The rules of Act 2

- **No Trainer Card, no trainer battles** (until `ACT2_GOT_TRAINER_CARD`, `Act2_TrainersLocked`): trainers don't
  notice the player (`CheckForTrainersWantingBattle`, `src/trainer_see.c`), and talking to one gets "Let me see your
  TRAINER CARD… Sorry, I only battle TRAINERS" (`ConfigureTrainerBattle`, `src/battle_setup.c`). Gym Leaders too.
  The story's own battles are the exception (`Act2_IsStoryTrainer`: the professors, KAI, MINA, DALE). Only wild
  Pokémon battle the player.
- **Act 2's own wild Pokémon**: some areas have an Act 2 table, used for all of Act 2 (`Act2_UsesAct2WildPokemon`).
  They're the `sAct2_*` entries at the end of `src/data/wild_encounters.json`, after the area's own;
  `GetCurrentMapWildMonHeaderId` picks the later one. An area needs its own table for this to work. Kanto's tables
  stay as they were for Act 3.

  | Area | Pokémon | Levels |
  |---|---|---|
  | Pokémon Mansion, all floors | Growlithe, Vulpix, Ponyta, Slugma, Magby, Charmander (rare); Rattata, Grimer, Koffing | 9-12 |
  | Route 24 | Grass: Psyduck, Poliwag, Wooper, Marill, Squirtle and Totodile (rare). Fishing: Magikarp, Goldeen, Poliwag, Horsea, Krabby, Staryu, Shellder, Psyduck | 13-17 |
  | Route 22 | Oddish, Bellsprout, Hoppip, Paras, Exeggcute, Bulbasaur and Chikorita (rare); Rattata, Spearow, Mankey | 16-19 |
  | Celadon City (fishing) | Lotad, Lombre (Water/Grass), Goldeen, Magikarp | 20-24 |
  | Route 7 | Capsakid (Grass; a Fire Stone from the Department Store makes Scovillain, Grass/Fire), Growlithe, Vulpix, Ponyta, Oddish, Meowth, Pidgey | 20-24 |

- **The time skip**: at the end of the prologue, the two Pokémon the player brought go up to level 10
  (`ACT2_START_LEVEL`) with the moves they'd know there (`Act2_RaisePartyLevels`). Blacking out returns the player
  to Celadon's Pokémon Center; on a field trip, to that city's.
- **Night**: the club scenes set the time of day to night (`special Act2_SetNight`) until the next warp.

## The battles

| Who | Where | Team |
|---|---|---|
| PROF. ASHFORD (Fire) | Cinnabar Island | Oddish 11, Paras 11, Bellsprout 12: weak to Fire |
| KAI (night 1) | Stadium | Growlithe 12, Vulpix 12, Ponyta 13 |
| PROF. MARLOWE (Water) | Cerulean City | Sandshrew 15, Geodude 15, Vulpix 16: weak to Water |
| MINA (night 2) | Stadium | Staryu 17, Psyduck 17, Poliwag 18 |
| PROF. FERN (Grass) | Viridian City | Psyduck 19, Diglett 19, Poliwhirl 20: weak to Grass |
| DALE, League examiner (night 3) | Stadium | Pidgeotto 23, Raticate 23, Kadabra 24 |

## Characters (names are placeholders)

| Who | Where | Sprite |
|---|---|---|
| KAI, MINA, ROWAN | Classmates: outside the dorm room at the start, on each field trip, the dorm lobby after each course, the stadium at night | Camper, Picnicker, Boy |
| PROF. ASHFORD | Lecture Hall A; Cinnabar | Scientist |
| PROF. MARLOWE | Audiovisual classroom; Cerulean, at the Nugget Bridge | Old man |
| PROF. FERN | Home Economics; Viridian, at the west exit | Beauty |
| DALE | The stadium, night 3 | Cooltrainer |
| The director | Science Research Institute (7,7) | Gentleman |

## Base-game scenes stripped for Act 2

- **Cinnabar Island**: Bill's trip to One Island and the return from the Sevii Islands (Bill and the Seagallop).
- **Cerulean City**: the rival on the Nugget Bridge, the Rocket in the burgled house's yard.
- **Route 24**: the Rocket at the end of the Nugget Bridge.
- **Viridian City**: the old man blocking the road and his catching tutorial (he stands by the road).
- **Route 22**: the rival.

## Not done yet

- The start menu still shows the Trainer Card from the start.
- Wild levels outside the course sites aren't tuned for Act 2: Route 7, just east of campus, is level 20-24 all of
  Act 2, and other areas keep Kanto's own levels.
- Teams of up to 3 aren't enforced.
