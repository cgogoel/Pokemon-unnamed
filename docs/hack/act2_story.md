# Act 2 story

Celadon University (the maps: [celadon_university.md](celadon_university.md)). Three courses, Fire, Water and Grass,
each a field trip with a research sheet to fill (see [The courses](#the-courses)), a TM made from the research, and
the professor's exam. Between courses,
the player's friends take them to a secret battle club at night. The third night is the exam for a real Trainer
Card; then comes the final project ([act2_finale.md](act2_finale.md)).

Progress is `VAR_ACT2_PROGRESS`, with stages `ACT2_*` in `include/constants/story.h`. The scripts are in
`data/scripts/act2_story.inc`, the C side in `src/act2_story.c`.

| Stage | Reached by | Then |
|---|---|---|
| `ACT2_START` | The end of the prologue: the player wakes in their dorm room | Stepping outside, KAI, MINA and ROWAN introduce themselves: they're in the Fire course too. |
| `ACT2_MET_FRIENDS` | Meeting them | PROF. ASHFORD (Lecture Hall A) introduces the course and the field trip leaves. On arrival, he explains the assignment, the Remote Desktop, and gives 5 Poké Balls (`Act2_EventScript_FireBriefing`, `ACT2_RESEARCH_BRIEFED`). |
| `ACT2_FIRE_TRIP` | The field trip: warped to Cinnabar Island (8,5) | The research sheet (the Pokémon Mansion), the TM (Flamethrower), the exam. |
| `ACT2_FIRE_SEE_KAI` | Passing (KAI: come see me in my dorm room when we get back; the party is healed): warped to the player's dorm room, at night | KAI's room (Dorms 2F, the first room): the secret club meets at the Sports Complex. He heads there. |
| `ACT2_FIRE_INVITED` | | The friends wait at the Sports Complex (the stadium); walking in, KAI battles the player. Then morning, in the dorm room (healed). |
| `ACT2_CLUB_1_DONE` | | PROF. MARLOWE (the audiovisual classroom) introduces the Water course; on arrival he explains the assignment and gives 5 Poké Balls and the Good Rod. |
| `ACT2_WATER_TRIP` | Warped to Vermilion City (32,19) | The research sheet (fishing at the harbor, or Route 6's grass; the docks), the TM (BubbleBeam), the exam. |
| `ACT2_WATER_SEE_MINA` | Only saves already at it (MINA used to ask the player over) | MINA's room (2F, the second room): club night at the Sports Complex. |
| `ACT2_WATER_INVITED` | Passing (MINA: see you at the meeting spot tonight, you know where it is): the player's room, at night | Night: MINA battles the player at the Sports Complex. |
| `ACT2_CLUB_2_DONE` | | PROF. FERN (Home Economics) introduces the Grass course; on arrival she explains the assignment and gives 5 Poké Balls. |
| `ACT2_GRASS_TRIP` | Warped to Viridian City (4,17) | The research sheet (Route 22, west of the city), the TM (Giga Drain), the exam. |
| `ACT2_GRASS_SEE_ROWAN` | Only saves already at it (ROWAN used to ask the player over) | ROWAN's room (3F, the last room): a League examiner is coming; battle for a real Trainer Card. |
| `ACT2_GRASS_INVITED` | Passing (ROWAN: see you at the meeting spot tonight; someone special is coming): the player's room, at night | The friends and DALE, a Pokémon League examiner, wait at the Sports Complex. Beating him: the Trainer Card. |
| `ACT2_GOT_TRAINER_CARD` | Winning | **Trainers battle the player again.** In the morning, the director's final project brief at the Science Research Institute. |
| `ACT2_FINAL_PROJECT` … `ACT2_DONE` | The brief | The finale: make a TM with 120 power or more. See [act2_finale.md](act2_finale.md). |

## The courses

Each course opens on campus: the professor introduces it and the class leaves on the field trip. On arrival, the
professor explains the assignment and gives 5 Poké Balls (a frame script, once per course: `ACT2_RESEARCH_BRIEFED`); a player out of
Poké Balls gets 5 more each time they talk to the professor (`Act2_EventScript_RefillBalls`).
After that, talking to the professor shows the **research sheet** (`Act2_BufferResearchSheet`):

1. Catch 3 different Pokémon (species) of the course's type: counted in the party and the PC, not the gifted Lapras
   (`Act2_CountCaughtSpeciesOfType`).
2. Use a move of that type in battle.
3. Record 3 different moves of that type in battle: the player's Pokémon's or wild ones'
   (`Act2_RecordMoveUse`, from `Cmd_printattackstring`; the first two in `VAR_ACT2_MOVE_SEEN_1/2`, the third sets
   `ACT2_RESEARCH_MOVES_DONE`). So there are enough different ones without training up first, a wild Pokémon of the
   course's type on the trip also knows its type's moves from up to 8 levels ahead, in place of its other moves
   (`Act2_TeachWildCourseMoves`, from `CreateWildMon`). None of them knows the course's TM move, so it can only be
   made, not caught. In the trip areas, 35% of wild Pokémon are the type's basic Pokémon from beyond Kanto instead
   (`Act2_ChooseWildSpecies`, `sOtherRegionFire/Water/Grass`). Every fishing cast gets a bite (`I_FISHING_ALWAYS_BITE`). Caught, it skips those moves when it reaches their level:
   the level-up code passes over a move a Pokémon already knows (`MON_ALREADY_KNOWS_MOVE`).
4. Solve the site's puzzle (below).

In battle, tasks 1 and 3 show their counter ("Research: 2 of 3 WATER moves recorded!" / "... POKéMON caught!") each
time they go up, until they're done: a new move once the move in progress is over (`MOVEEND_ACT2_RESEARCH`,
`Act2_TakeResearchMoveMessage`), a new species after it's caught (`BS_Act2ResearchCatch` in
`BattleScript_SuccessBallThrowEnd`, `Act2_BufferResearchCatchMessage`).

With all four done, the professor makes the course's **TM** with the player (TMs are reusable: `I_REUSABLE_TMS`).
Then the **exam**, a battle with the professor; any win passes. Progress is `VAR_ACT2_RESEARCH`
(`ACT2_RESEARCH_*`), reset when a course starts.

| Course | TM | Site | Puzzle (each part: a Pokémon of the type that knows a move of it, `Act2_FindCourseMoveUser`) | Its end |
|---|---|---|---|---|
| Fire | TM35 Flamethrower | Pokémon Mansion | Relight 3 cold furnaces on 1F (on the walls at (6,2), (31,2), (4,12); `METATILE_PokemonMansion_Furnace_Cold`, swapped for `_Lit` once relit, drawn by `tools/hack/draw_mansion_furnace.py`; the lit flames flicker, `InitTilesetAnim_PokemonMansion`). Until then, the statue switch is cold. | The switch opens the sealed room; the old research journal there (23,13), in it for the whole trip until read (readable once the furnaces are lit) |
| Water | TM51 BubbleBeam (new: `FOREACH_TM` in `include/constants/tms_hms.h`) | Vermilion City | Put out the 2 small dock fires (2x2 at (30,28) and (35,25)) with any WATER move | The big fire (2x3 at (24,28), across the pier) needs BubbleBeam; behind it a young Lapras surfaces (21,29) and asks to come along (see below) |
| Grass | TM19 Giga Drain | Route 22 | Regrow 3 withered plants ((24,13), (38,6), (44,10); `OBJ_EVENT_GFX_WITHERED_PLANT`, a dried-out shrub); they grow into young trees | The field blooms |

**The dock fires.** The docks catch fire when the player catches a WATER Pokémon on the trip (`Act2_OnCaughtMon`,
from the catch script; `ACT2_RESEARCH_DOCK_FIRE`). Back on Route 6 or in Vermilion after that battle, MINA runs up
(`Act2_EventScript_CheckDockFireAlarm` on map resume, then the map's frame script) and takes the player to the pier
to see it (`ACT2_RESEARCH_DOCK_FIRE_SEEN`, `_ARRIVING`). The fires are metatiles over the planks, drawn on Vermilion's
load while burning (`Act2_DrawDockFires`; art and flicker frames by `tools/hack/draw_dock_fire.py`, animated by `InitTilesetAnim_VermilionCity`), with a sign event on each square.
The puzzle task needs all three fires out. The small ones go out with any WATER move; the big one, which cuts off
the pier's west end, only with the TM move (`Act2_FindTmMoveUser`). So on the Water trip the TM is made from the
other three tasks (`Act2_IsReadyForTM`), and the exam waits for all four (`Act2_IsResearchDone`). PROF. MARLOWE's
lines follow the stage: the usual hint, then (fires started) the TM is urgent, then (TM made, big fire still
burning) go put it out, a POKéMON is cowering there, then the exam. A save that left the trip without the Lapras
finds it waiting by the pier for the rest of Act 2. When a puzzle's task is done, the message says whether the
sheet is finished (`Act2_EventScript_TaskDone`). MINA's alarm has her run in from out of view (up to 9 squares
straight out, over free land) to a land square beside the player (`Act2_FindSpotBesidePlayer`,
`Act2_WalkFriendToPlayer`). The dock workers (the harbor's sailors, and one on the pier during the trip) call
for help while it burns and thank the player after (`Act2_GetDockFireState`).

**The young Lapras.** It floats on the water, below the pier's elevation, so the player talks to it through a sign
event on its square (`Act2_EventScript_DockLaprasFromPier`). Once the big fire is out, the message sends the player to the Lapras (not the professor), and PROF. MARLOWE
holds the exam until it has come along. The Lapras nuzzles the player and asks to come along; saying no just
makes it ask again. It joins at level 15 (`ACT2_LAPRAS_LEVEL`): into the party, or straight to the PC if the party
is full. `FLAG_ACT2_GOT_LAPRAS` is set. It doesn't count as catching a Water Pokémon for the sheet (it's a gift),
but it can take the BubbleBeam TM and the exam. **In Act 3, this is the Lapras in Silph Co.'s lab** that masters the
ultimate move and goes to Red (in FireRed, the Silph worker on 7F gives the player a Lapras "kept in our lab").

The site objects follow `Act2_SetupSiteObjects` (each site map's transition): `FLAG_TEMP_1` the professor and
friends, `FLAG_TEMP_2-4` the unsolved parts, `FLAG_TEMP_5-7` what replaces them, `FLAG_TEMP_8` the reward.

## The club's teams

The club nights are 3v3 battles (`Battle Type: Triples`). Arriving at the Sports Complex (the gate's coord events),
the friends explain the night's battle and leave the player free to line up their team
(`ACT2_RESEARCH_CLUB_ARRIVED`); talking to any of them (or DALE) starts it (`Act2_EventScript_ClubTalk`).

- **Night 1:** the club's own 3v3 rules, explained by the three friends: three a side, one acts per turn; the
  formation (front, middle, back: defense in front, offense in back); direct attacks hit the front Pokémon, indirect
  ones (electric, psychic, flying) anyone; the move menu's red arrows (reach) and colored circles (how many it hits);
  party order sets the formation.
- **The team** is the first three Pokémon in the party able to battle: first in front, second in the middle, third
  in back (the formation follows party order). Talking to a friend shows the lineup and asks if the player is ready
  (`Act2_BufferClubTeam`). For the battle the rest of the party is set aside, and put back in its slots afterwards
  (`Act2_SetAsideClubBench`, `Act2_RestoreClubBench`).
- **Levels:** every foe is at the average level of the player's three, with its level's moves.
- **Before each battle** the opponent says how they fight: KAI breaks down the front line with direct attacks, MINA
  aims indirect attacks at the back line, DALE mixes both.
- **After each battle**, "The next day…" on black (`Act2_ShowNextDayCard`, the act-end cards of
  `src/prologue_end.c`), then the player wakes in their room. The stage moves on only once the screen is black, so
  it stays night through the talk after the battle.
- **Losing** doesn't white out or stop the story (`trainerbattle_earlyrival`, healed after): the friend says nobody
  wins every time and offers to go again. Saying no carries on as after a win; on night 3, DALE passes the player
  anyway.

Each friend brings their partner and what they caught on the field trips so far, picking species that aren't in
the player's party (the three battling) or already on their team (`Act2_MakeClubParty`, from `DoTrainerBattle`; the
`.party` entries are placeholders of the same size).

| Night | Who | Partner | Fire course catches | Water course catch |
|---|---|---|---|---|
| 1 | KAI | Mankey | Growlithe, Ponyta, Vulpix or Slugma (two of them) | |
| 2 | MINA | Clefairy | Vulpix, Ponyta, Growlithe or Slugma | Staryu, Psyduck, Poliwag, Horsea or Krabby |
| 3 | DALE (examiner) | Pidgeotto, Raticate, Kadabra | | |

## The Act 2 playtest build

`make act2` builds `pokemon-unnamed-act2.gba` (objects in `build/firered-act2`, `-DACT2_PLAYTEST`). A new game there
starts at Act 2's beginning, in the dorm room, with Teddiursa and Tynamo at level 10, 5 Potions and the National
Pokédex (`Act2_StartPlaytest`). It skips the title screen and Oak's speech: every boot goes from the copyright
screen straight into a new game, as GRAHAM (`CB2_Act2PlaytestNewGame`, from `src/intro_frlg.c`), so its saves
aren't loaded.

In both builds, the Pokémon the player starts Act 2 with know the moves they'd have learned by level 10, and at least
2: Tackle fills in for one that only has one (Blipbug) (`Act2_GiveStartingMoves`).

## The rules of Act 2

- **No Trainer Card, no trainer battles** (until `ACT2_GOT_TRAINER_CARD`, `Act2_TrainersLocked`): trainers don't
  notice the player (`CheckForTrainersWantingBattle`, `src/trainer_see.c`), and talking to one gets "Let me see your
  TRAINER CARD… Sorry, I only battle TRAINERS" (`ConfigureTrainerBattle`, `src/battle_setup.c`). Gym Leaders too.
  The story's own battles are the exception (`Act2_IsStoryTrainer`: the professors, KAI, MINA, DALE). Only wild
  Pokémon battle the player.
- **Wild levels follow the player**: every wild Pokémon in Act 2 is the area's level, give or take 3
  (`ACT2_WILD_LEVEL_SPREAD`, `Act2_GetWildLevel`, from `ChooseWildMonLevel`). An area's level is the party's average
  level when the player first came to it in this course (`Act2Road_GetAreaLevel`; see
  [act2_finale.md](act2_finale.md#the-road-home), "Area levels"). From the final project, the road home's
  rules take over ([act2_finale.md](act2_finale.md#the-road-home)). The tables below still pick the species;
  their levels only matter outside Act 2.
- **Act 2's own wild Pokémon**: some areas have an Act 2 table, used for all of Act 2 (`Act2_UsesAct2WildPokemon`).
  They're the `sAct2_*_FireRed` entries in `src/data/wild_encounters.json` (the name needs `FireRed`: a label
  without FireRed or LeafGreen is built for Emerald only). `GetCurrentMapWildMonHeaderId` tells them apart by their
  contents (`sAct2WildMonsInfos` in `src/wild_encounter.c`; add new ones there), since Porymap reorders the file.
  Their encounter rates are half the usual ones (5 in the Mansion, 10 in grass, 5 fishing). Kanto's tables stay as
  they were for Act 3.

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
- **The Heal Dash**: a key item (new, `ITEM_HEAL_DASH`, `src/heal_dash.c`) that spins the player away, as with
  Teleport, to the front of the nurse in the last Pokémon Center they went into (the white-out respawn point). On a
  field trip it's always the trip town's Pokémon Center, visited or not (`Act2_GetTripHealLocation`). Given with the
  Remote Desktop; PROF. ASHFORD mentions it in his Cinnabar briefing.
- **The time skip**: at the end of the prologue, the two Pokémon the player brought go up to level 10
  (`ACT2_START_LEVEL`) with the moves they'd know there (`Act2_RaisePartyLevels`). Blacking out returns the player
  to Celadon's Pokémon Center; on a field trip, to that city's.
- **Night**: from coming back to the dorms after a course until that night's club battle, it's night everywhere
  (`Act2_IsClubNight`, `UpdateTimeOfDay`). The Sports Complex's daytime students are gone then.
- **The friends' rooms**: KAI and MINA on Dorms 2F, ROWAN on 3F, with name plates by their doorways. Each is in their
  room on the night they asked the player over, and from the Trainer Card on.

## The battles

| Who | Where | Team |
|---|---|---|
| PROF. ASHFORD (Fire) | Cinnabar Island | Oddish 13, Paras 13, Bellsprout 14: weak to Fire |
| KAI (night 1) | Stadium | See the club's teams |
| PROF. MARLOWE (Water) | Vermilion City | Sandshrew 15, Geodude 15, Vulpix 16: weak to Water |
| MINA (night 2) | Stadium | See the club's teams |
| PROF. FERN (Grass) | Viridian City | Psyduck 19, Diglett 19, Poliwhirl 20: weak to Grass |
| DALE, League examiner (night 3) | Stadium | Pidgeotto 23, Raticate 23, Kadabra 24 |

From Act 2 on, the whole party gains experience from every battle, as with the Exp. Share on
(`IsGen6ExpShareEnabled`). There are also no Gym badges to earn, so the player's Pokémon always obey, whatever their level or OT
(`GetAttackerObedienceForAction`).

## Characters (names are placeholders)

| Who | Where | Sprite |
|---|---|---|
| KAI, MINA, ROWAN | Classmates: outside the dorm room at the start, on each field trip, KAI's room after the Fire course, the stadium at night | Camper, Picnicker, Boy |
| PROF. ASHFORD | Lecture Hall A; Cinnabar | Scientist |
| PROF. MARLOWE | Audiovisual classroom; Vermilion, by the harbor | Old man |
| PROF. FERN | Home Economics; Viridian, at the west exit | Beauty |
| DALE | The stadium, night 3 | Cooltrainer |
| The director | Science Research Institute (7,7) | Gentleman |

## The friends' families

Each friend grew up in the town of the type they love. Their parents are always home, in every act, and what they say
follows the story (`FriendHomes_EventScript_GetStage` in `data/scripts/friend_homes.inc`, where all their lines are):
Act 1 (the friends are kids), Act 2's courses (their kid is at Celadon University), the final project (from the
Trainer Card: proud and a little worried, and they know their kid's TM), and Act 3 (several years later: what their
kid does now). Each also knows the player as their kid's friend.

| Friend | Home | Parents (sprite) | In Act 3 | Also there |
|---|---|---|---|---|
| KAI (Fire) | Cinnabar Island, a house between the Mansion and the Gym, door (15,2) (`CinnabarIsland_KaisHouse`, new) | Two dads: the loud one, by the stove (Fat man), and the quiet one, at the table (Man) | Training Fire Pokémon full time; means to take on the Gym one day | A photo of KAI as a kid with his dads and a Vulpix |
| MINA (Water) | Cerulean City, the small house that was the Wonder News man's (`CeruleanCity_House4_Frlg`) | Her mom, by the stove (Woman 3); it's been just the two of them | At a marine research station out at sea; sends postcards | A photo of MINA as a kid with a Staryu |
| ROWAN (Grass) | Celadon City, the house by the Game Corner (`CeladonCity_House1_Frlg`, was the Game Corner owner's) | Mom, tending a plant (Woman 1), and Dad, at the table (Balding man) | Runs a greenhouse at a botanical lab, naming new plants | A photo of ROWAN as a kid; a plant labeled by ROWAN |

- On the Fire trip, KAI's class is on his island: his dads have their own lines then, and KAI says his dads' house is
  by the Gym (`Act2_EventScript_TripKaiCinnabar`).
- MINA and ROWAN mention their homes in their dorm rooms (`Act2_Text_RoomMina`, `Act2_Text_RoomRowan`).
- Cinnabar had no houses: `python3 tools/hack/add_kais_house.py` stamps the general tileset's route house (as on
  Route 2) into the island's layout. Its interior is a copy of FireRed's small house (`LAYOUT_CINNABAR_ISLAND_KAIS_HOUSE`,
  from `LAYOUT_HOUSE1_FRLG`), free to change in Porymap.

## Base-game scenes stripped for Act 2

- **Cinnabar Island**: Bill's trip to One Island and the return from the Sevii Islands (Bill and the Seagallop).
- **Vermilion City**: the player walking off the S.S. Anne.
- **Viridian City**: the old man blocking the road and his catching tutorial (he stands by the road).
- **Route 22**: the rival.

## Not done yet

- Teams of up to 3 aren't enforced.
