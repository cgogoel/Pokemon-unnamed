#include "global.h"
#include "event_data.h"
#include "farmyard.h"
#include "party_menu.h"
#include "pokemon_storage_system.h"
#include "constants/event_objects.h"
#include "constants/story.h"
#include "test/overworld_script.h"
#include "test/test.h"

// Act 1 farmyard, see docs/hack/befriending.md

static void ClearFarm(void)
{
    for (u32 i = 0; i < IN_BOX_COUNT; i++)
        ZeroBoxMonAt(FARM_BOX, i);
}

TEST("Farmyard: in Act 1 a befriending encounter only starts when no Pokémon is following")
{
    ZeroPlayerPartyMons();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    EXPECT(IsBefriendAllowed());

    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 5; );
    EXPECT(!IsBefriendAllowed());

    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    EXPECT(IsBefriendAllowed());
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Farmyard: the gate drops the following Pokémon off in the farm")
{
    ZeroPlayerPartyMons();
    ClearFarm();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TEDDIURSA, 5; );

    EXPECT_EQ(DepositFollowerInFarm(), 0);
    EXPECT_EQ(CalculatePlayerPartyCount(), 0);
    EXPECT_EQ(GetBoxMonDataAt(FARM_BOX, 0, MON_DATA_SPECIES), SPECIES_TEDDIURSA);
    EXPECT_EQ(CountFarmyardMons(), 1);
}

TEST("Farmyard: with nobody following, nothing is dropped off")
{
    ZeroPlayerPartyMons();
    ClearFarm();
    EXPECT_EQ(DepositFollowerInFarm(), FARM_DEPOSIT_NONE);
    EXPECT_EQ(CountFarmyardMons(), 0);
}

TEST("Farmyard: each farm Pokémon is shown on the map, empty spots are hidden")
{
    ZeroPlayerPartyMons();
    ClearFarm();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TYNAMO, 5; );
    DepositFollowerInFarm();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_HATENNA, 5; );
    DepositFollowerInFarm();

    SetupFarmyardMons();
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0) & ~(OBJ_EVENT_MON | OBJ_EVENT_MON_SHINY | OBJ_EVENT_MON_FEMALE), SPECIES_TYNAMO);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_1) & ~(OBJ_EVENT_MON | OBJ_EVENT_MON_SHINY | OBJ_EVENT_MON_FEMALE), SPECIES_HATENNA);
    EXPECT(VarGet(VAR_OBJ_GFX_ID_0) & OBJ_EVENT_MON);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_3));
    EXPECT(FlagGet(FLAG_TEMP_1 + FARM_SHOWN_COUNT - 1));
}
