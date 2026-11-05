# Pokémon Unnamed (working title)

A Pokémon ROM hack for the Game Boy Advance, set in Kanto and built on the FireRed mode of
[`pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).

## Overview

The game follows an ordinary person's life with Pokémon in three acts, each longer than the one before.

- **Act 1, Childhood.** The player grows up on a farm near a small town, with no training and no Poké Balls. The
  player must learn to befriend the local Pokémon without battling.
- **Act 2, University.** The player attends university, where Poké Balls and 3v3 battles are introduced.
- **Act 3, Workforce.** The player starts a career at Silph Co.

## Features added by this hack

### New locations

- **Act 1:** a farm just west of Pallet Town, with the player's farmhouse, a barn and a farmyard, and the Woods
  beyond it.
- **Act 2:** Celadon University, reached from Celadon City through a gate building. The campus has a main hall,
  lecture halls, a library, a cafeteria, a faculty office, a hospital, a science institute and student dorms.
- **Act 3:** none yet.

### Befriending (Act 1)

![A befriending battle](docs/readme/befriend.png)

In Act 1 the player cannot attack or catch Pokémon. A befriending battle replaces the wild battle: the player
chooses actions (such as holding out a hand or crouching down), offers items, or watches the Pokémon for hints.
Each Pokémon reacts differently to each action and item. The bar under its name shows its aggression, and the
Pokémon is befriended when it reaches zero. Every choice uses up some of its patience (the pips next to its name);
when it runs out, the Pokémon runs away.

One befriended Pokémon follows the player at a time. The others live in a farmyard at the player's house.

### 3v3 battles (Act 2 on)

![A 3v3 battle](docs/readme/battle_3v3.png) ![The move menu in a 3v3 battle](docs/readme/battle_3v3_moves.png)

Each side has three Pokémon on the field, but only one Pokémon per side acts each turn. The player picks which one
with L and R, then its move and target.

Each side's three Pokémon stand in a formation: front, middle and back (from right to left on screen). Pressing
SELECT moves the chosen Pokémon forward in the formation, which does not use up the turn. At the start of a battle
the formation follows the party order: the first Pokémon is in front.

Moves are either direct or indirect:

- A **direct** attack travels out from the user or makes contact, so it hits the front Pokémon, which blocks the
  others. Tackle is a direct attack.
- An **indirect** attack can be aimed at any of the three Pokémon. Electric, Psychic and Flying moves, sound moves
  and attacks from above or below are indirect. Thunder Shock is an indirect attack.

A move hits one, two or all three Pokémon. With two targets, the second takes half damage; with three, each takes
75%.

**The move badge.** In the move menu, the move's type moves up beside its PP, and the line below shows how the
selected move reaches its targets and how many Pokémon it hits:

![Move badges for five moves](docs/readme/move_indicators.png)

- **DIRECT** (red): hits the front Pokémon, which blocks the others.
- **INDIRECT** (blue): can be aimed at any of the Pokémon.
- **ALL** (dark): hits every Pokémon on the other side.
- **x1, x2, x3:** how many Pokémon the move hits.

After choosing a move that hits two or three Pokémon, they flash and the player confirms with A (B goes back).

The floor under each team is drawn in the style of the terrain's own battle platforms: the same colors, texture and
rim as in a normal battle on that terrain.

When each side has only two Pokémon, the same rules apply with two Pokémon per side.

### University courses (Act 2)

Each course (Fire, Water, Grass) is a field trip with a research sheet: catch three different Pokémon of the type,
use and record moves of the type, and solve a puzzle at the site using a Pokémon of the type. Completing the sheet
lets the player make the course's TM, and the course exam only counts if the TM's move is used.

### Key items (Acts 2 and 3)

- **Remote Desktop:** opens the Pokémon Storage System from anywhere.
- **Heal Dash:** returns the player to the last Pokémon Center they visited.

### Speed

The game runs faster than FireRed without an emulator's fast-forward, so music and sound effects play normally.

- **Walking:** the player walks at 1.5x the usual speed.
- **Running:** holding B runs from the start of the game (no Running Shoes needed), at 1.5x the faster walk.
- **Battles:** battles, including their animations and the transition into them, run at 1.5x. Sound and music stay
  at their usual pace.
- **Text:** all text prints at 3x, whatever the text speed option.

Scripted scenes in the overworld play at the usual speed, so characters keep in step.

## Status

- **Act 1:** complete.
- **Act 2:** in playtesting.
- **Act 3:** not started.

Design notes for the acts and the custom systems are in [`docs/hack/`](docs/hack/).

## Building

The hack builds the same way as `pokeemerald-expansion`; see [`INSTALL.md`](INSTALL.md) for the toolchain setup.
No game ROM is included or needed to build.

```
make               # pokemon-unnamed.gba
make act2          # pokemon-unnamed-act2.gba, a playtest build that starts at the beginning of Act 2
make check         # the test suite
```

## Credits

This project is based on [RHH's `pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion), which
is built on [pret's `pokeemerald`](https://github.com/pret/pokeemerald) decompilation project.

```
Based off RHH's pokeemerald-expansion 1.17.2 https://github.com/rh-hideout/pokeemerald-expansion/
```

See [`CREDITS.md`](CREDITS.md) for the contributors to `pokeemerald-expansion`.

## About `pokeemerald-expansion`

`pokeemerald-expansion` is a GBA ROM hack base built on top of pret's `pokeemerald`. Its feature list is in
[`FEATURES.md`](FEATURES.md) and its documentation is at
[rh-hideout.github.io/pokeemerald-expansion](https://rh-hideout.github.io/pokeemerald-expansion/). Questions and
bug reports about the base itself belong with the
[RHH project](https://github.com/rh-hideout/pokeemerald-expansion) and its
[Discord server](https://discord.gg/6CzjAG6GZk).
