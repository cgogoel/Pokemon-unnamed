#include "global.h"
#include "act2_story.h"
#include "event_data.h"
#include "fieldmap.h"
#include "quests.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/quests.h"
#include "constants/region_map_sections.h"
#include "constants/story.h"

// The quests the start menu's QUESTS shows (docs/hack/quests.md). Everything is worked out from the story's vars
// (VAR_ACT2_PROGRESS, VAR_ACT2_RESEARCH, VAR_ACT2_FINALE, ...) whenever it's asked for; VAR_QUEST_LOG keeps the
// little the story doesn't: the road home's stops and the ranger station's floors.

struct QuestDef
{
    const u8 *title;
    const u8 *listName;
    u8 firstStage; // Current from this VAR_ACT2_PROGRESS stage to lastStage, done after it
    u8 lastStage;
    void (*fill)(struct QuestInfo *info, u32 param, bool32 done);
    u8 param;
};

static void AddTask(struct QuestInfo *info, const u8 *text, u32 count, u32 total, bool32 done)
{
    struct QuestTask *task = &info->tasks[info->numTasks++];
    task->text = text;
    task->count = min(count, total);
    task->total = total;
    task->done = done;
    info->unitsDone += task->count;
    info->unitsTotal += total;
}

// A finished quest: every task done
static void FinishTasks(struct QuestInfo *info)
{
    u32 i;
    for (i = 0; i < info->numTasks; i++)
    {
        info->tasks[i].count = info->tasks[i].total;
        info->tasks[i].done = TRUE;
    }
    info->unitsDone = info->unitsTotal;
    info->next = NULL;
    info->targetMapSec = MAPSEC_NONE;
}

static void SetNext(struct QuestInfo *info, const u8 *next, u16 mapSec)
{
    if (info->next == NULL)
    {
        info->next = next;
        info->targetMapSec = mapSec;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// First day

static const u8 sText_MeetClassmates[] = _("Meet your classmates");
static const u8 sText_NextMeetClassmates[] = _("Step outside: your classmates are waiting.");

static void FillFirstDay(struct QuestInfo *info, u32 param, bool32 done)
{
    AddTask(info, sText_MeetClassmates, 0, 1, FALSE);
    SetNext(info, sText_NextMeetClassmates, MAPSEC_CELADON_CITY);
}

// ---------------------------------------------------------------------------------------------------------------
// The courses (docs/hack/act2_story.md): the research sheet, the TM, the exam

struct QuestCourse
{
    u8 tripStage;
    enum Type type;
    u16 siteMapSec; // Where the Pokémon and the puzzle are
    u16 profMapSec; // Where the professor is on the trip
    const u8 *catchTask, *useTask, *recordTask, *puzzleTask;
    const u8 *nextIntro, *nextBriefing, *nextCatch, *nextUse, *nextRecord, *nextPuzzle, *nextTM, *nextExam;
};

static const u8 sText_MakeTM[] = _("Make the TM");
static const u8 sText_PassExam[] = _("Pass the exam");

static const u8 sText_CatchFire[] = _("Catch 3 FIRE POKéMON");
static const u8 sText_UseFire[] = _("Use a FIRE move");
static const u8 sText_RecordFire[] = _("Record 3 FIRE moves");
static const u8 sText_PuzzleFire[] = _("Relight 3 furnaces");
static const u8 sText_NextIntroFire[] = _("Go to LECTURE HALL A on campus.");
static const u8 sText_NextBriefFire[] = _("Talk to PROF. ASHFORD.");
static const u8 sText_NextCatchFire[] = _("Catch FIRE POKéMON in the MANSION.");
static const u8 sText_NextUseFire[] = _("Use a FIRE move in battle.");
static const u8 sText_NextRecordFire[] = _("Record FIRE moves in battle.");
static const u8 sText_NextPuzzleFire[] = _("Relight the MANSION's cold furnaces.");
static const u8 sText_NextTMFire[] = _("Show PROF. ASHFORD your research.");
static const u8 sText_NextExamFire[] = _("Battle PROF. ASHFORD for the exam.");

static const u8 sText_CatchWater[] = _("Catch 3 WATER POKéMON");
static const u8 sText_UseWater[] = _("Use a WATER move");
static const u8 sText_RecordWater[] = _("Record 3 WATER moves");
static const u8 sText_PuzzleWater[] = _("Put out 3 dock fires");
static const u8 sText_NextIntroWater[] = _("Go to the AUDIOVISUAL room on campus.");
static const u8 sText_NextBriefWater[] = _("Talk to PROF. MARLOWE.");
static const u8 sText_NextCatchWater[] = _("Fish at the harbor, or try ROUTE 6.");
static const u8 sText_NextUseWater[] = _("Use a WATER move in battle.");
static const u8 sText_NextRecordWater[] = _("Record WATER moves in battle.");
static const u8 sText_NextPuzzleWater[] = _("Put out the fires on the docks.");
static const u8 sText_NextTMWater[] = _("Show PROF. MARLOWE your research.");
static const u8 sText_NextExamWater[] = _("Battle PROF. MARLOWE for the exam.");

static const u8 sText_CatchGrass[] = _("Catch 3 GRASS POKéMON");
static const u8 sText_UseGrass[] = _("Use a GRASS move");
static const u8 sText_RecordGrass[] = _("Record 3 GRASS moves");
static const u8 sText_PuzzleGrass[] = _("Guard the MOONVEIL");
static const u8 sText_NextIntroGrass[] = _("Go to HOME ECONOMICS on campus.");
static const u8 sText_NextBriefGrass[] = _("Talk to PROF. FERN.");
static const u8 sText_NextCatchGrass[] = _("Catch GRASS POKéMON in VIRIDIAN FOREST.");
static const u8 sText_NextUseGrass[] = _("Use a GRASS move in battle.");
static const u8 sText_NextRecordGrass[] = _("Record GRASS moves in battle.");
static const u8 sText_NextPuzzleGrass[] = _("Wake the GLOWCAPS in VIRIDIAN FOREST.");
static const u8 sText_NextOddishGrass[] = _("Follow the ODDISH to the MOONVEIL.");
static const u8 sText_NextGuardGrass[] = _("Guard the MOONVEIL until dawn.");
static const u8 sText_NextTMGrass[] = _("Show PROF. FERN your research.");
static const u8 sText_NextExamGrass[] = _("Battle PROF. FERN for the exam.");

// The Water course's puzzle: the big fire needs the TM, and a young Lapras waits behind it
static const u8 sText_NextBigFire[] = _("Put out the big fire with BUBBLEBEAM.");
static const u8 sText_NextLapras[] = _("Someone is waiting by the pier…");
// The Fire course's: the statue switch, then the sealed room's journal
static const u8 sText_NextMansionSwitch[] = _("Try the statue switch in the MANSION.");

static const struct QuestCourse sCourses[] =
{
    {
        .tripStage = ACT2_FIRE_TRIP, .type = TYPE_FIRE,
        .siteMapSec = MAPSEC_CINNABAR_ISLAND, .profMapSec = MAPSEC_CINNABAR_ISLAND,
        .catchTask = sText_CatchFire, .useTask = sText_UseFire, .recordTask = sText_RecordFire, .puzzleTask = sText_PuzzleFire,
        .nextIntro = sText_NextIntroFire, .nextBriefing = sText_NextBriefFire, .nextCatch = sText_NextCatchFire,
        .nextUse = sText_NextUseFire, .nextRecord = sText_NextRecordFire, .nextPuzzle = sText_NextPuzzleFire,
        .nextTM = sText_NextTMFire, .nextExam = sText_NextExamFire,
    },
    {
        .tripStage = ACT2_WATER_TRIP, .type = TYPE_WATER,
        .siteMapSec = MAPSEC_VERMILION_CITY, .profMapSec = MAPSEC_VERMILION_CITY,
        .catchTask = sText_CatchWater, .useTask = sText_UseWater, .recordTask = sText_RecordWater, .puzzleTask = sText_PuzzleWater,
        .nextIntro = sText_NextIntroWater, .nextBriefing = sText_NextBriefWater, .nextCatch = sText_NextCatchWater,
        .nextUse = sText_NextUseWater, .nextRecord = sText_NextRecordWater, .nextPuzzle = sText_NextPuzzleWater,
        .nextTM = sText_NextTMWater, .nextExam = sText_NextExamWater,
    },
    {
        .tripStage = ACT2_GRASS_TRIP, .type = TYPE_GRASS,
        .siteMapSec = MAPSEC_VIRIDIAN_FOREST, .profMapSec = MAPSEC_VIRIDIAN_CITY,
        .catchTask = sText_CatchGrass, .useTask = sText_UseGrass, .recordTask = sText_RecordGrass, .puzzleTask = sText_PuzzleGrass,
        .nextIntro = sText_NextIntroGrass, .nextBriefing = sText_NextBriefGrass, .nextCatch = sText_NextCatchGrass,
        .nextUse = sText_NextUseGrass, .nextRecord = sText_NextRecordGrass, .nextPuzzle = sText_NextPuzzleGrass,
        .nextTM = sText_NextTMGrass, .nextExam = sText_NextExamGrass,
    },
};

#define RESEARCH_TO_CATCH  3
#define RESEARCH_TO_RECORD 3
#define PUZZLE_PARTS       3

static u32 CountMovesRecorded(void)
{
    return (VarGet(VAR_ACT2_MOVE_SEEN_1) != MOVE_NONE) + (VarGet(VAR_ACT2_MOVE_SEEN_2) != MOVE_NONE)
         + ((VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_MOVES_DONE) != 0);
}

static void FillCourse(struct QuestInfo *info, u32 param, bool32 done)
{
    const struct QuestCourse *course = &sCourses[param];
    bool32 onTrip = !done && VarGet(VAR_ACT2_PROGRESS) == course->tripStage;
    u32 research = onTrip ? VarGet(VAR_ACT2_RESEARCH) : 0;
    u32 caught = onTrip ? Act2_CountCaughtSpeciesOfType(course->type) : 0;
    u32 recorded = onTrip ? CountMovesRecorded() : 0;
    u32 parts = ((research & ACT2_RESEARCH_PUZZLE_1) != 0) + ((research & ACT2_RESEARCH_PUZZLE_2) != 0)
              + ((research & ACT2_RESEARCH_PUZZLE_3) != 0);
    bool32 puzzleDone = (research & ACT2_RESEARCH_PUZZLE_DONE) != 0;
    bool32 gotTM = (research & ACT2_RESEARCH_GOT_TM) != 0;

    AddTask(info, course->catchTask, caught, RESEARCH_TO_CATCH, caught >= RESEARCH_TO_CATCH);
    AddTask(info, course->useTask, (research & ACT2_RESEARCH_USED_MOVE) != 0, 1, (research & ACT2_RESEARCH_USED_MOVE) != 0);
    AddTask(info, course->recordTask, recorded, RESEARCH_TO_RECORD, recorded >= RESEARCH_TO_RECORD);
    AddTask(info, course->puzzleTask, parts, PUZZLE_PARTS, puzzleDone);
    info->unitsDone += puzzleDone; // The puzzle's end is a step of its own
    info->unitsTotal++;
    AddTask(info, sText_MakeTM, gotTM, 1, gotTM);
    AddTask(info, sText_PassExam, 0, 1, FALSE);
    if (done)
        return;

    if (!onTrip)
        SetNext(info, course->nextIntro, MAPSEC_CELADON_CITY);
    else if (!(research & ACT2_RESEARCH_BRIEFED))
        SetNext(info, course->nextBriefing, course->profMapSec);
    if (caught < RESEARCH_TO_CATCH)
        SetNext(info, course->nextCatch, course->siteMapSec);
    if (!(research & ACT2_RESEARCH_USED_MOVE))
        SetNext(info, course->nextUse, course->siteMapSec);
    if (recorded < RESEARCH_TO_RECORD)
        SetNext(info, course->nextRecord, course->siteMapSec);
    if (course->type == TYPE_WATER)
    {
        // Act2_IsReadyForTM: on the Water trip the TM comes before the big fire, and the Lapras before the exam
        if ((research & (ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2)) != (ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2))
            SetNext(info, course->nextPuzzle, course->siteMapSec);
        if (!gotTM)
            SetNext(info, course->nextTM, course->profMapSec);
        if (!(research & ACT2_RESEARCH_PUZZLE_3))
            SetNext(info, sText_NextBigFire, course->siteMapSec);
        if (!FlagGet(FLAG_ACT2_GOT_LAPRAS))
            SetNext(info, sText_NextLapras, course->siteMapSec);
    }
    else if (course->type == TYPE_GRASS)
    {
        // The night bloom: the glowcaps, the ODDISH, the vigil; from dawn PROF. FERN is in the glade (the forest's
        // section) with the TM and the exam
        if (!(research & ACT2_RESEARCH_PUZZLE_1))
            SetNext(info, course->nextPuzzle, course->siteMapSec);
        if (!(research & ACT2_RESEARCH_PUZZLE_2))
            SetNext(info, sText_NextOddishGrass, course->siteMapSec);
        if (!puzzleDone)
            SetNext(info, sText_NextGuardGrass, course->siteMapSec);
        if (!gotTM)
            SetNext(info, course->nextTM, course->siteMapSec);
        SetNext(info, course->nextExam, course->siteMapSec);
        return;
    }
    else
    {
        if (parts < PUZZLE_PARTS)
            SetNext(info, course->nextPuzzle, course->siteMapSec);
        if (!puzzleDone)
            SetNext(info, sText_NextMansionSwitch, course->siteMapSec);
        if (!gotTM)
            SetNext(info, course->nextTM, course->profMapSec);
    }
    SetNext(info, course->nextExam, course->profMapSec);
}

// ---------------------------------------------------------------------------------------------------------------
// The club nights

static const u8 sText_GoToClub[] = _("Go to the SPORTS COMPLEX");
static const u8 sText_WinClub[] = _("Win the club battle");
static const u8 sText_BeatDale[] = _("Beat DALE");
static const u8 sText_NextVisitKai[] = _("Visit KAI's room, DORMS 2F.");
static const u8 sText_NextVisitMina[] = _("Visit MINA's room, DORMS 2F.");
static const u8 sText_NextVisitRowan[] = _("Visit ROWAN's room, DORMS 3F.");
static const u8 sText_NextGoToClub[] = _("Go to the SPORTS COMPLEX tonight.");
static const u8 sText_NextClubBattle[] = _("Talk to a friend to start the battle.");
static const u8 sText_NextDale[] = _("Talk to DALE when you're ready.");

static const struct { u8 seeStage; const u8 *nextVisit; } sClubs[] =
{
    { ACT2_FIRE_SEE_KAI,     sText_NextVisitKai },
    { ACT2_WATER_SEE_MINA,   sText_NextVisitMina },
    { ACT2_GRASS_SEE_ROWAN,  sText_NextVisitRowan },
};

static void FillClub(struct QuestInfo *info, u32 param, bool32 done)
{
    u32 stage = VarGet(VAR_ACT2_PROGRESS);
    bool32 arrived = !done && stage == sClubs[param].seeStage + 1 && (VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_CLUB_ARRIVED);
    AddTask(info, sText_GoToClub, arrived, 1, arrived);
    AddTask(info, param == 2 ? sText_BeatDale : sText_WinClub, 0, 1, FALSE);
    if (done)
        return;
    if (stage == sClubs[param].seeStage)
        SetNext(info, sClubs[param].nextVisit, MAPSEC_CELADON_CITY);
    else if (!arrived)
        SetNext(info, sText_NextGoToClub, MAPSEC_CELADON_CITY);
    else
        SetNext(info, param == 2 ? sText_NextDale : sText_NextClubBattle, MAPSEC_CELADON_CITY);
}

// ---------------------------------------------------------------------------------------------------------------
// The final project's road home (docs/hack/act2_finale.md), either way: south through Vermilion and Diglett's Cave,
// or north through Cerulean, Mt. Moon and Pewter

static const u8 sText_GetBrief[] = _("Get the project brief");
static const u8 sText_WalkHome[] = _("Walk home");
static const u8 sText_TalkParents[] = _("Talk to Mom and Dad");
static const u8 sText_NextBrief[] = _("Go to the SCIENCE RESEARCH INSTITUTE.");
static const u8 sText_NextSaffron[] = _("Head east on ROUTE 7 to SAFFRON CITY.");
static const u8 sText_NextVermilion[] = _("Go south on ROUTE 6 to VERMILION CITY.");
static const u8 sText_NextCerulean[] = _("Go north on ROUTE 5 to CERULEAN CITY.");
static const u8 sText_NextDiglett[] = _("Go east to ROUTE 11 and DIGLETT'S CAVE.");
static const u8 sText_NextPewter[] = _("Go west through MT. MOON to PEWTER.");
static const u8 sText_NextViridianSouth[] = _("Through the cave, then on to VIRIDIAN.");
static const u8 sText_NextViridianNorth[] = _("Go south through VIRIDIAN FOREST.");
static const u8 sText_NextPallet[] = _("Go south down ROUTE 1 to PALLET TOWN.");
static const u8 sText_NextFarm[] = _("Go home to the farm.");
static const u8 sText_NextParents[] = _("Talk to Mom and Dad in the farmhouse.");

static const struct { const u8 *south, *north; u16 southMapSec, northMapSec; } sRoadStops[QUEST_LOG_ROAD_STOPS] =
{
    { sText_NextSaffron,       sText_NextSaffron,       MAPSEC_SAFFRON_CITY,   MAPSEC_SAFFRON_CITY },
    { sText_NextVermilion,     sText_NextCerulean,      MAPSEC_VERMILION_CITY, MAPSEC_CERULEAN_CITY },
    { sText_NextDiglett,       sText_NextPewter,        MAPSEC_DIGLETTS_CAVE,  MAPSEC_PEWTER_CITY },
    { sText_NextViridianSouth, sText_NextViridianNorth, MAPSEC_VIRIDIAN_CITY,  MAPSEC_VIRIDIAN_CITY },
    { sText_NextPallet,        sText_NextPallet,        MAPSEC_PALLET_TOWN,    MAPSEC_PALLET_TOWN },
    { sText_NextFarm,          sText_NextFarm,          MAPSEC_PALLET_TOWN,    MAPSEC_PALLET_TOWN },
};

static void FillRoadHome(struct QuestInfo *info, u32 param, bool32 done)
{
    u32 log = VarGet(VAR_QUEST_LOG), stops = 0, i;
    bool32 briefed = VarGet(VAR_ACT2_PROGRESS) >= ACT2_FINAL_PROJECT;
    bool32 north = (log & QUEST_LOG_ROAD_NORTH) != 0;

    for (i = 0; i < QUEST_LOG_ROAD_STOPS; i++)
        stops += (log >> i) & 1;
    AddTask(info, sText_GetBrief, briefed, 1, briefed);
    AddTask(info, sText_WalkHome, stops, QUEST_LOG_ROAD_STOPS, stops >= QUEST_LOG_ROAD_STOPS);
    AddTask(info, sText_TalkParents, 0, 1, FALSE);
    if (done)
        return;
    if (!briefed)
    {
        SetNext(info, sText_NextBrief, MAPSEC_CELADON_CITY);
        return;
    }
    // The first stop not reached yet, on the way the player is taking (south until they go north)
    for (i = 0; i < QUEST_LOG_ROAD_STOPS; i++)
    {
        if (!(log & (1 << i)))
        {
            SetNext(info, north ? sRoadStops[i].north : sRoadStops[i].south,
                    north ? sRoadStops[i].northMapSec : sRoadStops[i].southMapSec);
            return;
        }
    }
    SetNext(info, sText_NextParents, MAPSEC_PALLET_TOWN);
}

// Entering a map on the final project: the stop it is, and which way home the player is taking
static void LogRoadHome(void)
{
    u32 log = VarGet(VAR_QUEST_LOG);
    u32 mapSec = gMapHeader.regionMapSectionId;
    u32 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;

    switch (mapSec)
    {
    case MAPSEC_SAFFRON_CITY:
        log |= QUEST_LOG_ROAD_SAFFRON;
        break;
    case MAPSEC_VERMILION_CITY:
        log = (log | QUEST_LOG_ROAD_MIDWAY) & ~QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_CERULEAN_CITY:
        log |= QUEST_LOG_ROAD_MIDWAY | QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_ROUTE_6:
    case MAPSEC_ROUTE_11:
        log &= ~QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_ROUTE_5:
    case MAPSEC_ROUTE_4:
    case MAPSEC_ROUTE_3:
    case MAPSEC_MT_MOON:
        log |= QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_DIGLETTS_CAVE:
        log = (log | QUEST_LOG_ROAD_CAVE) & ~QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_PEWTER_CITY:
        log |= QUEST_LOG_ROAD_CAVE | QUEST_LOG_ROAD_NORTH;
        break;
    case MAPSEC_VIRIDIAN_CITY:
        log |= QUEST_LOG_ROAD_VIRIDIAN;
        break;
    case MAPSEC_PALLET_TOWN:
        log |= QUEST_LOG_ROAD_PALLET;
        if (map == MAP_FARMYARD || map == MAP_WOODS)
            log |= QUEST_LOG_ROAD_FARM;
        break;
    }
    VarSet(VAR_QUEST_LOG, log);
}

// ---------------------------------------------------------------------------------------------------------------
// The noise in the Woods: the ranger station

static const u8 sText_FollowNoise[] = _("Follow the noise");
static const u8 sText_OpenBasement[] = _("Open the way down");
static const u8 sText_SetClock[] = _("Set the clock");
static const u8 sText_CrossFlood[] = _("Cross the flood");
static const u8 sText_FindPath[] = _("Find the hidden path");
static const u8 sText_FaceNoise[] = _("Find the noise's source");
static const u8 sText_NextFollowNoise[] = _("Follow the eerie noise in the WOODS.");
static const u8 sText_NextOpenBasement[] = _("Search the ranger station.");
static const u8 sText_NextSetClock[] = _("Set the clock. Read the explorers' notes.");
static const u8 sText_NextCrossFlood[] = _("Cross the flooded floor.");
static const u8 sText_NextFindPath[] = _("Find the hidden path through the grass.");
static const u8 sText_NextFaceNoise[] = _("Go down to what's making the noise.");

static void FillWoods(struct QuestInfo *info, u32 param, bool32 done)
{
    u32 log = VarGet(VAR_QUEST_LOG), finale = VarGet(VAR_ACT2_FINALE);
    bool32 entered = (log & QUEST_LOG_STATION_ENTERED) != 0;
    bool32 basement = (finale & ACT2_FINALE_BASEMENT_OPEN) != 0;
    bool32 clock = (finale & ACT2_FINALE_CLOCK_SOLVED) != 0;
    bool32 b3f = (log & QUEST_LOG_STATION_B3F) != 0;
    bool32 grass = (finale & ACT2_FINALE_GRASS_CROSSED) != 0;

    AddTask(info, sText_FollowNoise, entered, 1, entered);
    AddTask(info, sText_OpenBasement, basement, 1, basement);
    AddTask(info, sText_SetClock, clock, 1, clock);
    AddTask(info, sText_CrossFlood, b3f, 1, b3f);
    AddTask(info, sText_FindPath, grass, 1, grass);
    AddTask(info, sText_FaceNoise, 0, 1, FALSE);
    if (done)
        return;
    if (!entered)
        SetNext(info, sText_NextFollowNoise, MAPSEC_PALLET_TOWN);
    else if (!basement)
        SetNext(info, sText_NextOpenBasement, MAPSEC_PALLET_TOWN);
    else if (!clock)
        SetNext(info, sText_NextSetClock, MAPSEC_PALLET_TOWN);
    else if (!b3f)
        SetNext(info, sText_NextCrossFlood, MAPSEC_PALLET_TOWN);
    else if (!grass)
        SetNext(info, sText_NextFindPath, MAPSEC_PALLET_TOWN);
    else
        SetNext(info, sText_NextFaceNoise, MAPSEC_PALLET_TOWN);
}

// ---------------------------------------------------------------------------------------------------------------
// Act 3: a placeholder until its story has stages

static const u8 sText_TitleSilph[] = _("SILPH CO.");
static const u8 sText_ListSilph[] = _("Silph Co.");
static const u8 sText_StartWork[] = _("Start work at SILPH CO.");
static const u8 sText_NextSilph[] = _("Your work at SILPH CO. starts here.");

static void FillAct3(struct QuestInfo *info, u32 param, bool32 done)
{
    AddTask(info, sText_StartWork, 0, 1, FALSE);
    SetNext(info, sText_NextSilph, MAPSEC_SAFFRON_CITY);
}

// ---------------------------------------------------------------------------------------------------------------

static const u8 sText_TitleFirstDay[] = _("FIRST DAY");
static const u8 sText_ListFirstDay[] = _("First day");
static const u8 sText_TitleFire[] = _("FIRE COURSE");
static const u8 sText_ListFire[] = _("FIRE course");
static const u8 sText_TitleClub1[] = _("CLUB NIGHT 1");
static const u8 sText_ListClub1[] = _("Club night 1");
static const u8 sText_TitleWater[] = _("WATER COURSE");
static const u8 sText_ListWater[] = _("WATER course");
static const u8 sText_TitleClub2[] = _("CLUB NIGHT 2");
static const u8 sText_ListClub2[] = _("Club night 2");
static const u8 sText_TitleGrass[] = _("GRASS COURSE");
static const u8 sText_ListGrass[] = _("GRASS course");
static const u8 sText_TitleCard[] = _("TRAINER CARD");
static const u8 sText_ListCard[] = _("Trainer Card");
static const u8 sText_TitleRoad[] = _("THE ROAD HOME");
static const u8 sText_ListRoad[] = _("Road home");
static const u8 sText_TitleWoods[] = _("THE EERIE NOISE");
static const u8 sText_ListWoods[] = _("Eerie noise");

static const struct QuestDef sAct2Quests[] =
{
    { sText_TitleFirstDay, sText_ListFirstDay, ACT2_START,            ACT2_START,         FillFirstDay, 0 },
    { sText_TitleFire,     sText_ListFire,     ACT2_MET_FRIENDS,      ACT2_FIRE_TRIP,     FillCourse,   0 },
    { sText_TitleClub1,    sText_ListClub1,    ACT2_FIRE_SEE_KAI,     ACT2_FIRE_INVITED,  FillClub,     0 },
    { sText_TitleWater,    sText_ListWater,    ACT2_CLUB_1_DONE,      ACT2_WATER_TRIP,    FillCourse,   1 },
    { sText_TitleClub2,    sText_ListClub2,    ACT2_WATER_SEE_MINA,   ACT2_WATER_INVITED, FillClub,     1 },
    { sText_TitleGrass,    sText_ListGrass,    ACT2_CLUB_2_DONE,      ACT2_GRASS_TRIP,    FillCourse,   2 },
    { sText_TitleCard,     sText_ListCard,     ACT2_GRASS_SEE_ROWAN,  ACT2_GRASS_INVITED, FillClub,     2 },
    { sText_TitleRoad,     sText_ListRoad,     ACT2_GOT_TRAINER_CARD, ACT2_FINAL_PROJECT, FillRoadHome, 0 },
    { sText_TitleWoods,    sText_ListWoods,    ACT2_PARENTS_TALKED,   ACT2_PARENTS_TALKED, FillWoods,   0 },
};

static const struct QuestDef sAct3Quests[] =
{
    { sText_TitleSilph, sText_ListSilph, 0, 0xFF, FillAct3, 0 },
};

static const struct QuestDef *GetQuests(u32 *count)
{
    switch (VarGet(VAR_STORY_ACT))
    {
    case STORY_ACT_UNIVERSITY:
        *count = ARRAY_COUNT(sAct2Quests);
        return sAct2Quests;
    case STORY_ACT_WORKFORCE:
        *count = ARRAY_COUNT(sAct3Quests);
        return sAct3Quests;
    default:
        *count = 0;
        return NULL;
    }
}

static u32 GetActProgress(void)
{
    return VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY ? VarGet(VAR_ACT2_PROGRESS) : 0;
}

// The start menu's QUESTS: from Act 2 on
bool32 Quests_IsMenuAvailable(void)
{
    return VarGet(VAR_STORY_ACT) >= STORY_ACT_UNIVERSITY;
}

u32 Quests_GetActNumber(void)
{
    return VarGet(VAR_STORY_ACT) + 1;
}

u32 Quests_Count(void)
{
    u32 count;
    GetQuests(&count);
    return count;
}

void Quests_Get(u32 index, struct QuestInfo *info)
{
    u32 count, stage = GetActProgress();
    const struct QuestDef *quests = GetQuests(&count);
    const struct QuestDef *quest;

    memset(info, 0, sizeof(*info));
    info->targetMapSec = MAPSEC_NONE;
    if (index >= count)
        return;
    quest = &quests[index];
    info->title = quest->title;
    info->listName = quest->listName;
    if (stage < quest->firstStage)
        info->state = QUEST_LOCKED;
    else if (stage > quest->lastStage)
        info->state = QUEST_DONE;
    else
        info->state = QUEST_CURRENT;
    quest->fill(info, quest->param, info->state == QUEST_DONE);
    if (info->state == QUEST_DONE)
        FinishTasks(info);
}

// The quest under way (-1: none, the act's quests are all done)
s32 Quests_GetCurrentIndex(void)
{
    u32 count, stage = GetActProgress(), i;
    const struct QuestDef *quests = GetQuests(&count);
    for (i = 0; i < count; i++)
    {
        if (stage >= quests[i].firstStage && stage <= quests[i].lastStage)
            return i;
    }
    return -1;
}

u32 Quests_GetPercent(const struct QuestInfo *info)
{
    if (info->unitsTotal == 0)
        return info->state == QUEST_DONE ? 100 : 0;
    return info->unitsDone * 100 / info->unitsTotal;
}

// The act's %: the average of its quests' fractions
u32 Quests_GetActPercent(void)
{
    u32 count = Quests_Count(), i, sum = 0;
    struct QuestInfo info;
    if (count == 0)
        return 0;
    for (i = 0; i < count; i++)
    {
        Quests_Get(i, &info);
        sum += info.unitsTotal == 0 ? (info.state == QUEST_DONE ? 100 : 0) : info.unitsDone * 10000 / info.unitsTotal;
    }
    return sum / (count * 100);
}

// What blinks on the Town Map: the current quest's next step's place
u16 Quests_GetTargetMapSec(void)
{
    s32 index = Quests_GetCurrentIndex();
    struct QuestInfo info;
    if (index < 0)
        return MAPSEC_NONE;
    Quests_Get(index, &info);
    return info.targetMapSec;
}

// Loading a map (LoadMapFromWarp, LoadMapFromCameraTransition): what VAR_QUEST_LOG keeps
void Quests_OnMapEnter(void)
{
    u32 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    if (VarGet(VAR_STORY_ACT) != STORY_ACT_UNIVERSITY)
        return;
    if (VarGet(VAR_ACT2_PROGRESS) == ACT2_FINAL_PROJECT)
        LogRoadHome();
    if (map == MAP_RANGER_STATION_1F)
        VarSet(VAR_QUEST_LOG, VarGet(VAR_QUEST_LOG) | QUEST_LOG_STATION_ENTERED);
    else if (map == MAP_RANGER_STATION_B3F || map == MAP_RANGER_STATION_B4F)
        VarSet(VAR_QUEST_LOG, VarGet(VAR_QUEST_LOG) | QUEST_LOG_STATION_ENTERED | QUEST_LOG_STATION_B3F);
}
