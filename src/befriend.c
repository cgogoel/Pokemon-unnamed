#include "global.h"
#include "battle.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "befriend.h"
#include "item.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/battle_string_ids.h"
#include "constants/items.h"
#include "constants/species.h"

// Befriending battles (Act 1). See docs/hack/befriending.md

EWRAM_DATA bool8 gIsBefriendBattle = FALSE;

#include "data/befriend.h"

// What the player does, for "You {action}."
static const u8 *const sBefriendActionTexts[BEFRIEND_ACTION_COUNT] =
{
    [BEFRIEND_ACTION_CROUCH]        = COMPOUND_STRING("crouched down"),
    [BEFRIEND_ACTION_HUM]           = COMPOUND_STRING("hummed a tune"),
    [BEFRIEND_ACTION_HOLD_OUT_HAND] = COMPOUND_STRING("held out a hand"),
    [BEFRIEND_ACTION_APPROACH]      = COMPOUND_STRING("approached slowly"),
};

// Names in the Act menu
static const u8 *const sBefriendActionNames[BEFRIEND_ACTION_COUNT] =
{
    [BEFRIEND_ACTION_CROUCH]        = COMPOUND_STRING("Crouch down"),
    [BEFRIEND_ACTION_HUM]           = COMPOUND_STRING("Hum a tune"),
    [BEFRIEND_ACTION_HOLD_OUT_HAND] = COMPOUND_STRING("Reach out"),
    [BEFRIEND_ACTION_APPROACH]      = COMPOUND_STRING("Approach"),
};

// Jangmo-o, while the player carries the Hard Stone Dad gave them "in case it attacks" (docs/hack/act1_story.md):
// this Act slot becomes Throw stone. Jangmo-o hits back and the player blacks out; offering the stone works instead.
#define BEFRIEND_THROW_STONE_SLOT BEFRIEND_ACTION_HUM

// The battle script to continue with once the reaction is shown; see BS_BefriendFinish
static EWRAM_DATA const u8 *sBefriendNextScript = NULL;

const struct BefriendInfo *GetBefriendInfo(u16 species)
{
    for (u32 i = 0; i < ARRAY_COUNT(sBefriendInfo); i++)
    {
        if (sBefriendInfo[i].species == species)
            return &sBefriendInfo[i];
    }
    return &sBefriendDefault;
}

static enum BattlerId GetBefriendTarget(void)
{
    return GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
}

bool32 IsBefriendThrowStoneOffered(void)
{
    return gBattleMons[GetBefriendTarget()].species == SPECIES_JANGMO_O && CheckBagHasItem(ITEM_HARD_STONE, 1);
}

const u8 *GetBefriendActionName(enum BefriendAction action)
{
    if (action == BEFRIEND_THROW_STONE_SLOT && IsBefriendThrowStoneOffered())
        return COMPOUND_STRING("Throw stone");
    return (action < BEFRIEND_ACTION_COUNT) ? sBefriendActionNames[action] : COMPOUND_STRING("");
}

enum BefriendReaction GetBefriendActionReaction(const struct BefriendInfo *info, enum BefriendAction action)
{
    if (action == info->lovedAction)
        return BEFRIEND_REACTION_LOVED;
    if (action < BEFRIEND_ACTION_COUNT && (info->dislikedActions & (1 << action)))
        return BEFRIEND_REACTION_DISLIKED;
    return BEFRIEND_REACTION_NEUTRAL;
}

enum BefriendReaction GetBefriendItemReaction(const struct BefriendInfo *info, u16 item)
{
    if (item != ITEM_NONE && item == info->lovedItem)
        return BEFRIEND_REACTION_LOVED;
    for (u32 i = 0; i < BEFRIEND_DISLIKED_ITEMS; i++)
    {
        if (item != ITEM_NONE && item == info->dislikedItems[i])
            return BEFRIEND_REACTION_DISLIKED;
    }
    return BEFRIEND_REACTION_NEUTRAL;
}

// In percent of max HP; negative calms it down, scaled by how easily this Pokémon calms down.
s32 GetBefriendAggressionChange(const struct BefriendInfo *info, enum BefriendReaction reaction, bool32 isItem)
{
    s32 change;

    switch (reaction)
    {
    case BEFRIEND_REACTION_LOVED:
        change = isItem ? BEFRIEND_LOVED_ITEM : BEFRIEND_LOVED_ACTION;
        break;
    case BEFRIEND_REACTION_DISLIKED:
        return BEFRIEND_DISLIKED;
    default:
        change = BEFRIEND_NEUTRAL;
        break;
    }
    return change * info->calmRate / 100;
}

static const struct BefriendInfo *GetTargetBefriendInfo(void)
{
    return GetBefriendInfo(gBattleMons[GetBefriendTarget()].species);
}

void InitBefriendBattle(void)
{
    gBattleStruct->befriendPatience = GetBefriendInfo(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES))->patience;
    gBattleStruct->befriendLovedActionUsed = FALSE;
}

// Uses up one patience and picks what follows the reaction: befriended, ran away, a warning, or nothing.
static void SetBefriendOutcome(s32 percentChange)
{
    enum BattlerId target = GetBefriendTarget();
    s32 change = (gBattleMons[target].maxHP * percentChange) / 100;
    s32 hp;

    if (percentChange < 0 && change == 0)
        change = -1;
    hp = gBattleMons[target].hp + change;
    if (hp < 0)
        hp = 0;
    if (hp > gBattleMons[target].maxHP)
        hp = gBattleMons[target].maxHP;

    gBattlerTarget = target;
    gBattleStruct->passiveHpUpdate[target] = gBattleMons[target].hp - hp; // Positive lowers the bar
    if (gBattleStruct->befriendPatience > 0)
        gBattleStruct->befriendPatience--;

    if (hp == 0)
        sBefriendNextScript = BattleScript_BefriendSuccess;
    else if (gBattleStruct->befriendPatience == 0)
        sBefriendNextScript = BattleScript_BefriendFled;
    else if (gBattleStruct->befriendPatience == 1)
        sBefriendNextScript = BattleScript_BefriendImpatient;
    else
        sBefriendNextScript = BattleScript_BefriendEnd;
}

void HandleAction_BefriendAct(void)
{
    const struct BefriendInfo *info = GetTargetBefriendInfo();
    enum BefriendAction action = gBattleStruct->befriendChoice;
    enum BefriendReaction reaction = GetBefriendActionReaction(info, action);

    gBattlerAttacker = gBattlerByTurnOrder[gCurrentTurnActionNumber];
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    if (action == BEFRIEND_THROW_STONE_SLOT && IsBefriendThrowStoneOffered())
    {
        gBattlerTarget = GetBefriendTarget();
        gBattlescriptCurrInstr = BattleScript_BefriendThrowStone;
        gCurrentActionFuncId = B_ACTION_EXEC_SCRIPT;
        return;
    }
    // The loved action only works its magic once
    if (reaction == BEFRIEND_REACTION_LOVED)
    {
        if (gBattleStruct->befriendLovedActionUsed)
            reaction = BEFRIEND_REACTION_NEUTRAL;
        gBattleStruct->befriendLovedActionUsed = TRUE;
    }

    StringCopy(gStringVar2, (action < BEFRIEND_ACTION_COUNT) ? sBefriendActionTexts[action] : sBefriendActionTexts[0]);
    gBattleCommunication[MULTISTRING_CHOOSER] = reaction;
    SetBefriendOutcome(GetBefriendAggressionChange(info, reaction, FALSE));
    gBattlescriptCurrInstr = BattleScript_BefriendAct;
    gCurrentActionFuncId = B_ACTION_EXEC_SCRIPT;
}

void HandleAction_BefriendItem(void)
{
    u16 item = gBattleStruct->befriendChoice;
    const struct BefriendInfo *info = GetTargetBefriendInfo();
    enum BefriendReaction reaction = GetBefriendItemReaction(info, item);

    gBattlerAttacker = gBattlerByTurnOrder[gCurrentTurnActionNumber];
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    gLastUsedItem = item;
    if (item != ITEM_NONE)
        RemoveBagItem(item, 1); // Offered items are used up

    gBattleCommunication[MULTISTRING_CHOOSER] = reaction;
    SetBefriendOutcome(GetBefriendAggressionChange(info, reaction, TRUE));
    gBattlescriptCurrInstr = BattleScript_BefriendItem;
    gCurrentActionFuncId = B_ACTION_EXEC_SCRIPT;
}

// Watch: a clue to what it likes, for one patience. Aggression doesn't change.
void HandleAction_BefriendWatch(void)
{
    gBattlerAttacker = gBattlerByTurnOrder[gCurrentTurnActionNumber];
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    StringCopy(gStringVar1, GetTargetBefriendInfo()->watchHint);
    SetBefriendOutcome(0);
    gBattlescriptCurrInstr = BattleScript_BefriendWatch;
    gCurrentActionFuncId = B_ACTION_EXEC_SCRIPT;
}

// After the reaction: the script SetBefriendOutcome picked (see BS_BefriendFinish).
const u8 *TakeBefriendNextScript(void)
{
    const u8 *script = sBefriendNextScript;
    sBefriendNextScript = NULL;
    return script;
}

// The befriended Pokémon joins at full health (its HP bar was its aggression).
void RestoreBefriendedMonHp(void)
{
    enum BattlerId target = GetBefriendTarget();
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][gBattlerPartyIndexes[target]];
    u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);

    SetMonData(mon, MON_DATA_HP, &maxHP);
    gBattleMons[target].hp = maxHP;
}
