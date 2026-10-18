#include "global.h"
#include "act2_story.h"
#include "event_data.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "heal_location.h"
#include "heal_dash.h"
#include "overworld.h"
#include "constants/heal_locations.h"
#include "constants/story.h"

// The Heal Dash (Acts 2 and 3): the player spins away, as with Teleport, and lands in front of the nurse of the last
// Pokémon Center they went into (the one they'd go back to after blacking out). On an Act 2 field trip it's always
// the Pokémon Center of the trip's town, even if they haven't been inside yet.

static EWRAM_DATA bool8 sWarpPending = FALSE;

// Where the Heal Dash goes from here (HEAL_LOCATION_NONE: nowhere, or not before Act 2)
u32 HealDash_GetDestination(void)
{
    if (VarGet(VAR_STORY_ACT) < STORY_ACT_UNIVERSITY)
        return HEAL_LOCATION_NONE;
    if (Act2_GetTripHealLocation() != HEAL_LOCATION_NONE)
        return Act2_GetTripHealLocation();
    return GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation);
}

// Whether the player is already in that Pokémon Center
bool32 HealDash_IsAtDestination(u32 healLocation)
{
    struct WarpData saved = gSaveBlock1Ptr->lastHealLocation, inside;
    const struct HealLocation *loc = GetHealLocation(healLocation);
    bool32 here;
    u16 lastTalked = gSpecialVar_LastTalked, healer = gSpecialVar_0x800B;
    if (loc == NULL)
        return FALSE;
    gSaveBlock1Ptr->lastHealLocation = (struct WarpData) {
        .mapGroup = loc->mapGroup, .mapNum = loc->mapNum, .warpId = WARP_ID_NONE, .x = loc->x, .y = loc->y
    };
    SetWhiteoutRespawnWarpAndHealerNPC(&inside);
    gSaveBlock1Ptr->lastHealLocation = saved;
    gSpecialVar_LastTalked = lastTalked;
    gSpecialVar_0x800B = healer;
    here = inside.mapGroup == gSaveBlock1Ptr->location.mapGroup && inside.mapNum == gSaveBlock1Ptr->location.mapNum;
    return here;
}

// Spins the player away (the Teleport effect); they land in front of the nurse (HealDash_TakeWarp). That Pokémon
// Center becomes the one to go back to after blacking out.
void HealDash_Start(u32 healLocation)
{
    SetLastHealLocationWarp(healLocation);
    sWarpPending = TRUE;
    Overworld_ResetStateAfterTeleport();
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_ON_FOOT);
    FldEff_TeleportWarpOut();
}

// From SetWarpDestinationForTeleport: TRUE (with the warp) if the Teleport effect is a Heal Dash
bool32 HealDash_TakeWarp(struct WarpData *warp)
{
    if (!sWarpPending)
        return FALSE;
    sWarpPending = FALSE;
    SetWhiteoutRespawnWarpAndHealerNPC(warp);
    return TRUE;
}
