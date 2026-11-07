#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "tm_research.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/moves.h"

// The TM Database (docs/hack/tm_database.md)

#define TM(move) (CAT(ENUM_TM_HM_, move) + 1)

static void SetUp(void)
{
    u32 tm;
    memset(gSaveBlock3Ptr->tmResearch, 0, sizeof(gSaveBlock3Ptr->tmResearch));
    memset(gSaveBlock1Ptr->dexCaught, 0, sizeof(gSaveBlock1Ptr->dexCaught));
    for (tm = 1; tm <= NUM_TECHNICAL_MACHINES; tm++)
        RemoveBagItem(TmResearch_GetItem(tm), 99);
    FlagSet(FLAG_TM_DATABASE_UNLOCKED);
    TmResearch_TakeMoveMessage(); // Nothing pending from an earlier test
}

static void CatchSpecies(u32 species)
{
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
    TmResearch_BufferCatchMessage(species);
}

TEST("TM research: a TM's tier and requirement follow its move's power")
{
    EXPECT_EQ(TmResearch_GetTier(TM(CALM_MIND)), 1);    // Status
    EXPECT_EQ(TmResearch_GetTier(TM(BULLET_SEED)), 1);  // 25
    EXPECT_EQ(TmResearch_GetTier(TM(WATER_PULSE)), 1);  // 60
    EXPECT_EQ(TmResearch_GetTier(TM(SECRET_POWER)), 2); // 70
    EXPECT_EQ(TmResearch_GetTier(TM(FLAMETHROWER)), 2); // 90
    EXPECT_EQ(TmResearch_GetTier(TM(RETURN)), 2);       // Its power varies
    EXPECT_EQ(TmResearch_GetTier(TM(FIRE_BLAST)), 3);   // 110
    EXPECT_EQ(TmResearch_GetTier(TM(OVERHEAT)), 4);     // 130
    EXPECT_EQ(TmResearch_GetRequirement(TM(CALM_MIND)), 2);
    EXPECT_EQ(TmResearch_GetRequirement(TM(FLAMETHROWER)), 3);
    EXPECT_EQ(TmResearch_GetRequirement(TM(FIRE_BLAST)), 4);
    EXPECT_EQ(TmResearch_GetRequirement(TM(OVERHEAT)), 5);
    EXPECT_EQ(TmResearch_GetType(TM(FLAMETHROWER)), TYPE_FIRE);
}

TEST("TM research: every TM but Tri-Blast is in the database; owned ones show as owned")
{
    SetUp();
    EXPECT(!TmResearch_IsResearchable(TM(TRI_BLAST)));
    EXPECT_EQ(TmResearch_GetStatus(TM(TRI_BLAST)), TM_STATUS_UNAVAILABLE);
    EXPECT_EQ(TmResearch_CountResearchable(), NUM_TECHNICAL_MACHINES - 1);
    EXPECT_EQ(TmResearch_GetStatus(TM(FLAMETHROWER)), TM_STATUS_AVAILABLE);
    AddBagItem(ITEM_TM_FLAMETHROWER, 1);
    EXPECT_EQ(TmResearch_GetStatus(TM(FLAMETHROWER)), TM_STATUS_OWNED);
    EXPECT_EQ(TmResearch_CountOwned(), 1);
    EXPECT(!TmResearch_Start(0, TM(FLAMETHROWER)));
}

TEST("TM research: up to 3 slots, one TM per type")
{
    SetUp();
    EXPECT(TmResearch_Start(0, TM(FLAMETHROWER)));
    EXPECT_EQ(TmResearch_GetStatus(TM(FLAMETHROWER)), TM_STATUS_TRACKED);
    EXPECT_EQ(TmResearch_GetStatus(TM(FIRE_BLAST)), TM_STATUS_TYPE_TAKEN);
    EXPECT(!TmResearch_Start(1, TM(FIRE_BLAST)));
    EXPECT(!TmResearch_Start(0, TM(ICE_BEAM))); // The slot is taken
    EXPECT(TmResearch_Start(1, TM(ICE_BEAM)));
    EXPECT(TmResearch_Start(2, TM(THUNDERBOLT)));
    EXPECT_EQ(TmResearch_FindFreeSlot(), -1);
    EXPECT_EQ(TmResearch_GetStatus(TM(EARTHQUAKE)), TM_STATUS_NO_SLOT);
    EXPECT_EQ(TmResearch_FindSlotOfType(TYPE_ICE), 1);
}

TEST("TM research: different Pokémon and moves of the type count, from the start of the research")
{
    SetUp();
    CatchSpecies(SPECIES_VULPIX); // Before the research: doesn't count
    TmResearch_RecordMoveUse(MOVE_EMBER);
    EXPECT(!TmResearch_TakeMoveMessage());
    EXPECT(TmResearch_Start(0, TM(FLAMETHROWER)));
    EXPECT_EQ(TmResearch_GetCaughtCount(0), 0);

    EXPECT(TmResearch_BufferCatchMessage(SPECIES_VULPIX));
    EXPECT(!TmResearch_BufferCatchMessage(SPECIES_VULPIX));   // The same species again
    EXPECT(!TmResearch_BufferCatchMessage(SPECIES_PSYDUCK));  // Not FIRE
    EXPECT(TmResearch_BufferCatchMessage(SPECIES_SCOVILLAIN)); // GRASS/FIRE: its second type counts
    EXPECT_EQ(TmResearch_GetCaughtCount(0), 2);

    TmResearch_RecordMoveUse(MOVE_EMBER);
    EXPECT(TmResearch_TakeMoveMessage());
    TmResearch_RecordMoveUse(MOVE_EMBER);
    EXPECT(!TmResearch_TakeMoveMessage());
    TmResearch_RecordMoveUse(MOVE_WATER_GUN);
    TmResearch_RecordMoveUse(MOVE_STRUGGLE);
    EXPECT(!TmResearch_TakeMoveMessage());
    TmResearch_RecordMoveUse(MOVE_FIRE_SPIN);
    EXPECT(TmResearch_TakeMoveMessage());
    EXPECT_EQ(TmResearch_GetMoveCount(0), 2);
    EXPECT(!TmResearch_IsSlotReady(0));

    TmResearch_BufferCatchMessage(SPECIES_PONYTA);
    TmResearch_RecordMoveUse(MOVE_FLAME_WHEEL);
    EXPECT(TmResearch_TakeMoveMessage());
    EXPECT(TmResearch_IsSlotReady(0));
    // Full: no more counting
    TmResearch_RecordMoveUse(MOVE_FIRE_FANG);
    EXPECT(!TmResearch_TakeMoveMessage());
    EXPECT_EQ(TmResearch_GetMoveCount(0), 3);
}

TEST("TM research: the counter's message")
{
    SetUp();
    TmResearch_Start(0, TM(CALM_MIND));
    TmResearch_BufferCatchMessage(SPECIES_ABRA);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("TM research: 1 of 2 PSYCHIC\nPOKéMON caught!")), 0);
    TmResearch_BufferCatchMessage(SPECIES_DROWZEE);
    TmResearch_RecordMoveUse(MOVE_CONFUSION);
    TmResearch_RecordMoveUse(MOVE_PSYBEAM);
    TmResearch_TakeMoveMessage();
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("TM research: 2 of 2 PSYCHIC\nmoves recorded!\pCalm Mind is ready\nto craft in QUESTS!")), 0);
}

TEST("TM research: crafting gives the TM and frees the slot")
{
    SetUp();
    TmResearch_Start(1, TM(CALM_MIND));
    EXPECT(!TmResearch_CraftSlot(1)); // Not ready
    TmResearch_BufferCatchMessage(SPECIES_ABRA);
    TmResearch_BufferCatchMessage(SPECIES_DROWZEE);
    TmResearch_RecordMoveUse(MOVE_CONFUSION);
    TmResearch_RecordMoveUse(MOVE_PSYBEAM);
    EXPECT(TmResearch_CraftSlot(1));
    EXPECT(CheckBagHasItem(ITEM_TM_CALM_MIND, 1));
    EXPECT_EQ(TmResearch_GetSlotTm(1), 0);
    EXPECT_EQ(TmResearch_GetStatus(TM(CALM_MIND)), TM_STATUS_OWNED);
    // A new one starts from nothing
    EXPECT(TmResearch_Start(1, TM(PSYCHIC)));
    EXPECT_EQ(TmResearch_GetCaughtCount(1), 0);
    EXPECT_EQ(TmResearch_GetMoveCount(1), 0);
}

TEST("TM research: clearing a slot stops the research and loses its counts")
{
    SetUp();
    TmResearch_Start(2, TM(FLAMETHROWER));
    TmResearch_BufferCatchMessage(SPECIES_VULPIX);
    TmResearch_ClearSlot(2);
    EXPECT_EQ(TmResearch_GetSlotTm(2), 0);
    EXPECT_EQ(TmResearch_GetStatus(TM(FIRE_BLAST)), TM_STATUS_AVAILABLE);
    EXPECT(!TmResearch_BufferCatchMessage(SPECIES_GROWLITHE));
    TmResearch_Start(2, TM(FLAMETHROWER));
    EXPECT_EQ(TmResearch_GetCaughtCount(2), 0);
}

TEST("TM research: catching 25 POKéMON of a type unlocks all its TMs")
{
    u32 total, caught;
    SetUp();
    caught = TmResearch_CountTypeCaught(TYPE_FIRE, &total);
    EXPECT_EQ(caught, 0);
    EXPECT_EQ(total, 25);
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_CHARMANDER), FLAG_SET_CAUGHT);
    EXPECT_EQ(TmResearch_CountTypeCaught(TYPE_FIRE, &total), 1);
    EXPECT(!TmResearch_IsTypeComplete(TYPE_FIRE));

    TmResearch_Start(0, TM(FLAMETHROWER));
    memset(gSaveBlock1Ptr->dexCaught, 0xFF, sizeof(gSaveBlock1Ptr->dexCaught));
    EXPECT(TmResearch_IsTypeComplete(TYPE_FIRE));
    EXPECT(TmResearch_IsSlotReady(0));                                       // The tracked one
    EXPECT_EQ(TmResearch_GetStatus(TM(OVERHEAT)), TM_STATUS_READY);          // And the rest, untracked
    EXPECT(TmResearch_CraftReadyTm(TM(OVERHEAT)));
    EXPECT(CheckBagHasItem(ITEM_TM_OVERHEAT, 1));
    EXPECT_EQ(TmResearch_GetStatus(TM(OVERHEAT)), TM_STATUS_OWNED);
    EXPECT(TmResearch_CraftSlot(0));
    EXPECT(CheckBagHasItem(ITEM_TM_FLAMETHROWER, 1));
}
