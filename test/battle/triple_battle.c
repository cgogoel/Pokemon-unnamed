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
