# Opening

The new-game scene (`src/oak_speech.c`, texts in `data/text/new_game_intro_frlg.inc`).

1. **No controls guide.** The game goes straight to the Pikachu pages.
2. **Pikachu pages** (`gPikachuIntro_Text_Page1-3`): every story has people whose names are never told; this is
   one such life, a lead scientist at Silph Co. before Team Rocket's takeover; it starts on a small farm.
3. **Oak** introduces himself and Pokémon (unchanged), then asks the player's name. There's no boy/girl question:
   the player is always a boy.
4. After the name, **Oak comes back** (`gOakSpeech_Text_NotChosen`): the player wasn't chosen to get a Pokémon and
   set out with Poké Balls, because they don't know how Pokémon moves work. They should go home to the farm and find
   another path. He'll give them a Pokédex, waiting at his lab.
5. There's no rival introduction or naming: the rival is always **BLUE** (`gNameChoice_Blue`).

The game then starts outside the farmhouse (see [befriending.md](befriending.md)).

## The Pokédex

In Act 1, the aide by the door of Oak's lab (`PalletTown_ProfessorOaksLab_EventScript_Aide1`) hands over the
Pokédex (`FLAG_SYS_POKEDEX_GET`). The two Pokédexes on Oak's desk stay there for Red and Blue.

## The player's look

- Always a boy.
- **Act 1:** the Silph worker's head on Red's body. Portraits keep Red's look, with the worker's hair color.
- **Acts 2 and 3:** walking and running use the male Silph worker sprite (`OBJ_EVENT_GFX_WORKER_M`), as the Lapras
  giver on Silph Co. 7F. `GetObjectEventGraphicsInfo` swaps it in for the player from `STORY_ACT_UNIVERSITY`
  (`gObjectEventGraphicsInfo_PlayerWorker`). He has no run frames, so running reuses walking. Biking, surfing,
  fishing and the item poses keep the Act 1 sprites.

These are placeholders, to be redesigned. The files:

| What | File | Palette |
|---|---|---|
| Walk, Act 1 | `graphics/object_events/pics/people/player_boy/normal.png` | `graphics/object_events/palettes/player_boy.pal` (+ `player_boy_reflection.pal`) |
| Run and surf-riding | `.../player_boy/surf_run.png` | same |
| Item / field move pose | `.../player_boy/item.png` | same |
| Bike | `.../player_boy/bike.png` | same |
| Surf | `.../player_boy/surf.png` | same |
| Fishing | `.../player_boy/fish.png` | same |
| VS Seeker on the bike | `.../player_boy/vs_seeker_bike.png` | same |
| Walk and run, Acts 2-3 | `graphics/object_events/pics/people/worker_m.png` (shared with the Silph worker NPCs) | `graphics/object_events/palettes/npc_white.pal` (shared) |
| Intro picture (Oak's speech) | `graphics/oak_speech/red/pic.png` | `graphics/oak_speech/player_boy.pal` |
| Battle back sprite | `graphics/trainers/back_pics/player_boy.png` | in the PNG |

The trainer front picture (trainer card, Hall of Fame) is still Red's own (`graphics/trainers/front_pics/red.png`),
since Red appears in Act 3.

`tools/hack/compose_player_head.py` generated the `player_boy` files from Red's and the worker's originals.
**Don't re-run it after editing them by hand:** it overwrites them.
