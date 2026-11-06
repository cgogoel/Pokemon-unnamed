#include "global.h"
#include "test/battle.h"
#include "item.h"
#include "triple_battle.h"

// Tri-Blast, Act 2's finale move (docs/hack/act2_finale.md): ??? type, as effective as the best of Fire, Water and
// Grass against the target, and never below 1x.

ASSUMPTIONS
{
    ASSUME(GetMoveType(MOVE_TRI_BLAST) == TYPE_MYSTERY);
    ASSUME(GetMovePower(MOVE_TRI_BLAST) == 120);
    ASSUME(GetMoveCategory(MOVE_TRI_BLAST) == DAMAGE_CATEGORY_SPECIAL);
}

TEST("Tri-Blast is as effective as the best of Fire, Water and Grass, and never below 1x")
{
    static const struct { u16 species; uq4_12_t expected; } cases[] =
    {
        {SPECIES_RHYDON,     UQ_4_12(4.0)}, // Ground/Rock: Water and Grass 4x
        {SPECIES_FERROTHORN, UQ_4_12(4.0)}, // Grass/Steel: Fire 4x
        {SPECIES_CHARMANDER, UQ_4_12(2.0)}, // Fire: Water 2x
        {SPECIES_SQUIRTLE,   UQ_4_12(2.0)}, // Water: Grass 2x
        {SPECIES_BULBASAUR,  UQ_4_12(2.0)}, // Grass/Poison: Fire 2x
        {SPECIES_HEATRAN,    UQ_4_12(2.0)}, // Fire/Steel: Water 2x
        {SPECIES_WOBBUFFET,  UQ_4_12(1.0)}, // Psychic: neutral to all three
        {SPECIES_DRAGONITE,  UQ_4_12(1.0)}, // Dragon/Flying resists all three: still 1x
        {SPECIES_KINGDRA,    UQ_4_12(1.0)}, // Water/Dragon: 0.5x at best, still 1x
    };

    for (u32 j = 0; j < ARRAY_COUNT(cases); j++)
        EXPECT_EQ(CalcPartyMonTypeEffectivenessMultiplier(MOVE_TRI_BLAST, cases[j].species, ABILITY_NONE), cases[j].expected);
}

TEST("Tri-Blast: abilities that absorb Fire, Water or Grass don't stop it")
{
    EXPECT_EQ(CalcPartyMonTypeEffectivenessMultiplier(MOVE_TRI_BLAST, SPECIES_VAPOREON, ABILITY_WATER_ABSORB), UQ_4_12(2.0));
    EXPECT_EQ(CalcPartyMonTypeEffectivenessMultiplier(MOVE_TRI_BLAST, SPECIES_SAWSBUCK_SPRING, ABILITY_SAP_SIPPER), UQ_4_12(2.0));
}

TEST("Tri-Blast: Wonder Guard goes by the final multiplier")
{
    // Shedinja (Bug/Ghost) is weak to Fire
    EXPECT_EQ(CalcPartyMonTypeEffectivenessMultiplier(MOVE_TRI_BLAST, SPECIES_SHEDINJA, ABILITY_WONDER_GUARD), UQ_4_12(2.0));
}

SINGLE_BATTLE_TEST("Tri-Blast's effectiveness messages in battle")
{
    enum Species species = SPECIES_NONE;
    PARAMETRIZE { species = SPECIES_RHYDON; }
    PARAMETRIZE { species = SPECIES_SQUIRTLE; }
    PARAMETRIZE { species = SPECIES_DRAGONITE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species);
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_BLAST); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_BLAST, player);
        HP_BAR(opponent);
        if (species == SPECIES_RHYDON)
            MESSAGE("It's extremely effective!");
        else if (species == SPECIES_SQUIRTLE)
            MESSAGE("It's super effective!");
        else
            NONE_OF {
                MESSAGE("It's not very effective…");
                MESSAGE("It's mostly ineffective…");
                MESSAGE("It doesn't affect the opposing Dragonite…");
            }
    }
}

SINGLE_BATTLE_TEST("Tri-Blast gets no STAB from the user's Fire, Water or Grass type")
{
    // The same Charmander before and after Soak turns it from Fire into Water: the same damage both times
    s16 fireDamage, waterDamage;
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SOAK) == EFFECT_SOAK);
        PLAYER(SPECIES_CHARMANDER) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_BLAST); }
        TURN { MOVE(opponent, MOVE_SOAK); MOVE(player, MOVE_TRI_BLAST); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_BLAST, player);
        HP_BAR(opponent, captureDamage: &fireDamage);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SOAK, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_BLAST, player);
        HP_BAR(opponent, captureDamage: &waterDamage);
    } THEN {
        EXPECT_EQ(fireDamage, waterDamage);
    }
}

SINGLE_BATTLE_TEST("Tri-Blast deals the damage of a 120-power move at its multiplier", s16 damage)
{
    // Squirtle takes 2x, Wobbuffet 1x. Same stats on both, so the damage doubles.
    enum Species species = SPECIES_NONE;
    PARAMETRIZE { species = SPECIES_WOBBUFFET; }
    PARAMETRIZE { species = SPECIES_SQUIRTLE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { HP(500); MaxHP(500); SpDefense(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_BLAST); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, UQ_4_12(2.0), results[1].damage);
    }
}

TRIPLE_BATTLE_TEST("Tri-Blast has INDIRECT reach in 3v3 battles: it can hit the far foe")
{
    GIVEN {
        ASSUME(GetTripleMoveInfo(MOVE_TRI_BLAST) != NULL);
        ASSUME(GetTripleMoveInfo(MOVE_TRI_BLAST)->reach == TRIPLE_REACH_ANY);
        ASSUME(GetTripleMoveInfo(MOVE_TRI_BLAST)->aoe == 1);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_PIKACHU);
        OPPONENT(SPECIES_BULBASAUR);
        OPPONENT(SPECIES_CHARMANDER);
        OPPONENT(SPECIES_SQUIRTLE);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_TRI_BLAST, target: opponentMiddle); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_BLAST, playerLeft);
        HP_BAR(opponentMiddle);
        NONE_OF {
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}

TEST("Tri-Blast's TM can be taught to every species")
{
    EXPECT(CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_TRI_BLAST));
    EXPECT(CanLearnTeachableMove(SPECIES_CATERPIE, MOVE_TRI_BLAST));
    EXPECT(CanLearnTeachableMove(SPECIES_PIKACHU, MOVE_TRI_BLAST));
    EXPECT(CanLearnTeachableMove(SPECIES_MEW, MOVE_TRI_BLAST));
    EXPECT_EQ(GetItemTMHMMoveId(ITEM_TM_TRI_BLAST), MOVE_TRI_BLAST);
}
