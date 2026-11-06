#include "global.h"
#include "act2_end.h"
#include "event_data.h"
#include "overworld.h"
#include "prologue_end.h"
#include "constants/heal_locations.h"
#include "constants/story.h"

// The end of Act 2 (docs/hack/act2_finale.md, "The ending"): "Several years later" on black, then Act 3 starts with
// the player, now in a lab coat, in their lab at Silph Co.

static const u8 *const sCards[] =
{
    COMPOUND_STRING("Several years later"),
};

static void StartAct3(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_WORKFORCE);
    VarSet(VAR_ACT2_PROGRESS, ACT2_DONE);
    SetLastHealLocationWarp(HEAL_LOCATION_SAFFRON_CITY);
    SetWarpDestination(MAP_GROUP(ACT3_START_MAP), MAP_NUM(ACT3_START_MAP), WARP_ID_NONE, ACT3_START_X, ACT3_START_Y);
}

// special Act2_EndAct2, with the screen already faded to black (then waitstate)
void Act2_EndAct2(void)
{
    ShowActEndCards(sCards, ARRAY_COUNT(sCards), StartAct3);
}
