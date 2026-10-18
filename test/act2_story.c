#include "global.h"
#include "act2_story.h"
#include "event_data.h"
#include "pokemon.h"
#include "rtc.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/story.h"

// Act 2's story, see docs/hack/act2_story.md

TEST("Act 2: the playtest party starts healthy")
{
    u32 i;
    Act2_GivePlaytestParty();
    EXPECT_EQ(CalculatePlayerPartyCount(), 2);
    for (i = 0; i < 2; i++)
    {
        EXPECT_EQ(GetMonData(&gPlayerParty[i], MON_DATA_LEVEL), ACT2_START_LEVEL);
        EXPECT_GT(GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP), 0);
        EXPECT_EQ(GetMonData(&gPlayerParty[i], MON_DATA_HP), GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP));
        EXPECT_GT(GetMonData(&gPlayerParty[i], MON_DATA_ATK), 0);
    }
}

TEST("Act 2: wild Pokémon are within 3 levels of the party's average")
{
    static const struct WildPokemon sMon = { .minLevel = 40, .maxLevel = 45, .species = SPECIES_RATTATA };
    u32 i, level, lowest = MAX_LEVEL, highest = 0;
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_TEDDIURSA, 12, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[1], SPECIES_TYNAMO, 16, 0, OTID_STRUCT_PLAYER_ID); // Average 14
    for (i = 0; i < 200; i++)
    {
        level = ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND);
        lowest = min(lowest, level);
        highest = max(highest, level);
    }
    EXPECT_EQ(lowest, 11);
    EXPECT_EQ(highest, 17);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Act 2: outside Act 2, wild Pokémon keep their area's levels")
{
    static const struct WildPokemon sMon = { .minLevel = 40, .maxLevel = 45, .species = SPECIES_RATTATA };
    u32 i, level;
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_TEDDIURSA, 12, 0, OTID_STRUCT_PLAYER_ID);
    for (i = 0; i < 50; i++)
    {
        level = ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND);
        EXPECT(level >= 40 && level <= 45);
    }
}

TEST("Act 2: an area with an Act 2 table uses it in Act 2, and its own table otherwise")
{
    u32 act2Header, ownHeader;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ROUTE22);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ROUTE22);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    ownHeader = GetCurrentMapWildMonHeaderId();
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    act2Header = GetCurrentMapWildMonHeaderId();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    EXPECT_NE(ownHeader, act2Header);
    EXPECT_EQ(gWildMonHeaders[ownHeader].encounterTypes[TIME_OF_DAY_DEFAULT].landMonsInfo->wildPokemon[0].species, SPECIES_RATTATA);
    EXPECT_EQ(gWildMonHeaders[act2Header].encounterTypes[TIME_OF_DAY_DEFAULT].landMonsInfo->wildPokemon[0].species, SPECIES_ODDISH);
}

TEST("Act 2: it's night from coming back to the dorms until the club's battle")
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FIRE_SEE_KAI);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED);
    EXPECT_EQ(GetTimeOfDay(), TIME_NIGHT);
    VarSet(VAR_ACT2_PROGRESS, ACT2_CLUB_1_DONE);
    EXPECT(!Act2_IsClubNight());
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    VarSet(VAR_ACT2_PROGRESS, 0);
}

TEST("Act 2: the research needs 3 different species of the course's type, not counting the Lapras")
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_PSYDUCK, 10, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[1], SPECIES_PSYDUCK, 10, 0, OTID_STRUCT_PLAYER_ID); // Same species
    CreateMon(&gPlayerParty[2], SPECIES_LAPRAS, 15, 0, OTID_STRUCT_PLAYER_ID);  // A gift
    CreateMon(&gPlayerParty[3], SPECIES_GROWLITHE, 10, 0, OTID_STRUCT_PLAYER_ID); // Another type
    EXPECT_EQ(Act2_CountCaughtSpeciesOfType(TYPE_WATER), 1);
    CreateMon(&gPlayerParty[4], SPECIES_POLIWAG, 10, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[5], SPECIES_GOLDEEN, 10, 0, OTID_STRUCT_PLAYER_ID);
    EXPECT_EQ(Act2_CountCaughtSpeciesOfType(TYPE_WATER), 3);
    ZeroPlayerPartyMons();
}

TEST("Act 2: a new species of the course's type shows the catch counter")
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_WATER_TRIP);
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_PSYDUCK, 10, 0, OTID_STRUCT_PLAYER_ID);
    EXPECT(Act2_BufferResearchCatchMessage(SPECIES_PSYDUCK));
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("Research: 1 of 3 WATER\nPOKéMON caught!")), 0);
    CreateMon(&gPlayerParty[1], SPECIES_PSYDUCK, 10, 0, OTID_STRUCT_PLAYER_ID); // Not new
    EXPECT(!Act2_BufferResearchCatchMessage(SPECIES_PSYDUCK));
    CreateMon(&gPlayerParty[2], SPECIES_GROWLITHE, 10, 0, OTID_STRUCT_PLAYER_ID); // Another type
    EXPECT(!Act2_BufferResearchCatchMessage(SPECIES_GROWLITHE));
    CreateMon(&gPlayerParty[3], SPECIES_POLIWAG, 10, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[4], SPECIES_GOLDEEN, 10, 0, OTID_STRUCT_PLAYER_ID);
    EXPECT(Act2_BufferResearchCatchMessage(SPECIES_GOLDEEN));
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("Research: 3 of 3 WATER\nPOKéMON caught!")), 0);
    CreateMon(&gPlayerParty[5], SPECIES_STARYU, 10, 0, OTID_STRUCT_PLAYER_ID); // The task was done already
    EXPECT(!Act2_BufferResearchCatchMessage(SPECIES_STARYU));
    VarSet(VAR_ACT2_PROGRESS, 0);
    EXPECT(!Act2_BufferResearchCatchMessage(SPECIES_STARYU)); // Not on a course trip
    ZeroPlayerPartyMons();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}
