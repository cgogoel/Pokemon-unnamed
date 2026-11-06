#include "global.h"
#include "act2_finale.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "overworld.h"
#include "script.h"
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
