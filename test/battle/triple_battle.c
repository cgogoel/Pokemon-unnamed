#include "global.h"
#include "test/battle.h"
#include "triple_battle.h"

// 3v3 battles, see docs/hack/triple_battles.md

TRIPLE_BATTLE_TEST("Triple battles send out three Pokémon per side")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        PLAYER(SPECIES_SNORLAX);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
        OPPONENT(SPECIES_SQUIRTLE);
        OPPONENT(SPECIES_EEVEE);
    } WHEN {
        TURN {}
    } THEN {
        EXPECT_EQ(gBattlersCount, 6);
        EXPECT_EQ(playerLeft->species, SPECIES_WOBBUFFET);
        EXPECT_EQ(playerRight->species, SPECIES_WYNAUT);
        EXPECT_EQ(playerMiddle->species, SPECIES_PIKACHU);
        EXPECT_EQ(opponentLeft->species, SPECIES_BULBASAUR);
        EXPECT_EQ(opponentRight->species, SPECIES_CHARMANDER);
        EXPECT_EQ(opponentMiddle->species, SPECIES_SQUIRTLE);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: only one battler per side acts each turn")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
        OPPONENT(SPECIES_SQUIRTLE);
    } WHEN {
        TURN { MOVE(playerMiddle, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerMiddle);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentRight);
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerRight);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentMiddle);
        }
    }
}

TRIPLE_BATTLE_TEST("Triple battles: the left battler acts by default")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
        OPPONENT(SPECIES_SQUIRTLE);
    } WHEN {
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentLeft);
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerMiddle);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerRight);
        }
    }
}

// In triple tests a side with no MOVE uses Celebrate with its left battler, so name the actor once that battler has fainted.
// Formation starts in party order: party slot 0 (left) is near, slot 1 (right) is middle, slot 2 (middle position) is far.
#define TRIPLE_PARTIES                          \
    PLAYER(SPECIES_WOBBUFFET);                  \
    PLAYER(SPECIES_WOBBUFFET);                  \
    PLAYER(SPECIES_WOBBUFFET);                  \
    OPPONENT(SPECIES_WOBBUFFET);                \
    OPPONENT(SPECIES_WOBBUFFET);                \
    OPPONENT(SPECIES_WOBBUFFET);

TRIPLE_BATTLE_TEST("Triple battles: FRONT moves hit the nearest foe whatever the chosen target")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_POUND)->reach == TRIPLE_REACH_FRONT);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentMiddle); }
    } SCENE {
        HP_BAR(opponentLeft);
        NONE_OF {
            HP_BAR(opponentMiddle);
            HP_BAR(opponentRight);
        }
    }
}

TRIPLE_BATTLE_TEST("Triple battles: ANY moves can hit the far foe")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_THUNDERBOLT)->reach == TRIPLE_REACH_ANY);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentMiddle); }
    } SCENE {
        HP_BAR(opponentMiddle);
        NONE_OF {
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}

TRIPLE_BATTLE_TEST("Triple battles: 2-target FRONT moves hit the near foe fully and the middle foe for half")
{
    s16 nearDamage, middleDamage;
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_BUBBLE_BEAM)->aoe == 2);
        ASSUME(GetTripleMoveInfo(MOVE_BUBBLE_BEAM)->reach == TRIPLE_REACH_FRONT);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_BUBBLE_BEAM, target: opponentMiddle); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &nearDamage);
        HP_BAR(opponentRight, captureDamage: &middleDamage);
        NOT HP_BAR(opponentMiddle);
    } THEN {
        EXPECT_MUL_EQ(nearDamage, UQ_4_12(0.5), middleDamage);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: 2-target ANY moves hit the two chosen foes")
{
    s16 mainDamage, secondDamage;
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_ROCK_SLIDE)->aoe == 2);
        ASSUME(GetTripleMoveInfo(MOVE_ROCK_SLIDE)->reach == TRIPLE_REACH_ANY);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_ROCK_SLIDE, target: opponentMiddle, secondTarget: opponentRight, WITH_RNG(RNG_HOLD_EFFECT_FLINCH, FALSE)); }
    } SCENE {
        HP_BAR(opponentRight, captureDamage: &secondDamage);
        HP_BAR(opponentMiddle, captureDamage: &mainDamage);
        NOT HP_BAR(opponentLeft);
    } THEN {
        EXPECT_MUL_EQ(mainDamage, UQ_4_12(0.5), secondDamage);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: 3-target moves hit every foe for 75%")
{
    s16 spreadDamage[3];
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_HYPER_VOICE)->aoe == 3);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HYPER_VOICE, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &spreadDamage[0]);
        HP_BAR(opponentRight, captureDamage: &spreadDamage[1]);
        HP_BAR(opponentMiddle, captureDamage: &spreadDamage[2]);
        NONE_OF {
            HP_BAR(playerRight);
            HP_BAR(playerMiddle);
        }
    } THEN {
        EXPECT_EQ(spreadDamage[0], spreadDamage[1]);
        EXPECT_EQ(spreadDamage[1], spreadDamage[2]);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: moves never hit the user's allies")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_SURF)->aoe == 3);
        ASSUME(GetMoveTarget(MOVE_SURF) == TARGET_FOES_AND_ALLY);
        TRIPLE_PARTIES
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_SURF); }
    } SCENE {
        HP_BAR(opponentLeft);
        HP_BAR(opponentRight);
        HP_BAR(opponentMiddle);
        NONE_OF {
            HP_BAR(playerRight);
            HP_BAR(playerMiddle);
        }
    }
}

TRIPLE_BATTLE_TEST("Triple battles: 3-target moves deal 75% damage only when hitting more than one foe")
{
    s16 spreadDamage, singleDamage;
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_HYPER_VOICE)->aoe == 3);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); }
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &spreadDamage);
        MESSAGE("The opposing Wobbuffet fainted!");
        MESSAGE("The opposing Wobbuffet fainted!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HYPER_VOICE, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &singleDamage);
    } THEN {
        EXPECT_MUL_EQ(singleDamage, UQ_4_12(0.75), spreadDamage);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: a fainted battler's replacement takes its place in the formation")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentLeft); SEND_OUT(opponentLeft, 3); }
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentMiddle); }
    } SCENE {
        HP_BAR(opponentLeft);
        MESSAGE("The opposing Wobbuffet fainted!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POUND, playerLeft);
        HP_BAR(opponentLeft);
    } THEN {
        EXPECT_EQ(opponentLeft->species, SPECIES_WYNAUT);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: FRONT moves hit the next foe in line once the near foe is gone")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentLeft); }
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentMiddle); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POUND, playerLeft);
        HP_BAR(opponentRight);
        NOT HP_BAR(opponentMiddle);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: several battlers fainting at once are each replaced")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); SEND_OUT(opponentLeft, 3); SEND_OUT(opponentRight, 4); SEND_OUT(opponentMiddle, 5); }
    } THEN {
        EXPECT_EQ(opponentLeft->species, SPECIES_WYNAUT);
        EXPECT_EQ(opponentRight->species, SPECIES_WYNAUT);
        EXPECT_EQ(opponentMiddle->species, SPECIES_WYNAUT);
        EXPECT_GT(opponentLeft->hp, 0);
        EXPECT_GT(opponentRight->hp, 0);
        EXPECT_GT(opponentMiddle->hp, 0);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: the battle is won once all three foes faint")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        MESSAGE("The opposing Wobbuffet fainted!");
        MESSAGE("The opposing Wobbuffet fainted!");
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_WON);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: the player sends out a replacement for a fainted Pokémon")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_THUNDERBOLT, target: playerRight); SEND_OUT(playerRight, 3); }
    } THEN {
        EXPECT_EQ(playerRight->species, SPECIES_PIKACHU);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: FORMATION changes which foe is in front")
{
    GIVEN {
        TRIPLE_PARTIES
    } WHEN {
        TURN { FORMATION(opponentMiddle, opponentLeft, opponentRight); MOVE(playerLeft, MOVE_POUND, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POUND, playerLeft);
        HP_BAR(opponentMiddle);
        NONE_OF {
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}

TRIPLE_BATTLE_TEST("Triple battles: rearranging the formation doesn't use up the side's action")
{
    GIVEN {
        TRIPLE_PARTIES
    } WHEN {
        TURN { FORMATION(playerRight, playerMiddle, playerLeft); MOVE(playerLeft, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
    } THEN {
        EXPECT_EQ(GetTripleFormationRank(playerRight - gBattleMons), TRIPLE_RANK_NEAR);
        EXPECT_EQ(GetTripleFormationRank(playerMiddle - gBattleMons), TRIPLE_RANK_MIDDLE);
        EXPECT_EQ(GetTripleFormationRank(playerLeft - gBattleMons), TRIPLE_RANK_FAR);
    }
}

TRIPLE_BATTLE_TEST("Triple battles: switching in a benched Pokémon uses the side's action")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(playerMiddle, 3); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerRight);
        }
    } THEN {
        EXPECT_EQ(playerMiddle->species, SPECIES_PIKACHU);
    }
}

AI_TRIPLE_BATTLE_TEST("Triple battles: the AI acts with the battler that has the best move")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(20); }
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_PIKACHU) { Moves(MOVE_THUNDERBOLT); }
    } WHEN {
        TURN { EXPECT_MOVE(opponentMiddle, MOVE_THUNDERBOLT, target: playerLeft); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentRight);
        }
    }
}

AI_TRIPLE_BATTLE_TEST("Triple battles: the AI doesn't aim FRONT moves past the front foe")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_TACKLE)->reach == TRIPLE_REACH_FRONT);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponentLeft, MOVE_TACKLE, target: playerLeft); }
    }
}
