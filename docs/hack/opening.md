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
- Acts 2 and 3: the male Silph worker sprite (`OBJ_EVENT_GFX_WORKER_M`), as the Lapras giver on Silph Co. 7F.
