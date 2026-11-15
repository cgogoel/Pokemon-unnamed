#include "global.h"
#include "act2_night_bloom.h"
#include "act2_road.h"
#include "act2_story.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "game_time.h"
#include "move.h"
#include "overworld.h"
#include "pokemon.h"
#include "string_util.h"
#include "event_object_movement.h"
#include "constants/map_event_ids.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/story.h"

// The Grass course's night bloom (docs/hack/act2_story.md). Once a year the MOONVEIL blooms for one night somewhere
// in Viridian Forest. The trip runs at night (Act2_IsGrassTripNight). The site's puzzle, VAR_ACT2_NIGHT_BLOOM:
//  1. Light the way: the forest is dark (flash darkness); three dim glowcaps along the trail wake with a GRASS move,
//     and each widens the light (Act2_GetNightForestFlashLevel).
//  2. Follow the ODDISH: held still with SLEEP POWDER or STUN SPORE, each shows the next leg of the way; the third
//     leads to a hidden opening into the glade.
//  3. Guard the bloom: three waves of BUG POKéMON until dawn, one gap at a time: the player goes to the gap and
//     fights them, then the MORELULL's INGRAIN grows a hedge across it.
// Then dawn (day for the rest of the trip), the MOONVEIL opens, its dew is collected (the puzzle's end), the glade's
// MORELULL comes along and PROF. FERN makes the TM and gives the exam in the glade.

#include "data/night_bloom_layout.h"

#define IS_ON_MAP(map) (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(map) && gSaveBlock1Ptr->location.mapNum == MAP_NUM(map))

static u32 GetBloom(void)
{
    return VarGet(VAR_ACT2_NIGHT_BLOOM);
}

static void SetBloom(u32 bits)
{
    VarSet(VAR_ACT2_NIGHT_BLOOM, GetBloom() | bits);
}

static void SetResearch(u32 bits)
{
    VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | bits);
}

static u32 CountBits(u32 bits)
{
    u32 count = 0;
    for (; bits; bits >>= 1)
        count += bits & 1;
    return count;
}

// special Act2_SetNightBloomBit / specialvar VAR_RESULT, Act2_HasNightBloomBit with a NIGHT_BLOOM_* bit in VAR_0x8004
void Act2_SetNightBloomBit(void)
{
    SetBloom(gSpecialVar_0x8004);
}

bool16 Act2_HasNightBloomBit(void)
{
    return (GetBloom() & gSpecialVar_0x8004) != 0;
}

// ---------------------------------------------------------------------------------------------------------------
// Viridian Forest

// The forest's darkness on the Grass trip's night, by how many glowcaps are awake: none is a cave without FLASH, and
// each one widens the light (a smaller flash level is a bigger circle; 0 is no darkness)
static const u8 sGlowcapFlashLevels[] = { 7, 5, 3, 1 };

u32 Act2_GetNightFlashLevel(u32 glowcapsLit)
{
    return sGlowcapFlashLevels[min(glowcapsLit, ARRAY_COUNT(sGlowcapFlashLevels) - 1)];
}

// From SetDefaultFlashLevel: Viridian Forest's darkness (0 when it isn't dark)
u32 Act2_GetNightForestFlashLevel(void)
{
    if (!IS_ON_MAP(MAP_VIRIDIAN_FOREST) || !Act2_IsGrassTripNight())
        return 0;
    return Act2_GetNightFlashLevel(CountBits(GetBloom() & NIGHT_BLOOM_GLOWCAPS));
}

// The way to the next dim glowcap (the first not yet woken, in trail order): its trail of spores (no glow on the glowcap)
// (light sprites in Viridian Forest's map.json, FLAG_TEMP_11-13 for glowcaps 1-3), at night only. Light sprites go
// out when their flag is set (UpdateLightSprite).
static void SetGlowcapTrail(void)
{
    u32 bloom = GetBloom(), i;
    for (i = 0; i < 3; i++)
        FlagSet(FLAG_TEMP_11 + i);
    if (!Act2_IsGrassTripNight())
        return;
    for (i = 0; i < 3; i++)
    {
        if (!(bloom & (NIGHT_BLOOM_GLOWCAP_1 << i)))
        {
            FlagClear(FLAG_TEMP_11 + i);
            return;
        }
    }
}

// special Act2_SetupForestObjects, on Viridian Forest's transition. On the Grass trip's night, FLAG_TEMP_1 hides the
// forest's people. On the trip: FLAG_TEMP_2-4 the dim glowcaps (until woken), FLAG_TEMP_5-7 the lit ones, FLAG_TEMP_8-10
// the ODDISH (the one for the next leg, once the glowcaps are all lit), FLAG_TEMP_11-13 the way to the next glowcap,
// FLAG_TEMP_14 PROF. FERN's assistants at the north exit (the class stays in the forest). Off the trip, the forest is
// as usual.
void Act2_SetupForestObjects(void)
{
    u32 bloom = GetBloom(), i;
    for (i = 0; i < 10; i++)
        FlagSet(FLAG_TEMP_1 + i);
    for (i = 0; i < 4; i++)
        FlagSet(FLAG_TEMP_11 + i);
    if (!Act2_IsGrassTripNight())
        FlagClear(FLAG_TEMP_1);
    if (!Act2_IsGrassTrip())
        return;
    FlagClear(FLAG_TEMP_14);
    SetGlowcapTrail();
    for (i = 0; i < 3; i++)
        FlagClear(((bloom & (NIGHT_BLOOM_GLOWCAP_1 << i)) ? FLAG_TEMP_5 : FLAG_TEMP_2) + i);
    if ((bloom & NIGHT_BLOOM_GLOWCAPS) == NIGHT_BLOOM_GLOWCAPS)
    {
        for (i = 0; i < 3; i++)
        {
            if (!(bloom & (NIGHT_BLOOM_ODDISH_1 << i)))
            {
                FlagClear(FLAG_TEMP_8 + i);
                break;
            }
        }
    }
}

static void DrawSquares(const struct NightBloomSquare *squares, u32 count)
{
    u32 i;
    for (i = 0; i < count; i++)
        MapGridSetMetatileIdAt(squares[i].x + MAP_OFFSET, squares[i].y + MAP_OFFSET, squares[i].block);
}

// special Act2_DrawForestOpening (Viridian Forest's load, and when the third ODDISH shows it): the path into the
// glade, once found. It stays found.
void Act2_DrawForestOpening(void)
{
    if (IS_ON_MAP(MAP_VIRIDIAN_FOREST) && (GetBloom() & NIGHT_BLOOM_ODDISH_3))
        DrawSquares(sForestOpening, ARRAY_COUNT(sForestOpening));
}

// specialvar VAR_RESULT, Act2_WakeGlowcap with the glowcap (1-3) in VAR_0x8004: it's lit for the rest of the trip.
// VAR_0x8005 is the dim one's local id and VAR_0x8006 the lit one's (to swap them). TRUE if it was the last one.
bool16 Act2_WakeGlowcap(void)
{
    u32 n = gSpecialVar_0x8004 - 1;
    SetBloom(NIGHT_BLOOM_GLOWCAP_1 << n);
    FlagSet(FLAG_TEMP_2 + n);
    FlagClear(FLAG_TEMP_5 + n);
    gSpecialVar_0x8005 = LOCALID_FOREST_GLOWCAP_1 + n;
    gSpecialVar_0x8006 = LOCALID_FOREST_GLOWCAP_LIT_1 + n;
    SetGlowcapTrail();
    if ((GetBloom() & NIGHT_BLOOM_GLOWCAPS) != NIGHT_BLOOM_GLOWCAPS)
        return FALSE;
    SetResearch(ACT2_RESEARCH_PUZZLE_1);
    FlagClear(FLAG_TEMP_8); // The first ODDISH comes out
    return TRUE;
}

// specialvar VAR_RESULT, Act2_IsGlowcapLit with the glowcap (1-3) in VAR_0x8004
bool16 Act2_IsGlowcapLit(void)
{
    return (GetBloom() & (NIGHT_BLOOM_GLOWCAP_1 << (gSpecialVar_0x8004 - 1))) != 0;
}

// special Act2_BrightenForest, after a glowcap wakes: the light widens to match. waitstate after it.
void Act2_BrightenForest(void)
{
    u32 level = Act2_GetNightForestFlashLevel();
    TrySpawnLightSprites(0, 0); // The way to the next glowcap
    AnimateFlash(level);
    SetFlashLevel(level);
}

// From InitOverworldGraphicsRegisters: in the dark forest the spores show beyond the light (their OBJ-window twins,
// src/event_object_movement.c); everything else stays hidden in the dark, as in a cave
bool32 Act2_ShowLightsInDark(void)
{
    return Act2_GetNightForestFlashLevel() != 0;
}

// specialvar VAR_RESULT, Act2_FollowOddish with the ODDISH (1-3) in VAR_0x8004, once it's been held still: that leg
// of the way is known. VAR_0x8005 is its local id, VAR_0x8006 the next one's (the next leg's ODDISH waits where this
// one stops). TRUE if it was the last: the opening into the glade is found (the script redraws the map).
bool16 Act2_FollowOddish(void)
{
    u32 n = gSpecialVar_0x8004 - 1;
    SetBloom(NIGHT_BLOOM_ODDISH_1 << n);
    FlagSet(FLAG_TEMP_8 + n);
    gSpecialVar_0x8005 = LOCALID_FOREST_ODDISH_1 + n;
    if (n < 2)
    {
        FlagClear(FLAG_TEMP_8 + n + 1);
        gSpecialVar_0x8006 = LOCALID_FOREST_ODDISH_1 + n + 1;
        return FALSE;
    }
    SetResearch(ACT2_RESEARCH_PUZZLE_2);
    Act2_DrawForestOpening();
    return TRUE;
}

// The first party Pokémon that knows one of the moves (MOVE_NONE ends the list), or, failing that, one that knows a
// GRASS move that passes the fallback check. Its nickname goes in STR_VAR_1 and the move in STR_VAR_2.
static bool32 FindMoveUser(const u16 *moves, bool32 (*fallback)(u32 move))
{
    u32 pass, i, j, k;
    for (pass = 0; pass < 2; pass++)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                continue;
            for (j = 0; j < MAX_MON_MOVES; j++)
            {
                u32 move = GetMonData(mon, MON_DATA_MOVE1 + j);
                bool32 found = FALSE;
                if (move == MOVE_NONE)
                    continue;
                if (pass == 0)
                {
                    for (k = 0; moves[k] != MOVE_NONE; k++)
                        found |= moves[k] == move;
                }
                else
                {
                    found = GetMoveType(move) == TYPE_GRASS && fallback(move);
                }
                if (found)
                {
                    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
                    StringGet_Nickname(gStringVar1);
                    StringCopy(gStringVar2, GetMoveName(move));
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

static bool32 IsStatusMove(u32 move)
{
    return GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS;
}

// specialvar VAR_RESULT, Act2_FindHoldStillMoveUser: a party Pokémon that can hold an ODDISH still: SLEEP POWDER or
// STUN SPORE, or else any GRASS status move
bool16 Act2_FindHoldStillMoveUser(void)
{
    static const u16 sMoves[] = { MOVE_SLEEP_POWDER, MOVE_STUN_SPORE, MOVE_NONE };
    return FindMoveUser(sMoves, IsStatusMove);
}

// ---------------------------------------------------------------------------------------------------------------
// The glade

// special Act2_SetupGladeObjects, on the glade's transition: FLAG_TEMP_2 the MOONVEIL's bud (until dawn, and off the
// trip), FLAG_TEMP_3 its bloom (from dawn, on the trip), FLAG_TEMP_4 the MORELULL (on the trip until it comes along),
// FLAG_TEMP_5 PROF. FERN (once she's come, on the trip), FLAG_TEMP_6-8 the waves' BUG POKéMON and FLAG_TEMP_9 (the
// second's other), FLAG_TEMP_A-B (the third's others): the wave at its gap, from the arrival until it's beaten.
void Act2_SetupGladeObjects(void)
{
    u32 bloom = GetBloom(), i;
    bool32 trip = Act2_IsGrassTrip();
    for (i = 0; i < 10; i++)
        FlagSet(FLAG_TEMP_2 + i);
    FlagClear((trip && (bloom & NIGHT_BLOOM_DAWN)) ? FLAG_TEMP_3 : FLAG_TEMP_2);
    if (trip && !FlagGet(FLAG_ACT2_GOT_MORELULL))
        FlagClear(FLAG_TEMP_4);
    if (trip && (bloom & NIGHT_BLOOM_FERN))
        FlagClear(FLAG_TEMP_5);
    if (trip && (bloom & NIGHT_BLOOM_ARRIVED) && (bloom & NIGHT_BLOOM_WAVES) != NIGHT_BLOOM_WAVES)
    {
        for (i = 0; i < 3; i++)
        {
            if (!(bloom & (NIGHT_BLOOM_WAVE_1 << i)))
            {
                FlagClear(FLAG_TEMP_6 + i);
                if (i == 1)
                    FlagClear(FLAG_TEMP_9);
                if (i == 2)
                {
                    FlagClear(FLAG_TEMP_A);
                    FlagClear(FLAG_TEMP_B);
                }
                break;
            }
        }
    }
}

// specialvar VAR_RESULT, Act2_IsDewCollected: whether the player collected the MOONVEIL's dew (the puzzle's end)
bool16 Act2_IsDewCollected(void)
{
    return (VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_PUZZLE_DONE) != 0;
}

// specialvar VAR_RESULT, Act2_GetPlayerSideOfBloom: which side of the MOONVEIL (10,8) the player stands on, next to
// it (DIR_*), or DIR_NONE: where the MORELULL comes over to them from beside it (11,9)
u16 Act2_GetPlayerSideOfBloom(void)
{
    s16 x, y;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    if (x == 10 && y == 9)
        return DIR_SOUTH;
    if (x == 10 && y == 7)
        return DIR_NORTH;
    if (x == 9 && y == 8)
        return DIR_WEST;
    if (x == 11 && y == 8)
        return DIR_EAST;
    return DIR_NONE;
}

// specialvar VAR_RESULT, Act2_GetGapStepOut with the wave (1-3) in VAR_0x8004: the way to step (DIR_*) for the
// player and the Pokémon following them to be out of the wave's gap, where its hedge grows (west: x 4 on;
// north: y 6 on; east: x 16 back), or
// DIR_NONE when they're clear of it
u16 Act2_GetGapStepOut(void)
{
    s16 x, y;
    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;
    switch (gSpecialVar_0x8004)
    {
    case 1:
        return x < 4 ? DIR_EAST : DIR_NONE;
    case 2:
        return y < 6 ? DIR_SOUTH : DIR_NONE;
    case 3:
        return x > 16 ? DIR_WEST : DIR_NONE;
    }
    return DIR_NONE;
}

// special Act2_DrawGladeHedges, on the glade's load: the hedges planted this trip
void Act2_DrawGladeHedges(void)
{
    u32 bloom = GetBloom(), wave;
    if (!Act2_IsGrassTrip() || !IS_ON_MAP(MAP_VIRIDIAN_FOREST_GLADE))
        return;
    for (wave = 0; wave < 3; wave++)
    {
        if (bloom & (NIGHT_BLOOM_HEDGE_1 << wave))
            DrawSquares(sGladeHedges[wave], ARRAY_COUNT(sGladeHedges[wave]));
    }
}

// specialvar VAR_RESULT, Act2_GetGladeScene: what's due in the glade (GLADE_SCENE_*)
u16 Act2_GetGladeScene(void)
{
    u32 bloom = GetBloom();
    if (!Act2_IsGrassTrip() || !(bloom & NIGHT_BLOOM_ODDISH_3))
        return GLADE_SCENE_NONE;
    if (!(bloom & NIGHT_BLOOM_ARRIVED))
        return GLADE_SCENE_ARRIVAL;
    if ((bloom & NIGHT_BLOOM_WAVES) != NIGHT_BLOOM_WAVES)
        return GLADE_SCENE_VIGIL;
    if (!(bloom & NIGHT_BLOOM_DAWN))
        return GLADE_SCENE_DAWN;
    if (!(VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_PUZZLE_DONE))
        return GLADE_SCENE_NONE;            // The dew waits on the MOONVEIL for the player to collect it
    if (!(bloom & NIGHT_BLOOM_FERN))
        return GLADE_SCENE_MORNING;
    return GLADE_SCENE_NONE;
}

// specialvar VAR_RESULT, Act2_GetNextWave: the next wave (1-3: west, north, east), or 0 when all three are over.
// VAR_0x8005 is its BUG POKéMON's local id.
u16 Act2_GetNextWave(void)
{
    u32 bloom = GetBloom(), wave;
    for (wave = 0; wave < 3; wave++)
    {
        if (!(bloom & (NIGHT_BLOOM_WAVE_1 << wave)))
        {
            gSpecialVar_0x8005 = LOCALID_GLADE_BUG_1 + wave;
            return wave + 1;
        }
    }
    return 0;
}

// The wave's end (fought or kept out): TRUE if it was the last one (the puzzle's third part)
static bool32 EndWave(u32 wave)
{
    SetBloom(NIGHT_BLOOM_WAVE_1 << (wave - 1));
    FlagSet(FLAG_TEMP_6 + wave - 1);
    if ((GetBloom() & NIGHT_BLOOM_WAVES) != NIGHT_BLOOM_WAVES)
        return FALSE;
    SetResearch(ACT2_RESEARCH_PUZZLE_3);
    return TRUE;
}

// specialvar VAR_RESULT, Act2_EndWave with the wave (1-3) in VAR_0x8004: after a won battle
bool16 Act2_EndWave(void)
{
    return EndWave(gSpecialVar_0x8004);
}

// specialvar VAR_RESULT, Act2_PlantHedge with the wave (1-3) in VAR_0x8004: after the wave is beaten, the MORELULL's
// INGRAIN grows a hedge across its gap and the wave is over (the script redraws the map). TRUE after the last one.
bool16 Act2_PlantHedge(void)
{
    u32 wave = gSpecialVar_0x8004;
    SetBloom(NIGHT_BLOOM_HEDGE_1 << (wave - 1));
    if (IS_ON_MAP(MAP_VIRIDIAN_FOREST_GLADE))
        DrawSquares(sGladeHedges[wave - 1], ARRAY_COUNT(sGladeHedges[wave - 1]));
    return EndWave(wave);
}

// The waves: hungrier and bigger each time, at the area's level (Viridian Forest's, Act2Road_GetAreaLevel)
struct BloomWaveMon
{
    u16 species;
    s8 levelOffset;
};

static const struct BloomWaveMon sBloomWaves[3][3] =
{
    { { SPECIES_VENONAT, 0 } },
    { { SPECIES_PARAS, 0 }, { SPECIES_VENONAT, 0 } },
    { { SPECIES_VENOMOTH, 2 }, { SPECIES_BEEDRILL, 1 }, { SPECIES_VENONAT, 0 } },
};

u32 Act2_GetBloomWaveSize(u32 wave)
{
    u32 count = 0;
    while (count < 3 && sBloomWaves[wave - 1][count].species != SPECIES_NONE)
        count++;
    return count;
}

// For StartAct2BloomWaveBattle (src/battle_setup.c): the wave in VAR_0x8004 as the wild party. Returns its size.
u32 Act2_CreateBloomWaveParty(void)
{
    u32 wave = gSpecialVar_0x8004, count = Act2_GetBloomWaveSize(wave), i;
    s32 level = Act2Road_GetAreaLevel();
    ZeroEnemyPartyMons();
    for (i = 0; i < count; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
        CreateRandomMon(mon, sBloomWaves[wave - 1][i].species, min(max(level + sBloomWaves[wave - 1][i].levelOffset, 1), MAX_LEVEL));
        Act2_TeachWildCourseMoves(mon);
    }
    return count;
}

// special Act2_BreakDawn: the night is over. Day for the rest of the trip (the scene stepped through the dawn's
// hours with GameTime_ShowHour; this goes back to the game's own).
void Act2_BreakDawn(void)
{
    SetBloom(NIGHT_BLOOM_DAWN);
    SetTimeOfDay(0);
}

// specialvar VAR_RESULT, Act2_GiveGladeMorelull: the MORELULL that lit the glade comes along, at ACT2_MORELULL_LEVEL;
// to the party (0) or the PC (1), as givemon. A gift (fateful encounter): it doesn't count toward the sheet's catches.
u16 Act2_GiveGladeMorelull(void)
{
    struct Pokemon mon;
    bool8 gift = TRUE;
    u32 result;
    CreateRandomMon(&mon, SPECIES_MORELULL, ACT2_MORELULL_LEVEL);
    SetMonData(&mon, MON_DATA_MODERN_FATEFUL_ENCOUNTER, &gift);
    result = GiveScriptedMonToPlayer(&mon, PARTY_SIZE);
    if (result != MON_CANT_GIVE)
        FlagSet(FLAG_ACT2_GOT_MORELULL);
    return result;
}

// Whether a Pokémon is the glade's MORELULL (a gift, like the Lapras: not counted as caught)
bool32 Act2_IsGladeMorelull(u32 species, bool32 fateful)
{
    return species == SPECIES_MORELULL && fateful;
}
