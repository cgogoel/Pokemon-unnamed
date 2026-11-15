#include "global.h"
#include "event_data.h"
#include "fieldmap.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "quests.h"
#include "string_util.h"
#include "text.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/quests.h"
#include "constants/region_map_sections.h"
#include "constants/story.h"

// The quest menu's content (docs/hack/quests.md)

enum { QUEST_FIRST_DAY, QUEST_FIRE, QUEST_CLUB_1, QUEST_WATER, QUEST_CLUB_2, QUEST_GRASS, QUEST_CARD, QUEST_ROAD, QUEST_WOODS,
       QUEST_KAI_DADS };

static void SetUpAct2(u32 stage, u32 research)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, stage);
    VarSet(VAR_ACT2_RESEARCH, research);
    VarSet(VAR_ACT2_MOVE_SEEN_1, MOVE_NONE);
    VarSet(VAR_ACT2_MOVE_SEEN_2, MOVE_NONE);
    VarSet(VAR_ACT2_FINALE, 0);
    VarSet(VAR_QUEST_LOG, 0);
    FlagClear(FLAG_ACT2_GOT_LAPRAS);
    FlagClear(FLAG_ACT2_VISITED_KAI_DADS);
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
}

static void EnterMapSec(u32 mapSec, u32 map)
{
    gMapHeader.regionMapSectionId = mapSec;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(map);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(map);
    Quests_OnMapEnter();
}

TEST("Quests: the menu is there from Act 2 on")
{
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    EXPECT(!Quests_IsMenuAvailable());
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    EXPECT(Quests_IsMenuAvailable());
    EXPECT_EQ(Quests_Count(), 10); // 9 MAIN + the KAI's dads SIDE quest
    VarSet(VAR_STORY_ACT, STORY_ACT_WORKFORCE);
    EXPECT(Quests_IsMenuAvailable());
    EXPECT_EQ(Quests_Count(), 1);
    EXPECT_EQ(Quests_GetTargetMapSec(), MAPSEC_SAFFRON_CITY);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Quests: a course counts its research, its puzzle, the TM and the exam")
{
    struct QuestInfo info;
    SetUpAct2(ACT2_WATER_TRIP, ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE
                             | ACT2_RESEARCH_DOCK_FIRE | ACT2_RESEARCH_PUZZLE_1);
    VarSet(VAR_ACT2_MOVE_SEEN_1, MOVE_WATER_GUN);
    VarSet(VAR_ACT2_MOVE_SEEN_2, MOVE_BUBBLE);
    CreateMon(&gPlayerParty[0], SPECIES_PSYDUCK, 15, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[1], SPECIES_POLIWAG, 15, 0, OTID_STRUCT_PLAYER_ID);

    EXPECT_EQ(Quests_GetCurrentIndex(), QUEST_WATER);
    Quests_Get(QUEST_WATER, &info);
    EXPECT_EQ(info.state, QUEST_CURRENT);
    EXPECT_EQ(info.numTasks, 5);
    EXPECT_EQ(info.tasks[0].count, 2); // Caught 2 of 3
    EXPECT(!info.tasks[0].done);
    EXPECT(info.tasks[1].done);        // 3 moves recorded
    EXPECT_EQ(info.tasks[2].count, 1); // A small dock fire out
    EXPECT_EQ(info.unitsDone, 6);      // 2 + 3 + 1
    EXPECT_EQ(info.unitsTotal, 12);
    EXPECT_EQ(Quests_GetPercent(&info), 50);
    EXPECT_EQ(info.targetMapSec, MAPSEC_VERMILION_CITY);
    // The finished quests before it, and the course: (100 + 100 + 100 + 50) / 9 (the SIDE quest is left out)
    EXPECT_EQ(Quests_GetActPercent(), 38);
    Quests_Get(QUEST_FIRE, &info);
    EXPECT_EQ(info.state, QUEST_DONE);
    EXPECT_EQ(Quests_GetPercent(&info), 100);
    Quests_Get(QUEST_CLUB_2, &info);
    EXPECT_EQ(info.state, QUEST_LOCKED);
}

TEST("Quests: the Water course's TM comes before the big fire, then the Lapras, then the exam")
{
    struct QuestInfo info;
    u32 research = ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_DOCK_FIRE
                 | ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2;
    const u8 *afterSmallFires, *afterTM, *afterBigFire, *afterLapras;
    SetUpAct2(ACT2_WATER_TRIP, research);
    VarSet(VAR_ACT2_MOVE_SEEN_1, MOVE_WATER_GUN);
    VarSet(VAR_ACT2_MOVE_SEEN_2, MOVE_BUBBLE);
    CreateMon(&gPlayerParty[0], SPECIES_PSYDUCK, 15, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[1], SPECIES_POLIWAG, 15, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[2], SPECIES_STARYU, 15, 0, OTID_STRUCT_PLAYER_ID);

    Quests_Get(QUEST_WATER, &info);
    afterSmallFires = info.next;
    VarSet(VAR_ACT2_RESEARCH, research | ACT2_RESEARCH_GOT_TM);
    Quests_Get(QUEST_WATER, &info);
    afterTM = info.next;
    VarSet(VAR_ACT2_RESEARCH, research | ACT2_RESEARCH_GOT_TM | ACT2_RESEARCH_PUZZLE_3 | ACT2_RESEARCH_PUZZLE_DONE);
    Quests_Get(QUEST_WATER, &info);
    afterBigFire = info.next;
    FlagSet(FLAG_ACT2_GOT_LAPRAS);
    Quests_Get(QUEST_WATER, &info);
    afterLapras = info.next;
    FlagClear(FLAG_ACT2_GOT_LAPRAS);
    EXPECT_EQ(Quests_GetPercent(&info), 91); // 11 of 12: all but the exam (the Lapras isn't a step)
    EXPECT(afterSmallFires != afterTM);
    EXPECT(afterTM != afterBigFire);
    EXPECT(afterBigFire != afterLapras);
    EXPECT_EQ(info.targetMapSec, MAPSEC_VERMILION_CITY);
}

TEST("Quests: the road home counts its stops either way, and points to the next on the way taken")
{
    struct QuestInfo info;
    SetUpAct2(ACT2_FINAL_PROJECT, 0);
    EXPECT_EQ(Quests_GetCurrentIndex(), QUEST_ROAD);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_SAFFRON_CITY);
    EnterMapSec(MAPSEC_SAFFRON_CITY, MAP_SAFFRON_CITY);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_VERMILION_CITY); // South until the player goes north
    EnterMapSec(MAPSEC_ROUTE_5, MAP_ROUTE5);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_CERULEAN_CITY);
    EnterMapSec(MAPSEC_CERULEAN_CITY, MAP_CERULEAN_CITY);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_PEWTER_CITY);
    EXPECT_EQ(info.tasks[1].count, 2);
    EXPECT_EQ(Quests_GetPercent(&info), 37); // The brief and 2 stops, of 8
    // Turning back south
    EnterMapSec(MAPSEC_ROUTE_6, MAP_ROUTE6);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_DIGLETTS_CAVE);
    EnterMapSec(MAPSEC_DIGLETTS_CAVE, MAP_DIGLETTS_CAVE_B1F);
    EnterMapSec(MAPSEC_VIRIDIAN_CITY, MAP_VIRIDIAN_CITY);
    EnterMapSec(MAPSEC_PALLET_TOWN, MAP_PALLET_TOWN);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT_EQ(info.targetMapSec, MAPSEC_PALLET_TOWN);
    EnterMapSec(MAPSEC_PALLET_TOWN, MAP_FARMYARD);
    Quests_Get(QUEST_ROAD, &info);
    EXPECT(info.tasks[1].done);
    EXPECT_EQ(info.unitsDone, 7);
    // Stops before the final project don't count
    SetUpAct2(ACT2_WATER_TRIP, 0);
    EnterMapSec(MAPSEC_VERMILION_CITY, MAP_VERMILION_CITY);
    EXPECT_EQ(VarGet(VAR_QUEST_LOG), 0);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Quests: the ranger station's floors")
{
    struct QuestInfo info;
    SetUpAct2(ACT2_PARENTS_TALKED, 0);
    EXPECT_EQ(Quests_GetCurrentIndex(), QUEST_WOODS);
    Quests_Get(QUEST_WOODS, &info);
    EXPECT_EQ(info.unitsDone, 0);
    EXPECT_EQ(info.targetMapSec, MAPSEC_PALLET_TOWN);
    EnterMapSec(MAPSEC_PALLET_TOWN, MAP_RANGER_STATION_1F);
    VarSet(VAR_ACT2_FINALE, ACT2_FINALE_BASEMENT_OPEN | ACT2_FINALE_CLOCK_SOLVED);
    EnterMapSec(MAPSEC_PALLET_TOWN, MAP_RANGER_STATION_B3F);
    Quests_Get(QUEST_WOODS, &info);
    EXPECT_EQ(info.unitsDone, 4);
    VarSet(VAR_ACT2_PROGRESS, ACT2_DONE);
    EXPECT_EQ(Quests_GetCurrentIndex(), -1);
    EXPECT_EQ(Quests_GetActPercent(), 100);
    EXPECT_EQ(Quests_GetTargetMapSec(), MAPSEC_NONE);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Quests: every next step fits the quest menu's line")
{
    static const u32 sResearch[] =
    {
        0, ACT2_RESEARCH_BRIEFED, ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE,
        ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_PUZZLE_1,
        ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_PUZZLE_ALL,
        ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2 | ACT2_RESEARCH_GOT_TM,
        ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_PUZZLE_ALL | ACT2_RESEARCH_PUZZLE_DONE,
        ACT2_RESEARCH_BRIEFED | ACT2_RESEARCH_MOVES_DONE | ACT2_RESEARCH_PUZZLE_ALL | ACT2_RESEARCH_PUZZLE_DONE | ACT2_RESEARCH_GOT_TM,
        ACT2_RESEARCH_CLUB_ARRIVED,
    };
    u32 stage = 0, research = 0, i, j, k;
    struct QuestInfo info;
    for (i = ACT2_START; i <= ACT2_DONE; i++)
    {
        for (j = 0; j < ARRAY_COUNT(sResearch); j++)
            PARAMETRIZE { stage = i; research = sResearch[j]; }
    }
    SetUpAct2(stage, research);
    VarSet(VAR_QUEST_LOG, research & 0x7F); // Some of the road's stops, either way
    for (k = 0; k < Quests_Count(); k++)
    {
        Quests_Get(k, &info);
        if (info.next != NULL)
            EXPECT_LE(GetStringWidth(FONT_NARROW, info.next, 0), 192);
        for (i = 0; i < info.numTasks; i++)
            EXPECT_LE(GetStringWidth(FONT_SMALL, info.tasks[i].text, 0), 144 - 17);
        EXPECT_LE(GetStringWidth(FONT_SMALL, info.listName, 0), 72 - 12);
        EXPECT_LE(GetStringWidth(FONT_NORMAL, info.title, 0), 144 - 28);
    }
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Quests: KAI's dads is a SIDE quest, kept by its own flag")
{
    struct QuestInfo info;
    SetUpAct2(ACT2_MET_FRIENDS, 0);

    Quests_Get(QUEST_KAI_DADS, &info);
    EXPECT_EQ(info.kind, QUEST_SIDE);
    EXPECT_EQ(info.state, QUEST_CURRENT);
    EXPECT_EQ(info.numTasks, 1);
    EXPECT(!info.tasks[0].done);
    EXPECT_EQ(info.targetMapSec, MAPSEC_CELADON_CITY);

    // Calling on either dad sets the flag, and the quest reads as done from then on
    FlagSet(FLAG_ACT2_VISITED_KAI_DADS);
    Quests_Get(QUEST_KAI_DADS, &info);
    EXPECT_EQ(info.state, QUEST_DONE);
    EXPECT(info.tasks[0].done);
    EXPECT_EQ(Quests_GetPercent(&info), 100);
    FlagClear(FLAG_ACT2_VISITED_KAI_DADS);
}

TEST("Quests: the SIDE quest is locked before Act 2's friends are met")
{
    struct QuestInfo info;
    SetUpAct2(ACT2_START, 0);

    Quests_Get(QUEST_KAI_DADS, &info);
    EXPECT_EQ(info.state, QUEST_LOCKED);
}

TEST("Quests: the main quests are MAIN, and the SIDE quest moves neither the act's % nor the marker")
{
    struct QuestInfo info;
    u32 before, after, i;
    SetUpAct2(ACT2_MET_FRIENDS, 0);

    for (i = QUEST_FIRST_DAY; i <= QUEST_WOODS; i++)
    {
        Quests_Get(i, &info);
        EXPECT_EQ(info.kind, QUEST_MAIN);
    }

    // The quest under way and the map marker stay on the main story while the side quest is open
    EXPECT_EQ(Quests_GetCurrentIndex(), QUEST_FIRE);
    before = Quests_GetActPercent();
    FlagSet(FLAG_ACT2_VISITED_KAI_DADS);
    after = Quests_GetActPercent();
    EXPECT_EQ(Quests_GetCurrentIndex(), QUEST_FIRE);
    EXPECT_EQ(before, after); // Finishing a SIDE quest doesn't move the act's %
    FlagClear(FLAG_ACT2_VISITED_KAI_DADS);
}
