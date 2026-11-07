#include "global.h"
#include "act2_road.h"
#include "act2_story.h"
#include "battle.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/map_groups.h"
#include "constants/opponents.h"
#include "constants/regions.h"
#include "constants/story.h"

// Act 2's final project: the open world (docs/hack/act2_finale.md, "The road home"). From ACT2_FINAL_PROJECT, the
// route home is open both ways, and wild and trainers' Pokémon follow the party's level: close to it on the route,
// well above it anywhere else. Some are Pokémon from beyond Kanto.
//
// All of Act 2's levels (these and the courses' wild Pokémon, Act2_GetWildLevel) start from the area's level: the
// party's average level when the player first came to the area (its region map section) in this course, or in the
// final project (Act2Road_GetAreaLevel). Coming back or leveling up doesn't change it.

// How often (percent) a scaled wild or trainer's Pokémon is one from beyond Kanto instead
#define ACT2_ROAD_OTHER_REGION_CHANCE 30

#define ACT2_ROAD_STAGES 3 // Basic, stage 1, stage 2

struct Act2RoadPool
{
    const u16 *species;
    u16 count;
};

#include "data/act2_other_region_pools.h"

// The route home, Celadon to Pallet (the farm), both ways: the routes, the gates and caves on them, and the cities'
// outdoor maps. Buildings in the cities (gyms, shops...) aren't on it.
static const u16 sRouteHomeMaps[] =
{
    MAP_CELADON_UNIVERSITY,
    MAP_CELADON_CITY_UNIVERSITY_GATE,
    MAP_CELADON_CITY,
    MAP_ROUTE7,
    MAP_ROUTE7_EAST_ENTRANCE,
    MAP_SAFFRON_CITY_CONNECTION,
    MAP_SAFFRON_CITY,
    // The south way: Vermilion and Diglett's Cave
    MAP_ROUTE6_NORTH_ENTRANCE,
    MAP_ROUTE6,
    MAP_VERMILION_CITY,
    MAP_ROUTE11,
    MAP_DIGLETTS_CAVE_SOUTH_ENTRANCE,
    MAP_DIGLETTS_CAVE_B1F,
    MAP_DIGLETTS_CAVE_NORTH_ENTRANCE,
    // The north way: Cerulean, Mt. Moon, Pewter and Viridian Forest
    MAP_ROUTE5_SOUTH_ENTRANCE,
    MAP_ROUTE5,
    MAP_CERULEAN_CITY,
    MAP_ROUTE4,
    MAP_MT_MOON_1F,
    MAP_MT_MOON_B1F,
    MAP_MT_MOON_B2F,
    MAP_ROUTE3,
    MAP_PEWTER_CITY,
    MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE,
    MAP_VIRIDIAN_FOREST,
    MAP_ROUTE2_VIRIDIAN_FOREST_SOUTH_ENTRANCE,
    // Between them, under Saffron: Route 5 to Route 6
    MAP_UNDERGROUND_PATH_NORTH_ENTRANCE,
    MAP_UNDERGROUND_PATH_NORTH_SOUTH_TUNNEL,
    MAP_UNDERGROUND_PATH_SOUTH_ENTRANCE,
    // Both ways
    MAP_ROUTE2,
    MAP_ROUTE2_EAST_BUILDING,
    MAP_VIRIDIAN_CITY,
    MAP_ROUTE1,
    MAP_PALLET_TOWN,
    MAP_FARMYARD,
    MAP_WOODS,
};

// The last trainer in each area of the route home (both ways) with trainers on the way: the one nearest the area's
// exit toward home. Battled at the party's average level when the battle starts, not the area's.
static const u16 sAreaFinalTrainers[] =
{
    TRAINER_CAMPER_JEFF,        // Route 6, by the exit to Vermilion
    TRAINER_YOUNGSTER_EDDIE,    // Route 11, nearest Diglett's Cave
    TRAINER_LASS_CRISSY,        // Route 4, its only trainer
    TRAINER_LASS_IRIS,          // Mt. Moon (1F), nearest the exit to Route 4
    TRAINER_BUG_CATCHER_COLTON, // Route 3, nearest Pewter
    TRAINER_BUG_CATCHER_RICK,   // Viridian Forest, nearest the exit to Route 2
};

// The final project, in Act 2: levels scaled, other regions' Pokémon
bool32 Act2Road_IsActive(void)
{
    return VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY && VarGet(VAR_ACT2_PROGRESS) >= ACT2_FINAL_PROJECT;
}

// Saffron's four gates let the player through and Route 2's Cut trees are gone
bool32 Act2Road_IsRouteOpen(void)
{
    return FlagGet(FLAG_ACT2_ROUTE_OPEN) || Act2Road_IsActive();
}

bool16 Act2Road_IsRouteOpenSpecial(void)
{
    return Act2Road_IsRouteOpen();
}

bool32 Act2Road_IsOnRouteMap(u32 mapGroup, u32 mapNum)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sRouteHomeMaps); i++)
    {
        if (MAP_GROUP(sRouteHomeMaps[i]) == mapGroup && MAP_NUM(sRouteHomeMaps[i]) == mapNum)
            return TRUE;
    }
    return FALSE;
}

bool32 Act2Road_IsOnRoute(void)
{
    return Act2Road_IsOnRouteMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

u32 Act2Road_GetPartyAverageLevel(void)
{
    u32 i, total = 0, count = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        total += GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
        count++;
    }
    return count == 0 ? ACT2_START_LEVEL : (total + count / 2) / count;
}

// Which part of Act 2 the areas' levels belong to: each course (its field trip, then its night), and the final
// project. 0 outside Act 2.
static u32 GetLevelPeriod(void)
{
    u32 progress;
    if (VarGet(VAR_STORY_ACT) != STORY_ACT_UNIVERSITY)
        return 0;
    progress = VarGet(VAR_ACT2_PROGRESS);
    if (progress < ACT2_CLUB_1_DONE)
        return 1; // Fire
    if (progress < ACT2_CLUB_2_DONE)
        return 2; // Water
    if (progress < ACT2_FINAL_PROJECT)
        return 3; // Grass, and the Trainer Card
    return 4;     // The final project
}

static u8 *GetAreaLevelSlot(u32 mapsec)
{
    if (mapsec < KANTO_MAPSEC_START || mapsec >= KANTO_MAPSEC_START + KANTO_MAPSEC_COUNT)
        return NULL;
    return &gSaveBlock3Ptr->act2AreaLevels[mapsec - KANTO_MAPSEC_START];
}

// The current area's level: the party's average level when the player first came to the area (its region map
// section) in this course, or in the final project. A new course forgets the last one's areas.
u32 Act2Road_GetAreaLevel(void)
{
    u32 period = GetLevelPeriod();
    u8 *slot;
    if (period == 0)
        return Act2Road_GetPartyAverageLevel();
    if (gSaveBlock3Ptr->act2AreaLevelsPeriod != period)
    {
        memset(gSaveBlock3Ptr->act2AreaLevels, 0, sizeof(gSaveBlock3Ptr->act2AreaLevels));
        gSaveBlock3Ptr->act2AreaLevelsPeriod = period;
    }
    slot = GetAreaLevelSlot(Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup,
                                                                gSaveBlock1Ptr->location.mapNum)->regionMapSectionId);
    if (slot == NULL)
        return Act2Road_GetPartyAverageLevel();
    if (*slot == 0)
        *slot = Act2Road_GetPartyAverageLevel();
    return *slot;
}

// Entering a map (LoadMapFromWarp, LoadMapFromCameraTransition): a new area's level is set on arrival
void Act2Road_OnMapEntered(void)
{
    if (GetLevelPeriod() != 0)
        Act2Road_GetAreaLevel();
}

static u32 ClampLevel(s32 level)
{
    return min(max(level, 1), MAX_LEVEL);
}

static u32 GetScaledLevel(s32 offset)
{
    return ClampLevel((s32)Act2Road_GetAreaLevel() + offset);
}

bool32 Act2Road_IsAreaFinalTrainer(u32 trainerId)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sAreaFinalTrainers); i++)
    {
        if (sAreaFinalTrainers[i] == trainerId)
            return TRUE;
    }
    return FALSE;
}

// On the route, anywhere from ACT2_ROUTE_WILD_OFFSET below the average up to the average; elsewhere, exactly
// ACT2_OFF_ROUTE_OFFSET above
u32 Act2Road_GetWildLevel(void)
{
    if (Act2Road_IsOnRoute())
        return GetScaledLevel(ACT2_ROUTE_WILD_OFFSET + (s32)(Random() % (1 - ACT2_ROUTE_WILD_OFFSET)));
    return GetScaledLevel(ACT2_OFF_ROUTE_OFFSET);
}

// The area's level on the route, above it elsewhere; an area's final trainer on the route, the party's average level
// right now
u32 Act2Road_GetTrainerLevel(u32 trainerId)
{
    if (!Act2Road_IsOnRoute())
        return GetScaledLevel(ACT2_OFF_ROUTE_OFFSET);
    if (Act2Road_IsAreaFinalTrainer(trainerId))
        return ClampLevel((s32)Act2Road_GetPartyAverageLevel() + ACT2_ROUTE_TRAINER_OFFSET);
    return GetScaledLevel(ACT2_ROUTE_TRAINER_OFFSET);
}

// A level-up evolution's extra conditions, as far as they're about the Pokémon itself (its gender or personality)
// or Kanto. Anything else (friendship, time, items, moves...) doesn't count here.
static bool32 MeetsSimpleConditions(const struct EvolutionParam *params, u32 species, u32 personality)
{
    u32 i;
    u32 upper = (personality >> 16) % 10;
    for (i = 0; params != NULL && params[i].condition != CONDITIONS_END; i++)
    {
        switch (params[i].condition)
        {
        case IF_GENDER:
            if (GetGenderFromSpeciesAndPersonality(species, personality) != params[i].arg1)
                return FALSE;
            break;
        case IF_REGION:
            if (params[i].arg1 != REGION_KANTO)
                return FALSE;
            break;
        case IF_NOT_REGION:
            if (params[i].arg1 == REGION_KANTO)
                return FALSE;
            break;
        case IF_PID_UPPER_MODULO_10_GT:
            if (!(upper > params[i].arg1))
                return FALSE;
            break;
        case IF_PID_UPPER_MODULO_10_EQ:
            if (upper != params[i].arg1)
                return FALSE;
            break;
        case IF_PID_UPPER_MODULO_10_LT:
            if (!(upper <= params[i].arg1))
                return FALSE;
            break;
        case IF_PID_MODULO_100_GT:
            if (!(personality % 100 > params[i].arg1))
                return FALSE;
            break;
        case IF_PID_MODULO_100_EQ:
            if (personality % 100 != params[i].arg1)
                return FALSE;
            break;
        case IF_PID_MODULO_100_LT:
            if (!(personality % 100 < params[i].arg1))
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    return TRUE;
}

// What a Pokémon of this species would have evolved into by this level: level-up evolutions only
u32 Act2Road_EvolveSpeciesToLevel(u32 species, u32 level, u32 personality)
{
    u32 steps, i;
    for (steps = 0; steps < ACT2_ROAD_STAGES; steps++)
    {
        const struct Evolution *evolutions = GetSpeciesEvolutions(species);
        u32 next = SPECIES_NONE;
        if (evolutions == NULL)
            break;
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            if (evolutions[i].method != EVO_LEVEL || evolutions[i].param > level
             || !IsSpeciesEnabled(evolutions[i].targetSpecies)
             || !MeetsSimpleConditions(evolutions[i].params, species, personality))
                continue;
            next = evolutions[i].targetSpecies;
            break;
        }
        if (next == SPECIES_NONE)
            break;
        species = next;
    }
    return species;
}

u32 Act2Road_GetEvoStage(u32 species)
{
    if (species >= NUM_SPECIES)
        return 0;
    return min(sAct2RoadEvoStage[species], ACT2_ROAD_STAGES - 1);
}

static const struct Act2RoadPool *GetPool(enum Type type, u32 stage)
{
    if (type >= NUMBER_OF_MON_TYPES || sAct2RoadPools[type][stage].count == 0)
        return NULL;
    return &sAct2RoadPools[type][stage];
}

// A Pokémon from beyond Kanto of the same evolution stage and primary type (or else its second type)
u32 Act2Road_PickOtherRegionSpecies(u32 species)
{
    u32 stage = Act2Road_GetEvoStage(species);
    const struct Act2RoadPool *pool = GetPool(GetSpeciesType(species, 0), stage);
    if (pool == NULL)
        pool = GetPool(GetSpeciesType(species, 1), stage);
    if (pool == NULL)
        return species;
    return pool->species[Random() % pool->count];
}

static u32 ChooseSpecies(u32 species, u32 level, u32 personality)
{
    if (Random() % 100 < ACT2_ROAD_OTHER_REGION_CHANCE)
        species = Act2Road_PickOtherRegionSpecies(species);
    return Act2Road_EvolveSpeciesToLevel(species, level, personality);
}

// Turns a Pokémon into another species and level in place, keeping everything else (personality, IVs, EVs, held
// item, ball, the no-Dynamax/no-Tera markers), with the moves it'd know at that level
static void RebuildMon(struct Pokemon *mon, u32 species, u32 level)
{
    u32 oldSpecies = GetMonData(mon, MON_DATA_SPECIES);
    u32 abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM);
    u8 nickname[POKEMON_NAME_LENGTH + 1];

    GetMonData(mon, MON_DATA_NICKNAME, nickname);
    SetMonData(mon, MON_DATA_SPECIES, &species);
    if (StringCompare(nickname, GetSpeciesName(oldSpecies)) == 0)
        SetMonData(mon, MON_DATA_NICKNAME, GetSpeciesName(species));
    if (abilityNum >= NUM_ABILITY_SLOTS || GetSpeciesAbility(species, abilityNum) == ABILITY_NONE)
    {
        abilityNum = 0;
        SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
    }
    SetMonData(mon, MON_DATA_EXP, &gExperienceTables[gSpeciesInfo[species].growthRate][level]);
    GiveMonInitialMoveset(mon);
    CalculateMonStats(mon);
    SetMonData(mon, MON_DATA_HP, &(u32){ GetMonData(mon, MON_DATA_MAX_HP) });
}

// A wild Pokémon just made (TryGenerateWildMon, GenerateFishingWildMon): at the final project, sometimes from
// another region, and evolved as it would be at its level (Act2Road_GetWildLevel made the level)
void Act2Road_OnWildMonCreated(struct Pokemon *mon)
{
    u32 species, level;
    if (!Act2Road_IsActive())
        return;
    species = GetMonData(mon, MON_DATA_SPECIES);
    level = GetMonData(mon, MON_DATA_LEVEL);
    species = ChooseSpecies(species, level, GetMonData(mon, MON_DATA_PERSONALITY));
    if (species != GetMonData(mon, MON_DATA_SPECIES))
        RebuildMon(mon, species, level);
}

// Every base-game trainer is scaled, however the battle starts (trainerbattle_single, _no_intro, _double, a
// rematch...: they all make the party in DoTrainerBattle). The hack's own trainers are the story's (the professors,
// the club, the examiner, and any added after them) and keep their levels.
bool32 Act2Road_IsScaledTrainer(u32 trainerId)
{
    return trainerId != TRAINER_NONE && trainerId < TRAINER_ACT2_PROF_FIRE && !Act2_IsStoryTrainer(trainerId);
}

// After a trainer's party is made (DoTrainerBattle): every Pokémon at the scaled level, sometimes from another
// region, evolved for that level, with that level's moves
void Act2Road_ScaleTrainerParty(struct Pokemon *party, u32 trainerId)
{
    u32 i, level, species;
    if (!Act2Road_IsActive() || !Act2Road_IsScaledTrainer(trainerId))
        return;
    level = Act2Road_GetTrainerLevel(trainerId);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        species = GetMonData(&party[i], MON_DATA_SPECIES);
        if (species == SPECIES_NONE || GetMonData(&party[i], MON_DATA_IS_EGG))
            continue;
        species = ChooseSpecies(species, level, GetMonData(&party[i], MON_DATA_PERSONALITY));
        RebuildMon(&party[i], species, level);
    }
}
