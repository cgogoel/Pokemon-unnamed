#ifndef GUARD_ACT2_LEGENDS_H
#define GUARD_ACT2_LEGENDS_H

// Act 2's finale: the legendary trio's battle in the ranger station's B4F chamber (docs/hack/act2_finale.md).
// A wild 3v3 against ENTEI, SUICUNE and VIRIZION: the player acts once, the trio can't be knocked out and doesn't
// take a normal turn, then all three attack together (one animation) and the battle ends with no damage dealt.
// Started by Act2_EventScript_LegendsBattle (data/scripts/act2_legends.inc).

// gBattleOutcome / VAR_RESULT after the battle: the trio's attack cut it short. Never a blackout.
#define B_OUTCOME_ACT2_LEGENDS_INTERRUPTED B_OUTCOME_PLAYER_TELEPORTED

// Which beam a sprite of the three-beam animation (Tri-Blast, the trio's attack) is, in its args
enum TriBeamKind
{
    TRI_BEAM_FIRE,
    TRI_BEAM_WATER,
    TRI_BEAM_GRASS,
    TRI_BEAM_COUNT,
};

// Where a beam starts and ends: TRI_BEAM_FROM_* (include/constants/battle_anim.h)

extern bool8 gIsAct2LegendsBattle;

static inline bool32 IsAct2LegendsBattle(void)
{
    return gIsAct2LegendsBattle;
}

void CreateAct2LegendsParty(void);
void CB2_EndAct2LegendsBattle(void);
bool32 TryAct2LegendsTrioAttack(void);
void GetTriBeamBattlers(u32 source, enum TriBeamKind kind, u32 *from, u32 *to);

#endif // GUARD_ACT2_LEGENDS_H
