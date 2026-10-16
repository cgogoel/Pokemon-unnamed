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
// through the farmyard gate drops it off. The farmyard's Pokémon live in the last PC box and the first
// FARM_SHOWN_COUNT of them wander around the Farmyard map.

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

// special DepositFollowerInFarm: VAR_RESULT is the local id of its object on the Farmyard map (slot + 1) if it's
// one of the shown ones, FARM_DEPOSIT_NONE if nothing was dropped off, 0 if it's in the farm but not shown.
void Special_DepositFollowerInFarm(void)
{
    u32 slot = DepositFollowerInFarm();

    if (slot == FARM_DEPOSIT_NONE)
        gSpecialVar_Result = FARM_DEPOSIT_NONE;
    else
        gSpecialVar_Result = (slot < FARM_SHOWN_COUNT) ? slot + 1 : 0;
}

// Sets the Farmyard map's Pokémon objects: graphics from the farm box, hidden where the spot is empty.
void SetupFarmyardMons(void)
{
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
    {
        struct BoxPokemon *boxMon = GetBoxedMonPtr(FARM_BOX, i);
        u32 species = GetBoxMonData(boxMon, MON_DATA_SPECIES);

        if (species == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
        {
            FlagSet(FLAG_TEMP_1 + i);
            continue;
        }
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
