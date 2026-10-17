#include "global.h"
#include "act2_story.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "constants/opponents.h"
#include "constants/story.h"

// Act 2's story (docs/hack/act2_story.md)

// The hour Act2_SetNight shows (any night hour; 0 would mean no override)
#define CLUB_NIGHT_HOUR 2

// Until the player has a Trainer Card, trainers won't battle them: they don't notice the player
// (CheckForTrainersWantingBattle) and turn them down when talked to (ConfigureTrainerBattle).
bool32 Act2_TrainersLocked(void)
{
    return VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY && VarGet(VAR_ACT2_PROGRESS) < ACT2_GOT_TRAINER_CARD;
}

// The professors' exams, the club's battles and the Trainer Card exam happen before the Trainer Card
bool32 Act2_IsStoryTrainer(u32 trainerId)
{
    return trainerId >= TRAINER_ACT2_PROF_FIRE && trainerId <= TRAINER_ACT2_EXAMINER;
}

// Act 2 has its own wild Pokémon in some areas (the course sites, the final project's), at Act 2's levels. They're
// the sAct2_* entries at the end of wild_encounters.json, after each area's own (GetCurrentMapWildMonHeaderId).
bool32 Act2_UsesAct2WildPokemon(void)
{
    return VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY;
}

static bool32 MonHasType(struct Pokemon *mon, enum Type type)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    return GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type;
}

// specialvar VAR_RESULT, Act2_PartyHasType with the type in VAR_0x8004: whether a Pokémon in the party has it
bool16 Act2_PartyHasType(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && MonHasType(mon, gSpecialVar_0x8004))
            return TRUE;
    }
    return FALSE;
}

// specialvar VAR_RESULT, Act2_PartyHasProjectPokemon: the final project, a Pokémon that's two of Fire, Water and
// Grass (Lotad's family is Water and Grass; Scovillain, Capsakid evolved with a Fire Stone, is Grass and Fire)
bool16 Act2_PartyHasProjectPokemon(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (MonHasType(mon, TYPE_FIRE) + MonHasType(mon, TYPE_WATER) + MonHasType(mon, TYPE_GRASS) >= 2)
            return TRUE;
    }
    return FALSE;
}

// special Act2_SetNight: the club meets late at night, whatever the clock says. Lasts until the next warp.
void Act2_SetNight(void)
{
    SetTimeOfDay(CLUB_NIGHT_HOUR);
    UpdateTimeOfDay(TRUE);
}

// The time skip: the Pokémon the player brings to university have grown, to at least ACT2_START_LEVEL, and know
// the moves they'd know at that level
void Act2_RaisePartyLevels(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp;
        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || GetMonData(mon, MON_DATA_LEVEL) >= ACT2_START_LEVEL)
            continue;
        exp = gExperienceTables[gSpeciesInfo[species].growthRate][ACT2_START_LEVEL];
        SetMonData(mon, MON_DATA_EXP, &exp);
        CalculateMonStats(mon);
        GiveMonInitialMoveset(mon);
    }
}
