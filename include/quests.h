#ifndef GUARD_QUESTS_H
#define GUARD_QUESTS_H

// The quests the start menu's QUESTS shows (docs/hack/quests.md): read from the story's vars, so nothing here is
// saved except what VAR_QUEST_LOG tracks (the road home's stops, the ranger station's floors).

#define QUEST_MAX_TASKS 6

enum QuestState
{
    QUEST_LOCKED,
    QUEST_CURRENT,
    QUEST_DONE,
};

struct QuestTask
{
    const u8 *text;
    u8 count; // A counter (count of total) shows while it's under way, when total > 1
    u8 total;
    bool8 done;
};

struct QuestInfo
{
    const u8 *title;    // The detail pane's title
    const u8 *listName; // The list's name
    const u8 *next;     // The next step
    u16 targetMapSec;   // What blinks on the map (MAPSEC_NONE: nothing)
    u8 state;           // enum QuestState
    u8 numTasks;
    u8 unitsDone;       // The quest's % is unitsDone / unitsTotal
    u8 unitsTotal;
    struct QuestTask tasks[QUEST_MAX_TASKS];
};

bool32 Quests_IsMenuAvailable(void);
u32 Quests_GetActNumber(void);
u32 Quests_Count(void);
void Quests_Get(u32 index, struct QuestInfo *info);
s32 Quests_GetCurrentIndex(void);
u32 Quests_GetPercent(const struct QuestInfo *info);
u32 Quests_GetActPercent(void);
u16 Quests_GetTargetMapSec(void);
void Quests_OnMapEnter(void);

#endif // GUARD_QUESTS_H
