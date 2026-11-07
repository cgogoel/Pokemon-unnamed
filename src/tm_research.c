#include "global.h"
#include "tm_research.h"
#include "battle_main.h"
#include "data.h"
#include "event_data.h"
#include "item.h"
#include "move.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/moves.h"

// The TM Database (docs/hack/tm_database.md). Every TM but Tri-Blast can be researched. A TM's type is its move's,
// and its tier is its move's power: the stronger, the more different Pokémon of the type to catch and moves of the
// type to record, from when the research started. Catching every Pokémon of a type that can be found in the game
// (src/data/tm_research_species.h) makes all its TMs craftable at once.

#include "data/tm_research_species.h"

#define TM_TRI_BLAST (ENUM_TM_HM_TRI_BLAST + 1)

bool32 TmResearch_IsUnlocked(void)
{
    return FlagGet(FLAG_TM_DATABASE_UNLOCKED);
}

// special TmResearch_Unlock: the institute's researcher explained it
void TmResearch_Unlock(void)
{
    FlagSet(FLAG_TM_DATABASE_UNLOCKED);
}

bool32 TmResearch_IsResearchable(u32 tm)
{
    return tm >= 1 && tm <= NUM_TECHNICAL_MACHINES && tm != TM_TRI_BLAST;
}

u32 TmResearch_GetMove(u32 tm)
{
    return tm >= 1 && tm <= NUM_TECHNICAL_MACHINES ? GetTMHMMoveId(tm) : MOVE_NONE;
}

u32 TmResearch_GetItem(u32 tm)
{
    return tm >= 1 && tm <= NUM_TECHNICAL_MACHINES ? GetTMHMItemId(tm) : ITEM_NONE;
}

enum Type TmResearch_GetType(u32 tm)
{
    return GetMoveType(TmResearch_GetMove(tm));
}

// 1: a status move or up to 60 power; 2: 61-90, and damaging moves whose power varies (Return, Low Kick: their
// power is 0 or 1 in the data); 3: 91-110; 4: 111 and up
u32 TmResearch_GetTier(u32 tm)
{
    u32 move = TmResearch_GetMove(tm), power = GetMovePower(move);
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return 1;
    if (power <= 1)
        return 2;
    if (power <= 60)
        return 1;
    if (power <= 90)
        return 2;
    if (power <= 110)
        return 3;
    return 4;
}

// Different Pokémon to catch, and different moves to record: 2 to 5
u32 TmResearch_GetRequirement(u32 tm)
{
    return TmResearch_GetTier(tm) + 1;
}

bool32 TmResearch_IsOwned(u32 tm)
{
    u32 item = TmResearch_GetItem(tm);
    return item != ITEM_NONE && CheckBagHasItem(item, 1);
}

u32 TmResearch_CountOwned(void)
{
    u32 tm, count = 0;
    for (tm = 1; tm <= NUM_TECHNICAL_MACHINES; tm++)
        count += TmResearch_IsResearchable(tm) && TmResearch_IsOwned(tm);
    return count;
}

u32 TmResearch_CountResearchable(void)
{
    return NUM_TECHNICAL_MACHINES - 1;
}

// The type's name in capitals ("FIRE")
u8 *TmResearch_GetTypeName(u8 *dest, enum Type type)
{
    const u8 *src = gTypesInfo[type].name;
    while (*src != EOS)
    {
        u8 c = *src++;
        *dest++ = (c >= CHAR_a && c <= CHAR_z) ? c - CHAR_a + CHAR_A : c;
    }
    *dest = EOS;
    return dest;
}

// ---------------------------------------------------------------------------------------------------------------
// Every Pokémon of a type caught

static const struct TmResearchSpeciesList *GetTypeList(enum Type type)
{
    if (type >= NUMBER_OF_MON_TYPES || sTmResearchSpecies[type].count == 0)
        return NULL;
    return &sTmResearchSpecies[type];
}

// Catching this many different Pokémon of a type (from the generated list of the type's Pokémon found in the game)
// unlocks all its TMs; a type with fewer in the game needs all of them
#define TM_RESEARCH_TYPE_CATCH_GOAL 25

// How many of the type's Pokémon (the generated list) are caught in the Pokédex, up to the goal; the goal in total
u32 TmResearch_CountTypeCaught(enum Type type, u32 *total)
{
    const struct TmResearchSpeciesList *list = GetTypeList(type);
    u32 i, count = 0, goal;
    if (list == NULL)
    {
        *total = 0;
        return 0;
    }
    goal = min(list->count, TM_RESEARCH_TYPE_CATCH_GOAL);
    for (i = 0; i < list->count && count < goal; i++)
        count += GetSetPokedexFlag(SpeciesToNationalPokedexNum(list->species[i]), FLAG_GET_CAUGHT) != 0;
    *total = goal;
    return count;
}

bool32 TmResearch_IsTypeComplete(enum Type type)
{
    u32 total, caught = TmResearch_CountTypeCaught(type, &total);
    return total != 0 && caught >= total;
}

// ---------------------------------------------------------------------------------------------------------------
// The slots

static struct TmResearchSlot *GetSlot(u32 slot)
{
    return &gSaveBlock3Ptr->tmResearch[slot];
}

u32 TmResearch_GetSlotTm(u32 slot)
{
    return slot < TM_RESEARCH_SLOTS && TmResearch_IsResearchable(GetSlot(slot)->tm) ? GetSlot(slot)->tm : 0;
}

s32 TmResearch_FindFreeSlot(void)
{
    u32 i;
    for (i = 0; i < TM_RESEARCH_SLOTS; i++)
    {
        if (TmResearch_GetSlotTm(i) == 0)
            return i;
    }
    return -1;
}

s32 TmResearch_FindSlotOfType(enum Type type)
{
    u32 i;
    for (i = 0; i < TM_RESEARCH_SLOTS; i++)
    {
        u32 tm = TmResearch_GetSlotTm(i);
        if (tm != 0 && TmResearch_GetType(tm) == type)
            return i;
    }
    return -1;
}

static s32 FindSlotOfTm(u32 tm)
{
    u32 i;
    for (i = 0; i < TM_RESEARCH_SLOTS; i++)
    {
        if (TmResearch_GetSlotTm(i) == tm)
            return i;
    }
    return -1;
}

enum TmResearchStatus TmResearch_GetStatus(u32 tm)
{
    if (!TmResearch_IsResearchable(tm))
        return TM_STATUS_UNAVAILABLE;
    if (TmResearch_IsOwned(tm))
        return TM_STATUS_OWNED;
    if (FindSlotOfTm(tm) >= 0)
        return TM_STATUS_TRACKED;
    if (TmResearch_IsTypeComplete(TmResearch_GetType(tm)))
        return TM_STATUS_READY;
    if (TmResearch_FindSlotOfType(TmResearch_GetType(tm)) >= 0)
        return TM_STATUS_TYPE_TAKEN;
    if (TmResearch_FindFreeSlot() < 0)
        return TM_STATUS_NO_SLOT;
    return TM_STATUS_AVAILABLE;
}

// Starts researching a TM in an empty slot, counting from now: FALSE if it can't be (see TmResearch_GetStatus)
bool32 TmResearch_Start(u32 slot, u32 tm)
{
    if (slot >= TM_RESEARCH_SLOTS || TmResearch_GetSlotTm(slot) != 0 || TmResearch_GetStatus(tm) != TM_STATUS_AVAILABLE)
        return FALSE;
    memset(GetSlot(slot), 0, sizeof(struct TmResearchSlot));
    GetSlot(slot)->tm = tm;
    return TRUE;
}

// Stops the research: what it counted is lost
void TmResearch_ClearSlot(u32 slot)
{
    if (slot < TM_RESEARCH_SLOTS)
        memset(GetSlot(slot), 0, sizeof(struct TmResearchSlot));
}

static u32 CountList(const u16 *list)
{
    u32 i, count = 0;
    for (i = 0; i < TM_RESEARCH_MAX_COUNT; i++)
        count += list[i] != 0;
    return count;
}

u32 TmResearch_GetCaughtCount(u32 slot)
{
    return TmResearch_GetSlotTm(slot) == 0 ? 0 : CountList(GetSlot(slot)->species);
}

u32 TmResearch_GetMoveCount(u32 slot)
{
    return TmResearch_GetSlotTm(slot) == 0 ? 0 : CountList(GetSlot(slot)->moves);
}

bool32 TmResearch_IsSlotReady(u32 slot)
{
    u32 tm = TmResearch_GetSlotTm(slot), need;
    if (tm == 0)
        return FALSE;
    if (TmResearch_IsTypeComplete(TmResearch_GetType(tm)))
        return TRUE;
    need = TmResearch_GetRequirement(tm);
    return TmResearch_GetCaughtCount(slot) >= need && TmResearch_GetMoveCount(slot) >= need;
}

// Gives the slot's TM and frees the slot
bool32 TmResearch_CraftSlot(u32 slot)
{
    u32 tm = TmResearch_GetSlotTm(slot);
    if (!TmResearch_IsSlotReady(slot) || !AddBagItem(TmResearch_GetItem(tm), 1))
        return FALSE;
    TmResearch_ClearSlot(slot);
    return TRUE;
}

// A TM of a type whose Pokémon are all caught, without research
bool32 TmResearch_CraftReadyTm(u32 tm)
{
    if (TmResearch_GetStatus(tm) != TM_STATUS_READY)
        return FALSE;
    return AddBagItem(TmResearch_GetItem(tm), 1);
}

// ---------------------------------------------------------------------------------------------------------------
// Counting in battle, with a message when a count goes up (like the courses' research, act2_story.c)

static const u8 sText_ResearchCount[] = _("TM research: {STR_VAR_2} of {STR_VAR_3} ");
static const u8 sText_CaughtSuffix[] = _("\nPOKéMON caught!");
static const u8 sText_MovesSuffix[] = _("\nmoves recorded!");
static const u8 sText_NextMessage[] = _("\p");
static const u8 sText_Ready[] = _("\p{STR_VAR_2} is ready\nto craft in QUESTS!");

// Adds a value to a slot's list if it isn't there and the list isn't full yet: TRUE if it was added
static bool32 AddToList(u16 *list, u32 count, u32 value)
{
    u32 i;
    for (i = 0; i < count; i++)
    {
        if (list[i] == value)
            return FALSE;
        if (list[i] == 0)
        {
            list[i] = value;
            return TRUE;
        }
    }
    return FALSE;
}

// Appends "TM research: 2 of 4 FIRE\nPOKéMON caught!" (and the ready line) to gStringVar1
static void AppendCountMessage(u32 slot, u32 count, const u8 *suffix)
{
    u32 tm = TmResearch_GetSlotTm(slot);
    u8 *end = gStringVar1 + StringLength(gStringVar1);
    if (end != gStringVar1)
        end = StringCopy(end, sText_NextMessage);
    ConvertIntToDecimalStringN(gStringVar2, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(gStringVar3, TmResearch_GetRequirement(tm), STR_CONV_MODE_LEFT_ALIGN, 1);
    end = StringExpandPlaceholders(end, sText_ResearchCount);
    end = TmResearch_GetTypeName(end, TmResearch_GetType(tm));
    end = StringCopy(end, suffix);
    if (TmResearch_IsSlotReady(slot))
    {
        StringCopy(gStringVar2, GetMoveName(TmResearch_GetMove(tm)));
        StringExpandPlaceholders(end, sText_Ready);
    }
}

// The slot whose count went up with the move in progress, to show once it's over (MOVEEND_ACT2_RESEARCH)
static EWRAM_DATA u8 sPendingMoveSlot = 0; // The slot + 1, 0 for none

// From the battle, each time a Pokémon on either side uses a move (Cmd_printattackstring), as the courses count them
void TmResearch_RecordMoveUse(u32 move)
{
    s32 slot;
    if (!TmResearch_IsUnlocked() || move == MOVE_NONE || move == MOVE_STRUGGLE || move >= MOVES_COUNT)
        return;
    slot = TmResearch_FindSlotOfType(GetMoveType(move));
    if (slot < 0)
        return;
    if (AddToList(GetSlot(slot)->moves, TmResearch_GetRequirement(TmResearch_GetSlotTm(slot)), move))
        sPendingMoveSlot = slot + 1;
}

// From the battle, at the end of a move: TRUE when a tracked TM's move count went up (its message in gStringVar1)
bool32 TmResearch_TakeMoveMessage(void)
{
    s32 slot = (s32)sPendingMoveSlot - 1;
    sPendingMoveSlot = 0;
    if (slot < 0 || TmResearch_GetSlotTm(slot) == 0)
        return FALSE;
    gStringVar1[0] = EOS;
    AppendCountMessage(slot, TmResearch_GetMoveCount(slot), sText_MovesSuffix);
    return TRUE;
}

// From the battle, once a Pokémon was caught: counts it for the tracked TMs of its types, TRUE with their messages in
// gStringVar1 when a count went up
bool32 TmResearch_BufferCatchMessage(u32 species)
{
    u32 slot;
    bool32 counted = FALSE;
    if (!TmResearch_IsUnlocked() || species == SPECIES_NONE)
        return FALSE;
    gStringVar1[0] = EOS;
    for (slot = 0; slot < TM_RESEARCH_SLOTS; slot++)
    {
        u32 tm = TmResearch_GetSlotTm(slot);
        enum Type type;
        if (tm == 0)
            continue;
        type = TmResearch_GetType(tm);
        if (GetSpeciesType(species, 0) != type && GetSpeciesType(species, 1) != type)
            continue;
        if (AddToList(GetSlot(slot)->species, TmResearch_GetRequirement(tm), species))
        {
            AppendCountMessage(slot, TmResearch_GetCaughtCount(slot), sText_CaughtSuffix);
            counted = TRUE;
        }
    }
    return counted;
}
