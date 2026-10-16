#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "farmyard.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "constants/event_objects.h"
#include "constants/story.h"

// Act 1 farmyard (docs/hack/befriending.md). The player keeps one befriended Pokémon, which follows them; walking
// through the farmyard gate drops it off. The farmyard's Pokémon live in the last PC box and wander around the
// Farmyard map's FARM_SHOWN_COUNT spots: Tynamo's family in the pond spot, the others in the rest (GetFarmSpot).

static const u8 sText_FarmBoxName[] = _("FARM");

// Act 1: a befriending encounter only starts when no Pokémon is following (the party holds at most that one).
// When it doesn't, STR_VAR_1 is the follower's name.
bool32 IsBefriendAllowed(void)
{
    if (VarGet(VAR_STORY_ACT) != STORY_ACT_CHILDHOOD || CalculatePlayerPartyCount() == 0)
        return TRUE;
    GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    return FALSE;
}

// special CheckBefriendAllowed: VAR_RESULT TRUE/FALSE
void CheckBefriendAllowed(void)
{
    gSpecialVar_Result = IsBefriendAllowed();
}

static s32 GetEmptyFarmSlot(void)
{
    for (u32 i = 0; i < IN_BOX_COUNT; i++)
    {
        if (GetBoxMonDataAt(FARM_BOX, i, MON_DATA_SPECIES) == SPECIES_NONE)
            return i;
    }
    return -1;
}

// Moves the first party Pokémon (the follower) into the farm. Returns the farm slot it went to, or
// FARM_DEPOSIT_NONE when there was nothing to drop off or the farm is full. STR_VAR_1 is its name.
u32 DepositFollowerInFarm(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    s32 slot;

    if (CalculatePlayerPartyCount() == 0 || GetMonData(mon, MON_DATA_IS_EGG) || (slot = GetEmptyFarmSlot()) < 0)
        return FARM_DEPOSIT_NONE;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    SetBoxMonAt(FARM_BOX, slot, &mon->box);
    StringCopy(GetBoxNamePtr(FARM_BOX), sText_FarmBoxName);
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    UpdateFollowingPokemon();
    SetupFarmyardMons();
    return slot;
}

static bool32 LivesInWater(u32 species)
{
    return species == SPECIES_TYNAMO || species == SPECIES_EELEKTRIK || species == SPECIES_EELEKTROSS;
}

// Which spot on the Farmyard map the farm Pokémon in a box slot is shown at, or FARM_SPOT_NONE
u32 GetFarmSpot(u32 slot)
{
    u32 nextSpot = 0;
    bool32 waterTaken = FALSE;

    for (u32 i = 0; i < IN_BOX_COUNT; i++)
    {
        u32 species = GetBoxMonDataAt(FARM_BOX, i, MON_DATA_SPECIES);
        u32 spot = FARM_SPOT_NONE;

        if (species == SPECIES_NONE || GetBoxMonDataAt(FARM_BOX, i, MON_DATA_IS_EGG))
            continue;
        if (LivesInWater(species))
        {
            if (!waterTaken)
                spot = FARM_WATER_SPOT;
            waterTaken = TRUE;
        }
        else
        {
            if (nextSpot == FARM_WATER_SPOT)
                nextSpot++;
            if (nextSpot < FARM_SHOWN_COUNT)
                spot = nextSpot++;
        }
        if (i == slot)
            return spot;
    }
    return FARM_SPOT_NONE;
}

// special DepositFollowerInFarm: VAR_RESULT is the local id of its object on the Farmyard map (spot + 1) if it's
// one of the shown ones, FARM_DEPOSIT_NONE if nothing was dropped off, 0 if it's in the farm but not shown.
void Special_DepositFollowerInFarm(void)
{
    u32 slot = DepositFollowerInFarm();
    u32 spot;

    if (slot == FARM_DEPOSIT_NONE)
    {
        gSpecialVar_Result = FARM_DEPOSIT_NONE;
        return;
    }
    spot = GetFarmSpot(slot);
    gSpecialVar_Result = (spot != FARM_SPOT_NONE) ? spot + 1 : 0;
}

// Sets the Farmyard map's Pokémon objects: graphics from the farm box, hidden where the spot is empty.
void SetupFarmyardMons(void)
{
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
        FlagSet(FLAG_TEMP_1 + i);

    for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
    {
        struct BoxPokemon *boxMon = GetBoxedMonPtr(FARM_BOX, slot);
        u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);
        u32 i = GetFarmSpot(slot);

        if (i == FARM_SPOT_NONE)
            continue;
        u32 gfx = species + OBJ_EVENT_MON;
        if (GetBoxMonData(boxMon, MON_DATA_IS_SHINY))
            gfx += OBJ_EVENT_MON_SHINY;
        if (GetBoxMonGender(boxMon) == MON_FEMALE)
            gfx += OBJ_EVENT_MON_FEMALE;
        VarSet(VAR_OBJ_GFX_ID_0 + i, gfx);
        FlagClear(FLAG_TEMP_1 + i);
    }
}

u32 CountFarmyardMons(void)
{
    u32 count = 0;
    for (u32 i = 0; i < IN_BOX_COUNT; i++)
    {
        if (GetBoxMonDataAt(FARM_BOX, i, MON_DATA_SPECIES) != SPECIES_NONE)
            count++;
    }
    return count;
}
