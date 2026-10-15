#include "global.h"
#include "test/battle.h"
#include "triple_battle.h"

// The 3v3 rules played with 2 Pokémon per side, see docs/hack/triple_battles.md
// Formation starts in party order: party slot 0 (left) is near, slot 1 (right) is far.
#define PARTIES_2V2                             \
    PLAYER(SPECIES_WOBBUFFET);                  \
    PLAYER(SPECIES_WOBBUFFET);                  \
    OPPONENT(SPECIES_WOBBUFFET);                \
    OPPONENT(SPECIES_WOBBUFFET);

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: two Pokémon per side, no middle battlers")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN {}
    } THEN {
        EXPECT_EQ(gBattlersCount, 4);
        EXPECT_EQ(playerLeft->species, SPECIES_WOBBUFFET);
        EXPECT_EQ(playerRight->species, SPECIES_WYNAUT);
        EXPECT_EQ(GetTripleFormationRank(playerLeft - gBattleMons), TRIPLE_RANK_NEAR);
        EXPECT_EQ(GetTripleFormationRank(playerRight - gBattleMons), 1);
        EXPECT_EQ(GetBattlerLayoutPosition(playerLeft - gBattleMons), B_POSITION_PLAYER_RIGHT);
        EXPECT_EQ(GetBattlerLayoutPosition(playerRight - gBattleMons), B_POSITION_PLAYER_LEFT);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: only one battler per side acts each turn")
{
    GIVEN {
        PARTIES_2V2
    } WHEN {
        TURN { MOVE(playerRight, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentRight);
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponentLeft);
        }
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: FRONT moves hit the near foe whatever the chosen target")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_POUND)->reach == TRIPLE_REACH_FRONT);
        PARTIES_2V2
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentRight); }
    } SCENE {
        HP_BAR(opponentLeft);
        NOT HP_BAR(opponentRight);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: ANY moves can hit the far foe")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_THUNDERBOLT)->reach == TRIPLE_REACH_ANY);
        PARTIES_2V2
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_THUNDERBOLT, target: opponentRight); }
    } SCENE {
        HP_BAR(opponentRight);
        NOT HP_BAR(opponentLeft);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: 2-target moves hit the main target fully and the other foe for half")
{
    s16 mainDamage, secondDamage;
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_ROCK_SLIDE)->aoe == 2);
        PARTIES_2V2
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_ROCK_SLIDE, target: opponentRight, WITH_RNG(RNG_HOLD_EFFECT_FLINCH, FALSE)); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &secondDamage);
        HP_BAR(opponentRight, captureDamage: &mainDamage);
    } THEN {
        EXPECT_MUL_EQ(mainDamage, UQ_4_12(0.5), secondDamage);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: 3-target moves hit both foes for 75%")
{
    s16 damage[2];
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_HYPER_VOICE)->aoe == 3);
        PARTIES_2V2
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HYPER_VOICE); }
    } SCENE {
        HP_BAR(opponentLeft, captureDamage: &damage[0]);
        HP_BAR(opponentRight, captureDamage: &damage[1]);
        NOT HP_BAR(playerRight);
    } THEN {
        EXPECT_EQ(damage[0], damage[1]);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: FORMATION swaps which foe is in front")
{
    GIVEN {
        PARTIES_2V2
    } WHEN {
        TURN { FORMATION_2V2(opponentRight, opponentLeft); MOVE(playerLeft, MOVE_POUND, target: opponentLeft); }
    } SCENE {
        HP_BAR(opponentRight);
        NOT HP_BAR(opponentLeft);
    } THEN {
        EXPECT_EQ(GetBattlerLayoutPosition(opponentRight - gBattleMons), B_POSITION_OPPONENT_RIGHT);
        EXPECT_EQ(GetBattlerLayoutPosition(opponentLeft - gBattleMons), B_POSITION_OPPONENT_LEFT);
    }
}

TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: a fainted battler is replaced from the bench")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentLeft); SEND_OUT(opponentLeft, 2); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
    } THEN {
        EXPECT_EQ(opponentLeft->species, SPECIES_WYNAUT);
        EXPECT_EQ(GetTripleFormationRank(opponentLeft - gBattleMons), TRIPLE_RANK_NEAR);
    }
}

AI_TRIPLE_2V2_BATTLE_TEST("Triple battles 2v2: the AI acts with one battler")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { HP(20); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDERBOLT, MOVE_CELEBRATE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponentRight, MOVE_THUNDERBOLT, target: playerLeft); }
    }
}
