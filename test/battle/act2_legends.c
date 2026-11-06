#include "global.h"
#include "test/battle.h"
#include "act2_legends.h"
#include "battle_main.h"

// Act 2's finale: the legendary trio's battle (src/act2_legends.c, docs/hack/act2_finale.md)

ACT2_LEGENDS_BATTLE_TEST("Act 2 legends: the player acts once, then the trio attacks together and the battle ends")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_ENTEI) { Level(50); Moves(MOVE_FIRE_BLAST); }
        OPPONENT(SPECIES_SUICUNE) { Level(50); Moves(MOVE_HYDRO_PUMP); }
        OPPONENT(SPECIES_VIRIZION) { Level(50); Moves(MOVE_LEAF_BLADE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("ENTEI, SUICUNE and VIRIZION\nattacked!\p");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
        MESSAGE("ENTEI, SUICUNE and VIRIZION\nattacked together!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ACT2_LEGENDS_TRIO, opponentRight);
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_FIRE_BLAST, opponentLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_HYDRO_PUMP, opponentRight);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_LEAF_BLADE, opponentMiddle);
            HP_BAR(playerLeft);
            HP_BAR(playerRight);
            HP_BAR(playerMiddle);
        }
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_ACT2_LEGENDS_INTERRUPTED);
        EXPECT_EQ(playerLeft->hp, playerLeft->maxHP);
        EXPECT_EQ(playerRight->hp, playerRight->maxHP);
        EXPECT_EQ(playerMiddle->hp, playerMiddle->maxHP);
    }
}

ACT2_LEGENDS_BATTLE_TEST("Act 2 legends: the trio can't be knocked out")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_ENTEI) { Level(50); HP(1); }
        OPPONENT(SPECIES_SUICUNE) { Level(50); }
        OPPONENT(SPECIES_VIRIZION) { Level(50); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_TRI_BLAST, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_BLAST, playerLeft);
        MESSAGE("ENTEI, SUICUNE and VIRIZION\nattacked together!");
    } THEN {
        EXPECT_EQ(opponentLeft->hp, 1);
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_ACT2_LEGENDS_INTERRUPTED);
    }
}

ACT2_LEGENDS_BATTLE_TEST("Act 2 legends: the player can't run")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_ENTEI) { Level(50); }
        OPPONENT(SPECIES_SUICUNE) { Level(50); }
        OPPONENT(SPECIES_VIRIZION) { Level(50); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(IsRunningFromBattleImpossible(0), BATTLE_RUN_FORBIDDEN);
    }
}
