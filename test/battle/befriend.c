#include "global.h"
#include "test/battle.h"
#include "befriend.h"

// Befriending battles (Act 1), see docs/hack/befriending.md. The wild Pokémon's HP is its aggression.
// Eevee (src/data/befriend.h): loves humming and Oran Berries, dislikes being approached and Pecha Berries.

#define BEFRIEND_EEVEE OPPONENT(SPECIES_EEVEE) { HP(100); MaxHP(100); }

BEFRIEND_BATTLE_TEST("Befriending: the loved action lowers aggression a lot")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_EEVEE)->lovedAction == BEFRIEND_ACTION_HUM);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HUM); }
    } SCENE {
        MESSAGE("You hummed a tune.");
        MESSAGE("Eevee relaxed a lot!\nIt really likes that!");
        HP_BAR(opponent, damage: 40);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the loved action works less the second time")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HUM); }
        TURN { ACT(BEFRIEND_ACTION_HUM); }
    } SCENE {
        HP_BAR(opponent, damage: 40);
        HP_BAR(opponent, damage: 15);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: a disliked action raises aggression")
{
    GIVEN {
        ASSUME(GetBefriendActionReaction(GetBefriendInfo(SPECIES_EEVEE), BEFRIEND_ACTION_APPROACH) == BEFRIEND_REACTION_DISLIKED);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EEVEE) { HP(50); MaxHP(100); }
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_APPROACH); }
    } SCENE {
        MESSAGE("Eevee didn't like that!\nIt's getting agitated!");
        HP_BAR(opponent, damage: -25);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: other actions lower aggression a little")
{
    GIVEN {
        ASSUME(GetBefriendActionReaction(GetBefriendInfo(SPECIES_EEVEE), BEFRIEND_ACTION_CROUCH) == BEFRIEND_REACTION_NEUTRAL);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_CROUCH); }
    } SCENE {
        MESSAGE("Eevee doesn't seem\nto mind.");
        HP_BAR(opponent, damage: 10);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: offering the loved item lowers aggression the most")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_EEVEE)->lovedItem == ITEM_ORAN_BERRY);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { OFFER(ITEM_ORAN_BERRY); }
    } SCENE {
        MESSAGE("You offered the Oran Berry.");
        MESSAGE("Eevee happily took\nthe Oran Berry!");
        HP_BAR(opponent, damage: 60);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: offering a disliked item raises aggression")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EEVEE) { HP(50); MaxHP(100); }
    } WHEN {
        TURN { OFFER(ITEM_PECHA_BERRY); }
    } SCENE {
        MESSAGE("Eevee growled at\nthe Pecha Berry!");
        HP_BAR(opponent, damage: -25);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: Watch gives a clue and uses patience without changing aggression")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { WATCH(); }
    } SCENE {
        MESSAGE("You watched Eevee closely.\pIt keeps sniffing at your bag,\nand its ears perk up at any sound.");
    } THEN {
        EXPECT_EQ(opponent->hp, 100);
        EXPECT_EQ(gBattleStruct->befriendPatience, GetBefriendInfo(SPECIES_EEVEE)->patience - 1);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: at 0 aggression the Pokémon is befriended and joins the player")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HUM); }
        TURN { OFFER(ITEM_ORAN_BERRY); }
    } SCENE {
        MESSAGE("Eevee is comfortable\naround you now!");
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_CAUGHT);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_EEVEE);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_MAX_HP));
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the Pokémon runs away when it runs out of patience")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_EEVEE)->patience == 5);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_EEVEE
    } WHEN {
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
    } SCENE {
        MESSAGE("Eevee looks ready to bolt!");
        MESSAGE("{PLAY_SE SE_FLEE}Eevee ran away!");
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_MON_FLED);
    }
}
