#ifndef GUARD_ACT2_STORY_H
#define GUARD_ACT2_STORY_H

// Act 2's story (docs/hack/act2_story.md)
bool32 Act2_TrainersLocked(void);
bool32 Act2_IsStoryTrainer(u32 trainerId);
bool32 Act2_UsesAct2WildPokemon(void);
u32 Act2_GetWildLevel(void);
bool16 Act2_PartyHasType(void);
bool16 Act2_PartyHasProjectPokemon(void);
bool16 Act2_IsClubNight(void);

// The hour it is on a club night (any night hour)
#define ACT2_CLUB_NIGHT_HOUR 2
void Act2_RaisePartyLevels(void);
void Act2_GiveStartingMoves(struct Pokemon *mon);
void Act2_GiveStartingKit(void);
bool32 Act2_HasTrainerCard(void);
void Act2_RecordMoveUse(u32 battler, u32 move);
bool32 Act2_TakeResearchMoveMessage(void);
bool32 Act2_BufferResearchCatchMessage(u32 species);
bool16 Act2_IsResearchDone(void);
u32 Act2_CountCaughtSpeciesOfType(enum Type type);
void Act2_BufferResearchSheet(void);
bool16 Act2_PartyCanUseCourseTM(void);
void Act2_MakeClubParty(struct Pokemon *party, u32 trainerId);
bool16 Act2_FindCourseMoveUser(void);
bool16 Act2_SolvePuzzlePart(void);
bool16 Act2_IsPuzzlePartSolved(void);
void Act2_SetupSiteObjects(void);
void Act2_SetResearchFlag(void);
bool16 Act2_HasResearchFlag(void);
void Act2_ClearResearchFlag(void);
void Act2_GivePlaytestParty(void);
void Act2_StartPlaytest(void);

#endif // GUARD_ACT2_STORY_H
