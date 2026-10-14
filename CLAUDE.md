# CLAUDE.md

## Project
A Pokémon ROM hack (working title: TBD) built on **pokeemerald-expansion** (RHH),
a GBA hack base on top of pret's pokeemerald decompilation. This repo is a fork of
`rh-hideout/pokeemerald-expansion`. Output is a GBA ROM (`pokeemerald.gba`).

Current expansion version: `include/constants/expansion.h` (`EXPANSION_VERSION_*`; currently 1.17.2).
Per-release notes live in `docs/changelogs/<X.Y.x>/` (`docs/CHANGELOG.md` is just an mdBook include stub).

## Environment
- macOS, devkitARM toolchain (installed via devkitPro pacman, `DEVKITPRO`/`DEVKITARM` set in `~/.zshrc`)
- libpng, pkg-config, coreutils via Homebrew
- Emulator for testing: mGBA
- Map editor: Porymap (maps and tilesets are edited there by hand, not in code)

## Commands
- Build: `make -j$(sysctl -n hw.ncpu)`
- Build with debug symbols: `make debug`
- Release build (strips debug features): `make release`
- Run expansion test suite: `make check`
- Clean: `make clean`

Always run a build after making changes and fix any errors before reporting a task
as done. Run `make check` after touching battle logic, species data, moves or abilities.

## Where things live (verify paths before editing; they shift between versions)
- Feature toggles / generation behavior: `include/config/` (e.g. `battle.h`, `pokemon.h`, `debug.h`, `overworld.h`)
- Constants (species, moves, items, flags, vars): `include/constants/`
- Species data: `src/data/pokemon/species_info/`
- Trainer parties: `src/data/trainers.party` (FRLG variant: `trainers_frlg.party`; `trainers.h` is generated)
- Wild encounters: `src/data/wild_encounters.json` (`wild_encounters.h` is generated from it)
- Maps (headers, events, connections): `data/maps/<MapName>/map.json`
- Map scripts: `data/maps/<MapName>/scripts.inc`
- Shared event scripts: `data/scripts/`
- Graphics: `graphics/`
- Expansion docs: `docs/`

## Working rules
- Prefer changing config flags in `include/config/` over editing engine code when a toggle exists.
- Don't hand-edit map layout binaries (`*.bin`) — those are Porymap's job. Editing `map.json`
  events and `scripts.inc` in code is fine; tell me when a change needs a Porymap step.
- Keep changes small and on feature branches; commit with clear messages.
- Never commit built ROMs (`*.gba`, `*.elf`) or any original game ROM.
- When unsure how the expansion implements something, read the existing code and `docs/`
  before inventing a new approach.

## Upstream
- `origin` = `https://github.com/cgogoel/Pokemon-unnamed` (this hack)
- Upstream remote `RHH` = `https://github.com/rh-hideout/pokeemerald-expansion` — not configured locally yet;
  add with `git remote add RHH https://github.com/rh-hideout/pokeemerald-expansion && git fetch RHH --tags`
- Update incrementally by release tag: `git pull RHH expansion/X.Y.Z`, resolve conflicts, rebuild, `make check`.

## Hack design
**Premise:** An ordinary person's life with Pokémon, from childhood through university to a career.
They're a small-town kid who loves Pokémon but has no training. Told in a short prologue and three acts
that grow in length. Act 3 retells Kanto events (Red, Blue, Oak, Giovanni, Silph Co.) from an insider's view.

### Prologue / Act 1 — Childhood (short)
- Small hometown. The player is untrained and has no Poké Balls.
- The professor visits town but doesn't choose the player for their study because they lack schooling.
- The player learns moves by experimenting with Pokémon and discovers new kinds of moves through battling and befriending them.
- **Befriending replaces catching.** One Pokémon follows the player at a time. The rest live in a farmyard at the player's house.
- **Act ends** once the player has beaten the town's 2 strongest unique Pokémon and befriended a 3rd.

### Act 2 — University (medium)
- Time skip: the player finished local schooling and was accepted to university.
- Introduces Poké Balls and teams of up to 3.
- **3v3 battles:** each turn the player chooses which of their 3 Pokémon attacks and which of the
  3 opponents it targets. The enemy side does the same. Each move hits 1, 2 or all 3 targets.
- Structure: one course per major type. Each course is about finding that type's best move and testing
  what Pokémon can reach through inherent ability and training.
- **Act ends** with a final exam where the player discovers an **ultimate move that's super effective against every type**.

### Act 3 — Workforce (large)
- Time skip: the player is a lead researcher at Silph Co.
- Insider-threat plot: an employee is leaking information to Team Rocket. The player investigates
  at work and out in the region, which pushes exploration of the region.
- Familiar characters seen from a new angle: Red, Blue, Professor Oak, Giovanni and others.
- **Climax:** Team Rocket invades Silph. The player defends a key Pokémon, a **Lapras that has mastered the
  ultimate move**, and hands it to Red, who uses it to defeat Giovanni.

### Open decisions
- Hometown / Act 1–2 region: Kanto or a new region? Act 3 is Kanto, so the expansion's FRLG mode
  (`IS_FRLG`, `docs/tutorials/how_to_frlg.md`) may be a better base than Emerald.
- Names: player, hometown, university, the professor, the mole.
- Difficulty goals; which Pokémon generations appear.

### Technical notes (custom systems the design needs)
- **3v3 battles:** pokeemerald-expansion has no triple-battle support. This is the biggest engine job;
  scope and prototype it before building Act 2 content.
- **Befriending without balls (Act 1):** custom battle outcome and script hooks. Farmyard = a house map
  whose Pokémon are shown with `OBJ_EVENT_GFX_SPECIES` (`OW_POKEMON_OBJECT_EVENTS` is already on). Storage
  could reuse the PC box system behind the scenes.
- **One follower:** `OW_FOLLOWERS_ENABLED` in `include/config/overworld.h` (currently FALSE).
- **Discovering moves by experimenting:** custom move-unlock tracking (flags/vars or save data) instead of level-up learnsets.
- **Ultimate move:** custom move that's super effective vs. all types (special-case it in the type effectiveness logic).
- **Act time skips:** story vars gating maps/events; possibly a change of player sprite per act.
