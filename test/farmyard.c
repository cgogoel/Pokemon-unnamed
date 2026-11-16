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
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_HATENNA, 5; );
    DepositFollowerInFarm();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TEDDIURSA, 5; );
    DepositFollowerInFarm();

    SetupFarmyardMons();
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0) & ~(OBJ_EVENT_MON | OBJ_EVENT_MON_SHINY | OBJ_EVENT_MON_FEMALE), SPECIES_HATENNA);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_1) & ~(OBJ_EVENT_MON | OBJ_EVENT_MON_SHINY | OBJ_EVENT_MON_FEMALE), SPECIES_TEDDIURSA);
    EXPECT(VarGet(VAR_OBJ_GFX_ID_0) & OBJ_EVENT_MON);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_1 + FARM_WATER_SPOT));
    EXPECT(FlagGet(FLAG_TEMP_1 + FARM_SHOWN_COUNT - 1));
}

TEST("Farmyard: Tynamo goes to the pond spot, whenever it was dropped off")
{
    ZeroPlayerPartyMons();
    ClearFarm();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TYNAMO, 5; );
    DepositFollowerInFarm();
    EXPECT_EQ(GetFarmSpot(0), FARM_WATER_SPOT);

    SetupFarmyardMons();
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0 + FARM_WATER_SPOT) & ~(OBJ_EVENT_MON | OBJ_EVENT_MON_SHINY | OBJ_EVENT_MON_FEMALE), SPECIES_TYNAMO);
    EXPECT(!FlagGet(FLAG_TEMP_1 + FARM_WATER_SPOT));
    EXPECT(FlagGet(FLAG_TEMP_1));
}

TEST("Farmyard: no other Pokémon takes the pond spot, even when the farm is full")
{
    static const u16 species[] = { SPECIES_BLIPBUG, SPECIES_HATENNA, SPECIES_TEDDIURSA, SPECIES_JANGMO_O,
                                   SPECIES_DOTTLER, SPECIES_HATTREM, SPECIES_URSARING, SPECIES_HAKAMO_O, SPECIES_ORBEETLE };
    ClearFarm();
    for (u32 i = 0; i < ARRAY_COUNT(species); i++)
    {
        struct Pokemon mon;
        CreateRandomMon(&mon, species[i], 5);
        SetBoxMonAt(FARM_BOX, i, &mon.box);
    }

    for (u32 slot = 0; slot < ARRAY_COUNT(species); slot++)
        EXPECT_NE(GetFarmSpot(slot), FARM_WATER_SPOT);
    EXPECT_EQ(GetFarmSpot(0), 0);
    EXPECT_EQ(GetFarmSpot(2), 3); // Skips the pond
    EXPECT_EQ(GetFarmSpot(6), FARM_SHOWN_COUNT - 1);
    EXPECT_EQ(GetFarmSpot(7), FARM_SPOT_NONE); // 7 land spots
}

// Acts 2 and 3: the Pokémon the player grew up with run out to play when they're brought home

// givemon rolls gender and shininess, so the object graphics a spot should hold depend on the Pokemon that got it.
// What these tests are really checking is which spot each Pokemon is put in.
static u32 SpotGfxFor(struct Pokemon *mon)
{
    u32 gfx = GetMonData(mon, MON_DATA_SPECIES) + OBJ_EVENT_MON;

    if (GetMonData(mon, MON_DATA_IS_SHINY))
        gfx += OBJ_EVENT_MON_SHINY;
    if (GetMonGender(mon) == MON_FEMALE)
        gfx += OBJ_EVENT_MON_FEMALE;
    return gfx;
}

static void SetUpVisit(u32 act)
{
    ZeroPlayerPartyMons();
    ClearFarm();
    VarSet(VAR_STORY_ACT, act);
    FlagClear(FLAG_TEMP_HIDE_FOLLOWER);
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
    {
        FlagSet(FLAG_TEMP_1 + i);
        VarSet(VAR_OBJ_GFX_ID_0 + i, 0);
    }
}

TEST("Farmyard: in Act 1 nobody runs out to play, since the party Pokémon is the one following")
{
    SetUpVisit(STORY_ACT_CHILDHOOD);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 5; );

    EXPECT_EQ(ShowPartyFarmFamily(), 0);
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
        EXPECT(FlagGet(FLAG_TEMP_1 + i));
}

TEST("Farmyard: the lead runs out whatever it is, the rest only if they grew up here")
{
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 20; );

    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT(!FlagGet(FLAG_TEMP_1)); // The first land spot is shown...
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), SpotGfxFor(&gParties[B_TRAINER_PLAYER][0]));
    EXPECT(FlagGet(FLAG_TEMP_1 + 1)); // ...and nothing else is

    // One the player never grew up with, at the head of the party: out it goes all the same, since it's the one
    // that would otherwise be walking at their heel
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_PIKACHU, 20; );
    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), SpotGfxFor(&gParties[B_TRAINER_PLAYER][0]));

    // Behind the lead, only the farm's own: the PIKACHU leads and comes out, the BLIPBUG grew up here and follows
    // it out, the RATTATA stays in its ball
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_PIKACHU, 20; );
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 20; );
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_RATTATA, 20; );
    EXPECT_EQ(ShowPartyFarmFamily(), 2);

    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Farmyard: it still counts once it has evolved, and Tynamo's line keeps to the pond")
{
    SetUpVisit(STORY_ACT_WORKFORCE);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_EELEKTRIK, 40; );

    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT(!FlagGet(FLAG_TEMP_1 + FARM_WATER_SPOT));
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0 + FARM_WATER_SPOT), SpotGfxFor(&gParties[B_TRAINER_PLAYER][0]));
    EXPECT(FlagGet(FLAG_TEMP_1)); // The land spots stay empty

    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Farmyard: they take spots the farm's own Pokémon aren't standing in")
{
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TEDDIURSA, 5; );
    EXPECT_EQ(DepositFollowerInFarm(), 0); // Lives on the farm, in the first land spot
    SetupFarmyardMons();
    EXPECT(!FlagGet(FLAG_TEMP_1));

    u32 farmGfx = VarGet(VAR_OBJ_GFX_ID_0);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_HATENNA, 20; );
    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0), farmGfx); // The farm's own, undisturbed in the first land spot
    EXPECT(!FlagGet(FLAG_TEMP_1 + 1)); // The party's own took the next one
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_0 + 1), SpotGfxFor(&gParties[B_TRAINER_PLAYER][0]));

    ClearFarm();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

// Acts 2 and 3: taking one of the left-behind farm Pokémon along

TEST("Farmyard: in Act 2 the farm Pokémon the player is talking to can be taken along")
{
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TEDDIURSA, 5; );
    EXPECT_EQ(DepositFollowerInFarm(), 0); // Left behind on the farm, in the first land spot
    SetupFarmyardMons();

    gSpecialVar_LastTalked = 1; // Its object: the first land spot + 1
    EXPECT_EQ(GetFarmMonToTake(), TRUE);
    EXPECT_EQ(gSpecialVar_0x8004, SPECIES_TEDDIURSA);
    EXPECT_EQ(gSpecialVar_0x8005, 0);

    EXPECT(TakeFarmMonIntoParty());
    EXPECT_EQ(CalculatePlayerPartyCount(), 1);
    EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), SPECIES_TEDDIURSA);
    EXPECT_EQ(GetBoxMonDataAt(FARM_BOX, 0, MON_DATA_SPECIES), SPECIES_NONE); // Gone from the farm
    EXPECT_EQ(CountFarmyardMons(), 0);
    EXPECT(FlagGet(FLAG_TEMP_1)); // Its spot is empty again

    // And now that it's one of the player's, it runs out to play when they come home
    EXPECT_EQ(ShowPartyFarmFamily(), 1);

    ClearFarm();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Farmyard: talking to nothing, or to one with a full team, takes nobody along")
{
    SetUpVisit(STORY_ACT_UNIVERSITY);

    // No farm Pokémon has that object
    gSpecialVar_LastTalked = 1;
    EXPECT_EQ(GetFarmMonToTake(), FALSE);

    RUN_OVERWORLD_SCRIPT( givemon SPECIES_HATENNA, 5; );
    EXPECT_EQ(DepositFollowerInFarm(), 0);
    SetupFarmyardMons();
    for (u32 i = 0; i < PARTY_SIZE; i++)
        RUN_OVERWORLD_SCRIPT( givemon SPECIES_PIKACHU, 5; );
    EXPECT_EQ(CalculatePlayerPartyCount(), PARTY_SIZE);

    EXPECT_EQ(GetFarmMonToTake(), FARM_TAKE_PARTY_FULL);
    EXPECT(!TakeFarmMonIntoParty()); // Refused, and it stays put
    EXPECT_EQ(GetBoxMonDataAt(FARM_BOX, 0, MON_DATA_SPECIES), SPECIES_HATENNA);
    EXPECT_EQ(CalculatePlayerPartyCount(), PARTY_SIZE);

    ZeroPlayerPartyMons();
    ClearFarm();
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Farmyard: coming home in Acts 2 and 3 puts the follower away, Act 1 keeps it out")
{
    // Act 1: the one Pokémon walking with the player is that act's whole mechanic, so it stays at their heel.
    SetUpVisit(STORY_ACT_CHILDHOOD);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 5; );
    ShowPartyFarmFamily();
    EXPECT(!FlagGet(FLAG_TEMP_HIDE_FOLLOWER));

    // Act 2: it's one of the ones running around the farmyard, so having it heel as well would show it twice
    SetUpVisit(STORY_ACT_UNIVERSITY);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 5; );
    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT(FlagGet(FLAG_TEMP_HIDE_FOLLOWER));

    // Act 3, with nobody from the farm in the party: the lead is out playing, so it's put away as a follower too
    SetUpVisit(STORY_ACT_WORKFORCE);
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_PIKACHU, 5; );
    EXPECT_EQ(ShowPartyFarmFamily(), 1);
    EXPECT(FlagGet(FLAG_TEMP_HIDE_FOLLOWER));

    FlagClear(FLAG_TEMP_HIDE_FOLLOWER);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}
