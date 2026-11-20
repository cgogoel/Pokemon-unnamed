#include "global.h"
#include "act2_station.h"
#include "act2_road.h"
#include "random.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "move.h"
#include "pokemon.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/story.h"

// The abandoned ranger station, the Act 2 finale's dungeon (docs/hack/act2_finale.md). Its maps are
// RangerStation_1F to _B4F; their layouts and src/data/act2_station_layout.h come from
// tools/hack/compose_ranger_station.py.

#include "data/act2_station_layout.h"

extern const u8 RangerStation_B3F_EventScript_Ambush[];

#define METATILE_GATE_COUNTER_END   0x295 // gTileset_GenericBuilding2: the counter's bottom end
#define METATILE_GATE_FLOOR         0x283
#define METATILE_GATE_SWITCH_PULLED 0x33A // The wall switch with its lever down (tools/hack/draw_station_tiles.py)
#define METATILE_MANSION_FLOOR      0x286 // The basement floor
#define METATILE_FURNACE_COLD       0x382
#define METATILE_FURNACE_LIT        0x383

#define DEAD_END_LEVEL_ABOVE_AREA  2
#define GUARD_LEVEL_ABOVE_AREA     3
// Random encounters sit just under the station's set pieces, the way the road home's wild Pokemon sit just under
// its trainers: from this far below the station's level up to it. Without a rule of its own the station would take
// the +15 the final project puts on anything off the road home, which is meant to keep the player from wandering
// off it - and the station is where the road ends, not a detour (docs/hack/act2_finale.md).
#define WILD_LEVEL_BELOW_AREA      3

#define IS_ON_MAP(map) (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(map) && gSaveBlock1Ptr->location.mapNum == MAP_NUM(map))

static const u16 sStationMaps[] =
{
    MAP_RANGER_STATION_1F,
    MAP_RANGER_STATION_B1F,
    MAP_RANGER_STATION_B2F,
    MAP_RANGER_STATION_B3F,
    MAP_RANGER_STATION_B4F,
};

// In the order sDeadEnds lists them (down the floor, then across), so the four that were already there keep
// the Pokemon they had: LANTURN is the new one, waiting at the false right turn off the climb.
static const u16 sDeadEndSpecies[] = { SPECIES_KINGDRA, SPECIES_TENTACRUEL, SPECIES_LANTURN, SPECIES_GYARADOS,
                                       SPECIES_CLOYSTER, SPECIES_STARMIE };
static const u16 sDeadEndFlags[] = { FLAG_ACT2_DEAD_END_1, FLAG_ACT2_DEAD_END_2, FLAG_ACT2_DEAD_END_3,
                                     FLAG_ACT2_DEAD_END_4, FLAG_ACT2_DEAD_END_5, FLAG_ACT2_DEAD_END_6 };

STATIC_ASSERT(ARRAY_COUNT(sDeadEndSpecies) == ARRAY_COUNT(sDeadEnds), DeadEndCount);
STATIC_ASSERT(ARRAY_COUNT(sClockPieces) == ACT2_CLOCK_SUN + 1, ClockPieceCount);

static u32 GetLevelAboveArea(u32 above);

static void SetMetatile(s32 x, s32 y, u32 metatile)
{
    MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, metatile);
}

// specialvar VAR_RESULT, Act2Station_HasFinaleBit with the ACT2_FINALE_* bit in VAR_0x8004
bool16 Act2Station_HasFinaleBit(void)
{
    return (VarGet(VAR_ACT2_FINALE) & gSpecialVar_0x8004) != 0;
}

// special Act2Station_SetFinaleBit with the ACT2_FINALE_* bit in VAR_0x8004
void Act2Station_SetFinaleBit(void)
{
    VarSet(VAR_ACT2_FINALE, VarGet(VAR_ACT2_FINALE) | gSpecialVar_0x8004);
}

// A party Pokémon that knows a move of that type (any Pokémon): its nickname goes in STR_VAR_1 and the move in STR_VAR_2
static bool32 FindMoveUserOfType(enum Type type)
{
    u32 i, j;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u32 move = GetMonData(mon, MON_DATA_MOVE1 + j);
            if (move != MOVE_NONE && GetMoveType(move) == type)
            {
                GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
                StringGet_Nickname(gStringVar1);
                StringCopy(gStringVar2, GetMoveName(move));
                return TRUE;
            }
        }
    }
    return FALSE;
}

// specialvar VAR_RESULT, Act2Station_FindFireMoveUser: a party Pokémon with a FIRE-type move (STR_VAR_1, STR_VAR_2)
bool16 Act2Station_FindFireMoveUser(void)
{
    return FindMoveUserOfType(TYPE_FIRE);
}

// specialvar VAR_RESULT, Act2Station_FindWaterMoveUser: a party Pokémon with a WATER-type move (STR_VAR_1, STR_VAR_2)
bool16 Act2Station_FindWaterMoveUser(void)
{
    return FindMoveUserOfType(TYPE_WATER);
}

// --- B1F: the clock --------------------------------------------------------------------------------------------------

static bool32 IsClockPieceLit(u32 piece)
{
    return (VarGet(VAR_ACT2_CLOCK) >> piece) & 1;
}

static void DrawClockPiece(u32 piece)
{
    u32 metatile;
    if (piece < ACT2_CLOCK_MOON)
        metatile = IsClockPieceLit(piece) ? METATILE_RangerStation_TorchLit : METATILE_RangerStation_TorchUnlit;
    else
        metatile = IsClockPieceLit(piece) ? METATILE_FURNACE_LIT : METATILE_FURNACE_COLD;
    SetMetatile(sClockPieces[piece][0], sClockPieces[piece][1], metatile | MAPGRID_IMPASSABLE);
}

static void DrawMosaicOpen(void)
{
    SetMetatile(STATION_MOSAIC_X, STATION_MOSAIC_Y, METATILE_RangerStation_MosaicOpen);
}

// specialvar VAR_RESULT, Act2Station_GetFacedClockPiece: the torch or furnace the player faces (0-11 the torches at
// 1-12 o'clock, then ACT2_CLOCK_MOON and ACT2_CLOCK_SUN), or 0xFF. Puts "torch" or "furnace" in STR_VAR_3.
u16 Act2Station_GetFacedClockPiece(void)
{
    s16 x, y;
    u32 i;
    PlayerGetDestCoords(&x, &y);
    MoveCoords(GetPlayerFacingDirection(), &x, &y);
    for (i = 0; i < ARRAY_COUNT(sClockPieces); i++)
    {
        if (sClockPieces[i][0] + MAP_OFFSET == x && sClockPieces[i][1] + MAP_OFFSET == y)
        {
            StringCopy(gStringVar3, i < ACT2_CLOCK_MOON ? COMPOUND_STRING("torch") : COMPOUND_STRING("furnace"));
            return i;
        }
    }
    return 0xFF;
}

// specialvar VAR_RESULT, Act2Station_IsClockPieceLit with the piece in VAR_0x8004
bool16 Act2Station_IsClockPieceLit(void)
{
    return IsClockPieceLit(gSpecialVar_0x8004);
}

// specialvar VAR_RESULT, Act2Station_ToggleClockPiece with the piece in VAR_0x8004: lights it or puts it out (the
// script redraws the map). TRUE if that set the clock to the solution and the mosaic is still shut.
bool16 Act2Station_ToggleClockPiece(void)
{
    u32 clock = VarGet(VAR_ACT2_CLOCK) ^ (1 << gSpecialVar_0x8004);
    VarSet(VAR_ACT2_CLOCK, clock);
    DrawClockPiece(gSpecialVar_0x8004);
    return clock == ACT2_CLOCK_SOLUTION && !(VarGet(VAR_ACT2_FINALE) & ACT2_FINALE_CLOCK_SOLVED);
}

// special Act2Station_SlideMosaic with the step in VAR_0x8004: the slab in the middle of the mosaic slides away from
// the ladder down, halfway (1), then all the way, for good (2). The script redraws the map after each.
void Act2Station_SlideMosaic(void)
{
    if (gSpecialVar_0x8004 == 1)
    {
        SetMetatile(STATION_MOSAIC_X, STATION_MOSAIC_Y, METATILE_RangerStation_MosaicHalf);
        return;
    }
    VarSet(VAR_ACT2_FINALE, VarGet(VAR_ACT2_FINALE) | ACT2_FINALE_CLOCK_SOLVED);
    DrawMosaicOpen();
}

// special Act2Station_DrawFloor (each floor's ON_LOAD): what the player changed, and B1F's clock
void Act2Station_DrawFloor(void)
{
    u32 finale = VarGet(VAR_ACT2_FINALE), i;
    if (IS_ON_MAP(MAP_RANGER_STATION_1F))
    {
        if (finale & ACT2_FINALE_BASEMENT_OPEN)
        {
            SetMetatile(STATION_1F_COUNTER_X, STATION_1F_COUNTER_Y, METATILE_GATE_COUNTER_END | MAPGRID_IMPASSABLE);
            SetMetatile(STATION_1F_COUNTER_X, STATION_1F_COUNTER_Y + 1, METATILE_GATE_FLOOR);
            SetMetatile(STATION_1F_SWITCH_X, STATION_1F_SWITCH_Y, METATILE_GATE_SWITCH_PULLED | MAPGRID_IMPASSABLE);
        }
    }
    else if (IS_ON_MAP(MAP_RANGER_STATION_B1F))
    {
        if (!(finale & ACT2_FINALE_CLOCK_SET))
        {
            VarSet(VAR_ACT2_CLOCK, ACT2_CLOCK_START);
            VarSet(VAR_ACT2_FINALE, finale | ACT2_FINALE_CLOCK_SET);
        }
        for (i = 0; i < ARRAY_COUNT(sClockPieces); i++)
            DrawClockPiece(i);
        if (finale & ACT2_FINALE_CLOCK_SOLVED)
            DrawMosaicOpen();
    }
}

// Hook (StandardWildEncounter): no wild Pokémon in B1F's clock room, only in the rooms and corridor round it
bool32 Act2Station_IsEncounterFree(void)
{
    s16 x, y;
    if (!IS_ON_MAP(MAP_RANGER_STATION_B1F))
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    return x >= STATION_CLOCK_X && x < STATION_CLOCK_X + STATION_CLOCK_W
        && y >= STATION_CLOCK_Y && y < STATION_CLOCK_Y + STATION_CLOCK_H;
}

// Whether the player is anywhere in the ranger station. Its floors share PALLET TOWN's map section, so the map
// has to be named: Act2Road_GetAreaLevel uses this to give the station a level slot of its own.
bool32 Act2Station_IsInStation(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sStationMaps); i++)
    {
        if (IS_ON_MAP(sStationMaps[i]))
            return TRUE;
    }
    return FALSE;
}

// A random encounter in the station: WILD_LEVEL_BELOW_AREA under its level up to the level itself, so the floors'
// wild Pokémon stay below the dead ends and the doorway guards the player is meant to remember.
u32 Act2Station_GetWildLevel(void)
{
    s32 level = (s32)Act2Road_GetAreaLevel() - (s32)(Random() % (WILD_LEVEL_BELOW_AREA + 1));
    return min(max(level, 1), MAX_LEVEL);
}

// special Act2Station_CreateGuardMon with its species in VAR_0x8006, after setwildbattle: a Pokémon hiding in a room's
// door, 3 levels above the station's level
void Act2Station_CreateGuardMon(void)
{
    CreateScriptedWildMon(gSpecialVar_0x8006, GetLevelAboveArea(GUARD_LEVEL_ABOVE_AREA), ITEM_NONE);
}

// specialvar VAR_RESULT, Act2Station_CountNotes: how many of B1F's notes have been read, also in STR_VAR_1
u16 Act2Station_CountNotes(void)
{
    u32 count = __builtin_popcount(VarGet(VAR_ACT2_FINALE) & ACT2_FINALE_NOTES);
    ConvertIntToDecimalStringN(gStringVar1, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    return count;
}

// --- B2F: the flood --------------------------------------------------------------------------------------------------

// Hook (PlayerNotOnBikeMoving): on B2F, walking into one square of water with a stone (or floor) right behind it
// jumps over it, the way a ledge does
bool32 Act2Station_CanJumpGap(enum Direction direction)
{
    s16 x, y, x2, y2;
    if (!IS_ON_MAP(MAP_RANGER_STATION_B2F))
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    MoveCoords(direction, &x, &y);
    if (!MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y)))
        return FALSE;
    x2 = x, y2 = y;
    MoveCoords(direction, &x2, &y2);
    if (MapGridGetCollisionAt(x2, y2) || MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x2, y2)))
        return FALSE;
    // Not over a dead end's Pokémon, nor onto anyone
    return GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT && GetObjectEventIdByXY(x2, y2) == OBJECT_EVENTS_COUNT;
}

// Hook (FollowablePlayerMovement_Step): the follower's next square is a gap the player jumped, so it jumps it too
// (the player's previous square is the stone behind it). Otherwise it would wade into the water a step at a time
// and fall further behind with every jump.
bool32 Act2Station_IsFollowerGap(s16 x, s16 y)
{
    return IS_ON_MAP(MAP_RANGER_STATION_B2F) && MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y));
}

// From SetDefaultFlashLevel: B2F is dark, a small circle of light round the player, just wide enough to show the
// stones one jump away (flash radius 40 px; a jump is 32 px), so the way through has to be found stone by stone
#define FLOOD_FLASH_LEVEL 5
u32 Act2Station_GetFloodFlashLevel(void)
{
    return IS_ON_MAP(MAP_RANGER_STATION_B2F) ? FLOOD_FLASH_LEVEL : 0;
}

static u32 GetDeadEnd(void)
{
    u32 deadEnd = VarGet(VAR_TEMP_1) - 1;
    return deadEnd < ARRAY_COUNT(sDeadEndSpecies) ? deadEnd : 0;
}

// specialvar VAR_RESULT, Act2Station_IsDeadEndBeaten with the dead end (1-5) in VAR_TEMP_1
bool16 Act2Station_IsDeadEndBeaten(void)
{
    return FlagGet(sDeadEndFlags[GetDeadEnd()]);
}

// special Act2Station_PrepareDeadEnd with the dead end (1-5) in VAR_TEMP_1: its species in VAR_0x8006 (for its cry)
// and its name in STR_VAR_1
void Act2Station_PrepareDeadEnd(void)
{
    u32 species = sDeadEndSpecies[GetDeadEnd()];
    // Hidden under the water until the player reaches its stone (FLAG_TEMP_1-5, set by the map's ON_TRANSITION):
    // clear its flag so addobject brings it up
    FlagClear(FLAG_TEMP_1 + GetDeadEnd());
    gSpecialVar_0x8006 = species;
    StringCopy(gStringVar1, GetSpeciesName(species));
}

// The station's level (an area of its own: the party's average level when the player first came in,
// Act2Road_GetAreaLevel), plus some
static u32 GetLevelAboveArea(u32 above)
{
    return min(Act2Road_GetAreaLevel() + above, MAX_LEVEL);
}

// special Act2Station_CreateDeadEndMon with the dead end (1-5) in VAR_TEMP_1, after setwildbattle: the dead end's
// Pokémon, 2 levels above the station's level
void Act2Station_CreateDeadEndMon(void)
{
    CreateScriptedWildMon(sDeadEndSpecies[GetDeadEnd()], GetLevelAboveArea(DEAD_END_LEVEL_ABOVE_AREA), ITEM_NONE);
}

// special Act2Station_SetDeadEndBeaten with the dead end (1-5) in VAR_TEMP_1
void Act2Station_SetDeadEndBeaten(void)
{
    FlagSet(sDeadEndFlags[GetDeadEnd()]);
}

// --- B3F: the overgrowth ---------------------------------------------------------------------------------------------

// Hook (TryStartMiscWalkingScripts): on B3F, a step into the tall grass off the hidden path is an ambush. The path
// (sOvergrowthPath) is generated from the same grid as the tablet that shows it (compose_ranger_station.B3_PATH).
bool32 Act2Station_TryOvergrowthStep(void)
{
    s16 x, y;
    if (!IS_ON_MAP(MAP_RANGER_STATION_B3F))
        return FALSE;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    if (y == STATION_FIELD_Y - 1)
    {
        VarSet(VAR_ACT2_FINALE, VarGet(VAR_ACT2_FINALE) | ACT2_FINALE_GRASS_CROSSED);
        return FALSE;
    }
    if (x < STATION_FIELD_X || x >= STATION_FIELD_X + STATION_FIELD_SIZE
     || y < STATION_FIELD_Y || y >= STATION_FIELD_Y + STATION_FIELD_SIZE)
        return FALSE;
    if ((sOvergrowthPath[y - STATION_FIELD_Y] >> (x - STATION_FIELD_X)) & 1)
        return FALSE;
    ScriptContext_SetupScript(RangerStation_B3F_EventScript_Ambush);
    return TRUE;
}

// special Act2Station_GetPathStart: where the hidden path starts (below the field), in VAR_0x8004 and VAR_0x8005
void Act2Station_GetPathStart(void)
{
    gSpecialVar_0x8004 = STATION_PATH_START_X;
    gSpecialVar_0x8005 = STATION_PATH_START_Y;
}
