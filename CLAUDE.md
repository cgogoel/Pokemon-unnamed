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

## Hack design (fill in as decided)
- Region / setting:
- Story premise:
- Generation scope (which Pokémon / mechanics):
- Difficulty goals:
- Planned features:
