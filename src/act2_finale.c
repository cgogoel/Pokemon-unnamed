#include "global.h"
#include "act2_finale.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "constants/farmyard.h"
#include "constants/story.h"

// Act 2's finale, the final project (docs/hack/act2_finale.md)

// ---------------------------------------------------------------------------------------------------------------
// The Woods' noise: after Mom and Dad's talk, an eerie noise comes from the ranger station (its door, top left).
// Every WOODS_NOISE_STEP closer it's louder, every WOODS_NOISE_STEP farther it's quieter. VAR_ACT2_WOODS_SOUND is
// the distance when the noise was last described; VAR_TEMP_1 is 2 once the first line has played this visit.
#define WOODS_STATION_X  5
#define WOODS_STATION_Y  9
#define WOODS_NOISE_STEP 10
#define WOODS_NOISE_NEAR 12 // Closer than this, the louder line says where the noise comes from

static bool32 IsWoodsNoiseActive(void)
{
    return VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY
        && VarGet(VAR_ACT2_PROGRESS) == ACT2_PARENTS_TALKED
        && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_WOODS)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_WOODS);
}

// Straight-line distance in squares, rounded down
static u32 GetWoodsNoiseDistance(void)
{
    s16 x, y;
    u32 dx, dy, d2, d = 0;
    PlayerGetDestCoords(&x, &y);
    dx = abs(x - MAP_OFFSET - WOODS_STATION_X);
    dy = abs(y - MAP_OFFSET - WOODS_STATION_Y);
    d2 = dx * dx + dy * dy;
    while ((d + 1) * (d + 1) <= d2)
        d++;
    return d;
}

// special Act2_ResetWoodsNoise: the first line plays here (on entering the Woods)
void Act2_ResetWoodsNoise(void)
{
    VarSet(VAR_ACT2_WOODS_SOUND, GetWoodsNoiseDistance());
}

// From TryStartMiscWalkingScripts, after each step
bool32 Act2_TryStartWoodsNoise(void)
{
    u32 distance, last;
    if (!IsWoodsNoiseActive() || VarGet(VAR_TEMP_1) != 2)
        return FALSE;
    distance = GetWoodsNoiseDistance();
    last = VarGet(VAR_ACT2_WOODS_SOUND);
    if (distance + WOODS_NOISE_STEP <= last)
    {
        VarSet(VAR_ACT2_WOODS_SOUND, distance);
        ScriptContext_SetupScript(distance < WOODS_NOISE_NEAR ? Act2_EventScript_WoodsNoiseNear : Act2_EventScript_WoodsNoiseLouder);
        return TRUE;
    }
    if (distance >= last + WOODS_NOISE_STEP)
    {
        VarSet(VAR_ACT2_WOODS_SOUND, distance);
        ScriptContext_SetupScript(Act2_EventScript_WoodsNoiseQuieter);
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------------------------
// The chamber's battle is always a 3v3. specialvar VAR_RESULT, Act2_FillChamberParty (on arriving at B4F): with
// fewer than three Pokémon in the party, the first ones stored in the PC join it (the farm's box last). VAR_RESULT:
// how many came along.
#define CHAMBER_PARTY_SIZE 3

static bool32 TakeFromBox(u32 box)
{
    u32 slot;
    for (slot = 0; slot < IN_BOX_COUNT; slot++)
    {
        struct BoxPokemon *boxMon = GetBoxedMonPtr(box, slot);
        if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == SPECIES_NONE || GetBoxMonData(boxMon, MON_DATA_IS_EGG))
            continue;
        BoxMonToMon(boxMon, &gParties[B_TRAINER_PLAYER][gPlayerPartyCount]);
        ZeroBoxMonAt(box, slot);
        CalculatePlayerPartyCount();
        return TRUE;
    }
    return FALSE;
}

u16 Act2_FillChamberParty(void)
{
    u32 box, moved = 0;
    CalculatePlayerPartyCount();
    while (gPlayerPartyCount < CHAMBER_PARTY_SIZE)
    {
        bool32 found = FALSE;
        for (box = 0; box < TOTAL_BOXES_COUNT && !found; box++)
        {
            if (box != FARM_BOX)
                found = TakeFromBox(box);
        }
        if (!found)
            found = TakeFromBox(FARM_BOX);
        if (!found)
            break;
        moved++;
    }
    return moved;
}
