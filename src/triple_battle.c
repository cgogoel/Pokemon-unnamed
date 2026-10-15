#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "event_data.h"
#include "pokemon_storage_system.h"
#include "sprite.h"
#include "triple_battle.h"
#include "constants/battle_ai.h"
#include "constants/moves.h"
#include "constants/story.h"

// 3v3 battles with one acting battler per side per turn. See docs/hack/triple_battles.md

#include "data/triple_battle_moves.h"

static bool32 IsFoeTargetingType(enum MoveTarget moveTarget);

static const struct TripleMoveInfo sUnlistedMoveInfo = {0};

// A side's battlers in formation pack order (left, right, then middle in 3v3). Returns how many (the side size).
static u32 GetTripleSideBattlers(enum BattleSide side, enum BattlerId *battlers)
{
    battlers[0] = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT | side);
    battlers[1] = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT | side);
    if (GetTripleSideSize() == 2)
        return 2;
    battlers[2] = GetBattlerAtPosition(B_POSITION_PLAYER_MIDDLE | side);
    return 3;
}

// The spot each rank is drawn at, from the front: in 2v2 the front and rear spots, with no middle.
static enum BattlerPosition GetTripleRankLayout(u32 rank)
{
    static const enum BattlerPosition sRankLayout3[TRIPLE_RANK_COUNT] = { B_POSITION_PLAYER_RIGHT, B_POSITION_PLAYER_MIDDLE, B_POSITION_PLAYER_LEFT };
    static const enum BattlerPosition sRankLayout2[2] = { B_POSITION_PLAYER_RIGHT, B_POSITION_PLAYER_LEFT };

    if (GetTripleSideSize() == 2)
        return sRankLayout2[rank % 2];
    return sRankLayout3[rank % TRIPLE_RANK_COUNT];
}

// A side's spots on screen from left (rear) to right (front). Returns how many.
u32 GetTripleScreenSpots(enum BattleSide side, enum BattlerPosition *spots)
{
    u32 size = GetTripleSideSize();
    for (u32 i = 0; i < size; i++)
        spots[i] = GetTripleRankLayout(size - 1 - i) | side;
    return size;
}

const struct TripleMoveInfo *GetTripleMoveInfo(enum Move move)
{
    if (move >= MOVES_COUNT)
        return &sUnlistedMoveInfo;
    return &gTripleMoveInfo[move];
}

// The starting formation follows party order: the lowest party slot on the field is near, the highest is far.
void InitTripleFormation(void)
{
    ResetTripleMoveIndicator();

    for (enum BattleSide side = B_SIDE_PLAYER; side < NUM_BATTLE_SIDES; side++)
    {
        enum BattlerId battlers[TRIPLE_RANK_COUNT];
        u32 size = GetTripleSideBattlers(side, battlers);

        for (u32 i = 0; i < size; i++)
        {
            u32 rank = 0;
            for (u32 j = 0; j < size; j++)
            {
                if (gBattlerPartyIndexes[battlers[j]] < gBattlerPartyIndexes[battlers[i]])
                    rank++;
            }
            gBattleStruct->tripleRank[battlers[i]] = rank;
        }
    }
}

// Applies a new formation for a side. Fails (and changes nothing) unless the ranks are a permutation of the side's
// ranks (near/middle/far, or near/far in 2v2, which ignores the middle rank in the packed byte).
bool32 TrySetTripleFormation(enum BattleSide side, u8 packedRanks)
{
    enum BattlerId battlers[TRIPLE_RANK_COUNT];
    u32 size = GetTripleSideBattlers(side, battlers);
    u32 seenRanks = 0;

    for (u32 i = 0; i < size; i++)
        seenRanks |= 1u << ((packedRanks >> (2 * i)) & 3);
    if (seenRanks != (1u << size) - 1)
        return FALSE;

    for (u32 i = 0; i < size; i++)
        gBattleStruct->tripleRank[battlers[i]] = (packedRanks >> (2 * i)) & 3;
    return TRUE;
}

enum TripleFormationRank GetTripleFormationRank(enum BattlerId battler)
{
    return gBattleStruct->tripleRank[battler];
}

// Where a battler is drawn. In triple battles that follows the formation: the near battler takes the spot closest to
// the middle of the field and the far one the spot at the edge, whatever its battle position.
enum BattlerPosition GetBattlerLayoutPosition(enum BattlerId battler)
{
    if (!IsTripleBattle() || gBattleStruct == NULL)
        return GetBattlerPosition(battler);
    return GetTripleRankLayout(GetTripleFormationRank(battler)) | GetBattlerSide(battler);
}

enum BattlerId GetBattlerAtLayoutPosition(enum BattlerPosition layoutPosition)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerLayoutPosition(battler) == layoutPosition)
            return battler;
    }
    return gBattlersCount;
}

// 2-target moves that can come from any direction: the player picks both targets.
bool32 IsTripleTwoTargetChoiceMove(enum Move move)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);
    return info->aoe == 2 && info->reach == TRIPLE_REACH_ANY;
}

enum TripleTargetChoice GetTripleTargetChoice(enum Move move, enum MoveTarget canonTarget)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);

    if (info->aoe == 0 || !IsFoeTargetingType(canonTarget))
        return TRIPLE_TARGET_CHOICE_CANON;
    if (IsTripleTwoTargetChoiceMove(move))
        return TRIPLE_TARGET_CHOICE_TWO;
    if (info->aoe == 1 && info->reach == TRIPLE_REACH_ANY)
        return TRIPLE_TARGET_CHOICE_ONE;
    return TRIPLE_TARGET_CHOICE_NONE;
}

// Each side chooses its action left, middle, right. Returns the battler choosing before this one (skipping absent
// battlers), or MAX_BATTLERS_COUNT.
enum BattlerId GetTriplePreviousChoosingBattler(enum BattlerId battler)
{
    enum BattleSide side = GetBattlerSide(battler);
    const enum BattlerPosition order[] = { B_POSITION_PLAYER_LEFT | side, B_POSITION_PLAYER_MIDDLE | side, B_POSITION_PLAYER_RIGHT | side };
    s32 index = ARRAY_COUNT(order) - 1;

    while (index >= 0 && GetBattlerAtPosition(order[index]) != battler)
        index--;
    for (index--; index >= 0; index--)
    {
        enum BattlerId previous = GetBattlerAtPosition(order[index]);
        if (previous < gBattlersCount && !(gAbsentBattlerFlags & (1u << previous)))
            return previous;
    }
    return MAX_BATTLERS_COUNT;
}

// Order in which a side's battlers choose their action.
static s32 GetTripleChoosingIndex(enum BattlerId battler)
{
    enum BattlerPosition position = GetBattlerPosition(battler);
    if ((position & ~BIT_SIDE) == B_POSITION_PLAYER_LEFT)
        return 0;
    if ((position & ~BIT_SIDE) == B_POSITION_PLAYER_MIDDLE)
        return 1;
    return 2;
}

// Whether battlerTo chooses after battlerFrom.
bool32 DoesTripleBattlerChooseLater(enum BattlerId battlerFrom, enum BattlerId battlerTo)
{
    return GetTripleChoosingIndex(battlerTo) > GetTripleChoosingIndex(battlerFrom);
}

// The ally on screen to the left (step = -1, toward the rear) or right (step = 1, toward the front), wrapping around;
// MAX_BATTLERS_COUNT if there's none.
enum BattlerId GetTripleScreenNeighbor(enum BattlerId battler, s32 step)
{
    enum BattlerPosition spots[TRIPLE_RANK_COUNT];
    s32 size = GetTripleScreenSpots(GetBattlerSide(battler), spots);
    s32 index;

    for (index = 0; index < size && GetBattlerAtLayoutPosition(spots[index]) != battler; index++)
        ;
    if (index == size)
        return MAX_BATTLERS_COUNT;
    for (s32 i = 1; i < size; i++)
    {
        enum BattlerId neighbor = GetBattlerAtLayoutPosition(spots[(index + step * i + 2 * size) % size]);
        if (neighbor < gBattlersCount && !(gAbsentBattlerFlags & (1u << neighbor)) && IsBattlerAlive(neighbor))
            return neighbor;
    }
    return MAX_BATTLERS_COUNT;
}

// The formation after moving a battler one spot forward (the near battler wraps around to the far spot).
u8 GetTripleFormationMovingUp(enum BattlerId battler)
{
    enum BattlerId battlers[TRIPLE_RANK_COUNT];
    u32 size = GetTripleSideBattlers(GetBattlerSide(battler), battlers);
    u32 ranks[TRIPLE_RANK_COUNT] = {0};
    u32 rank = GetTripleFormationRank(battler);
    u32 aheadRank = (rank + size - 1) % size;

    for (u32 i = 0; i < size; i++)
    {
        ranks[i] = GetTripleFormationRank(battlers[i]);
        if (ranks[i] == aheadRank)
            ranks[i] = rank;
        else if (battlers[i] == battler)
            ranks[i] = aheadRank;
    }
    return TRIPLE_FORMATION_PACK(ranks[0], ranks[1], ranks[2]);
}

// Moves a side's sprites and healthboxes to match a new formation.
void UpdateTripleFormationGraphics(enum BattleSide side)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        if (GetBattlerSide(battler) != side || !IsBattlerSpritePresent(battler))
            continue;

        struct Sprite *sprite = &gSprites[gBattlerSpriteIds[battler]];
        // Sprites are scaled by depth, so the pic is reloaded for its new spot
        BattleLoadSubstituteOrMonSpriteGfx(battler, !gBattleSpritesDataPtr->battlerData[battler].behindSubstitute);
        DmaCopy32(3, gMonSpritesGfxPtr->spritesGfx[GetBattlerPosition(battler)], (void *)(OBJ_VRAM0 + sprite->oam.tileNum * TILE_SIZE_4BPP), MON_PIC_SIZE);
        sprite->x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X_2);
        sprite->y = GetBattlerSpriteDefault_Y(battler);
        sprite->subpriority = GetBattlerSpriteSubpriority(battler);
        InitBattlerHealthboxCoords(battler);
    }
}

// Returns the alive foe of battlerAtk that is aliveIndex-th from the front, or MAX_BATTLERS_COUNT.
enum BattlerId GetTripleFoeAtRank(enum BattlerId battlerAtk, u32 aliveIndex)
{
    for (u32 rank = TRIPLE_RANK_NEAR; rank < GetTripleSideSize(); rank++)
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

// The foes a 2-target move can hit along with the main target: its neighbours in the foes' line (counting only foes
// still standing). Returns how many (0-2); the one nearer the front comes first.
u32 GetTripleSecondTargetChoices(enum BattlerId battlerAtk, enum BattlerId mainTarget, enum BattlerId *choices)
{
    enum BattlerId line[TRIPLE_RANK_COUNT];
    u32 count = 0, mainIndex = TRIPLE_RANK_COUNT, choiceCount = 0;

    for (u32 rank = 0; rank < GetTripleSideSize(); rank++)
    {
        enum BattlerId foe = GetTripleFoeAtRank(battlerAtk, rank);
        if (foe == MAX_BATTLERS_COUNT)
            break;
        if (foe == mainTarget)
            mainIndex = count;
        line[count++] = foe;
    }
    if (mainIndex == TRIPLE_RANK_COUNT)
        return 0;
    if (mainIndex > 0)
        choices[choiceCount++] = line[mainIndex - 1];
    if (mainIndex + 1 < count)
        choices[choiceCount++] = line[mainIndex + 1];
    return choiceCount;
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
            // The second target is next to the main one in the foes' line: the middle foe for the front or rear one,
            // and for the middle foe whichever neighbour was picked (the front one by default).
            enum BattlerId choices[2];
            u32 count = GetTripleSecondTargetChoices(battlerAtk, main, choices);
            second = (count == 0) ? MAX_BATTLERS_COUNT : choices[0];
            if (count == 2 && chosenSecond != 0 && chosenSecond - 1 == choices[1])
                second = choices[1];
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

// AI: scores every AI battler on the side once per turn and picks the one with the best move to act.
// If the best move is weak and a battler wants to switch out, that battler acts (by switching) instead.
void ChooseTripleAiActor(enum BattlerId battler, bool32 (*isAiBattler)(enum BattlerId))
{
    enum BattleSide side = GetBattlerSide(battler);
    enum BattlerId actor = battler, switcher = MAX_BATTLERS_COUNT;
    s32 bestScore = -1;

    if (gBattleStruct->tripleAiDecidedTurn[side] == gBattleTurnCounter + 1)
        return;
    gBattleStruct->tripleAiDecidedTurn[side] = gBattleTurnCounter + 1;

    // A battler locked into a move or recharging acts no matter what.
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
    {
        if (GetBattlerSide(i) == side && IsBattlerAlive(i)
         && (gBattleMons[i].volatiles.multipleTurns || gBattleMons[i].volatiles.rechargeTimer > 0))
        {
            gBattleStruct->tripleAiActor[side] = i;
            return;
        }
    }

    for (enum BattlerId i = 0; i < gBattlersCount; i++)
    {
        if (GetBattlerSide(i) != side || !IsBattlerAlive(i) || gAbsentBattlerFlags & (1u << i) || !isAiBattler(i))
            continue;

        ComputeAiBattlerDecisions(i);
        u32 target = gAiBattleData->chosenTarget[i];
        u32 moveIndex = gAiBattleData->chosenMoveIndex[i];
        s32 score = (target < gBattlersCount && moveIndex < MAX_MON_MOVES) ? gAiBattleData->finalScore[i][target][moveIndex] : 0;

        if (score > bestScore)
        {
            bestScore = score;
            actor = i;
        }
        if (switcher == MAX_BATTLERS_COUNT && gAiLogicData->shouldSwitch & (1u << i))
            switcher = i;
    }

    if (switcher != MAX_BATTLERS_COUNT && bestScore < AI_SCORE_DEFAULT)
        actor = switcher;
    gBattleStruct->tripleAiActor[side] = actor;

    // The AI's shared working data now belongs to the last battler scored; redo the actor so its action uses its own.
    ComputeAiBattlerDecisions(actor);
}

bool32 IsTripleAiPassing(enum BattlerId battler)
{
    enum BattleSide side = GetBattlerSide(battler);
    return gBattleStruct->tripleAiDecidedTurn[side] == gBattleTurnCounter + 1 && gBattleStruct->tripleAiActor[side] != battler;
}

// AI: for a 2-target move aimed at the middle foe, the neighbour to also hit (for half damage), encoded for the chosen
// target byte: one it can knock out, otherwise the one losing the biggest share of its HP. 0 if there's no choice.
u32 GetTripleAiSecondTargetByte(enum BattlerId battlerAtk, enum BattlerId mainTarget, u32 moveIndex)
{
    enum BattlerId choices[2];
    s32 bestScore = -1;
    enum BattlerId best = MAX_BATTLERS_COUNT;

    if (moveIndex >= MAX_MON_MOVES || !IsTripleTwoTargetChoiceMove(gBattleMons[battlerAtk].moves[moveIndex])
     || GetTripleSecondTargetChoices(battlerAtk, mainTarget, choices) < 2)
        return 0;

    for (u32 i = 0; i < 2; i++)
    {
        u32 hp = max(gBattleMons[choices[i]].hp, 1);
        u32 damage = gAiLogicData->simulatedDmg[battlerAtk][choices[i]][moveIndex].median / 2;
        s32 score = (damage >= hp) ? 1000 : (damage * 100) / hp;
        if (score > bestScore)
        {
            bestScore = score;
            best = choices[i];
        }
    }
    return TRIPLE_TARGET_ENCODE_SECOND(best);
}

// AI: a FRONT move aimed at a foe that isn't in front would hit the front foe instead, so it's worthless against that foe.
void RemoveUnreachableTripleMoveScores(enum BattlerId battlerAtk, enum BattlerId battlerDef, s32 *scores)
{
    if (IsBattlerAlly(battlerAtk, battlerDef) || battlerDef == GetTripleFoeAtRank(battlerAtk, 0))
        return;

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        const struct TripleMoveInfo *info = GetTripleMoveInfo(gBattleMons[battlerAtk].moves[i]);
        if (info->aoe != 0 && info->aoe != 3 && info->reach == TRIPLE_REACH_FRONT)
            scores[i] = 0;
    }
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

// Battle type flags for a trainer set to doubles or triples. From Act 2 on these use the 3v3 rules, with as many
// Pokémon per side as both sides can field (up to 3): the player's Pokémon able to battle and the trainer's party.
// 2 each is a 2v2 with the 3v3 rules, 1 each a single battle. Before Act 2 they're double battles.
u32 GetTripleTrainerBattleTypeFlags(u16 trainerId)
{
    return GetTripleBattleTypeFlagsForFoes(GetTrainerPartySizeFromId(trainerId));
}

u32 GetTripleBattleTypeFlagsForFoes(u32 foePartySize)
{
    u32 size;

    // Two trainers at once or a partner: a normal double battle
    if (VarGet(VAR_STORY_ACT) < STORY_ACT_UNIVERSITY || gBattleTypeFlags & (BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_PLAYER_HAS_PARTNER))
        return BATTLE_TYPE_DOUBLE;

    size = min(CountPartyAliveNonEggMonsExcept(PARTY_SIZE), foePartySize);
    if (size >= 3)
        return BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE;
    if (size == 2)
        return BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE | BATTLE_TYPE_TRIPLE_2V2;
    return 0;
}
