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
| Line | Hatenna (> Hattrem > Hatterene) | Hum a tune, Chesto Berry | Approach, Reach out, Cheri Berry | 4 | 100% |
| Line | Teddiursa (> Ursaring > Ursaluna) | Reach out, Honey | Approach, Rawst Berry | 5 | 100% |
| Boss | Jangmo-o (> Hakamo-o > Kommo-o) | Approach, Hard Stone | Crouch down, Hum a tune, Oran and Pecha Berries | 6 | 50% |

Why: Blipbug is curious and harmless and wants to study you at its level; Tynamo is weak alone and lives in schools, so it
wants company; Hatenna senses emotions and only warms to calm people, so a soft, soothing tune; Teddiursa loves honey and licks its
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
  (`FARM_BOX`, named FARM) and shown at the map's first 8 object events (graphics `VAR_OBJ_GFX_ID_0..7`, hidden by
  `FLAG_TEMP_1..8` while empty, set by `SetupFarmyardMons` on map load). Spot 3 is in the pond and only for Tynamo
  and its evolutions (`FARM_WATER_SPOT`); the others fill the other 7 spots in box order (`GetFarmSpot`).
- **Woods** (`data/maps/Woods`): reached only from the farmyard: the farmyard's left edge leads to the Woods' right edge (for now a warp: stepping on
  the farmyard's x 0 goes to the Woods, and the Woods' exit returns to (0, 10), one step before the gate). All five Act 1 Pokémon live
  there; talking to one starts its befriending encounter, and once befriended it's gone from the Woods
  (`FLAG_BEFRIENDED_*`). Levels: Blipbug 3, Tynamo 5, Hatenna 5, Teddiursa 6, Jangmo-o 10.
- A new game starts in the farmyard, in front of the farmhouse the player grew up in (`FARMYARD_START_X/Y`,
  `include/constants/farmyard.h`). Farmyard maps: the farmhouse (`PalletTown_PlayersHouse_1F/2F_Frlg_Farm`, Mom on 1F) and
  the barn (`PalletTown_Farm_Barn`). `HEAL_LOCATION_PALLET_TOWN` points at the farmhouse door; whiting out puts the player
  next to Mom in the farmhouse.
- Pallet Town's left edge and the farmyard's right edge are a map connection (rows 8-11).

## Coming home in Acts 2 and 3
The farm Pokemon the player left behind keep wandering the farmyard, and the ones they took with them **run out to
play** whenever they're brought home (`ShowPartyFarmFamily`, `src/farmyard.c`, called from `Farmyard_OnTransition` and
from the gates):

- Who counts: any party Pokemon whose evolution line starts at one of Act 1's five (Blipbug, Tynamo, Hatenna,
  Teddiursa, Jangmo-o), so an evolved one still knows the yard. Eggs don't.
- They're only **shown**, never moved: they take the farm spots the farm's own Pokemon aren't standing in (Tynamo's
  line keeps to the pond), and stay in the party the whole time, so nothing has to put them back. Walking off the map
  is enough, which is why leaving to Pallet Town needs no script of its own - that side is a map connection.
- The gate says so once a visit (`VAR_TEMP_8`): which gate the player is on and which way they're facing tell coming
  home from going out, since Pallet Town is to the right of the farmyard and the Woods to the left.
- **No follower while they're home.** The Pokemon running around the yard are the player's own party, so one still
  walking at their heel would be out here twice over. `ShowPartyFarmFamily` sets `FLAG_TEMP_HIDE_FOLLOWER` and calls
  `RemoveFollowingPokemon`; being a temp flag it clears itself on the next map, so the follower is back at their side
  the moment they leave. Act 1 keeps its follower - that act's whole mechanic is the one Pokemon walking with you.
  The flag alone isn't enough coming in from Pallet Town: a map connection keeps the objects of the map being left and
  never calls `UpdateFollowingPokemon`, so the gate plays `hidefollower` first and the player sees it go back into its
  ball instead of vanishing. Entering from the farmhouse or the barn is an ordinary warp, which never spawns one.

**Taking one along.** Talking to a farm Pokemon offers to bring it with you (`Farmyard_EventScript_TakeAlong`,
`GetFarmMonToTake` / `TakeFarmMonIntoParty`), shown the way the end of Act 1's choice is - its picture, then
"are you sure?". It leaves the farm box for the party in a Poke Ball of the player's own, its object is removed, and
its spot frees up for whoever the farm box shows next. From then on it's one of the player's, so it runs out to play
with the rest when they come home.

- Room is the party's normal capacity (`PARTY_SIZE`), not Act 2's three-a-side battle rule, which is about how many
  can be sent out rather than how many can be carried. A full team gets a message and the Pokemon stays.
- Taking one costs no Poke Ball, unlike the end of Act 1, where Oak lent two of his.
- There's no way to leave one behind again from here: the gates only drop a follower off in Act 1, so a Pokemon goes
  back to the farm through the PC (the farm is `FARM_BOX`, the last box, named FARM).

## Act 1's five are found nowhere else

They're the player's own, so Kanto never hands one out somewhere else:

- They're in no wild encounter table (`src/data/wild_encounters.json`). The only one that was is Teddiursa, one of
  the Altering Cave's nine rotating species; it's Phanpy there now, in both Altering Caves and Hoenn's Safari Zone.
- The road home's substitution, which can swap a wild or a trainer's Pokemon for one from another region, skips
  them: `EXCLUDE_SPECIES` in `tools/hack/gen_other_region_pools.py`, the same list that holds the glade's Morelull.
  Test: "the Pokémon the player befriended in Act 1 are never substituted in" (`test/act2_road.c`).
- No trainer carries one.
- **Their evolutions are deliberately left in** - a wild Ursaring is its own Pokemon, not the player's Teddiursa.
- The TM Database's checklist is unaffected: the five still count as findable through the Woods' `befriendbattle`
  scripts, which is what `gen_tm_research_species.py` reads them from.
- The gates **only drop a follower off in Act 1** (`VAR_STORY_ACT`). They used to do it in every act, so walking in or
  out of the farmyard in Act 2 moved the lead party Pokemon into the farm box.
- Act 1's last stage is its highest, so `VAR_ACT1_PROGRESS` is set to `ACT1_DONE` when Act 2 starts (`StartAct2`,
  `src/prologue_end.c`). Left at `ACT1_CHOOSING_FRIENDS` it kept the end-of-Act-1 scene armed: Mom, Dad and Oak stood
  in the farmyard in Acts 2 and 3, the choose-a-friend triggers fired again, and the farm Pokemon still offered to
  come along.
- Acts 1-2: the original player's house in Pallet Town is empty and for sale. Act 3 gets its own Pallet Town and house maps.
- Act 1 stays in Pallet Town: its north exit to Route 1 turns the player back ("It's too far to go on my own."), on
  triggers listed before Oak's so his scene doesn't start. The south is water, and there's no Surf in Act 1.

**Porymap:** the Farmyard and the Woods are drawn. They're joined by warp triggers, not a map connection: the game
draws a connected map with the current map's tilesets, and the Woods (Viridian Forest tileset) would come out as
garbled Pallet Town tiles next to the farmyard (Porymap draws each map with its own, so it looks fine there). A map
connection only works between maps that share tilesets, like Pallet Town and the farmyard; otherwise only primary
tileset metatiles may show near the shared edge.

**Music:** befriending encounters play the standard wild battle music.

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
