#include "global.h"
#include "act1_story.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "script.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/map_event_ids.h"
#include "constants/story.h"

// Act 1's story (docs/hack/act1_story.md)

// How far Jangmo-o sees, straight ahead
#define JANGMO_O_SIGHT 4

// Where Jangmo-o turns up: near each of the other Woods Pokémon. While it's
// off-screen, it's moved to the one nearest the player that's still out of view, so it appears there as they
// approach. Out of view: farther than the game spawns objects (TrySpawnObjectEvents), plus a tile.
static const struct Coords16 sJangmoOSpawns[] =
{
    {15, 32},  // Tynamo's pond
    {36,  6},  // Hatenna
    {48, 38},  // Teddiursa
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

// Caught by Jangmo-o while a Woods Pokémon is following (it hasn't been dropped off at the farm yet): it runs back to
// where it was befriended and has to be befriended again. If befriending it moved the story on, that's undone too.
static const struct
{
    u16 species;
    u16 befriendedFlag;
    u8 befriendedStage; // VAR_ACT1_PROGRESS once it's befriended...
    u8 previousStage;   // ...and before
} sWoodsPokemon[] =
{
    { SPECIES_BLIPBUG,   FLAG_BEFRIENDED_BLIPBUG,   ACT1_BEFRIENDED_BLIPBUG, ACT1_DAD_SENT_TO_WOODS },
    { SPECIES_TYNAMO,    FLAG_BEFRIENDED_TYNAMO,    ACT1_BEFRIENDED_THREE,   ACT1_DAD_WARNED_JANGMO_O },
    { SPECIES_HATENNA,   FLAG_BEFRIENDED_HATENNA,   ACT1_BEFRIENDED_THREE,   ACT1_DAD_WARNED_JANGMO_O },
    { SPECIES_TEDDIURSA, FLAG_BEFRIENDED_TEDDIURSA, ACT1_BEFRIENDED_THREE,   ACT1_DAD_WARNED_JANGMO_O },
};

// special Act1_ReleaseFollowerToWoods: VAR_RESULT TRUE if the follower ran back (its name in STR_VAR_1)
void Act1_ReleaseFollowerToWoods(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][0];
    u32 species;

    gSpecialVar_Result = FALSE;
    if (CalculatePlayerPartyCount() == 0 || GetMonData(mon, MON_DATA_IS_EGG))
        return;
    species = GetMonData(mon, MON_DATA_SPECIES);
    for (u32 i = 0; i < ARRAY_COUNT(sWoodsPokemon); i++)
    {
        if (sWoodsPokemon[i].species != species)
            continue;
        GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
        StringGet_Nickname(gStringVar1);
        ZeroMonData(mon);
        CompactPartySlots();
        CalculatePlayerPartyCount();
        UpdateFollowingPokemon();
        FlagClear(sWoodsPokemon[i].befriendedFlag);
        if (VarGet(VAR_ACT1_PROGRESS) == sWoodsPokemon[i].befriendedStage)
            VarSet(VAR_ACT1_PROGRESS, sWoodsPokemon[i].previousStage);
        gSpecialVar_Result = TRUE;
        return;
    }
}
