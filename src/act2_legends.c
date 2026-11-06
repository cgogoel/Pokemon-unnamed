#include "global.h"
#include "act2_legends.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sprite.h"
#include "triple_battle.h"
#include "constants/battle.h"
#include "constants/battle_anim.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/story.h"

// Act 2's finale: the legendary trio's battle (docs/hack/act2_finale.md, include/act2_legends.h).

EWRAM_DATA bool8 gIsAct2LegendsBattle = FALSE;

struct Act2Legend
{
    u16 species;
    u16 moves[MAX_MON_MOVES];
};

// Party order is formation order: ENTEI in front, SUICUNE in the middle, VIRIZION at the back.
// The order matches enum TriBeamKind: each one fires the beam of its type in the joint attack.
static const struct Act2Legend sAct2Legends[TRI_BEAM_COUNT] =
{
    [TRI_BEAM_FIRE]  = {SPECIES_ENTEI,    {MOVE_FIRE_BLAST, MOVE_EXTRASENSORY, MOVE_CRUNCH, MOVE_STOMP}},
    [TRI_BEAM_WATER] = {SPECIES_SUICUNE,  {MOVE_HYDRO_PUMP, MOVE_EXTRASENSORY, MOVE_ICE_BEAM, MOVE_CALM_MIND}},
    [TRI_BEAM_GRASS] = {SPECIES_VIRIZION, {MOVE_LEAF_BLADE, MOVE_SACRED_SWORD, MOVE_GIGA_DRAIN, MOVE_SWORDS_DANCE}},
};

void CB2_EndAct2LegendsBattle(void)
{
    CpuFill16(0, (void *)(BG_PLTT), BG_PLTT_SIZE);
    ResetOamRange(0, 128);
    gIsAct2LegendsBattle = FALSE;
    gSpecialVar_Result = gBattleOutcome;
    // Never a blackout: the script goes on to Dad and Mew whatever happened
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// The trio in the wild party, for StartAct2LegendsBattle (src/battle_setup.c)
void CreateAct2LegendsParty(void)
{
    ZeroEnemyPartyMons();
    for (u32 i = 0; i < TRI_BEAM_COUNT; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];

        CreateRandomMon(mon, sAct2Legends[i].species, ACT2_LEGENDS_LEVEL);
        for (u32 j = 0; j < MAX_MON_MOVES; j++)
            SetMonMoveSlot(mon, sAct2Legends[i].moves[j], j);
    }
}

// Called first thing at the end of turn 1 (EndTurnEvents): once the player's action is done, the trio attacks together
// and the battle ends. BattleScript_Act2LegendsTrioAttack sets gBattleOutcome, so this runs once.
bool32 TryAct2LegendsTrioAttack(void)
{
    u32 from, to;

    if (!IsAct2LegendsBattle() || gBattleOutcome != 0)
        return FALSE;

    GetTriBeamBattlers(TRI_BEAM_FROM_TRIO, TRI_BEAM_WATER, &from, &to);
    gBattlerAttacker = from;
    gBattlerTarget = to;
    BattleScriptCall(BattleScript_Act2LegendsTrioAttack);
    return TRUE;
}

static u32 GetOnFieldTrioMember(enum TriBeamKind kind)
{
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (!IsOnPlayerSide(battler) && IsBattlerAlive(battler) && gBattleMons[battler].species == sAct2Legends[kind].species)
            return battler;
    }
    return MAX_BATTLERS_COUNT;
}

// The beams' ends. For the trio: the trio member of that type, aimed at the player's Pokémon in the same formation rank
// (the beams run side by side), or another of the player's Pokémon when that spot is empty.
void GetTriBeamBattlers(u32 source, enum TriBeamKind kind, u32 *from, u32 *to)
{
    *from = gBattleAnimAttacker;
    *to = gBattleAnimTarget;
    if (source != TRI_BEAM_FROM_TRIO)
        return;

    u32 legend = GetOnFieldTrioMember(kind);
    if (legend == MAX_BATTLERS_COUNT)
    {
        // Not on the field (a smaller battle): fire from whichever trio member is
        for (u32 i = 0; i < TRI_BEAM_COUNT && legend == MAX_BATTLERS_COUNT; i++)
            legend = GetOnFieldTrioMember(i);
        if (legend == MAX_BATTLERS_COUNT)
            return;
    }
    *from = legend;

    u32 rank = IsTripleBattle() ? GetTripleFormationRank(legend) : 0;
    u32 fallback = MAX_BATTLERS_COUNT;
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (!IsOnPlayerSide(battler) || !IsBattlerAlive(battler))
            continue;
        if (!IsTripleBattle() || GetTripleFormationRank(battler) == rank)
        {
            *to = battler;
            return;
        }
        if (fallback == MAX_BATTLERS_COUNT)
            fallback = battler;
    }
    if (fallback != MAX_BATTLERS_COUNT)
        *to = fallback;
}
