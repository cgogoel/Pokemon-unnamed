#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

// AI_FLAG_TARGET_THREAT: in doubles (and the hack's 3v3), attacks favor the foe expected to deal the most damage to the
// AI's side (GetBiggestThreatToSide, src/battle_ai_main.c).

AI_DOUBLE_BATTLE_TEST("AI targets the foe that threatens its side most in doubles")
{
    bool32 threatOnLeft = FALSE;
    PARAMETRIZE { threatOnLeft = TRUE; }
    PARAMETRIZE { threatOnLeft = FALSE; }
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_TARGET_THREAT);
        PLAYER(SPECIES_WOBBUFFET) { Moves(threatOnLeft ? MOVE_TACKLE : MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Moves(threatOnLeft ? MOVE_CELEBRATE : MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            if (threatOnLeft)
                EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft);
            else
                EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerRight);
        }
        TURN {
            if (threatOnLeft)
                EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft);
            else
                EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerRight);
        }
    }
}
