# Pokémon Unnamed (working title)

A Pokémon ROM hack for the Game Boy Advance, set in Kanto and built on the FireRed mode of
[`pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).

## Overview

The game follows an ordinary person's life with Pokémon in three acts, each longer than the one before.

- **Act 1, Childhood.** The player grows up in a small town with no training and no Poké Balls. Pokémon are
  befriended rather than caught, one Pokémon follows the player at a time, and moves are discovered by
  experimenting. The act ends after the player has beaten the town's two strongest Pokémon and befriended a third.
- **Act 2, University.** The player attends Celadon University. Poké Balls and teams of up to three are introduced,
  along with 3v3 battles in which each side chooses one of its three Pokémon to act each turn. The act is organized
  around three courses (Fire, Water and Grass), each with a field trip, a research assignment, a TM and an exam, and
  ends with a final research project.
- **Act 3, Workforce.** The player works as a researcher at Silph Co. and investigates an employee who is passing
  information to Team Rocket. This act revisits events from FireRed, including Team Rocket's takeover of Silph Co.,
  from the point of view of someone working there.

## Features added by this hack

These are changes made on top of `pokeemerald-expansion`.

**Story structure**
- Three acts with time skips between them. The current act is stored in a variable, and maps, events and rules
  check it.

**Act 1**
- Befriending replaces catching. In a befriending battle the player uses actions and items instead of attacking,
  and each Pokémon reacts differently to them.
- One Pokémon follows the player at a time. The others stay in a farmyard at the player's house.

**Battles**
- 3v3 battles from Act 2 on. Each side has three Pokémon on the field and picks one of them to act each turn.
- The three Pokémon stand in a front, middle and back formation that can be rearranged each turn without using up
  the turn. Each move has a reach (only the front Pokémon, or any Pokémon) and hits one, two or three targets.
  The move menu shows this for each move.
- When each side has two Pokémon, the same rules apply as a 2v2.
- Battle messages wait for a button press, and battles and battle transitions run at 1.5x speed.

**Act 2**
- Three courses (Fire, Water, Grass). Each has a field trip with a research sheet: catch three different Pokémon
  of the type, use and record moves of the type, and solve a puzzle at the site. Completing the sheet produces a TM,
  and the course exam only counts if the TM's move is used.
- Wild Pokémon levels follow the party's average level. On field trips, some wild Pokémon are Pokémon of the
  course's type from other regions.
- Trainers will not battle the player until they earn a Trainer Card.
- A night battle club with the player's friends, played 3v3 with the first three Pokémon in the party. Opponents
  are set to the average level of those three, and losing does not stop the story.

**Items and overworld**
- Remote Desktop: a key item that opens the Pokémon Storage System from anywhere (Acts 2 and 3).
- Heal Dash: a key item that returns the player to the last Pokémon Center they visited (Acts 2 and 3).
- From Act 2 on, the whole party shares experience and all Pokémon obey the player regardless of level.
- The player walks at 1.5x speed and can run with B from the start, at 1.5x the walking speed.
- Text prints at 3x speed, and fishing always gets a bite.

**Development**
- An Act 2 playtest build (`make act2`) that starts a new game at the beginning of Act 2.

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
