#include "global.h"
#include "event_data.h"
#include "item.h"
#include "tm_research.h"
#include "test/battle.h"
#include "constants/story.h"

// The TM Database's counters in battle (docs/hack/tm_database.md)

static void TrackTm(u32 tm)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_WORKFORCE);
    FlagSet(FLAG_TM_DATABASE_UNLOCKED);
    memset(gSaveBlock3Ptr->tmResearch, 0, sizeof(gSaveBlock3Ptr->tmResearch));
    gSaveBlock3Ptr->tmResearch[0].tm = tm;
}

// A trainer battle: in this test setup, a wild Pokémon doesn't take the move it's given
SINGLE_BATTLE_TEST("TM research: a new move of a tracked TM's type shows its counter once, from either side")
{
    GIVEN {
        TrackTm(ENUM_TM_HM_FLAMETHROWER + 1);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
        TURN { MOVE(opponent, MOVE_EMBER); }
        TURN { MOVE(opponent, MOVE_FLAME_WHEEL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        MESSAGE("TM research: 1 of 3 FIRE\nmoves recorded!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, opponent);
        NONE_OF {
            MESSAGE("TM research: 1 of 3 FIRE\nmoves recorded!");
            MESSAGE("TM research: 2 of 3 FIRE\nmoves recorded!");
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FLAME_WHEEL, opponent);
        MESSAGE("TM research: 2 of 3 FIRE\nmoves recorded!");
    } THEN {
        EXPECT_EQ(TmResearch_GetMoveCount(0), 2);
    }
}

WILD_BATTLE_TEST("TM research: no counter for an untracked type or without the database")
{
    bool32 unlocked;
    PARAMETRIZE { unlocked = TRUE; }
    PARAMETRIZE { unlocked = FALSE; }
    GIVEN {
        TrackTm(ENUM_TM_HM_ICE_BEAM + 1);
        if (!unlocked)
        {
            FlagClear(FLAG_TM_DATABASE_UNLOCKED);
            gSaveBlock3Ptr->tmResearch[0].tm = ENUM_TM_HM_FLAMETHROWER + 1;
        }
        PLAYER(SPECIES_VULPIX);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        NOT MESSAGE("TM research: 1 of 3 FIRE\nmoves recorded!");
    } THEN {
        EXPECT_EQ(TmResearch_GetMoveCount(0), 0);
        FlagSet(FLAG_TM_DATABASE_UNLOCKED);
    }
}

WILD_BATTLE_TEST("TM research: the last count says the TM is ready")
{
    GIVEN {
        TrackTm(ENUM_TM_HM_CALM_MIND + 1);
        gSaveBlock3Ptr->tmResearch[0].species[0] = SPECIES_ABRA;
        gSaveBlock3Ptr->tmResearch[0].species[1] = SPECIES_DROWZEE;
        gSaveBlock3Ptr->tmResearch[0].moves[0] = MOVE_PSYBEAM;
        PLAYER(SPECIES_ABRA);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CONFUSION); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSION, player);
        MESSAGE("TM research: 2 of 2 PSYCHIC\nmoves recorded!\pCalm Mind is ready\nto craft in QUESTS!");
    } THEN {
        EXPECT(TmResearch_IsSlotReady(0));
    }
}
