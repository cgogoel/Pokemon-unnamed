#include "global.h"
#include "act2_story.h"
#include "battle.h"
#include "event_data.h"
#include "move.h"
#include "pokemon.h"
#include "rtc.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
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

TEST("Act 2: the Woods' Pokémon start Act 2 with their level's moves, at least 2 of them")
{
    static const u16 sWoods[] = { SPECIES_BLIPBUG, SPECIES_TYNAMO, SPECIES_HATENNA, SPECIES_TEDDIURSA, SPECIES_JANGMO_O };
    u32 i, j, moves;
    ZeroPlayerPartyMons();
    for (i = 0; i < ARRAY_COUNT(sWoods); i++)
        CreateMon(&gPlayerParty[i], sWoods[i], 5, 0, OTID_STRUCT_PLAYER_ID); // CreateMon gives no moves
    Act2_RaisePartyLevels();
    for (i = 0; i < ARRAY_COUNT(sWoods); i++)
    {
        EXPECT_EQ(GetMonData(&gPlayerParty[i], MON_DATA_LEVEL), ACT2_START_LEVEL);
        for (moves = 0, j = 0; j < MAX_MON_MOVES; j++)
        {
            if (GetMonData(&gPlayerParty[i], MON_DATA_MOVE1 + j) != MOVE_NONE)
            {
                moves++;
                EXPECT_GT(GetMonData(&gPlayerParty[i], MON_DATA_PP1 + j), 0);
            }
        }
        EXPECT_GE(moves, 2);
    }
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_MOVE1), MOVE_STRUGGLE_BUG); // Blipbug's only move by then
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_MOVE2), MOVE_TACKLE);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_MOVE1), MOVE_TACKLE);       // Tynamo's level-1 moves
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_MOVE4), MOVE_CHARGE_BEAM);
    ZeroPlayerPartyMons();
}

TEST("Act 2: the playtest party knows its moves")
{
    Act2_GivePlaytestParty();
    EXPECT_NE(GetMonData(&gPlayerParty[0], MON_DATA_MOVE1), MOVE_NONE);
    EXPECT_NE(GetMonData(&gPlayerParty[0], MON_DATA_MOVE2), MOVE_NONE);
    EXPECT_NE(GetMonData(&gPlayerParty[1], MON_DATA_MOVE2), MOVE_NONE);
    ZeroPlayerPartyMons();
}

TEST("Act 2: the research counts 3 different moves of the type, whatever their IDs")
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP);
    VarSet(VAR_ACT2_RESEARCH, 0);
    Act2_RecordMoveUse(1, MOVE_EMBER);
    Act2_RecordMoveUse(1, MOVE_EMBER);         // Seen already
    Act2_RecordMoveUse(1, MOVE_SCRATCH);        // Not Fire
    Act2_RecordMoveUse(1, MOVE_FLAME_WHEEL);
    EXPECT(Act2_TakeResearchMoveMessage());
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("Research: 2 of 3 FIRE\nmoves recorded!")), 0);
    Act2_RecordMoveUse(1, MOVE_FLAME_CHARGE);
    EXPECT(Act2_TakeResearchMoveMessage());
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("Research: 3 of 3 FIRE\nmoves recorded!")), 0);
    Act2_RecordMoveUse(1, MOVE_INCINERATE);    // Done already: no more counters
    EXPECT(!Act2_TakeResearchMoveMessage());
}

static bool32 MonKnowsMoveForTest(struct Pokemon *mon, u32 move)
{
    u32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    }
    return FALSE;
}

TEST("Act 2: on a field trip, wild Pokémon of the course's type know its moves from a few levels ahead")
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 i, fireMoves = 0;
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP);
    CreateWildMon(SPECIES_GROWLITHE, 8);
    EXPECT(MonKnowsMoveForTest(mon, MOVE_EMBER));
    EXPECT(MonKnowsMoveForTest(mon, MOVE_FLAME_WHEEL)); // Level 12 in its learnset
    for (i = 0; i < MAX_MON_MOVES; i++)
        fireMoves += GetMoveType(GetMonData(mon, MON_DATA_MOVE1 + i)) == TYPE_FIRE;
    EXPECT_GE(fireMoves, 2);
    CreateWildMon(SPECIES_RATTATA, 8); // Not the course's type: its usual moves
    EXPECT(!MonKnowsMoveForTest(mon, MOVE_EMBER));
}

TEST("Act 2: a wild Pokémon that knew a move early skips it when it levels up to it")
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][0];
    u32 i, j, level = 12; // Growlithe learns Flame Wheel at 12
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP);
    CreateWildMon(SPECIES_GROWLITHE, 8);
    EXPECT(MonKnowsMoveForTest(mon, MOVE_FLAME_WHEEL));
    SetMonData(mon, MON_DATA_LEVEL, &level);
    EXPECT_EQ(MonTryLearningNewMove(mon, TRUE), MON_ALREADY_KNOWS_MOVE);
    EXPECT_EQ(MonTryLearningNewMove(mon, FALSE), MOVE_NONE);
    for (i = 0; i < MAX_MON_MOVES; i++) // Nothing learned twice
    {
        for (j = i + 1; j < MAX_MON_MOVES; j++)
        {
            if (GetMonData(mon, MON_DATA_MOVE1 + i) != MOVE_NONE)
                EXPECT_NE(GetMonData(mon, MON_DATA_MOVE1 + i), GetMonData(mon, MON_DATA_MOVE1 + j));
        }
    }

    // Off the trip, the same Growlithe (with 4 moves) is offered it at 12 as usual
    VarSet(VAR_ACT2_PROGRESS, ACT2_START);
    CreateWildMon(SPECIES_GROWLITHE, 8);
    EXPECT(!MonKnowsMoveForTest(mon, MOVE_FLAME_WHEEL));
    SetMonData(mon, MON_DATA_LEVEL, &level);
    EXPECT_EQ(MonTryLearningNewMove(mon, TRUE), MON_HAS_MAX_MOVES);
    EXPECT_EQ(gMoveToLearn, MOVE_FLAME_WHEEL);
}
