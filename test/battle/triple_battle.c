#include "global.h"
#include "test/battle.h"

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

TRIPLE_BATTLE_TEST("Triple battles: a single-target move can hit the middle foe")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
        OPPONENT(SPECIES_SQUIRTLE);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_POUND, target: opponentMiddle); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POUND, playerRight);
        HP_BAR(opponentMiddle);
        NONE_OF {
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}
