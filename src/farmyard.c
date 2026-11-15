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

// Act 1's five befriendable Pokemon (sWoodsPokemon in src/act1_story.c, plus Jangmo-o). A party Pokemon whose line
// starts at one of these is one the player grew up with on the farm, however far it has evolved since.
static const u16 sFarmFamilies[] =
{
    SPECIES_BLIPBUG,
    SPECIES_TYNAMO,
    SPECIES_HATENNA,
    SPECIES_TEDDIURSA,
    SPECIES_JANGMO_O,
};

static bool32 IsFarmFamily(u32 species)
{
    u32 base = species, pre;

    while ((pre = GetSpeciesPreEvolution(base)) != SPECIES_NONE && pre != base)
        base = pre;
    for (u32 i = 0; i < ARRAY_COUNT(sFarmFamilies); i++)
    {
        if (sFarmFamilies[i] == base)
            return TRUE;
    }
    return FALSE;
}

// The farmyard spots the Pokemon living in the farm box are already standing in
static void GetTakenFarmSpots(bool32 *taken)
{
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
        taken[i] = FALSE;
    for (u32 slot = 0; slot < IN_BOX_COUNT; slot++)
    {
        u32 spot = GetFarmSpot(slot);

        if (spot != FARM_SPOT_NONE && spot < FARM_SHOWN_COUNT)
            taken[spot] = TRUE;
    }
}

static u32 TakeFreeFarmSpot(bool32 *taken)
{
    for (u32 i = 0; i < FARM_SHOWN_COUNT; i++)
    {
        if (i == FARM_WATER_SPOT || taken[i])
            continue;
        taken[i] = TRUE;
        return i;
    }
    return FARM_SPOT_NONE;
}

// Acts 2 and 3: the Pokemon the player grew up with run out to play whenever they bring them home. They're only
// shown on the map - they stay in the party the whole time, so they're back at the player's side on the way out.
// Returns how many came out, with the first one's name in STR_VAR_1.
u32 ShowPartyFarmFamily(void)
{
    bool32 taken[FARM_SHOWN_COUNT];
    u32 count = 0, partyCount = CalculatePlayerPartyCount();

    if (VarGet(VAR_STORY_ACT) == STORY_ACT_CHILDHOOD)
        return 0;
    GetTakenFarmSpots(taken);

    for (u32 i = 0; i < partyCount; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 spot, gfx;

        if (GetMonData(mon, MON_DATA_IS_EGG) || !IsFarmFamily(species))
            continue;
        if (LivesInWater(species) && !taken[FARM_WATER_SPOT])
        {
            spot = FARM_WATER_SPOT;
            taken[spot] = TRUE;
        }
        else
        {
            spot = TakeFreeFarmSpot(taken);
        }
        if (spot == FARM_SPOT_NONE)
            continue;

        gfx = species + OBJ_EVENT_MON;
        if (GetMonData(mon, MON_DATA_IS_SHINY))
            gfx += OBJ_EVENT_MON_SHINY;
        if (GetMonGender(mon) == MON_FEMALE)
            gfx += OBJ_EVENT_MON_FEMALE;
        VarSet(VAR_OBJ_GFX_ID_0 + spot, gfx);
        FlagClear(FLAG_TEMP_1 + spot);
        if (count == 0)
        {
            GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
            StringGet_Nickname(gStringVar1);
        }
        count++;
    }
    return count;
}

// special ShowPartyFarmFamily: VAR_RESULT is how many came out to play
void Special_ShowPartyFarmFamily(void)
{
    gSpecialVar_Result = ShowPartyFarmFamily();
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

        if (i == FARM_SPOT_NONE || (VarGet(VAR_ACT1_CHOSEN_FRIENDS) & (1 << slot)))
            continue; // Not shown, or chosen to come along at the end of Act 1 (docs/hack/act1_story.md)
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
