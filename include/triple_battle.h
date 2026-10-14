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

extern const struct TripleMoveInfo gTripleMoveInfo[];

const struct TripleMoveInfo *GetTripleMoveInfo(enum Move move);
void InitTripleFormation(void);
enum TripleFormationRank GetTripleFormationRank(enum BattlerId battler);
enum BattlerId GetTripleFoeAtRank(enum BattlerId battlerAtk, u32 aliveIndex);
enum MoveTarget GetTripleMoveTargetType(enum Move move, enum MoveTarget canonTarget);
enum BattlerId SetTripleMoveTargets(enum BattlerId battlerAtk, enum Move move, enum BattlerId battlerDef);
void MarkTripleExcludedTargets(enum BattlerId battlerAtk, enum Move move);
u32 CountTripleSpreadTargets(enum BattlerId battlerAtk);
uq4_12_t GetTripleTargetDamageModifier(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 targetCount);

#endif // GUARD_TRIPLE_BATTLE_H
