# Time of day

Day and night are the game's, not the real-time clock's. Code: `src/game_time.c`. Tests: `test/game_time.c`.

- **The state** is `VAR_GAME_TIME`: `GAME_TIME_DAY` (the default) or `GAME_TIME_NIGHT`. `UpdateTimeOfDay`
  (`src/overworld.c`) tints by the game's hour (`GameTime_GetHour`: 12 by day, 2 by night) instead of the RTC's, so
  everything that asks `GetTimeOfDay` follows it: the overworld tint, evolutions and forms by time, night-only
  events, night music, the battle animations that look at the time (`AnimTask_GetTimeOfDay`) and `gettime` in
  scripts. The debug menu's time change sets it too (morning/day = day, evening/night = night).
- **The story holds the time** (`GameTime_IsStoryHeld`): Act 2's club nights (`Act2_IsClubNight`) are night, and the
  Grass trip is night until dawn breaks at the night bloom (`Act2_IsGrassTripNight`), then day for the rest of the
  trip. Outside those, the game time decides.
- **Scenes** can show another hour for a while (`special GameTime_ShowHour` with the hour in `VAR_0x8004`, 0 to go
  back; cleared on the next map load): the Grass trip's dawn steps through 6, 7, 8 and 9 o'clock.
- Wild encounters don't change by time (`OW_TIME_OF_DAY_ENCOUNTERS` is off); the areas' tables don't either.

## The DOOM SCROLL

A key item (`ITEM_DOOM_SCROLL`, the Scroll of Darkness icon): "Scroll away the hours. Wait until night or day."
The director hands it out at the final project's brief (`Act2_EventScript_FinalBrief`); saves already past the
brief get it with the starting kit (`Act2_GiveStartingKit`). Used from the bag (or registered to SELECT,
`EventScript_DoomScroll` in `data/scripts/doom_scroll.inc`), it offers the other time ("It's daytime. Scroll until
night?"); yes fades out, "{PLAYER} scrolled for hours…", and the map reloads where the player stands at the new time
(`GameTime_ScrollToOtherTime`).

`GameTime_GetDoomScrollState`:

| State | When | It says |
|---|---|---|
| `DOOM_SCROLL_STORY_TIME` | The story holds the time (a club night, the Grass trip) | "No time for scrolling now." |
| `DOOM_SCROLL_TOO_EARLY` | Act 1, or Act 2 before `ACT2_FINAL_PROJECT` | "There's coursework to do." |
| `DOOM_SCROLL_USABLE` | From the final project on, and in Act 3 | Wait until night / day |

## Places that change at night

- Celadon University: night music, locked academic buildings, a different crowd ([celadon_university.md](celadon_university.md), "Day and night").
