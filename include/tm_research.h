#ifndef GUARD_TM_RESEARCH_H
#define GUARD_TM_RESEARCH_H

// The TM Database (docs/hack/tm_database.md): from Act 2's final project on, the player researches TMs the way the
// courses made theirs. Up to 3 at a time (TM_RESEARCH_SLOTS), one per type: catch different Pokémon of the TM's type
// and record different moves of it in battle, more for stronger moves; then craft the TM from QUESTS.

enum TmResearchStatus
{
    TM_STATUS_AVAILABLE,     // Can be researched in a free slot
    TM_STATUS_NO_SLOT,       // Every slot is taken
    TM_STATUS_TYPE_TAKEN,    // A TM of its type is being researched
    TM_STATUS_TRACKED,       // Being researched
    TM_STATUS_READY,         // Every Pokémon of its type caught: craftable without research
    TM_STATUS_OWNED,
    TM_STATUS_UNAVAILABLE,   // Not in the database (Tri-Blast)
};

struct TmResearchSpeciesList
{
    const u16 *species;
    u16 count;
};

bool32 TmResearch_IsUnlocked(void);
void TmResearch_Unlock(void);
bool32 TmResearch_IsResearchable(u32 tm);
u32 TmResearch_GetMove(u32 tm);
u32 TmResearch_GetItem(u32 tm);
enum Type TmResearch_GetType(u32 tm);
u32 TmResearch_GetTier(u32 tm);
u32 TmResearch_GetRequirement(u32 tm);
bool32 TmResearch_IsOwned(u32 tm);
enum TmResearchStatus TmResearch_GetStatus(u32 tm);
u32 TmResearch_CountOwned(void);
u32 TmResearch_CountResearchable(void);
u8 *TmResearch_GetTypeName(u8 *dest, enum Type type);

u32 TmResearch_GetSlotTm(u32 slot);
s32 TmResearch_FindFreeSlot(void);
s32 TmResearch_FindSlotOfType(enum Type type);
bool32 TmResearch_Start(u32 slot, u32 tm);
void TmResearch_ClearSlot(u32 slot);
u32 TmResearch_GetCaughtCount(u32 slot);
u32 TmResearch_GetMoveCount(u32 slot);
bool32 TmResearch_IsSlotReady(u32 slot);
bool32 TmResearch_CraftSlot(u32 slot);
bool32 TmResearch_CraftReadyTm(u32 tm);

bool32 TmResearch_IsTypeComplete(enum Type type);
u32 TmResearch_CountTypeCaught(enum Type type, u32 *total);

void TmResearch_RecordMoveUse(u32 move);
bool32 TmResearch_TakeMoveMessage(void);
bool32 TmResearch_BufferCatchMessage(u32 species);

#endif // GUARD_TM_RESEARCH_H
