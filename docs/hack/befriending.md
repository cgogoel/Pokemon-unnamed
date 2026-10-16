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
`src/data/befriend.h`: for each species, a loved action and item, disliked actions and items, its patience, how easily
it calms down (`calmRate`, percent of the usual calming; raising isn't scaled) and its Watch clue. Unlisted species use
`sBefriendDefault`.

| Role | Pokémon (line) | Loves | Dislikes | Patience | Calm rate |
|---|---|---|---|---|---|
| Tutorial | Blipbug (> Dottler > Orbeetle) | Crouch down, Oran Berry | Approach | 6 | 150% |
| Line | Tynamo (> Eelektrik > Eelektross) | Reach out, Cheri Berry | Approach, Pecha Berry | 5 | 100% |
| Line | Hatenna (> Hattrem > Hatterene) | Crouch down, Chesto Berry | Approach, Reach out, Cheri Berry | 4 | 100% |
| Line | Teddiursa (> Ursaring > Ursaluna) | Reach out, Honey | Approach, Rawst Berry | 5 | 100% |
| Boss | Jangmo-o (> Hakamo-o > Kommo-o) | Approach, Hard Stone | Crouch down, Hum a tune, Oran and Pecha Berries | 6 | 50% |

Why: Blipbug is curious and harmless and wants to study you at its level; Tynamo is weak alone and lives in schools, so it
wants company; Hatenna senses emotions and only warms to calm people, so stay still; Teddiursa loves honey and licks its
sweetened palms; Jangmo-o grows by battling and respects boldness, so gentleness reads as weakness.

## Choosing the Act 1 Pokémon
`docs/hack/act1_befriend_rankings.csv` ranks every species in the base, regenerated with
`python3 tools/hack/rank_befriend_candidates.py`. It holds three separate rankings (`ranking` column):
- `three_stage_line`: 3-stage evolution lines, scored mostly on the base Pokémon (the one befriended). Act 1 uses at
  least 3 of these.
- `boss`: the most intimidating Pokémon, for the final one to befriend, which doesn't fit the lore.
- `tutorial`: the easiest to befriend, for the tutorial encounter.

Scores are heuristics from the species data: Pokédex entry and category keywords (`lore_for` / `lore_against`), base
friendship, catch rate, size, base stats, types, egg groups and abilities. Legendaries, mythicals, Ultra Beasts,
Paradox Pokémon, Megas/Gigantamax and battle or cosmetic forms are left out; regional forms are kept.

## The Act 1 loop
- The player has at most **one** Pokémon in Act 1, and it follows them (`OW_FOLLOWERS_ENABLED`). A befriended Pokémon
  joins the party, so it starts following right after the battle.
- While one is following, another befriending encounter doesn't start: "You should take Blipbug home to the farmyard
  first." (`IsBefriendAllowed`, `src/farmyard.c`; `befriendbattle` then sets `VAR_RESULT` to `BEFRIEND_RESULT_BLOCKED`).
- **Farmyard** (`data/maps/Farmyard`): walking in through a gate (coord triggers across its left entrance from the
  Woods, x 1, and its right entrance from Pallet Town, x 22) drops the follower off: it leaves the party and starts wandering the farmyard. Farm Pokémon are kept in the last PC box
  (`FARM_BOX`, named FARM); the first 8 are shown as the map's first 8 object events (graphics `VAR_OBJ_GFX_ID_0..7`,
  hidden by `FLAG_TEMP_1..8` while empty, set by `SetupFarmyardMons` on map load).
- **Woods** (`data/maps/Woods`): reached only from the farmyard: the farmyard's left edge leads to the Woods' right edge (for now a warp: stepping on
  the farmyard's x 0 goes to the Woods, and the Woods' exit returns to (0, 10), one step before the gate). All five Act 1 Pokémon live
  there; talking to one starts its befriending encounter, and once befriended it's gone from the Woods
  (`FLAG_BEFRIENDED_*`). Levels: Blipbug 3, Tynamo 5, Hatenna 5, Teddiursa 6, Jangmo-o 10.
- A new game starts in the farmyard, in front of the farmhouse the player grew up in (`FARMYARD_START_X/Y`,
  `include/constants/farmyard.h`). Farmyard maps: the farmhouse (`PalletTown_PlayersHouse_1F/2F_Frlg_Farm`, Mom on 1F) and
  the barn (`PalletTown_Farm_Barn`). `HEAL_LOCATION_PALLET_TOWN` points at the farmhouse door; whiting out puts the player
  next to Mom in the farmhouse.
- Pallet Town's left edge and the farmyard's right edge are a map connection (rows 8-11).
- Acts 1-2: the original player's house in Pallet Town is empty and for sale. Act 3 gets its own Pallet Town and house maps.
- Act 1 stays in Pallet Town: its north exit to Route 1 turns the player back ("It's too far to go on my own."), on
  triggers listed before Oak's so his scene doesn't start. The south is water, and there's no Surf in Act 1.

**Porymap:** the Farmyard is drawn. The Woods is still a copy of Viridian Forest, with its warps removed and a solid
right edge. Once it has an opening on its right edge, the Farmyard ↔ Woods warp triggers can be swapped for the map
connection: set the offset on both sides so the openings line up.

## Starting one
In a map script:

```
befriendbattle SPECIES_TEDDIURSA, 5
@ VAR_RESULT: B_OUTCOME_CAUGHT (befriended), B_OUTCOME_MON_FLED (ran away), B_OUTCOME_RAN (player left)
```

Debug menu → Party → *Befriend (Act 1)…* → pick one of the Act 1 Pokémon; it also gives every item they like or dislike.

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
