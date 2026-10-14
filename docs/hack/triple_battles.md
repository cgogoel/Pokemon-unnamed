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
- Changing formation order during battle is still to be designed (proposal: a "Shift" command that uses the side's turn).

### Area of effect
- Each move hits **1**, **2** or **3** foes.
  - **2:** the main target takes full damage, the second takes **50%**.
    - Reach `ANY` (lore says it can come from any direction: wind/Flying, electric, psychic, sound, falling from above, homing):
      the player picks both targets.
    - Reach `FRONT` (travels out from the user, e.g. fire, seeds, sprays): always hits the front and middle foes, no choice.
  - **3:** hits all foes for **75%** each.
  - `hits_allies = Y` moves (Earthquake, Explosion, ...) also hit the user's own allies, as in canon.
- Per-move data lives in **`src/data/triple_battle_moves.csv`** (`aoe`, `hits_allies`, `reach` columns), hand-editable.
  The first pass was derived from canon targets, contact flags, descriptions and anime lore; each row has a `reason`.
  Moves not in the file (self/field/ally moves, Z/Max moves) keep their normal targeting.
- Out of scope: link battles, recorded battles, Battle Frontier facilities in triple mode.

### Move info indicator (UI)
Shown with each move in the battle move menu (next to type/PP) and in the summary screen's move description.
- **Damage squares:** 3 small squares, one per target slot.
  - Dark = 100% damage, gray = reduced (50% or 75%), white = not hit.
  - 1 target: dark, white, white. 2 targets: dark, gray, white. 3 targets: gray, gray, gray.
- **Reach outline (red):**
  - `FRONT`: only the first square has a red outline.
  - `ANY`: every square has a red outline.
- **Ally marker:** a small extra icon (e.g. red "!") when the move also hits the user's own allies (`hits_allies = Y`).

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
- [ ] Phase 1
- [ ] Phase 2
- [ ] Phase 3
- [ ] Phase 4
- [ ] Phase 5
