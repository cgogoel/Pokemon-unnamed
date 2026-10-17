# Act 1 story

The opening of the game, after Oak's speech ([opening.md](opening.md)). The befriending rules are in
[befriending.md](befriending.md).

Progress is `VAR_ACT1_PROGRESS`, with stages `ACT1_*` in `include/constants/story.h`.

| Stage | Reached by | Then |
|---|---|---|
| `ACT1_START` | New game | Mom: sorry you weren't picked; pick up the Pokédex at the lab. |
| `ACT1_GOT_POKEDEX` | Oak's aide by the lab door hands it over | Aide: battling isn't the only way to get to know Pokémon. Dad can be talked to right away (Mom suggests it, but isn't required). |
| `ACT1_MOM_SENT_TO_DAD` | Talking to Mom | Mom: go talk to Dad in the storage shed (the barn). |
| `ACT1_DAD_SENT_TO_WOODS` | Talking to Dad | Dad: he never caught the farm's Pokémon; kindness and patience; Blipbug at the Woods entrance. Gives an Oran Berry. **The Woods open.** |
| `ACT1_BEFRIENDED_BLIPBUG` | Befriending Blipbug | Dropping it off: "I should talk to Dad to make sure it's OK." **The Woods close until Dad's talk.** Mom: Dad wants to talk again. Hint NPCs start. |
| `ACT1_DAD_WARNED_JANGMO_O` | Talking to Dad | Dad: well done; others in the Woods; avoid the angry Jangmo-o. **Tynamo, Hatenna and Teddiursa can be befriended.** Mom: the folks in town might know about the Woods' Pokémon. |
| `ACT1_BEFRIENDED_THREE` | Befriending all three | Mom: go show Dad. **The Woods close until Dad gives the Hard Stone** (the edge says who to see next). |
| `ACT1_DAD_SAYS_TOO_DANGEROUS` | Talking to Dad | Dad: Jangmo-o is still too dangerous; go see Mom. |
| `ACT1_MOM_SAYS_FOLLOW_HEART` | Talking to Mom | Mom: Dad's scared for you, but follow your heart; your kindness may reach Jangmo-o. |
| `ACT1_GOT_HARD_STONE` | Talking to Dad | Dad gives the Hard Stone, "throw it if it attacks". **Talking to Jangmo-o starts its encounter.** |
| `ACT1_BEFRIENDED_JANGMO_O` | Befriending Jangmo-o | Bringing it home through the farmyard gate: Mom and Dad can't believe it (the player must understand Pokémon), and send them to bed. |
| `ACT1_RESTED` | Waking up in the farmhouse 2F | Mom and Dad (Dad's in the house now): show your Pokédex to Oak. The Woods are closed. Walking out toward Pallet Town (x 21), Oak arrives: Jangmo-o was pacified? Befriended, not battled? Not a trainer yet, but a distinguished youth scholar for Celadon University. Mom and Dad: is that what you want? (asked until yes). Oak: bring two friends, here are two Poké Balls. |
| `ACT1_CHOOSING_FRIENDS` | Saying yes | The player stays in the farmyard (its exits and doors turn them back) and talks to the farm Pokémon they befriended: a picture and "Are you sure you want to bring X with you?". After the second, Oak's last words, then the prologue ends. |

## The end of the prologue

After the second friend: "End of Prologue", then "Years Later at Celadon University" (`src/prologue_end.c`, each on
black for 3 seconds or until A/B). `VAR_STORY_ACT` becomes `STORY_ACT_UNIVERSITY` and the player wakes up at Act 2's
start, `ACT2_START_MAP` in `include/constants/story.h`: a placeholder Celadon Condominiums room (2F) for the
University dorms. The chosen friends are in the party (Poké Balls); the others stay on the farm.

The scenes put Mom, Dad and Oak next to the player during a fade (`special Act1_PlaceNearPlayer`), so they work from
any row of the farmyard's entrances; Oak walks in from the Pallet Town edge on the player's row.

## The Woods

- **Gate** (`Farmyard_EventScript_ToWoods`): until Dad sends the player, the Woods' edge turns them back with a
  line about what to do first.
- **Tynamo, Hatenna, Teddiursa**: until Blipbug is befriended and Dad has warned about Jangmo-o, they keep their
  distance ("start with the Blipbug" / "maybe Dad knows something").
- **Their loved items have to be found**: the loved action only works once (then it counts as neutral), so actions
  alone fall short. Offering an item uses it up; if the Pokémon then runs away, the item is back where it was found
  next time (`special Act1_RestockLovedItems`).

  | Pokémon | Loved item | Found at (placeholder) | Hint from |
  |---|---|---|---|
  | Tynamo | Cheri Berry | Pallet Town (19,17), by the water | The fat man in Pallet Town |
  | Hatenna | Chesto Berry | The mailbox by the Keep Out sign, Woods (7,21) | Aide 2 in Oak's lab: someone leaves berries at the abandoned ranger station to appease what lives there |
  | Teddiursa | Honey | Woods: hidden in the tree at (5-6,42), found by checking either tile (nothing shows it) | Daisy, Blue's house: look for lots of flowers; flowers mean bees, bees mean Honey |

  Hints start once Blipbug is befriended and stop once that Pokémon is.
- **Tynamo** roams its pond, which the player can walk into. A Pokémon object moving on its own stays in water or on
  land, whichever it's on (`IsPokemonLeavingItsGround` in `src/event_object_movement.c`), so Tynamo stays in its pond
  and the farm's Pokémon stay out of the farm pond; the farmyard's pond spot is Tynamo's there.
- **Jangmo-o** wanders anywhere in the Woods (no range limit). It has a spawn area near each of the other three
  (`sJangmoOSpawns` in `src/act1_story.c`: (15,32) by Tynamo's pond, (36,6) by Hatenna, (48,38) by Teddiursa): while
  it's off-screen, it's moved to the one nearest the player that's still out of view, so it turns up wherever the
  player is heading.
- **Caught with a Pokémon following** (not yet dropped off at the farm): it runs back to where it was befriended and
  has to be befriended again (`special Act1_ReleaseFollowerToWoods`); if it was the one that moved the story on, the
  story steps back too. Its loved item comes back where it was found.
- **Signs**: "Quincy Woods" past the entrance (39,59), "Warning: Aggressive POKéMON Nearby" by Jangmo-o's area
  (43,26), "Danger: Keep Out" top left (6,20). Until the player has the Hard Stone, it charges if it sees them (straight ahead, up to 4
  tiles, `src/act1_story.c`) or is talked to: the player blacks out and wakes up at home next to Mom
  (`Act1_EventScript_BlackOut`). With the stone, talking to it starts the encounter, where Hum a tune becomes
  **Throw stone**: Jangmo-o hits back ("Who throws a stone? Honestly! …") and the player blacks out, keeping the
  stone. Offering the stone from the bag (and one bold Approach) wins it over. If an offered stone is lost, Dad has
  another.

## Characters

| Who | Where | Notes |
|---|---|---|
| Mom | Farmhouse 1F | `Farmhouse1F_EventScript_Mom` (Act 1); the vanilla Mom script in other acts |
| Dad | Barn (8,6) | `Barn_EventScript_Dad`. Placeholder sprite: `OBJ_EVENT_GFX_HIKER_FRLG` |
| Blue | His house (2,5), Act 1 only | "I'll be old enough soon; I won't get passed over like you" |
| Farm Pokémon | Farmyard | Miltank, Mareep, Tauros, Doduo: talking plays their cry. Not befriendable |

## Placeholders to set in Porymap

- Dad's spot and sprite (barn), Blue's spot (his house), the four farm Pokémon.
- The mailbox in the Woods (7,20)-(7,21) has no collision yet, so the player can walk onto it.

## Base-game scenes are stripped

The hack tells its own story, so the base game's scenes come out as they're reached: detach the scene (its map.json
event, `map_script` entry or `goto`), then run `python3 tools/hack/prune_dead_scripts.py data/maps/<Map>/scripts.inc`
to delete the code and texts only it used. Done so far:

- **Oak's lab**: the starter choice, rival battle, parcel and Pokédex hand-off, National Dex upgrade, all their
  triggers, and the Oak and Blue objects that were only there for them. The starter Poké Balls just say they're for
  Oak's new trainers.
- **Pallet Town**: Oak's "Don't go out!" trigger at the north exit, his post-League rating scene and his object.
- **Blue's house**: Daisy's Town Map errand and her lines that depend on the lab scenes.

## Scripts checked for Act 1

Fixed:
- Mom's vanilla lines ("Oak, next door, was looking for you") → `Farmhouse1F_EventScript_Mom`.
- Blue waiting in the lab with "Gramps isn't around" → removed from the lab, at home instead.
- Daisy: "my brother is out at Grandpa's lab" → Act 1 lines and the Teddiursa hint.
- The fat man's PC line → Act 1 lines and the Tynamo hint.
- The starter Poké Balls on Oak's table → "for Prof. Oak's new trainers".
- Aide 1 in the lab: could say nothing once the Pokédex was given (stale `VAR_RESULT`) → rewritten.
- The farmhouse 2F notice pointing at a Help system the game doesn't have → a chore chart.

Left as they are (mild, or not reachable in Act 1):
- Pallet Town's sign "Shades of your journey await!" and the Trainer Tips sign.
- The sign lady's later line "I'm raising Pokémon, too…".
- The lab computer's Pokémon League e-mail.
- A real white-out (not the Act 1 blackout) would play Mom's vanilla "Prof. Oak / Pokémon Center" heal text; it
  can't happen in Act 1.
- The empty for-sale house's 2F still sets the respawn point on its first visit (harmless: it's the farmhouse now).
