# Befriending battles (Act 1)

In Act 1 the player has no Poké Balls: instead of catching, they calm a wild Pokémon down until it's comfortable
around them, and it joins them.

## Rules
- The player stands alone on the field (no Pokémon of theirs); the wild Pokémon never attacks, it only reacts.
- The Pokémon's HP bar is its **Aggression** (`src/befriend.c`). Getting it to 0 befriends the Pokémon.
- Each turn the player picks:
  - **Act**: one of four actions — Crouch down, Hum a tune, Reach out, Approach.
  - **Item**: offer an item from the bag (opens on the Berries pocket). Offered items are used up.
  - **Watch**: a clue to what the Pokémon likes (its `watchHint`), without changing its aggression.
  - **Leave**: end the encounter.
- Every Act, Item or Watch costs one **patience**, shown as pips in the Pokémon's box. Out of patience, it runs away.
  The player can come back and try again.
- How it reacts (percent of its max HP; constants in `include/befriend.h`):

  | Choice | Aggression |
  |---|---|
  | Its loved action, first time | −40% |
  | Its loved action again | −15% |
  | Its loved item | −60% |
  | Anything neutral | −10% |
  | A disliked action or item | +25% |

- Befriended: "Eevee is comfortable around you now!", then it joins like a caught Pokémon (party, or PC when the party
  is full) at full health. The battle outcome is `B_OUTCOME_CAUGHT`.

## Per-Pokémon likes
`src/data/befriend.h`: for each species, a loved action and item, disliked actions and items, its patience and its Watch
clue. Unlisted species use `sBefriendDefault`. The entries there now (Eevee, Pidgey) are placeholders until the Act 1
Pokémon are decided.

## Starting one
In a map script:

```
befriendbattle SPECIES_EEVEE, 5
@ VAR_RESULT: B_OUTCOME_CAUGHT (befriended), B_OUTCOME_MON_FLED (ran away), B_OUTCOME_RAN (player left)
```

Debug menu → Party → *Befriend Eevee* gives a few berries and starts one.

## How it's built
- Built on the Safari battle (`BATTLE_TYPE_SAFARI`) plus `gIsBefriendBattle`: no player Pokémon, the player's back
  sprite stays, the player's controller is the Safari one with befriending branches (`src/battle_controller_safari.c`).
- New battle actions `B_ACTION_BEFRIEND_ACT`, `_ITEM`, `_WATCH` (`HandleAction_Befriend*`); the wild Pokémon passes.
- Reactions are battle scripts in `data/battle_scripts_2.s` (`BattleScript_Befriend*`); the aggression change goes through
  the normal HP bar update.
- The wild Pokémon's box: patience pips replace the level, and an "Aggression" label with its own shorter bar replaces
  the HP bar (`DrawBefriendAggressionBar`, `src/battle_interface.c`).
- The Act list uses the move menu's layout, with "Patience x/y" on the right.

## Tests
`test/battle/befriend.c`: `BEFRIEND_BATTLE_TEST` with `ACT(action)`, `OFFER(item)` and `WATCH()` in `TURN`.
