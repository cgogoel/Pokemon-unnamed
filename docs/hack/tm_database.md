# The TM Database

From Act 2's final project on (and through Act 3), the player makes TMs the way the courses made theirs: research a
TM's type in the field, then craft it. It's the main way to get new TMs; the base game's TM sources (shops, item balls,
gifts) stay. Code: `src/tm_research.c` (the rules and the battle counters) and `src/quest_menu.c` (the screens).
Tests: `test/tm_research.c`, `test/battle/tm_research.c`.

## Unlocking it

- At the final project's brief (`Act2_EventScript_FinalBrief`, `data/scripts/act2_finale.inc`), after the director's
  speech, one more line: see the researcher on 3F about the TM MACHINE (`Act2_Text_BriefTmMachine`).
- The **researcher** stands beside the TM MACHINE in the Science Institute's research room, 3F
  (`MAP_CELADON_UNIVERSITY_SCIENCE_RESEARCH_ROOM`, (12,2), `LOCALID_LAB_TM_RESEARCHER`). From `ACT2_FINAL_PROJECT` on, or
  in Act 3, the first talk explains the database and sets `FLAG_TM_DATABASE_UNLOCKED` (`special TmResearch_Unlock`);
  after that, a short reminder. Before the final project: finish your courses first. They're hidden with the room's
  other staff for the Tri-Blast scene (`FLAG_TEMP_5`).
- The **TM MACHINE** is the right-hand terminal on 3F's back wall (11,1). Before the researcher's talk: "Ask the
  researcher about it first." After: it opens the database (`special TmDatabase_OpenFromMachine`) and B leaves.
- Both are in `tools/hack/build_university_maps.py`'s tables, so rebuilding the campus keeps them.

## The rules

- **Every TM** in `FOREACH_TM` can be researched except Tri-Blast. A TM in the bag is **owned**: shown with a
  checkmark, not researchable (the three course TMs are owned once made).
- A TM's **type** is its move's type. Its **tier** follows the move's power (`TmResearch_GetTier`):

  | Tier | Moves | Catch | Record |
  |---|---|---|---|
  | 1 | Status moves, power 60 or less | 2 | 2 |
  | 2 | 61-90, and damaging moves whose power varies (Return, Frustration: their power is 0 or 1 in the data) | 3 | 3 |
  | 3 | 91-110 | 4 | 4 |
  | 4 | 111 and up | 5 | 5 |

- **Slots**: up to 3 TMs at a time (`TM_RESEARCH_SLOTS`), at most one per type.
- **Counting** starts when the research does (a slot starts empty):
  - *Catch*: each different species of the type caught in battle (either of its types counts), from
    `BS_Act2ResearchCatch` after the catch. Gifts don't go through a battle catch, so they don't count.
  - *Record*: each different move of the type used in battle by any Pokémon, the player's or the opponent's, as the
    courses count them (`Cmd_printattackstring`). Struggle doesn't count.
  - Both stop at the requirement: the slot keeps the species and moves it counted (up to 5 each).
- **Messages**: when a tracked TM's count goes up, the battle says so once, like the courses' counters:
  "TM research: 2 of 4 FIRE / POKéMON caught!" or "… moves recorded!" (after the move, `MOVEEND_ACT2_RESEARCH`; after
  a catch, `BattleScript_Act2ResearchMessage`). Untracked types say nothing. The last count adds "Flamethrower is
  ready / to craft in QUESTS!". A Pokémon of two tracked types gets both counters in one message.
- **Crafting**: when both counts are met, the slot is ready. A on it in TM RESEARCH (or on its TM in the browse list)
  asks "Craft …?"; yes gives the TM (`TmResearch_CraftSlot`) and frees the slot.
- **Clearing**: SELECT on a tracked slot asks to clear it; its counts are lost.
- **Catching 25 of a type**: once 25 different Pokémon of a type are caught (Pokédex caught flags; species from the
  type's list below; a type with fewer than 25 in the game needs all of them, `TM_RESEARCH_TYPE_CATCH_GOAL`),
  every TM of that type is ready, tracked or not: the browse list shows "All FIRE TMs unlocked!" and A crafts any of
  them (`TmResearch_CraftReadyTm`). The browse list and slots show the way there: "All FIRE POKéMON: 12/25".

### Every Pokémon of a type

`python3 tools/hack/gen_tm_research_species.py` writes `src/data/tm_research_species.h`, one list per type. A species
is in the game when it's in:

- Kanto's FireRed wild tables (`src/data/wild_encounters.json`, labels with FireRed, not the Sevii Islands' maps),
  the hack's Act 2 tables included;
- the other-region Pokémon Act 2 mixes in: the courses' (`sOtherRegionFire/Water/Grass`, `src/act2_story.c`) and the
  road home's (`src/data/act2_other_region_pools.h`);
- the gifts and static encounters: `givemon`, `giveegg`, `setwildbattle` and `befriendbattle` in the scripts of Kanto
  maps (`"region": "REGION_KANTO"`, not the Sevii Islands) and the hack's own shared scripts (`data/scripts/act*.inc`),
  plus the glade's MORELULL and the ranger station's dead-end Pokémon (made in C).

Left out: legendaries and mythicals (`INCLUDE_LEGENDARIES` in the script), and species without a National Dex number.
A species counts for both its types; forms sharing a National Dex number count once. Rerun the script after changing
any of those sources.

| Type | Pokémon | Type | Pokémon | Type | Pokémon |
|---|---|---|---|---|---|
| Normal | 113 | Bug | 81 | Grass | 110 |
| Fighting | 51 | Ghost | 56 | Electric | 51 |
| Flying | 86 | Steel | 50 | Psychic | 63 |
| Poison | 63 | Fire | 64 | Ice | 40 |
| Ground | 62 | Water | 134 | Dragon | 49 |
| Rock | 61 | | | Dark | 57 |
| | | | | Fairy | 39 |

## The screens

The TM database lives in the QUESTS screen ([quests.md](quests.md)), in the same layout. Once it's unlocked, the
header shows two tabs, QUESTS and TM RESEARCH, and L/R switch between them.

**TM RESEARCH** (the slots view):

- **Left**: TMs owned out of 51 with a bar, the 3 slots (a tracked TM's name in blue, red when ready; "Empty slot" in
  gray), and ALL TMs.
- **Right**, for a tracked slot: the TM's number and name, its %, "FIRE  POW 110  TIER 3", a bar, the two
  requirements with counters (`2/4`), what was caught and recorded so far ("Caught: Vulpix, Ponyta +1"), and "Ready to
  craft!" or the catch-them-all progress. For an empty slot: how research works. For ALL TMs: TMs owned, slots in
  use, the rules in short.
- **Bottom**: the keys for the row ("A Craft TM   SELECT Clear slot", "A Browse TMs"), a question ("Craft Fire Blast?
  A YES  B NO") or what just happened ("You made the TM! It's in your BAG.").
- A on an empty slot or ALL TMs opens the browse list; on a ready slot, craft; SELECT on a tracked slot, clear. B
  leaves (to the start menu, or to the field from the machine).

**The browse list** (the header reads TM DATABASE; the TM MACHINE opens straight into it):

- **Left**: the type filter (ALL, or a type: L/R or Left/Right cycle through the types that have TMs) with owned/total
  and a bar, then the TMs by number, five at a time. Available ones are dark, the tracked one blue, ready ones red,
  owned ones gray with a checkmark, the rest (type taken, no free slot) gray.
- **Right**: number and name, type, power and tier, the two requirements, the type's catch-them-all progress, and its
  status (Owned, Being researched, Ready to craft, Its type's slot is taken, All 3 slots are in use).
- A: research it (into the slot it was opened from, else the first free one: "Research Thunderbolt? A YES  B NO"),
  craft it if it's ready, or a short reason it can't. B goes back to the slots view.

## State

`gSaveBlock3Ptr->tmResearch[3]` (`struct TmResearchSlot` in `include/global.h`, 22 bytes each): the TM number (0 =
empty), and the species and moves counted (5 `u16` each, 0 = none yet). SaveBlock3 is 180 bytes (`test/save.c`); old
saves read zeros there, so all slots start empty. `FLAG_TM_DATABASE_UNLOCKED` (0x0B0) is the researcher's talk.
