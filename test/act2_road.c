#include "global.h"
#include "act2_road.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "pokemon.h"
#include "task.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/story.h"

// Act 2's final project: the road home (docs/hack/act2_finale.md)

static void MoveTo(u32 mapGroup, u32 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
}

// A party whose average level is partyLevel
static void SetPartyLevel(u32 partyLevel)
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_TEDDIURSA, partyLevel - 2, 0, OTID_STRUCT_PLAYER_ID);
    CreateMon(&gPlayerParty[1], SPECIES_TYNAMO, partyLevel + 2, 0, OTID_STRUCT_PLAYER_ID);
}

// Starts the final project afresh: no area's level set yet
static void SetUpFinalProject(u32 mapGroup, u32 mapNum, u32 partyLevel)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT);
    gSaveBlock3Ptr->act2AreaLevelsPeriod = 0;
    MoveTo(mapGroup, mapNum);
    SetPartyLevel(partyLevel);
}

static void ExpectWildLevels(u32 lowest, u32 highest)
{
    static const struct WildPokemon sMon = { .minLevel = 2, .maxLevel = 4, .species = SPECIES_RATTATA };
    u32 i, level, seenLowest = MAX_LEVEL, seenHighest = 0;
    for (i = 0; i < 100; i++)
    {
        level = ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND);
        seenLowest = min(seenLowest, level);
        seenHighest = max(seenHighest, level);
    }
    EXPECT_EQ(seenLowest, lowest);
    EXPECT_EQ(seenHighest, highest);
}

static u32 ScaledTrainerLevel(u32 trainerId)
{
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    ZeroPartyMons(party);
    CreateMon(&party[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_RANDOM_NO_SHINY);
    Act2Road_ScaleTrainerParty(party, trainerId);
    return GetMonData(&party[0], MON_DATA_LEVEL);
}

static void TearDown(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    VarSet(VAR_ACT2_PROGRESS, 0);
}

static bool32 IsFromBeyondKanto(u32 species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    return info->natDexNum > 151 && !info->isRestrictedLegendary && !info->isSubLegendary && !info->isMythical
        && !info->isUltraBeast && !info->isParadox && !info->isAlolanForm && !info->isGalarianForm
        && !info->isHisuianForm && !info->isPaldeanForm && !info->isMegaEvolution && !info->isGigantamax;
}

TEST("Act 2 road: a Pokémon evolves the way it would have by its level")
{
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_CHARMANDER, 15, 0), SPECIES_CHARMANDER);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_CHARMANDER, 16, 0), SPECIES_CHARMELEON);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_CHARMANDER, 36, 0), SPECIES_CHARIZARD);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_CHARMELEON, 100, 0), SPECIES_CHARIZARD);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_CUBONE, 28, 0), SPECIES_MAROWAK); // Not Alola's
    // Not by friendship, items or trades
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_ZUBAT, 60, 0), SPECIES_GOLBAT);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_EEVEE, 60, 0), SPECIES_EEVEE);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_PIKACHU, 60, 0), SPECIES_PIKACHU);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_MACHOP, 60, 0), SPECIES_MACHOKE);
}

TEST("Act 2 road: a gendered level-up evolution follows the Pokémon's gender")
{
    u32 female = 0, male = 0xFF;
    ASSUME(GetGenderFromSpeciesAndPersonality(SPECIES_COMBEE, female) == MON_FEMALE);
    ASSUME(GetGenderFromSpeciesAndPersonality(SPECIES_COMBEE, male) == MON_MALE);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_COMBEE, 30, female), SPECIES_VESPIQUEN);
    EXPECT_EQ(Act2Road_EvolveSpeciesToLevel(SPECIES_COMBEE, 30, male), SPECIES_COMBEE);
}

TEST("Act 2 road: another region's Pokémon matches the type and evolution stage")
{
    static const u16 sKanto[] = { SPECIES_CHARMANDER, SPECIES_CHARMELEON, SPECIES_CHARIZARD, SPECIES_PIDGEY,
                                  SPECIES_GEODUDE, SPECIES_GOLEM, SPECIES_MAGIKARP, SPECIES_PIKACHU, SPECIES_ODDISH };
    u32 i, j;
    for (i = 0; i < ARRAY_COUNT(sKanto); i++)
    {
        for (j = 0; j < 20; j++)
        {
            u32 species = Act2Road_PickOtherRegionSpecies(sKanto[i]);
            EXPECT(IsFromBeyondKanto(species));
            EXPECT_EQ(GetSpeciesType(species, 0), GetSpeciesType(sKanto[i], 0));
            EXPECT_EQ(Act2Road_GetEvoStage(species), Act2Road_GetEvoStage(sKanto[i]));
        }
    }
    EXPECT_EQ(Act2Road_GetEvoStage(SPECIES_PIKACHU), 0); // Babies don't count
    EXPECT_EQ(Act2Road_GetEvoStage(SPECIES_RAICHU), 1);
    EXPECT_EQ(Act2Road_GetEvoStage(SPECIES_CHARIZARD), 2);
}

TEST("Act 2 road: wild Pokémon are 0-3 below the party on the route home, 15 above off it")
{
    static const struct WildPokemon sMon = { .minLevel = 2, .maxLevel = 4, .species = SPECIES_RATTATA };
    u32 i, level, seen = 0;
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE2), MAP_NUM(MAP_ROUTE2), 30);
    for (i = 0; i < 200; i++)
    {
        level = ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND);
        EXPECT(level >= 27 && level <= 30);
        seen |= 1 << (level - 27);
    }
    EXPECT_EQ(seen, 0xF); // Every level in the range turns up
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22), 30);
    EXPECT_EQ(ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND), 45);
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22), 95);
    EXPECT_EQ(ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND), MAX_LEVEL);
    TearDown();
}

TEST("Act 2 road: the route home is the cities' outdoors, not their buildings")
{
    EXPECT(Act2Road_IsOnRouteMap(MAP_GROUP(MAP_DIGLETTS_CAVE_B1F), MAP_NUM(MAP_DIGLETTS_CAVE_B1F)));
    EXPECT(Act2Road_IsOnRouteMap(MAP_GROUP(MAP_VERMILION_CITY), MAP_NUM(MAP_VERMILION_CITY)));
    EXPECT(!Act2Road_IsOnRouteMap(MAP_GROUP(MAP_VERMILION_CITY_GYM), MAP_NUM(MAP_VERMILION_CITY_GYM)));
    EXPECT(!Act2Road_IsOnRouteMap(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22)));
}

TEST("Act 2 road: a trainer's Pokémon are rebuilt at the party's level with its moves")
{
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    u32 i, species;
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE11), MAP_NUM(MAP_ROUTE11), 30);
    for (i = 0; i < 20; i++)
    {
        ZeroPartyMons(party);
        CreateMon(&party[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_RANDOM_NO_SHINY);
        SetMonData(&party[0], MON_DATA_DYNAMAX_LEVEL, &(u32){ BLOCK_AI_DYNAMAX });
        SetMonData(&party[0], MON_DATA_HELD_ITEM, &(u32){ ITEM_ORAN_BERRY });
        Act2Road_ScaleTrainerParty(party, TRAINER_YOUNGSTER_BEN);
        species = GetMonData(&party[0], MON_DATA_SPECIES);
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 30);
        EXPECT(species == SPECIES_RATICATE || IsFromBeyondKanto(species));
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_HP), GetMonData(&party[0], MON_DATA_MAX_HP));
        EXPECT_NE(GetMonData(&party[0], MON_DATA_MOVE1), MOVE_NONE);
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_HELD_ITEM), ITEM_ORAN_BERRY);
        EXPECT_EQ(GetMonData(&party[0], MON_DATA_DYNAMAX_LEVEL), BLOCK_AI_DYNAMAX);
        EXPECT_EQ(GetMonData(&party[1], MON_DATA_SPECIES), SPECIES_NONE);
    }
    TearDown();
}

TEST("Act 2 road: the story's trainers, and battles before the final project, aren't scaled")
{
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE11), MAP_NUM(MAP_ROUTE11), 30);
    ZeroPartyMons(party);
    CreateMon(&party[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_RANDOM_NO_SHINY);
    Act2Road_ScaleTrainerParty(party, TRAINER_ACT2_EXAMINER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 5);
    VarSet(VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD);
    Act2Road_ScaleTrainerParty(party, TRAINER_YOUNGSTER_BEN);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 5);
    EXPECT(!Act2Road_IsRouteOpen());
    VarSet(VAR_STORY_ACT, STORY_ACT_WORKFORCE);
    VarSet(VAR_ACT2_PROGRESS, ACT2_DONE);
    Act2Road_ScaleTrainerParty(party, TRAINER_YOUNGSTER_BEN);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 5);
    TearDown();
}

TEST("Act 2 road: an area's level is set on the first visit, not by coming back or leveling up")
{
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE2), MAP_NUM(MAP_ROUTE2), 30);
    Act2Road_OnMapEntered();
    SetPartyLevel(40);
    ExpectWildLevels(27, 30); // Route 2: set at 30
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_YOUNGSTER_BEN), 30);
    MoveTo(MAP_GROUP(MAP_ROUTE1), MAP_NUM(MAP_ROUTE1));
    Act2Road_OnMapEntered();
    SetPartyLevel(50);
    ExpectWildLevels(37, 40); // Route 1: first came at 40
    MoveTo(MAP_GROUP(MAP_ROUTE2), MAP_NUM(MAP_ROUTE2));
    Act2Road_OnMapEntered();
    ExpectWildLevels(27, 30); // Back on Route 2: still 30
    MoveTo(MAP_GROUP(MAP_ROUTE22), MAP_NUM(MAP_ROUTE22));
    ExpectWildLevels(65, 65); // Off the route, set at its first encounter (50): 15 above
    SetPartyLevel(20);
    ExpectWildLevels(65, 65);
    TearDown();
}

TEST("Act 2 road: an area is a region map section: a route's gates, a city's buildings, a cave's floors")
{
    SetUpFinalProject(MAP_GROUP(MAP_MT_MOON_1F), MAP_NUM(MAP_MT_MOON_1F), 30);
    Act2Road_OnMapEntered();
    SetPartyLevel(40);
    MoveTo(MAP_GROUP(MAP_MT_MOON_B2F), MAP_NUM(MAP_MT_MOON_B2F));
    Act2Road_OnMapEntered();
    EXPECT_EQ(Act2Road_GetAreaLevel(), 30);
    MoveTo(MAP_GROUP(MAP_ROUTE4), MAP_NUM(MAP_ROUTE4));
    Act2Road_OnMapEntered();
    EXPECT_EQ(Act2Road_GetAreaLevel(), 40);
    MoveTo(MAP_GROUP(MAP_ROUTE4_POKEMON_CENTER_1F), MAP_NUM(MAP_ROUTE4_POKEMON_CENTER_1F));
    SetPartyLevel(50);
    EXPECT_EQ(Act2Road_GetAreaLevel(), 40);
    // The Pokémon Center isn't on the route: 15 above Route 4's level
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_YOUNGSTER_BEN), 55);
    TearDown();
}

TEST("Act 2 road: each course, and the final project, sets its areas' levels afresh")
{
    static const struct WildPokemon sMon = { .minLevel = 2, .maxLevel = 4, .species = SPECIES_RATTATA };
    u32 i, level;
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_WATER_TRIP);
    MoveTo(MAP_GROUP(MAP_VERMILION_CITY), MAP_NUM(MAP_VERMILION_CITY));
    SetPartyLevel(20);
    Act2Road_OnMapEntered();
    SetPartyLevel(25);
    for (i = 0; i < 50; i++)
    {
        level = ChooseWildMonLevel(&sMon, 0, WILD_AREA_LAND); // A field trip's: within 3 of the area's level
        EXPECT(level >= 17 && level <= 23);
    }
    VarSet(VAR_ACT2_PROGRESS, ACT2_WATER_INVITED);
    EXPECT_EQ(Act2Road_GetAreaLevel(), 20); // The same course
    VarSet(VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP);
    EXPECT_EQ(Act2Road_GetAreaLevel(), 25); // The next course
    SetPartyLevel(30);
    VarSet(VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT);
    EXPECT_EQ(Act2Road_GetAreaLevel(), 30); // The final project
    VarSet(VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED);
    SetPartyLevel(35);
    EXPECT_EQ(Act2Road_GetAreaLevel(), 30); // Still the final project
    TearDown();
}

TEST("Act 2 road: an area's final trainer on the way home is at the party's level when battled")
{
    SetUpFinalProject(MAP_GROUP(MAP_ROUTE6), MAP_NUM(MAP_ROUTE6), 30);
    Act2Road_OnMapEntered();
    SetPartyLevel(36);
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_CAMPER_RICKY), 30); // The area's level
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_CAMPER_JEFF), 36);  // Route 6's last, by the exit to Vermilion
    SetPartyLevel(38);
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_CAMPER_JEFF), 38);
    EXPECT(Act2Road_IsAreaFinalTrainer(TRAINER_YOUNGSTER_EDDIE));
    EXPECT(Act2Road_IsAreaFinalTrainer(TRAINER_LASS_CRISSY));
    EXPECT(Act2Road_IsAreaFinalTrainer(TRAINER_LASS_IRIS));
    EXPECT(Act2Road_IsAreaFinalTrainer(TRAINER_BUG_CATCHER_COLTON));
    EXPECT(Act2Road_IsAreaFinalTrainer(TRAINER_BUG_CATCHER_RICK));
    TearDown();
}

TEST("Act 2 road: both ways home are on the route")
{
    static const u16 sMaps[] =
    {
        MAP_ROUTE6, MAP_VERMILION_CITY, MAP_ROUTE11, MAP_DIGLETTS_CAVE_B1F,
        MAP_ROUTE5_SOUTH_ENTRANCE, MAP_ROUTE5, MAP_CERULEAN_CITY, MAP_ROUTE4, MAP_MT_MOON_1F, MAP_MT_MOON_B1F,
        MAP_MT_MOON_B2F, MAP_ROUTE3, MAP_PEWTER_CITY, MAP_ROUTE2_VIRIDIAN_FOREST_NORTH_ENTRANCE, MAP_VIRIDIAN_FOREST,
        MAP_ROUTE2_VIRIDIAN_FOREST_SOUTH_ENTRANCE, MAP_UNDERGROUND_PATH_NORTH_SOUTH_TUNNEL, MAP_ROUTE2, MAP_ROUTE1,
    };
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
        EXPECT(Act2Road_IsOnRouteMap(MAP_GROUP(sMaps[i]), MAP_NUM(sMaps[i])));
    EXPECT(!Act2Road_IsOnRouteMap(MAP_GROUP(MAP_ROUTE4_POKEMON_CENTER_1F), MAP_NUM(MAP_ROUTE4_POKEMON_CENTER_1F)));
    EXPECT(!Act2Road_IsOnRouteMap(MAP_GROUP(MAP_ROUTE24), MAP_NUM(MAP_ROUTE24)));
}

TEST("Act 2 road: every trainer from the base game is scaled, the story's aren't")
{
    u32 trainerId;
    for (trainerId = TRAINER_NONE + 1; trainerId < TRAINER_ACT2_PROF_FIRE; trainerId++)
        EXPECT(Act2Road_IsScaledTrainer(trainerId));
    for (trainerId = TRAINER_ACT2_PROF_FIRE; trainerId <= TRAINER_ACT2_EXAMINER; trainerId++)
        EXPECT(!Act2Road_IsScaledTrainer(trainerId));
    EXPECT(!Act2Road_IsScaledTrainer(TRAINER_NONE));
}

// Cerulean's Rocket battles from a script (trainerbattle_no_intro), not by spotting the player. Every trainer battle
// starts in BattleSetup_StartTrainerBattle, which makes the party and scales it. (Tests have their own trainers:
// trainer 1 is the test data's, with Pokémon at level 5.)
TEST("Act 2 road: a trainer battle started from a script is scaled, Cerulean's Rocket too")
{
    struct Pokemon *party = gParties[B_TRAINER_OPPONENT_A];
    u32 i;
    SetUpFinalProject(MAP_GROUP(MAP_CERULEAN_CITY), MAP_NUM(MAP_CERULEAN_CITY), 24);
    InitTrainerBattleParameter();
    TRAINER_BATTLE_PARAM.opponentA = 1;
    BattleSetup_StartTrainerBattle();
    ResetTasks(); // The battle's start (its transition) isn't run
    EXPECT_NE(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_NONE);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE)
            EXPECT_EQ(GetMonData(&party[i], MON_DATA_LEVEL), 24);
    }
    // Cerulean is on the north way home now: the Rocket is at the area's level, not 15 above
    EXPECT_EQ(ScaledTrainerLevel(TRAINER_TEAM_ROCKET_GRUNT_5), 24);
    TearDown();
}
