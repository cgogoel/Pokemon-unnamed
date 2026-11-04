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
