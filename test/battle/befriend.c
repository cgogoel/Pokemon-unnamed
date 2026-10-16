#include "global.h"
#include "test/battle.h"
#include "befriend.h"

// Befriending battles (Act 1), see docs/hack/befriending.md. The wild Pokémon's HP is its aggression.
// Teddiursa (src/data/befriend.h): loves a held-out hand and Honey, dislikes being approached and Rawst Berries.

#define BEFRIEND_TEDDIURSA OPPONENT(SPECIES_TEDDIURSA) { HP(100); MaxHP(100); }

BEFRIEND_BATTLE_TEST("Befriending: the loved action lowers aggression a lot")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_TEDDIURSA)->lovedAction == BEFRIEND_ACTION_HOLD_OUT_HAND);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HOLD_OUT_HAND); }
    } SCENE {
        MESSAGE("You held out a hand.");
        MESSAGE("Teddiursa relaxed a lot!\nIt really likes that!");
        HP_BAR(opponent, damage: 40);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the loved action works less the second time")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HOLD_OUT_HAND); }
        TURN { ACT(BEFRIEND_ACTION_HOLD_OUT_HAND); }
    } SCENE {
        HP_BAR(opponent, damage: 40);
        HP_BAR(opponent, damage: 15);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: a disliked action raises aggression")
{
    GIVEN {
        ASSUME(GetBefriendActionReaction(GetBefriendInfo(SPECIES_TEDDIURSA), BEFRIEND_ACTION_APPROACH) == BEFRIEND_REACTION_DISLIKED);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TEDDIURSA) { HP(50); MaxHP(100); }
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_APPROACH); }
    } SCENE {
        MESSAGE("Teddiursa didn't like that!\nIt's getting agitated!");
        HP_BAR(opponent, damage: -25);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: other actions lower aggression a little")
{
    GIVEN {
        ASSUME(GetBefriendActionReaction(GetBefriendInfo(SPECIES_TEDDIURSA), BEFRIEND_ACTION_CROUCH) == BEFRIEND_REACTION_NEUTRAL);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_CROUCH); }
    } SCENE {
        MESSAGE("Teddiursa doesn't seem\nto mind.");
        HP_BAR(opponent, damage: 10);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: offering the loved item lowers aggression the most")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_TEDDIURSA)->lovedItem == ITEM_HONEY);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { OFFER(ITEM_HONEY); }
    } SCENE {
        MESSAGE("You offered the Honey.");
        MESSAGE("Teddiursa happily took\nthe Honey!");
        HP_BAR(opponent, damage: 60);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: offering a disliked item raises aggression")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_TEDDIURSA) { HP(50); MaxHP(100); }
    } WHEN {
        TURN { OFFER(ITEM_RAWST_BERRY); }
    } SCENE {
        MESSAGE("Teddiursa growled at\nthe Rawst Berry!");
        HP_BAR(opponent, damage: -25);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: Watch gives a clue and uses patience without changing aggression")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { WATCH(); }
    } SCENE {
        MESSAGE("You watched Teddiursa closely.\pIt keeps licking its paws and sniffing\nthe air for something sweet.");
    } THEN {
        EXPECT_EQ(opponent->hp, 100);
        EXPECT_EQ(gBattleStruct->befriendPatience, GetBefriendInfo(SPECIES_TEDDIURSA)->patience - 1);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: at 0 aggression the Pokémon is befriended and joins the player")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_HOLD_OUT_HAND); }
        TURN { OFFER(ITEM_HONEY); }
    } SCENE {
        MESSAGE("Teddiursa is comfortable\naround you now!");
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_CAUGHT);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_SPECIES), SPECIES_TEDDIURSA);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_MAX_HP));
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the Pokémon runs away when it runs out of patience")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_TEDDIURSA)->patience == 5);
        PLAYER(SPECIES_WOBBUFFET);
        BEFRIEND_TEDDIURSA
    } WHEN {
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
        TURN { WATCH(); }
    } SCENE {
        MESSAGE("Teddiursa looks ready to bolt!");
        MESSAGE("{PLAY_SE SE_FLEE}Teddiursa ran away!");
        NOT MESSAGE("{PLAY_SE SE_FLEE}The wild Wobbuffet fled!"); // The engine's own message, about the wrong Pokémon
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_MON_FLED);
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the boss calms down at half the usual rate")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_JANGMO_O)->calmRate == 50);
        ASSUME(GetBefriendInfo(SPECIES_JANGMO_O)->lovedAction == BEFRIEND_ACTION_APPROACH);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_JANGMO_O) { HP(60); MaxHP(100); }
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_APPROACH); }
        TURN { ACT(BEFRIEND_ACTION_CROUCH); }
    } SCENE {
        HP_BAR(opponent, damage: 20);
        MESSAGE("Jangmo-o didn't like that!\nIt's getting agitated!");
        HP_BAR(opponent, damage: -25); // Raising isn't scaled
    }
}

BEFRIEND_BATTLE_TEST("Befriending: the tutorial Pokémon calms down faster")
{
    GIVEN {
        ASSUME(GetBefriendInfo(SPECIES_BLIPBUG)->calmRate == 150);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BLIPBUG) { HP(100); MaxHP(100); }
    } WHEN {
        TURN { ACT(BEFRIEND_ACTION_CROUCH); }
        TURN { OFFER(ITEM_ORAN_BERRY); }
    } SCENE {
        HP_BAR(opponent, damage: 60);
        MESSAGE("Blipbug is comfortable\naround you now!");
    }
}
