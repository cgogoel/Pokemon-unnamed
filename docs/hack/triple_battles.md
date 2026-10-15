# 3v3 ("triple") battles — design and build plan

Custom battle mode for Act 2 onward. Not the Gen 5 Triple Battle: **only one Pokémon per side acts each turn.**

## Rules (decided)
- 3 Pokémon on the field per side; all 6 have their own HP, status, stat stages, etc.
- Each turn the player picks **which one of their 3** acts, then its move and target. The opponent side
  (AI) does the same. The other two on each side take no action that turn.
- Bag / Pokémon / Run count as the side's one action.

### Formation (near / middle / far)
- Each side's 3 Pokémon stand in a formation order: **near** (front), **middle**, **far** (back).
  Idea: shield a glass cannon behind a tank.
- Each move has a **reach**, decided by lore:
  - `FRONT`: the attack travels out from the user or makes contact, so it must hit the nearest foe still standing
    (punches, tackles, fire, water, beams, seeds, sprays, powders).
  - `ANY`: the attack can come from any direction, so any foe can be picked: Psychic, Electric, Flying and Ghost moves,
    sound moves, attacks from the sky or from under the target, homing moves that never miss, most status moves,
    and fast contact moves that dart around the front (Quick Attack, Extreme Speed, Aerial Ace...).
- **Reordering is free:** each turn, before choosing its action, a side may rearrange its 3 on-field Pokémon
  (near/middle/far) without giving up its attack. The new order applies before any moves that turn resolve.
- **Switching in from the bench costs the turn:** sending in a party Pokémon that isn't on the field is the side's one action.
- Starting order follows party order; a fainted Pokémon's replacement takes its spot.

### Area of effect
- Each move hits **1**, **2** or **3** foes.
  - **2:** the main target takes full damage, the second takes **50%**.
    - Reach `ANY` (lore says it can come from any direction: wind/Flying, electric, psychic, sound, falling from above, homing):
      the player picks both targets.
    - Reach `FRONT` (travels out from the user, e.g. fire, seeds, sprays): always hits the front and middle foes, no choice.
  - **3:** hits all foes for **75%** each.
  - No move hits the user's own allies: canon ally-hitting moves (Earthquake, Surf, Explosion, ...) hit only the 3 foes.
    The `hits_allies` column is kept (all `N`) in case a move should hit allies again.
- Per-move data lives in **`src/data/triple_battle_moves.csv`** (`aoe`, `hits_allies`, `reach` columns), hand-editable.
  The first pass was derived from canon targets, contact flags, descriptions and anime lore; each row has a `reason`.
  Moves not in the file (self/field/ally moves, Z/Max moves) keep their normal targeting.
- Out of scope: link battles, recorded battles, Battle Frontier facilities in triple mode.

### Move info indicator (UI)
Shown with each move in the battle move menu, before the move type (`ShowTripleMoveIndicator`, `src/triple_battle_ui.c`).
- A small copy of the foes' floor ovals: rear section on the left, middle, front section on the right.
- Each section is shaded by the damage that foe takes: dark = full, gray = reduced (50% or 75%), white = not hit.
  1 target: front dark. 2 targets: front dark, middle gray. 3 targets: all gray. (For ANY moves the dark section is
  whichever foe is picked.)
- Red arrows for reach: over the front section for `FRONT` moves, over all three for `ANY` moves, none for moves that
  hit all three foes.

### Field layout
- Each team stands on a diagonal: rear Pokémon on the left, front Pokémon on the right, so the two front Pokémon face off
  in the right-hand column. Spots: `sBattlerCoords[BATTLE_COORDS_TRIPLES]` (`src/battle_anim_mons.c`), indexed by layout
  position (left = rear, middle, right = front; see `GetBattlerLayoutPosition`). Reordering moves Pokémon between spots.
- Sprites are shrunk by depth (`sTripleSpriteScale`, `src/triple_battle_ui.c`): the player's rear Pokémon (closest to the
  camera) is biggest, the foes' rear Pokémon smallest. The pic is scaled in memory when it loads, so animations still use
  the normal sprite.
- The player's back sprites are cut off at the bottom (as in every Pokémon game), so the player's team is bottom-aligned
  with the cut behind the text box; its ovals sit in the strip just above the text box.
- Haze (`sTripleHaze`): farther Pokémon fade toward the floor color, strongest for the foes' rear Pokémon.
- Floor (`DrawTripleBattleFloor`): nested ovals along each team's diagonal on top of the location's background, with
  the 1v1 platforms painted out. A solid base oval under the rear Pokémon, a half-shaded section out to the middle one
  and an outlined outer ring out to the front one. Colors come from the location's floor color (same hue,
  more saturated, dark base oval to pale outer section; `sFloorShadeSteps`). The oval tiles use BG palette 6, a copy of
  the floor palette with these colors added. Not drawn while a terrain (Electric Terrain...) replaces the background.

### HP bars and name tags
- A short HP bar floats over each Pokémon's head, with a small status pill (`triple_status_pills.png`) on its left when
  it has a status condition. No HP numbers.
- A name tag (name, level or status) pops up over the Pokémon choosing an action or move, and over the target under the
  cursor. It's the compact 64x16 healthbox (`healthbox_triples.png`), hidden the rest of the time.
- These keep sprite memory free for move animations: six full healthboxes left too little.

### Player controls (action menu)
The prompt reads "What will Pikachu do?" with a hint line: `L R Pick`, `SELECT Order`.
- **L / R**: pick the Pokémon to the left / right on screen (toward the rear / the front) to act instead; wraps around.
  The engine still asks the side's Pokémon in battler order; the ones not picked pass or hand the choice back
  automatically (`StepTowardTriplePickedActor`).
- **SELECT**: move the selected Pokémon one spot forward in the formation (front wraps to back). Free.
- **B** (middle/right): go back to the previous Pokémon in the engine's order.
- Moves: FRONT and 3-target moves skip target selection; 2-target ANY moves pick two different foes.

### Trying it
- Debug menu (hold R + START in the overworld) → Party → *Start Debug Battle* starts a 3v3 against `DEBUG_TRAINER_AI`
  (`src/data/debug_trainers.party`).
- Trainers: `Battle Type: Triples` in `src/data/trainers.party`.

### Development aids
- Tests: `TRIPLE_BATTLE_TEST`, `AI_TRIPLE_BATTLE_TEST`, `MOVE(..., secondTarget: x)`, `FORMATION(near, middle, far)`
  (see `test/battle/triple_battle.c`).
- Screenshots without an emulator window: `SCREEN_DUMP(turn)`, `SCREEN_DUMP_FRAME(frame)` (a frame from the battle's
  start, e.g. the opening) or `UI_PREVIEW("A.D>D")` in a scratch test, then
  `python3 tools/screen_dump/render_screen_dump.py <make check log> <out dir>`.
- Animation check: set `T_SHOULD_RUN_MOVE_ANIM` to `TRUE` in `include/config/test.h` and run
  `make check TESTS="Move Animations don't leak when used - Triples"` (every move used by a middle battler, from the front
  and from the back row; several minutes). Set it back to `FALSE` afterwards.

## Engine approach
- New flag `BATTLE_TYPE_TRIPLE` (reuses the free bit 14) with `IsTripleBattle()`. It is included in
  `BATTLE_TYPE_MORE_THAN_TWO_BATTLERS`, so `IsDoubleBattle()` is also true and the multi-target code paths stay on.
- `MAX_BATTLERS_COUNT` 4 → 6. New positions `B_POSITION_PLAYER_MIDDLE = 4`, `B_POSITION_OPPONENT_MIDDLE = 5`
  (`pos & BIT_SIDE` still gives the side). `gBattlersCount` is 6 only in triples; singles 2, doubles 4.
- "Partner": `GetPartnerBattler()` in triples returns an ally (left↔right, middle → first alive flank) so
  existing single-ally code stays in bounds. Effects that should touch *both* allies are moved to ally loops case by case.
- Non-acting battlers use the existing no-op action `B_ACTION_NOTHING_FAINTED`.

## Phases
1. **Engine capacity (no behavior change):** MAX_BATTLERS_COUNT = 6 with every existing singles/doubles
   test still passing. Fixes: `gBattleCommunication` slot collision, `HITMARKER_FAINTED` bits, speed-tie
   permutation table, recorded-battle save struct, narrow bitfields, `GetTargetFromSlotId`, fixed `[4]` arrays,
   graphics tables gaining 5th/6th entries.
2. **Triple mode core logic + tests:** battle type, battler/position/party setup, partner rules, formation order,
   targeting from the move CSV (reach, 1/2/3 targets, 50% secondary damage), one-actor-per-side turn flow,
   faint replacement for 3 slots, exp.
   `TRIPLE_BATTLE_TEST` in the test runner so all of this is checked by `make check`.
3. **AI:** run move scoring for each of the AI's 3 battlers and act with the best-scoring one; switching logic for 3 on field.
4. **Graphics / UI:** sprite and healthbox layout for 6 (compact healthboxes for OBJ VRAM), intro send-out of 3,
   actor selection + 6-way target cursor, move info indicator (damage squares, reach outline, ally marker),
   party menu for 3 active, animation fixes (BG layering, partner anims).
5. **Content hooks:** `Battle Type: Triples` in `trainers.party` (trainerproc), `BattleSetup` for trainer triples,
   optional wild triples, Act gating (triples only from Act 2).

## Status
- [x] Phase 1 (engine supports 6 battlers; all existing tests pass)
- [x] Phase 2 (engine rules, covered by test/battle/triple_battle.c)
- [x] Phase 3 (AI picks its best actor; respects FRONT reach)
- [ ] Phase 4 (in progress: diagonal field layout with depth scaling, oval floor, floating HP bars and name tags,
  controls, target selection, move indicator and the animation check done; the in-battle party menu lists the 3 active
  Pokémon first but still uses the doubles layout)
- [ ] Phase 5 (in progress: `Battle Type: Triples` and the debug battle work; Act gating still to do)
