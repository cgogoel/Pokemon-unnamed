#ifndef GUARD_GAME_TIME_H
#define GUARD_GAME_TIME_H

// Day and night are the game's, not the real-time clock's (src/game_time.c, docs/hack/time_of_day.md)

// The hours the game's day and night stand for (UpdateTimeOfDay tints by hour)
#define GAME_TIME_DAY_HOUR   12
#define GAME_TIME_NIGHT_HOUR 2

u32 GameTime_GetHour(void);
bool32 GameTime_IsNight(void);
bool32 GameTime_IsStoryHeld(void);
void GameTime_Set(u32 time);
u16 GameTime_GetDoomScrollState(void);
void GameTime_ScrollToOtherTime(void);
void GameTime_ShowHour(void);

#endif // GUARD_GAME_TIME_H
