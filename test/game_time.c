#include "global.h"
#include "item.h"
#include "event_data.h"
#include "game_time.h"
#include "overworld.h"
#include "rtc.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/story.h"

// Day and night are the game's (src/game_time.c, docs/hack/time_of_day.md)

static void ResetStory(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    VarSet(VAR_ACT2_PROGRESS, 0);
    VarSet(VAR_ACT2_NIGHT_BLOOM, 0);
    GameTime_Set(GAME_TIME_DAY);
}

TEST("Game time: it's day by default, whatever the clock says")
{
    ResetStory();
    RtcCalcLocalTimeOffset(0, 23, 0, 0); // 11 p.m. on the clock
    EXPECT_EQ(GetTimeOfDay(), TIME_DAY);
    RtcCalcLocalTimeOffset(0, 3, 0, 0);
    EXPECT_EQ(GetTimeOfDay(), TIME_DAY);
    GameTime_Set(GAME_TIME_NIGHT);
    RtcCalcLocalTimeOffset(0, 12, 0, 0); // Noon on the clock
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    ResetStory();
}

TEST("Game time: the story's nights hold whatever the game time is")
{
    ResetStory();
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_WATER_INVITED);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    EXPECT(GameTime_IsStoryHeld());
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    // Dawn at the night bloom: day for the rest of the trip, even if the game time says night
    GameTime_Set(GAME_TIME_NIGHT);
    VarSet(VAR_ACT2_NIGHT_BLOOM, NIGHT_BLOOM_DAWN);
    EXPECT_EQ(GetTimeOfDay(), TIME_DAY);
    EXPECT(GameTime_IsStoryHeld());
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT);
    EXPECT(!GameTime_IsStoryHeld());
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    ResetStory();
}

TEST("Game time: the DOOM SCROLL works from the final project on, outside the story's nights")
{
    ResetStory();
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_TOO_EARLY);
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_CLUB_2_DONE);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_TOO_EARLY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_STORY_TIME);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_STORY_TIME);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_TOO_EARLY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_USABLE);
    VarSet(VAR_STORY_ACT, STORY_ACT_WORKFORCE);
    EXPECT_EQ(GameTime_GetDoomScrollState(), DOOM_SCROLL_USABLE);
    ResetStory();
}

TEST("Game time: the DOOM SCROLL is a key item for the field")
{
    EXPECT_EQ(GetItemPocket(ITEM_DOOM_SCROLL), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemType(ITEM_DOOM_SCROLL), ITEM_USE_FIELD);
}
