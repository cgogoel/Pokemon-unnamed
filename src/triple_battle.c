#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "triple_battle.h"
#include "constants/moves.h"

// 3v3 battles with one acting battler per side per turn. See docs/hack/triple_battles.md

#include "data/triple_battle_moves.h"

static const struct TripleMoveInfo sUnlistedMoveInfo = {0};

const struct TripleMoveInfo *GetTripleMoveInfo(enum Move move)
{
    if (move >= MOVES_COUNT)
        return &sUnlistedMoveInfo;
    return &gTripleMoveInfo[move];
}

// The starting formation follows party order: the lowest party slot on the field is near, the highest is far.
void InitTripleFormation(void)
{
    for (enum BattleSide side = B_SIDE_PLAYER; side < NUM_BATTLE_SIDES; side++)
    {
        enum BattlerId battlers[TRIPLE_RANK_COUNT] =
        {
            GetBattlerAtPosition(B_POSITION_PLAYER_LEFT | side),
            GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT | side),
            GetBattlerAtPosition(B_POSITION_PLAYER_MIDDLE | side),
        };

        for (u32 i = 0; i < TRIPLE_RANK_COUNT; i++)
        {
            u32 rank = 0;
            for (u32 j = 0; j < TRIPLE_RANK_COUNT; j++)
            {
                if (gBattlerPartyIndexes[battlers[j]] < gBattlerPartyIndexes[battlers[i]])
                    rank++;
            }
            gBattleStruct->tripleRank[battlers[i]] = rank;
        }
    }
}

// Applies a new formation for a side. Fails (and changes nothing) unless the ranks are a permutation of near/middle/far.
bool32 TrySetTripleFormation(enum BattleSide side, u8 packedRanks)
{
    const enum BattlerPosition positions[TRIPLE_RANK_COUNT] = { B_POSITION_PLAYER_LEFT | side, B_POSITION_PLAYER_RIGHT | side, B_POSITION_PLAYER_MIDDLE | side };
    u32 seenRanks = 0;

    for (u32 i = 0; i < TRIPLE_RANK_COUNT; i++)
        seenRanks |= 1u << ((packedRanks >> (2 * i)) & 3);
    if (seenRanks != (1u << TRIPLE_RANK_COUNT) - 1)
        return FALSE;

    for (u32 i = 0; i < TRIPLE_RANK_COUNT; i++)
        gBattleStruct->tripleRank[GetBattlerAtPosition(positions[i])] = (packedRanks >> (2 * i)) & 3;
    return TRUE;
}

enum TripleFormationRank GetTripleFormationRank(enum BattlerId battler)
{
    return gBattleStruct->tripleRank[battler];
}

// Returns the alive foe of battlerAtk that is aliveIndex-th from the front, or MAX_BATTLERS_COUNT.
enum BattlerId GetTripleFoeAtRank(enum BattlerId battlerAtk, u32 aliveIndex)
{
    for (enum TripleFormationRank rank = TRIPLE_RANK_NEAR; rank < TRIPLE_RANK_COUNT; rank++)
    {
        for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
        {
            if (IsBattlerAlly(battler, battlerAtk) || !IsBattlerAlive(battler) || GetTripleFormationRank(battler) != rank)
                continue;
            if (aliveIndex == 0)
                return battler;
            aliveIndex--;
        }
    }
    return MAX_BATTLERS_COUNT;
}

static bool32 IsFoeTargetingType(enum MoveTarget moveTarget)
{
    switch (moveTarget)
    {
    case TARGET_SELECTED:
    case TARGET_OPPONENT:
    case TARGET_SMART:
    case TARGET_BOTH:
    case TARGET_FOES_AND_ALLY:
        return TRUE;
    default:
        return FALSE;
    }
}

// 3-target moves use the canon spread types; 2-target moves are spread moves whose excluded foe is marked as done.
enum MoveTarget GetTripleMoveTargetType(enum Move move, enum MoveTarget canonTarget)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);

    if (info->aoe == 0 || !IsFoeTargetingType(canonTarget))
        return canonTarget;

    switch (info->aoe)
    {
    case 3:
        return info->hitsAllies ? TARGET_FOES_AND_ALLY : TARGET_BOTH;
    case 2:
        return TARGET_BOTH;
    default:
        return TARGET_SELECTED;
    }
}

static bool32 IsAliveFoe(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    return battlerDef < gBattlersCount && !IsBattlerAlly(battlerAtk, battlerDef) && IsBattlerAlive(battlerDef);
}

// Picks the targets of a listed move from its reach and the chosen target, and returns the main target.
enum BattlerId SetTripleMoveTargets(enum BattlerId battlerAtk, enum Move move, enum BattlerId battlerDef)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);
    enum BattlerId front = GetTripleFoeAtRank(battlerAtk, 0);
    enum BattlerId main = MAX_BATTLERS_COUNT, second = MAX_BATTLERS_COUNT;
    u32 chosenSecond = gBattleStruct->tripleSecondTarget[battlerAtk];

    gBattleStruct->tripleMainTarget[battlerAtk] = MAX_BATTLERS_COUNT;
    if (info->aoe == 0 || front == MAX_BATTLERS_COUNT || !IsFoeTargetingType(GetBattlerMoveTargetType(battlerAtk, move)))
        return battlerDef;

    if (info->aoe == 3)
        return front;

    if (info->reach == TRIPLE_REACH_FRONT)
    {
        main = front;
        if (info->aoe == 2)
            second = GetTripleFoeAtRank(battlerAtk, 1);
    }
    else
    {
        main = IsAliveFoe(battlerAtk, battlerDef) ? battlerDef : front;
        if (info->aoe == 2)
        {
            if (chosenSecond != 0 && IsAliveFoe(battlerAtk, chosenSecond - 1) && chosenSecond - 1 != main)
                second = chosenSecond - 1;
            else
                second = (main == front) ? GetTripleFoeAtRank(battlerAtk, 1) : front;
        }
    }

    gBattleStruct->tripleMainTarget[battlerAtk] = main;
    gBattleStruct->tripleHitSecond[battlerAtk] = second;
    return main;
}

// Called after the canon target marking: excludes the foe a 2-target move doesn't hit.
void MarkTripleExcludedTargets(enum BattlerId battlerAtk, enum Move move)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);

    if (info->aoe != 2 || gBattleStruct->tripleMainTarget[battlerAtk] == MAX_BATTLERS_COUNT)
        return;

    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (IsBattlerAlly(battler, battlerAtk))
            continue;
        if (battler != gBattleStruct->tripleMainTarget[battlerAtk] && battler != gBattleStruct->tripleHitSecond[battlerAtk])
            gBattleStruct->battlerState[battlerAtk].targetsDone[battler] = TRUE;
    }
}

// Number of battlers a spread move will hit, after exclusions.
u32 CountTripleSpreadTargets(enum BattlerId battlerAtk)
{
    u32 count = 0;
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (battler != battlerAtk && IsBattlerAlive(battler) && !gBattleStruct->battlerState[battlerAtk].targetsDone[battler])
            count++;
    }
    return count;
}

// 2-target moves: the main target takes full damage and the second takes half. 3-target moves: 75% each.
uq4_12_t GetTripleTargetDamageModifier(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 targetCount)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);

    if (targetCount < 2)
        return UQ_4_12(1.0);
    if (info->aoe == 2)
        return battlerDef == gBattleStruct->tripleMainTarget[battlerAtk] ? UQ_4_12(1.0) : UQ_4_12(0.5);
    return UQ_4_12(0.75);
}
