#include "global.h"
#include "act2_story.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/heal_locations.h"
#include "constants/pokeball.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/story.h"

// Act 2's story (docs/hack/act2_story.md)

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

// In Act 2, every wild Pokémon is within ACT2_WILD_LEVEL_SPREAD levels of the party's average level (from
// ChooseWildMonLevel). The areas' tables still pick the species.
u32 Act2_GetWildLevel(void)
{
    u32 i, total = 0, count = 0;
    s32 level;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        total += GetMonData(mon, MON_DATA_LEVEL);
        count++;
    }
    level = (count == 0) ? ACT2_START_LEVEL : (total + count / 2) / count;
    level += (s32)(Random() % (ACT2_WILD_LEVEL_SPREAD * 2 + 1)) - ACT2_WILD_LEVEL_SPREAD;
    return min(max(level, 1), MAX_LEVEL);
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

// From coming back to the dorms after a course until the club's battle that night, it's night (UpdateTimeOfDay),
// whatever the clock says
bool16 Act2_IsClubNight(void)
{
    if (VarGet(VAR_STORY_ACT) != STORY_ACT_UNIVERSITY)
        return FALSE;
    switch (VarGet(VAR_ACT2_PROGRESS))
    {
    case ACT2_FIRE_SEE_KAI:
    case ACT2_FIRE_INVITED:
    case ACT2_WATER_SEE_MINA:
    case ACT2_WATER_INVITED:
    case ACT2_GRASS_SEE_ROWAN:
    case ACT2_GRASS_INVITED:
        return TRUE;
    }
    return FALSE;
}

// What Act 2 starts with: the POKéMON start menu entry (Act 1 never sets it: no Poké Balls, the Pokémon follow), and
// the Remote Desktop, the Pokémon Storage System from anywhere in Acts 2 and 3 (the expansion's Pokémon Box Link,
// renamed). Given when Act 2 starts; special Act2_GiveStartingKit on campus covers saves already in Act 2.
void Act2_GiveStartingKit(void)
{
    if (VarGet(VAR_STORY_ACT) < STORY_ACT_UNIVERSITY)
        return;
    FlagSet(FLAG_SYS_POKEMON_GET);
    if (!CheckBagHasItem(ITEM_REMOTE_DESKTOP, 1))
        AddBagItem(ITEM_REMOTE_DESKTOP, 1);
}

// Whether the player has a real Trainer Card yet: from the third club night in Act 2 on (the start menu's player entry
// says where it would go until then)
bool32 Act2_HasTrainerCard(void)
{
    if (VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY)
        return VarGet(VAR_ACT2_PROGRESS) >= ACT2_GOT_TRAINER_CARD;
    return VarGet(VAR_STORY_ACT) > STORY_ACT_UNIVERSITY;
}

// The time skip: the Pokémon the player brings to university have grown, to at least ACT2_START_LEVEL, and know
// the moves they'd know at that level
#define ACT2_MIN_STARTING_MOVES 2

static u32 CountMonMoves(struct Pokemon *mon)
{
    u32 i, count = 0;
    for (i = 0; i < MAX_MON_MOVES; i++)
        count += GetMonData(mon, MON_DATA_MOVE1 + i) != MOVE_NONE;
    return count;
}

// The moves a Pokémon starts Act 2 with: the ones it would have learned by its level, at least
// ACT2_MIN_STARTING_MOVES of them (Tackle fills in)
void Act2_GiveStartingMoves(struct Pokemon *mon)
{
    u32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, MOVE_NONE, i);
    GiveMonInitialMoveset(mon);
    if (CountMonMoves(mon) < ACT2_MIN_STARTING_MOVES)
        GiveMoveToMon(mon, MOVE_TACKLE);
}

void Act2_RaisePartyLevels(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp;
        if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (GetMonData(mon, MON_DATA_LEVEL) < ACT2_START_LEVEL)
        {
            exp = gExperienceTables[gSpeciesInfo[species].growthRate][ACT2_START_LEVEL];
            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
        }
        Act2_GiveStartingMoves(mon);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The courses' research (docs/hack/act2_story.md). On a field trip, the professor's research sheet has four tasks:
// catch a Pokémon of the course's type, use a move of that type, record three different moves of that type in
// battle (the player's or wild Pokémon's), and finish the site's puzzle. Then the professor helps make the course's
// TM, and the exam only counts if the player's Pokémon uses its move.

#define RESEARCH_MOVES_TO_RECORD 3

struct Act2Course
{
    u16 tripStage;
    enum Type type;
    u16 tmMove;
    u16 professor;
    const u8 *typeName;
    const u8 *puzzleTask;
};

static const u8 sText_Fire[] = _("FIRE");
static const u8 sText_Water[] = _("WATER");
static const u8 sText_Grass[] = _("GRASS");
static const u8 sText_PuzzleFire[] = _("Relight the old furnaces.");
static const u8 sText_PuzzleWater[] = _("Put out the fires on the docks.");
static const u8 sText_PuzzleGrass[] = _("Regrow the withered patches.");

static const struct Act2Course sCourses[] =
{
    { ACT2_FIRE_TRIP,  TYPE_FIRE,  MOVE_FLAMETHROWER, TRAINER_ACT2_PROF_FIRE,  sText_Fire,  sText_PuzzleFire },
    { ACT2_WATER_TRIP, TYPE_WATER, MOVE_BUBBLE_BEAM,  TRAINER_ACT2_PROF_WATER, sText_Water, sText_PuzzleWater },
    { ACT2_GRASS_TRIP, TYPE_GRASS, MOVE_GIGA_DRAIN,   TRAINER_ACT2_PROF_GRASS, sText_Grass, sText_PuzzleGrass },
};

static const struct Act2Course *GetCurrentCourse(void)
{
    u32 i;
    if (VarGet(VAR_STORY_ACT) != STORY_ACT_UNIVERSITY)
        return NULL;
    for (i = 0; i < ARRAY_COUNT(sCourses); i++)
    {
        if (VarGet(VAR_ACT2_PROGRESS) == sCourses[i].tripStage)
            return &sCourses[i];
    }
    return NULL;
}

static u32 CountMovesRecorded(void)
{
    return (VarGet(VAR_ACT2_MOVE_SEEN_1) != MOVE_NONE) + (VarGet(VAR_ACT2_MOVE_SEEN_2) != MOVE_NONE)
         + ((VarGet(VAR_ACT2_RESEARCH) & ACT2_RESEARCH_MOVES_DONE) != 0);
}

// TRUE if the move is new: the first two are kept, the third only needs to be counted
static bool32 RecordMoveSeen(u32 move)
{
    u32 first = VarGet(VAR_ACT2_MOVE_SEEN_1), second = VarGet(VAR_ACT2_MOVE_SEEN_2);
    if (move == first || move == second || CountMovesRecorded() >= RESEARCH_MOVES_TO_RECORD)
        return FALSE;
    if (first == MOVE_NONE)
        VarSet(VAR_ACT2_MOVE_SEEN_1, move);
    else if (second == MOVE_NONE)
        VarSet(VAR_ACT2_MOVE_SEEN_2, move);
    else
        VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | ACT2_RESEARCH_MOVES_DONE);
    return TRUE;
}

// A research counter to show once the move in progress is over (MOVEEND_ACT2_RESEARCH)
static EWRAM_DATA bool8 sResearchMessagePending = FALSE;

static const u8 sText_ResearchCount[] = _("Research: {STR_VAR_2} of 3 {STR_VAR_3}\n");
static const u8 sText_ResearchMoves[] = _("moves recorded!");
static const u8 sText_ResearchPokemon[] = _("POKéMON caught!");

// The counter's message in gStringVar1, for STRINGID_ACT2RESEARCH
static void BufferResearchMessage(const struct Act2Course *course, u32 count, const u8 *what)
{
    ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    StringCopy(gStringVar3, course->typeName);
    StringExpandPlaceholders(gStringVar1, sText_ResearchCount);
    StringAppend(gStringVar1, what);
}

// From the battle, each time a Pokémon uses a move (Cmd_printattackstring)
void Act2_RecordMoveUse(u32 battler, u32 move)
{
    const struct Act2Course *course = GetCurrentCourse();
    if (course == NULL || GetMoveType(move) != course->type)
        return;
    if (RecordMoveSeen(move))
        sResearchMessagePending = TRUE;
    if (!IsOnPlayerSide(battler))
        return;
    VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | ACT2_RESEARCH_USED_MOVE);
    if (move == course->tmMove && (gBattleTypeFlags & BATTLE_TYPE_TRAINER) && TRAINER_BATTLE_PARAM.opponentA == course->professor)
        VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | ACT2_RESEARCH_USED_TM_MOVE);
}

// On a field trip, a wild Pokémon of the course's type knows its type's moves from up to this many levels ahead, so
// there are a few different ones to record without training up first
#define ACT2_TYPE_MOVE_LOOKAHEAD 8

static bool32 MonKnowsMove(struct Pokemon *mon, u32 move)
{
    u32 i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            return TRUE;
    }
    return FALSE;
}

// From CreateWildMon: a course-type Pokémon's other moves make way for its course-type ones (learned or coming up)
void Act2_TeachWildCourseMoves(struct Pokemon *mon)
{
    const struct Act2Course *course = GetCurrentCourse();
    const struct LevelUpMove *learnset;
    u32 species = GetMonData(mon, MON_DATA_SPECIES), level = GetMonData(mon, MON_DATA_LEVEL), i, slot;
    if (course == NULL || (GetSpeciesType(species, 0) != course->type && GetSpeciesType(species, 1) != course->type))
        return;
    learnset = GetSpeciesLevelUpLearnset(species);
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END && learnset[i].level <= level + ACT2_TYPE_MOVE_LOOKAHEAD; i++)
    {
        u32 move = learnset[i].move;
        if (GetMoveType(move) != course->type || MonKnowsMove(mon, move))
            continue;
        for (slot = 0; slot < MAX_MON_MOVES; slot++)
        {
            u32 known = GetMonData(mon, MON_DATA_MOVE1 + slot);
            if (known == MOVE_NONE || GetMoveType(known) != course->type)
                break;
        }
        if (slot == MAX_MON_MOVES)
            return;
        SetMonMoveSlot(mon, move, slot);
    }
}

// From the battle, at the end of a move: TRUE when a new move was recorded (its counter is in gStringVar1)
bool32 Act2_TakeResearchMoveMessage(void)
{
    const struct Act2Course *course = GetCurrentCourse();
    if (!sResearchMessagePending)
        return FALSE;
    sResearchMessagePending = FALSE;
    if (course == NULL)
        return FALSE;
    BufferResearchMessage(course, CountMovesRecorded(), sText_ResearchMoves);
    return TRUE;
}

#define RESEARCH_SPECIES_TO_CATCH 3

static bool32 CountsAsCaught(u32 species, bool32 isEgg, enum Type type, u16 *seen, u32 *count)
{
    u32 i;
    // The Lapras from Vermilion's docks was a gift
    if (species == SPECIES_NONE || species == SPECIES_LAPRAS || isEgg)
        return FALSE;
    if (GetSpeciesType(species, 0) != type && GetSpeciesType(species, 1) != type)
        return FALSE;
    for (i = 0; i < *count; i++)
    {
        if (seen[i] == species)
            return FALSE;
    }
    seen[(*count)++] = species;
    return TRUE;
}

static u32 CountCaughtSpecies(enum Type type, u32 limit)
{
    u16 seen[RESEARCH_SPECIES_TO_CATCH + 1];
    u32 count = 0, i, box, pos;
    for (i = 0; i < PARTY_SIZE && count < limit; i++)
        CountsAsCaught(GetMonData(&gPlayerParty[i], MON_DATA_SPECIES), GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG), type, seen, &count);
    for (box = 0; box < TOTAL_BOXES_COUNT && count < limit; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT && count < limit; pos++)
            CountsAsCaught(GetBoxMonDataAt(box, pos, MON_DATA_SPECIES), GetBoxMonDataAt(box, pos, MON_DATA_IS_EGG), type, seen, &count);
    }
    return count;
}

// The first task: how many different species of the type the player has, in the party and the PC (up to the 3 needed)
u32 Act2_CountCaughtSpeciesOfType(enum Type type)
{
    return CountCaughtSpecies(type, RESEARCH_SPECIES_TO_CATCH);
}

static u32 CountSpeciesOwned(u32 species)
{
    u32 count = 0, i, box, pos;
    for (i = 0; i < PARTY_SIZE; i++)
        count += GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species;
    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
            count += GetBoxMonDataAt(box, pos, MON_DATA_SPECIES) == species;
    }
    return count;
}

// From the battle, once a caught Pokémon was given to the player: TRUE when it's a new species toward the course's
// first task (its counter is in gStringVar1)
bool32 Act2_BufferResearchCatchMessage(u32 species)
{
    const struct Act2Course *course = GetCurrentCourse();
    u32 count;
    if (course == NULL || species == SPECIES_LAPRAS
     || (GetSpeciesType(species, 0) != course->type && GetSpeciesType(species, 1) != course->type)
     || CountSpeciesOwned(species) != 1)
        return FALSE;
    // One more than needed tells whether the task was already done before this one
    count = CountCaughtSpecies(course->type, RESEARCH_SPECIES_TO_CATCH + 1);
    if (count > RESEARCH_SPECIES_TO_CATCH)
        return FALSE;
    BufferResearchMessage(course, count, sText_ResearchPokemon);
    return TRUE;
}

static u32 GetResearchDone(void)
{
    const struct Act2Course *course = GetCurrentCourse();
    u32 research = VarGet(VAR_ACT2_RESEARCH), done = 0;
    if (course == NULL)
        return 0;
    if (Act2_CountCaughtSpeciesOfType(course->type) >= RESEARCH_SPECIES_TO_CATCH)
        done |= 1 << 0;
    if (research & ACT2_RESEARCH_USED_MOVE)
        done |= 1 << 1;
    if (CountMovesRecorded() >= RESEARCH_MOVES_TO_RECORD)
        done |= 1 << 2;
    if (research & ACT2_RESEARCH_PUZZLE_DONE)
        done |= 1 << 3;
    return done;
}

// specialvar VAR_RESULT, Act2_IsResearchDone: all four tasks
bool16 Act2_IsResearchDone(void)
{
    return GetResearchDone() == 0xF;
}

static const u8 sText_SheetTitle[] = _("RESEARCH SHEET: the ");
static const u8 sText_Course[] = _(" course");
static const u8 sText_Done[] = _("Done!");
static const u8 sText_NotYet[] = _("Not yet.");
static const u8 sText_Catch[] = _("Catch 3 different ");
static const u8 sText_PokemonSuffix[] = _(" POKéMON.");
static const u8 sText_Caught[] = _("Caught: ");
static const u8 sText_Use[] = _("Use a ");
static const u8 sText_MoveSuffix[] = _(" move in battle.");
static const u8 sText_Record[] = _("Record 3 different ");
static const u8 sText_Moves[] = _(" moves.");
static const u8 sText_Recorded[] = _("Recorded: ");
static const u8 sText_OfThree[] = _(" of 3.");
static const u8 sText_NewLine[] = _("\n");
static const u8 sText_NextPage[] = _("\p");

static void AppendTask(u8 *dest, const u8 *a, const u8 *b, const u8 *c)
{
    StringAppend(dest, sText_NextPage);
    StringAppend(dest, a);
    if (b != NULL)
        StringAppend(dest, b);
    if (c != NULL)
        StringAppend(dest, c);
    StringAppend(dest, sText_NewLine);
}

// special Act2_BufferResearchSheet: the sheet's tasks and how far along they are, in gStringVar4
void Act2_BufferResearchSheet(void)
{
    const struct Act2Course *course = GetCurrentCourse();
    u32 done = GetResearchDone();
    u8 *dest = gStringVar4;
    u8 count[4];
    if (course == NULL)
    {
        dest[0] = EOS;
        return;
    }
    StringCopy(dest, sText_SheetTitle);
    StringAppend(dest, course->typeName);
    StringAppend(dest, sText_Course);
    AppendTask(dest, sText_Catch, course->typeName, sText_PokemonSuffix);
    if (done & (1 << 0))
    {
        StringAppend(dest, sText_Done);
    }
    else
    {
        StringAppend(dest, sText_Caught);
        ConvertIntToDecimalStringN(count, Act2_CountCaughtSpeciesOfType(course->type), STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(dest, count);
        StringAppend(dest, sText_OfThree);
    }
    AppendTask(dest, sText_Use, course->typeName, sText_MoveSuffix);
    StringAppend(dest, (done & (1 << 1)) ? sText_Done : sText_NotYet);
    AppendTask(dest, sText_Record, course->typeName, sText_Moves);
    if (done & (1 << 2))
    {
        StringAppend(dest, sText_Done);
    }
    else
    {
        StringAppend(dest, sText_Recorded);
        ConvertIntToDecimalStringN(count, CountMovesRecorded(), STR_CONV_MODE_LEFT_ALIGN, 1);
        StringAppend(dest, count);
        StringAppend(dest, sText_OfThree);
    }
    AppendTask(dest, course->puzzleTask, NULL, NULL);
    StringAppend(dest, (done & (1 << 3)) ? sText_Done : sText_NotYet);
}

// specialvar VAR_RESULT, Act2_PartyCanUseCourseTM: a Pokémon of the course's type that knows its TM's move
bool16 Act2_PartyCanUseCourseTM(void)
{
    const struct Act2Course *course = GetCurrentCourse();
    u32 i, j;
    if (course == NULL)
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || !MonHasType(mon, course->type))
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            if (GetMonData(mon, MON_DATA_MOVE1 + j) == course->tmMove)
                return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------------------------
// The club's battles (3v3): each friend brings their partner and what they caught on the field trips so far, picking
// species the player doesn't have in their party right now. Every foe is at the average level of the player's three.

struct ClubSlot
{
    u16 species[5]; // In order of preference; SPECIES_NONE ends the list
};

static const struct ClubSlot sKaiTeam[] =
{
    { { SPECIES_MANKEY } },
    { { SPECIES_GROWLITHE, SPECIES_PONYTA, SPECIES_VULPIX, SPECIES_SLUGMA } },
    { { SPECIES_PONYTA, SPECIES_VULPIX, SPECIES_SLUGMA, SPECIES_GROWLITHE } },
};

static const struct ClubSlot sMinaTeam[] =
{
    { { SPECIES_CLEFAIRY } },
    { { SPECIES_VULPIX, SPECIES_PONYTA, SPECIES_GROWLITHE, SPECIES_SLUGMA } },
    { { SPECIES_STARYU, SPECIES_PSYDUCK, SPECIES_POLIWAG, SPECIES_HORSEA, SPECIES_KRABBY } },
};

static const struct ClubSlot sExaminerTeam[] =
{
    { { SPECIES_PIDGEOTTO } },
    { { SPECIES_RATICATE } },
    { { SPECIES_KADABRA } },
};

#define CLUB_IVS     14
#define EXAMINER_IVS 18

static bool32 PlayerPartyHasSpecies(u32 species)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 TeamHasSpecies(struct Pokemon *party, u32 count, u32 species)
{
    u32 i;
    for (i = 0; i < count; i++)
    {
        if (GetMonData(&party[i], MON_DATA_SPECIES) == species)
            return TRUE;
    }
    return FALSE;
}

// The average level of the player's Pokémon (in a club battle, just the three battling)
static u32 GetPlayerPartyAverageLevel(void)
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

// After the trainer's party is made (DoTrainerBattle)
void Act2_MakeClubParty(struct Pokemon *party, u32 trainerId)
{
    const struct ClubSlot *team;
    u32 count, i, j, level, ivs = CLUB_IVS;
    if (trainerId == TRAINER_ACT2_KAI)
        team = sKaiTeam, count = ARRAY_COUNT(sKaiTeam);
    else if (trainerId == TRAINER_ACT2_MINA)
        team = sMinaTeam, count = ARRAY_COUNT(sMinaTeam);
    else if (trainerId == TRAINER_ACT2_EXAMINER)
        team = sExaminerTeam, count = ARRAY_COUNT(sExaminerTeam), ivs = EXAMINER_IVS;
    else
        return;
    level = GetPlayerPartyAverageLevel();
    for (i = 0; i < count; i++)
    {
        u32 species = team[i].species[0];
        for (j = 0; j < ARRAY_COUNT(team[i].species) && team[i].species[j] != SPECIES_NONE; j++)
        {
            species = team[i].species[j];
            if (!PlayerPartyHasSpecies(species) && !TeamHasSpecies(party, i, species))
                break;
        }
        CreateMonWithIVs(&party[i], species, level, Random32(), OTID_STRUCT_RANDOM_NO_SHINY, ivs);
        GiveMonInitialMoveset(&party[i]);
    }
}

// The club battles with the first three Pokémon in the party able to battle: first in front, second in the middle,
// third in back (the starting formation follows party order). The rest sit out: set aside for the battle
// (Act2_SetAsideClubBench) and put back in their slots after it (Act2_RestoreClubBench).
#define CLUB_TEAM_SIZE 3

static EWRAM_DATA struct Pokemon sClubFullParty[PARTY_SIZE] = {0};
static EWRAM_DATA u8 sClubTeamSlots[CLUB_TEAM_SIZE] = {0};
static EWRAM_DATA u8 sClubTeamCount = 0;

static u32 GetClubTeamSlots(u8 *slots)
{
    u32 i, count = 0;
    for (i = 0; i < PARTY_SIZE && count < CLUB_TEAM_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) != 0)
            slots[count++] = i;
    }
    return count;
}

static const u8 sText_ClubTeam3[] = _("So it's {STR_VAR_1} in front,\n{STR_VAR_2} in the middle\land {STR_VAR_3} in back.");
static const u8 sText_ClubTeam2[] = _("So it's {STR_VAR_1} in front\nand {STR_VAR_2} in back.");
static const u8 sText_ClubTeam1[] = _("So it's just {STR_VAR_1}.");

static void BufferNickname(u8 *dest, u32 slot)
{
    GetMonData(&gPlayerParty[slot], MON_DATA_NICKNAME, dest);
    StringGet_Nickname(dest);
}

// specialvar VAR_RESULT, Act2_BufferClubTeam: who battles and where, in gStringVar4; returns how many
u16 Act2_BufferClubTeam(void)
{
    static const u8 *const sTexts[] = { sText_ClubTeam1, sText_ClubTeam1, sText_ClubTeam2, sText_ClubTeam3 };
    u8 slots[CLUB_TEAM_SIZE];
    u32 count = GetClubTeamSlots(slots);
    u8 *const vars[] = { gStringVar1, gStringVar2, gStringVar3 };
    u32 i;
    for (i = 0; i < count; i++)
        BufferNickname(vars[i], slots[i]);
    StringExpandPlaceholders(gStringVar4, sTexts[count]);
    return count;
}

// special Act2_SetAsideClubBench: right before the club battle, the party is just the three battling, in order
void Act2_SetAsideClubBench(void)
{
    u32 i;
    sClubTeamCount = GetClubTeamSlots(sClubTeamSlots);
    for (i = 0; i < PARTY_SIZE; i++)
        sClubFullParty[i] = gPlayerParty[i];
    ZeroPlayerPartyMons();
    for (i = 0; i < sClubTeamCount; i++)
        gPlayerParty[i] = sClubFullParty[sClubTeamSlots[i]];
    CalculatePlayerPartyCount();
}

// special Act2_RestoreClubBench: right after it, the three go back to their slots and the rest return
void Act2_RestoreClubBench(void)
{
    u32 i;
    if (sClubTeamCount == 0)
        return;
    for (i = 0; i < sClubTeamCount; i++)
        sClubFullParty[sClubTeamSlots[i]] = gPlayerParty[i];
    for (i = 0; i < PARTY_SIZE; i++)
        gPlayerParty[i] = sClubFullParty[i];
    sClubTeamCount = 0;
    CalculatePlayerPartyCount();
}

// ---------------------------------------------------------------------------------------------------------------
// The site puzzles: three parts, each done by a Pokémon of the course's type using a move of that type (the
// Mansion's furnaces, Vermilion's dock fires, Route 22's withered patches). Their end is the last research task.

// specialvar VAR_RESULT, Act2_FindCourseMoveUser: a Pokémon of the course's type that knows a move of that type.
// Its nickname goes in STR_VAR_1 and the move in STR_VAR_2.
bool16 Act2_FindCourseMoveUser(void)
{
    const struct Act2Course *course = GetCurrentCourse();
    u32 i, j;
    if (course == NULL)
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || !MonHasType(mon, course->type))
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            u32 move = GetMonData(mon, MON_DATA_MOVE1 + j);
            if (move != MOVE_NONE && GetMoveType(move) == course->type)
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

// specialvar VAR_RESULT, Act2_SolvePuzzlePart with the part (1-3) in VAR_0x8004: TRUE if that was the last one
bool16 Act2_SolvePuzzlePart(void)
{
    u32 research = VarGet(VAR_ACT2_RESEARCH) | (ACT2_RESEARCH_PUZZLE_1 << (gSpecialVar_0x8004 - 1));
    VarSet(VAR_ACT2_RESEARCH, research);
    return (research & ACT2_RESEARCH_PUZZLE_ALL) == ACT2_RESEARCH_PUZZLE_ALL;
}

// specialvar VAR_RESULT, Act2_IsPuzzlePartSolved with the part (1-3, or 0 for all three) in VAR_0x8004
bool16 Act2_IsPuzzlePartSolved(void)
{
    u32 research = VarGet(VAR_ACT2_RESEARCH);
    if (gSpecialVar_0x8004 == 0)
        return (research & ACT2_RESEARCH_PUZZLE_ALL) == ACT2_RESEARCH_PUZZLE_ALL;
    return (research & (ACT2_RESEARCH_PUZZLE_1 << (gSpecialVar_0x8004 - 1))) != 0;
}

// special Act2_SetupSiteObjects with the course's trip stage in VAR_0x8004, on the site's map transition. Off the
// trip, everything is hidden. On it: FLAG_TEMP_1 (the professor and friends) shows; FLAG_TEMP_2-4 hide each
// puzzle part once it's solved; FLAG_TEMP_5-7 show what replaces it; FLAG_TEMP_8 shows the puzzle's reward once all
// three are solved.
void Act2_SetupSiteObjects(void)
{
    u32 research = VarGet(VAR_ACT2_RESEARCH), i;
    bool32 onTrip = VarGet(VAR_STORY_ACT) == STORY_ACT_UNIVERSITY && VarGet(VAR_ACT2_PROGRESS) == gSpecialVar_0x8004;
    for (i = 0; i < 8; i++)
        FlagSet(FLAG_TEMP_1 + i);
    if (!onTrip)
        return;
    FlagClear(FLAG_TEMP_1);
    for (i = 0; i < 3; i++)
    {
        if (research & (ACT2_RESEARCH_PUZZLE_1 << i))
            FlagClear(FLAG_TEMP_5 + i);
        else
            FlagClear(FLAG_TEMP_2 + i);
    }
    if ((research & ACT2_RESEARCH_PUZZLE_ALL) == ACT2_RESEARCH_PUZZLE_ALL && !(research & ACT2_RESEARCH_PUZZLE_DONE))
        FlagClear(FLAG_TEMP_8);
}

// special Act2_SetResearchFlag / specialvar VAR_RESULT, Act2_HasResearchFlag / special Act2_ClearResearchFlag, with
// an ACT2_RESEARCH_* bit in VAR_0x8004
void Act2_SetResearchFlag(void)
{
    VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) | gSpecialVar_0x8004);
}

bool16 Act2_HasResearchFlag(void)
{
    return (VarGet(VAR_ACT2_RESEARCH) & gSpecialVar_0x8004) != 0;
}

void Act2_ClearResearchFlag(void)
{
    VarSet(VAR_ACT2_RESEARCH, VarGet(VAR_ACT2_RESEARCH) & ~gSpecialVar_0x8004);
}

// ---------------------------------------------------------------------------------------------------------------
// The Act 2 playtest build (make act2, pokemon-unnamed-act2.gba): a new game starts at Act 2, in the dorm room, as
// if Act 1 had just ended, with two of the Woods' Pokémon at ACT2_START_LEVEL
// The playtest party: two of the Woods' Pokémon, ready to go
void Act2_GivePlaytestParty(void)
{
    static const u16 sFriends[] = { SPECIES_TEDDIURSA, SPECIES_TYNAMO };
    u32 i;
    ZeroPlayerPartyMons();
    for (i = 0; i < ARRAY_COUNT(sFriends); i++)
    {
        // CreateMon leaves the stats (and HP) at 0: CalculateMonStats fills them in
        CreateMon(&gPlayerParty[i], sFriends[i], ACT2_START_LEVEL, Random32(), OTID_STRUCT_PLAYER_ID);
        SetMonData(&gPlayerParty[i], MON_DATA_POKEBALL, &(u32){ BALL_POKE });
        CalculateMonStats(&gPlayerParty[i]);
        Act2_GiveStartingMoves(&gPlayerParty[i]);
    }
    CalculatePlayerPartyCount();
}

#ifdef ACT2_PLAYTEST
void Act2_StartPlaytest(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT1_PROGRESS, ACT1_CHOOSING_FRIENDS);
    VarSet(VAR_ACT2_PROGRESS, ACT2_START);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    EnableNationalPokedex();
    Act2_GivePlaytestParty();
    AddBagItem(ITEM_POTION, 5);
    Act2_GiveStartingKit();
    SetLastHealLocationWarp(HEAL_LOCATION_CELADON_CITY);
    SetWarpDestination(MAP_GROUP(ACT2_START_MAP), MAP_NUM(ACT2_START_MAP), WARP_ID_NONE, ACT2_START_X, ACT2_START_Y);
    WarpIntoMap();
}
#endif
