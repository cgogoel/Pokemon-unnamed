#ifndef GUARD_TRIPLE_BATTLE_H
#define GUARD_TRIPLE_BATTLE_H

// 3v3 battles with one acting battler per side per turn. See docs/hack/triple_battles.md

enum TripleReach
{
    TRIPLE_REACH_FRONT, // Must hit the nearest foe still standing (or the front two for 2-target moves).
    TRIPLE_REACH_ANY,   // Can target any foe.
};

enum TripleFormationRank
{
    TRIPLE_RANK_NEAR,
    TRIPLE_RANK_MIDDLE,
    TRIPLE_RANK_FAR,
    TRIPLE_RANK_COUNT,
};

// Per-move data from src/data/triple_battle_moves.csv. aoe == 0 means the move isn't listed and keeps its normal targeting.
struct TripleMoveInfo
{
    u8 aoe:2;
    u8 hitsAllies:1;
    u8 reach:1;
};

// The chosen target byte carries the second target of 2-target moves in its high nibble (battler + 1, 0 = none).
#define TRIPLE_TARGET_MAIN(targetByte)          ((targetByte) & 0xF)
#define TRIPLE_TARGET_SECOND(targetByte)        ((((targetByte) >> 4) & 0xF) - 1)
#define TRIPLE_TARGET_HAS_SECOND(targetByte)    (((targetByte) >> 4) & 0xF)
#define TRIPLE_TARGET_ENCODE_SECOND(battler)    (((battler) + 1) << 4)

// A formation is packed into one byte: 2 bits of rank for the left, right and middle battler, in that order.
#define TRIPLE_FORMATION_PACK(leftRank, rightRank, middleRank) ((leftRank) | ((rightRank) << 2) | ((middleRank) << 4))

// How the player picks targets for a move in a triple battle.
enum TripleTargetChoice
{
    TRIPLE_TARGET_CHOICE_CANON,  // Not listed in the move table, or doesn't target foes: normal targeting
    TRIPLE_TARGET_CHOICE_NONE,   // The targets follow from the formation (FRONT moves, 3-target moves)
    TRIPLE_TARGET_CHOICE_ONE,    // Pick one target
    TRIPLE_TARGET_CHOICE_TWO,    // Pick two foes
};

extern const struct TripleMoveInfo gTripleMoveInfo[];

const struct TripleMoveInfo *GetTripleMoveInfo(enum Move move);
void InitTripleFormation(void);
bool32 TrySetTripleFormation(enum BattleSide side, u8 packedRanks);
enum TripleFormationRank GetTripleFormationRank(enum BattlerId battler);
enum BattlerPosition GetBattlerLayoutPosition(enum BattlerId battler);
enum BattlerId GetBattlerAtLayoutPosition(enum BattlerPosition layoutPosition);
bool32 IsTripleTwoTargetChoiceMove(enum Move move);
enum TripleTargetChoice GetTripleTargetChoice(enum Move move, enum MoveTarget canonTarget);
enum BattlerId GetTriplePreviousChoosingBattler(enum BattlerId battler);
u8 GetTripleFormationMovingUp(enum BattlerId battler);
bool32 DoesTripleBattlerChooseLater(enum BattlerId battlerFrom, enum BattlerId battlerTo);
enum BattlerId GetTripleScreenNeighbor(enum BattlerId battler, s32 step);
void UpdateTripleFormationGraphics(enum BattleSide side);
enum BattlerId GetTripleFoeAtRank(enum BattlerId battlerAtk, u32 aliveIndex);
enum MoveTarget GetTripleMoveTargetType(enum Move move, enum MoveTarget canonTarget);
enum BattlerId SetTripleMoveTargets(enum BattlerId battlerAtk, enum Move move, enum BattlerId battlerDef);
void MarkTripleExcludedTargets(enum BattlerId battlerAtk, enum Move move);
u32 CountTripleSpreadTargets(enum BattlerId battlerAtk);
void ChooseTripleAiActor(enum BattlerId battler, bool32 (*isAiBattler)(enum BattlerId));
bool32 IsTripleAiPassing(enum BattlerId battler);
void RemoveUnreachableTripleMoveScores(enum BattlerId battlerAtk, enum BattlerId battlerDef, s32 *scores);
// triple_battle_ui.c
#define TRIPLE_INDICATOR_TEXT_X 30 // Pixels before the move type name when the area of effect indicator is shown
void ResetTripleMoveIndicator(void);
void HideTripleMoveIndicator(void);
bool32 ShowTripleMoveIndicator(enum Move move);
u32 GetTripleSpriteScale(enum BattlerId battler);
u32 ScaleTripleMonPic(u8 *pic, enum BattlerId battler);
#define TRIPLE_HEALTHBAR_OFFSET_Y 7 // From the healthbox to the HP bar under it
#define TRIPLE_HEAD_GAP 6           // Between the top of the HP bar and the Pokémon's head
#define TRIPLE_HEALTHBOX_MIN_Y (-6) // Keeps the HP bar on screen; the name tag can go partly off the top
void GetTripleHealthboxCoords(enum BattlerId battler, s16 *x, s16 *y);
void SetTripleNameTags(u32 battlerMask);
void DrawTripleBattleFloor(void);
void ApplyTripleHaze(enum BattlerId battler);

uq4_12_t GetTripleTargetDamageModifier(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 targetCount);

#endif // GUARD_TRIPLE_BATTLE_H
