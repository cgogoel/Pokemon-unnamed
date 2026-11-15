#include "global.h"
#include "act2_night_bloom.h"
#include "act2_story.h"
#include "event_data.h"
#include "pokemon.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/story.h"

// The Grass course's night bloom (src/act2_night_bloom.c, docs/hack/act2_story.md)

static void StartGrassTrip(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP);
    VarSet(VAR_ACT2_RESEARCH, 0);
    VarSet(VAR_ACT2_NIGHT_BLOOM, 0);
}

static void EndStory(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    VarSet(VAR_ACT2_PROGRESS, 0);
    VarSet(VAR_ACT2_RESEARCH, 0);
    VarSet(VAR_ACT2_NIGHT_BLOOM, 0);
    FlagClear(FLAG_ACT2_GOT_MORELULL);
}

static void GoTo(u32 mapGroup, u32 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
}

TEST("Night bloom: each woken glowcap widens the forest's light")
{
    EXPECT_EQ(Act2_GetNightFlashLevel(0), 7);
    EXPECT_EQ(Act2_GetNightFlashLevel(1), 5);
    EXPECT_EQ(Act2_GetNightFlashLevel(2), 3);
    EXPECT_EQ(Act2_GetNightFlashLevel(3), 1);
}

TEST("Night bloom: Viridian Forest is dark only on the Grass trip's night")
{
    GoTo(MAP_GROUP(MAP_VIRIDIAN_FOREST), MAP_NUM(MAP_VIRIDIAN_FOREST));
    EndStory();
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 0);
    StartGrassTrip();
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 7);
    VarSet(VAR_ACT2_NIGHT_BLOOM, NIGHT_BLOOM_GLOWCAP_1 | NIGHT_BLOOM_GLOWCAP_3);
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 3);
    VarSet(VAR_ACT2_NIGHT_BLOOM, NIGHT_BLOOM_DAWN);
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 0);
    VarSet(VAR_ACT2_NIGHT_BLOOM, 0);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT); // The north way home: an ordinary forest
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 0);
    GoTo(MAP_GROUP(MAP_VIRIDIAN_FOREST_GLADE), MAP_NUM(MAP_VIRIDIAN_FOREST_GLADE));
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP);
    EXPECT_EQ(Act2_GetNightForestFlashLevel(), 0); // The glade is moonlit
    EndStory();
}

TEST("Night bloom: the puzzle's parts are the glowcaps, the ODDISH and the waves")
{
    u32 i;
    GoTo(MAP_GROUP(MAP_PALLET_TOWN), MAP_NUM(MAP_PALLET_TOWN)); // Nothing to draw
    StartGrassTrip();
    for (i = 1; i <= 3; i++)
    {
        gSpecialVar_0x8004 = i;
        EXPECT_EQ(Act2_WakeGlowcap(), i == 3);
    }
    EXPECT(VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_PUZZLE_1);
    for (i = 1; i <= 3; i++)
    {
        gSpecialVar_0x8004 = i;
        EXPECT_EQ(Act2_FollowOddish(), i == 3);
    }
    EXPECT(VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_PUZZLE_2);
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_ARRIVAL);
    gSpecialVar_0x8004 = NIGHT_BLOOM_ARRIVED;
    Act2_SetNightBloomBit();
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_VIGIL);
    // One gap at a time, west, north, east: each wave beaten, then the MORELULL's hedge fills its gap
    for (i = 1; i <= 3; i++)
    {
        EXPECT_EQ(Act2_GetNextWave(), i);
        gSpecialVar_0x8004 = i;
        EXPECT_EQ(Act2_PlantHedge(), i == 3);
        EXPECT(VarGet(VAR_ACT2_NIGHT_BLOOM) & (NIGHT_BLOOM_HEDGE_1 << (i - 1)));
    }
    EXPECT_EQ(Act2_GetNextWave(), 0);
    EXPECT(VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_PUZZLE_3);
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_DAWN);
    Act2_BreakDawn();
    EXPECT(!Act2_IsGrassTripNight());
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_NONE);     // The dew waits for the player on the MOONVEIL
    EXPECT(!Act2_IsDewCollected());
    VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | ACT2_RESEARCH_PUZZLE_DONE);
    EXPECT(Act2_IsDewCollected());
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_MORNING);
    gSpecialVar_0x8004 = NIGHT_BLOOM_FERN;
    Act2_SetNightBloomBit();
    EXPECT_EQ(Act2_GetGladeScene(), GLADE_SCENE_NONE);
    EndStory();
}

TEST("Night bloom: the waves grow, at the area's level")
{
    EXPECT_EQ(Act2_GetBloomWaveSize(1), 1);
    EXPECT_EQ(Act2_GetBloomWaveSize(2), 2);
    EXPECT_EQ(Act2_GetBloomWaveSize(3), 3);
}

TEST("Night bloom: a sleeping ODDISH wakes to a GRASS move, the same check the glowcaps use")
{
    StartGrassTrip();
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_BULBASAUR, 15, 0, OTID_STRUCT_PLAYER_ID);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_TACKLE, 0);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_GROWTH, 1); // GROWTH is a NORMAL move, so it won't do
    SetMonMoveSlot(&gPlayerParty[0], MOVE_NONE, 2);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_NONE, 3);
    EXPECT(!Act2_FindCourseMoveUser());
    // Any GRASS move rouses it now, damaging or not: no more SLEEP POWDER / STUN SPORE special case
    SetMonMoveSlot(&gPlayerParty[0], MOVE_ABSORB, 2);
    EXPECT(Act2_FindCourseMoveUser());
    SetMonMoveSlot(&gPlayerParty[0], MOVE_NONE, 2);
    SetMonMoveSlot(&gPlayerParty[0], MOVE_COTTON_SPORE, 3);
    EXPECT(Act2_FindCourseMoveUser());
    ZeroPlayerPartyMons();
    EndStory();
}

TEST("Night bloom: the glade's MORELULL is a gift, not a catch; a wild one counts")
{
    bool8 fateful = TRUE;
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_MORELULL, 18, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&gPlayerParty[0], MON_DATA_MODERN_FATEFUL_ENCOUNTER, &fateful);
    EXPECT_EQ(Act2_CountCaughtSpeciesOfType(TYPE_GRASS), 0);
    CreateMon(&gPlayerParty[1], SPECIES_MORELULL, 18, 0, OTID_STRUCT_PLAYER_ID);
    EXPECT_EQ(Act2_CountCaughtSpeciesOfType(TYPE_GRASS), 1);
    ZeroPlayerPartyMons();
}

TEST("Night bloom: Viridian Forest's night table is only for the Grass trip")
{
    u32 trip, roadHome;
    GoTo(MAP_GROUP(MAP_VIRIDIAN_FOREST), MAP_NUM(MAP_VIRIDIAN_FOREST));
    StartGrassTrip();
    trip = GetCurrentMapWildMonHeaderId();
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT);
    roadHome = GetCurrentMapWildMonHeaderId();
    EndStory();
    EXPECT_NE(trip, roadHome);
    EXPECT_EQ(gWildMonHeaders[trip].encounterTypes[TIME_OF_DAY_DEFAULT].landMonsInfo->wildPokemon[0].species, SPECIES_ODDISH);
    EXPECT_EQ(gWildMonHeaders[trip].encounterTypes[TIME_OF_DAY_DEFAULT].landMonsInfo->wildPokemon[10].species, SPECIES_PHANTUMP);
    EXPECT_NE(gWildMonHeaders[roadHome].encounterTypes[TIME_OF_DAY_DEFAULT].landMonsInfo->wildPokemon[0].species, SPECIES_ODDISH);
}
