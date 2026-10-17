#include "global.h"
#include "act2_story.h"
#include "event_data.h"
#include "pokemon.h"
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
