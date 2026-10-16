#ifndef GUARD_BEFRIEND_H
#define GUARD_BEFRIEND_H

// Befriending battles (Act 1): instead of catching, the player calms a wild Pokémon down. See docs/hack/befriending.md
// The Pokémon's HP stands for its aggression; at 0 it's befriended. Each action, offered item or Watch costs one
// patience; out of patience it runs away.

enum BefriendAction
{
    BEFRIEND_ACTION_CROUCH,
    BEFRIEND_ACTION_HUM,
    BEFRIEND_ACTION_HOLD_OUT_HAND,
    BEFRIEND_ACTION_APPROACH,
    BEFRIEND_ACTION_COUNT,
};

enum BefriendReaction
{
    BEFRIEND_REACTION_LOVED,
    BEFRIEND_REACTION_NEUTRAL,
    BEFRIEND_REACTION_DISLIKED,
};

#define BEFRIEND_DISLIKED_ITEMS 2

// What a Pokémon likes and dislikes. Choices not listed are neutral.
struct BefriendInfo
{
    u16 species;
    u16 lovedItem;
    u16 dislikedItems[BEFRIEND_DISLIKED_ITEMS];
    u8 lovedAction;         // enum BefriendAction
    u8 dislikedActions;     // Bit per enum BefriendAction
    u8 patience;            // Choices before it runs away
    u8 calmRate;            // Percent of the usual calming it takes (50 = twice as hard); raising isn't scaled
    const u8 *watchHint;    // Shown by Watch, a clue to what it likes
};

// Aggression change, in percent of the Pokémon's max HP (negative calms it down)
#define BEFRIEND_LOVED_ACTION_FIRST   -40 // The first time the loved action is used
#define BEFRIEND_LOVED_ACTION_REPEAT  -15 // Again: it still likes it, but less
#define BEFRIEND_LOVED_ITEM           -60
#define BEFRIEND_NEUTRAL              -10
#define BEFRIEND_DISLIKED             +25
#define BEFRIEND_DEFAULT_PATIENCE     5

extern bool8 gIsBefriendBattle;

static inline bool32 IsBefriendBattle(void)
{
    return gIsBefriendBattle;
}

const struct BefriendInfo *GetBefriendInfo(u16 species);
enum BefriendReaction GetBefriendActionReaction(const struct BefriendInfo *info, enum BefriendAction action);
enum BefriendReaction GetBefriendItemReaction(const struct BefriendInfo *info, u16 item);
s32 GetBefriendAggressionChange(const struct BefriendInfo *info, enum BefriendReaction reaction, bool32 isItem, bool32 repeatedLovedAction);
const u8 *GetBefriendActionName(enum BefriendAction action);
void InitBefriendBattle(void);
void HandleAction_BefriendAct(void);
void HandleAction_BefriendItem(void);
void HandleAction_BefriendWatch(void);
const u8 *TakeBefriendNextScript(void);
void RestoreBefriendedMonHp(void);

#endif // GUARD_BEFRIEND_H
