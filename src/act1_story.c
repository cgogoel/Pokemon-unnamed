#include "global.h"
#include "act1_story.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "script.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/map_event_ids.h"
#include "constants/story.h"

// Act 1's story (docs/hack/act1_story.md)

// How far Jangmo-o sees, straight ahead
#define JANGMO_O_SIGHT 4

// Where Jangmo-o turns up: near each of the other Woods Pokémon (placeholders until the Woods is redrawn). While it's
// off-screen, it's moved to the one nearest the player that's still out of view, so it appears there as they
// approach. Out of view: farther than the game spawns objects (TrySpawnObjectEvents), plus a tile.
static const struct Coords16 sJangmoOSpawns[] =
{
    { 4, 40},  // Tynamo
    {13, 19},  // Hatenna
    {42, 33},  // Teddiursa
};
#define SPAWN_VIEW_LEFT   10
#define SPAWN_VIEW_RIGHT  11
#define SPAWN_VIEW_UP     8
#define SPAWN_VIEW_DOWN   10

static void UpdateWoodsJangmoOSpawn(void)
{
    s32 best = -1, bestDistance = 0;
    u8 objectEventId;

    if (!TryGetObjectEventIdByLocalIdAndMap(LOCALID_WOODS_JANGMO_O, MAP_NUM(MAP_WOODS), MAP_GROUP(MAP_WOODS), &objectEventId))
        return; // On screen: it roams freely

    for (u32 i = 0; i < ARRAY_COUNT(sJangmoOSpawns); i++)
    {
        s32 dx = sJangmoOSpawns[i].x - gSaveBlock1Ptr->pos.x;
        s32 dy = sJangmoOSpawns[i].y - gSaveBlock1Ptr->pos.y;
        s32 distance = abs(dx) + abs(dy);

        if (dx >= -SPAWN_VIEW_LEFT && dx <= SPAWN_VIEW_RIGHT && dy >= -SPAWN_VIEW_UP && dy <= SPAWN_VIEW_DOWN)
            continue;
        if (best < 0 || distance < bestDistance)
        {
            best = i;
            bestDistance = distance;
        }
    }
    if (best >= 0)
        SetObjEventTemplateCoords(LOCALID_WOODS_JANGMO_O, sJangmoOSpawns[best].x, sJangmoOSpawns[best].y);
}

// Until the player has Dad's Hard Stone, Jangmo-o charges at them as soon as they're in its sight, and they black
// out. With the stone it doesn't (it's talked to instead), so a failed encounter doesn't start another right away.
bool32 TryStartWoodsJangmoOCharge(void)
{
    struct ObjectEvent *jangmoO, *player;
    s32 dx, dy, distance, stepX = 0, stepY = 0;
    u8 objectEventId;

    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_WOODS) || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_WOODS))
        return FALSE;
    if (FlagGet(FLAG_BEFRIENDED_JANGMO_O))
        return FALSE;
    UpdateWoodsJangmoOSpawn();
    if (VarGet(VAR_ACT1_PROGRESS) >= ACT1_GOT_HARD_STONE)
        return FALSE;
    if (TryGetObjectEventIdByLocalIdAndMap(LOCALID_WOODS_JANGMO_O, MAP_NUM(MAP_WOODS), MAP_GROUP(MAP_WOODS), &objectEventId))
        return FALSE;
    jangmoO = &gObjectEvents[objectEventId];
    if (jangmoO->invisible)
        return FALSE;

    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    dx = player->currentCoords.x - jangmoO->currentCoords.x;
    dy = player->currentCoords.y - jangmoO->currentCoords.y;
    switch (jangmoO->facingDirection)
    {
    case DIR_SOUTH:
        stepY = 1;
        break;
    case DIR_NORTH:
        stepY = -1;
        break;
    case DIR_WEST:
        stepX = -1;
        break;
    case DIR_EAST:
        stepX = 1;
        break;
    default:
        return FALSE;
    }
    if (stepX != 0 && (dy != 0 || dx * stepX <= 0))
        return FALSE;
    if (stepY != 0 && (dx != 0 || dy * stepY <= 0))
        return FALSE;
    distance = (stepX != 0) ? dx * stepX : dy * stepY;
    if (distance > JANGMO_O_SIGHT)
        return FALSE;
    // Nothing solid in between
    for (s32 i = 1; i < distance; i++)
    {
        if (MapGridGetCollisionAt(jangmoO->currentCoords.x + stepX * i, jangmoO->currentCoords.y + stepY * i))
            return FALSE;
    }

    gSpecialVar_LastTalked = LOCALID_WOODS_JANGMO_O;
    ScriptContext_SetupScript(Woods_EventScript_JangmoOSpotted);
    return TRUE;
}

// The loved items the player finds for Tynamo, Hatenna and Teddiursa. Offering one uses it up, so if it's gone and
// that Pokémon still isn't befriended, it's back where it was found (special Act1_RestockLovedItems, on entering the
// Woods and Pallet Town).
static const struct
{
    u16 befriendedFlag;
    u16 itemFlag;
    u16 item;
} sAct1LovedItems[] =
{
    { FLAG_BEFRIENDED_TYNAMO,    FLAG_ACT1_ITEM_CHERI_BERRY,  ITEM_CHERI_BERRY },
    { FLAG_BEFRIENDED_HATENNA,   FLAG_ACT1_ITEM_CHESTO_BERRY, ITEM_CHESTO_BERRY },
    { FLAG_BEFRIENDED_TEDDIURSA, FLAG_ACT1_ITEM_HONEY,        ITEM_HONEY },
};

void Act1_RestockLovedItems(void)
{
    for (u32 i = 0; i < ARRAY_COUNT(sAct1LovedItems); i++)
    {
        if (!FlagGet(sAct1LovedItems[i].befriendedFlag) && FlagGet(sAct1LovedItems[i].itemFlag)
         && !CheckBagHasItem(sAct1LovedItems[i].item, 1))
            FlagClear(sAct1LovedItems[i].itemFlag);
    }
}
