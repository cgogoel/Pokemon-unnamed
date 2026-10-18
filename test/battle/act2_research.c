#include "global.h"
#include "event_data.h"
#include "test/battle.h"
#include "constants/story.h"

// Act 2's research counters during a course trip (docs/hack/act2_story.md)

WILD_BATTLE_TEST("Act 2: a newly recorded course-type move shows the research counter once")
{
    GIVEN {
        VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
        VarSet(VAR_ACT2_PROGRESS, ACT2_WATER_TRIP);
        PLAYER(SPECIES_PSYDUCK);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
        MESSAGE("Research: 1 of 3 WATER\nmoves recorded!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
        NOT MESSAGE("Research: 1 of 3 WATER\nmoves recorded!");
    }
}
