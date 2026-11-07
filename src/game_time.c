#include "global.h"
#include "game_time.h"
#include "act2_story.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "overworld.h"
#include "constants/story.h"

// Day and night are the game's, not the real-time clock's (docs/hack/time_of_day.md). UpdateTimeOfDay (the tint,
// GetTimeOfDay for evolutions, night-only events and the rest) reads the hour from here instead of the RTC.
// It's day unless the story or the player makes it night:
// - The story holds the time at its nights: Act 2's club nights (Act2_IsClubNight) and the Grass trip until dawn at
//   the night bloom (Act2_IsGrassTripNight); the rest of that trip stays day.
// - Otherwise it's VAR_GAME_TIME (GAME_TIME_DAY or GAME_TIME_NIGHT), which the DOOM SCROLL flips (from the final
//   project on).

// Whether the story decides the time right now (the DOOM SCROLL can't change it then): TRUE at its nights, and on
// the Grass trip after dawn
bool32 GameTime_IsStoryHeld(void)
{
    return Act2_IsClubNight() || Act2_IsGrassTrip();
}

static bool32 IsStoryNight(void)
{
    return Act2_IsClubNight() || Act2_IsGrassTripNight();
}

bool32 GameTime_IsNight(void)
{
    if (GameTime_IsStoryHeld())
        return IsStoryNight();
    return VarGet(VAR_GAME_TIME) == GAME_TIME_NIGHT;
}

// The hour UpdateTimeOfDay tints by
u32 GameTime_GetHour(void)
{
    return GameTime_IsNight() ? GAME_TIME_NIGHT_HOUR : GAME_TIME_DAY_HOUR;
}

void GameTime_Set(u32 time)
{
    VarSet(VAR_GAME_TIME, time);
}

// specialvar VAR_RESULT, GameTime_GetDoomScrollState: DOOM_SCROLL_USABLE from Act 2's final project on (Act 3
// included), unless the story holds the time
u16 GameTime_GetDoomScrollState(void)
{
    u32 act = VarGet(VAR_STORY_ACT);
    if (GameTime_IsStoryHeld())
        return DOOM_SCROLL_STORY_TIME;
    if (act < STORY_ACT_UNIVERSITY || (act == STORY_ACT_UNIVERSITY && VarGet(VAR_ACT2_PROGRESS) < ACT2_FINAL_PROJECT))
        return DOOM_SCROLL_TOO_EARLY;
    return DOOM_SCROLL_USABLE;
}

// special GameTime_ScrollToOtherTime (the DOOM SCROLL, with the screen black): day becomes night or night day, and
// the map reloads where the player stands so its tint, lights and music follow. waitstate after it.
void GameTime_ScrollToOtherTime(void)
{
    GameTime_Set(GameTime_IsNight() ? GAME_TIME_DAY : GAME_TIME_NIGHT);
    SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE,
                       gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
    DoDiveWarp();
    ResetInitialPlayerAvatarState();
}

// special GameTime_ShowHour with an hour in VAR_0x8004: the tint shows that hour for now (until the next map load;
// 0 goes back to the game's own). For scenes like the Grass trip's dawn.
void GameTime_ShowHour(void)
{
    SetTimeOfDay(gSpecialVar_0x8004);
}
