#ifndef GUARD_ACT2_STORY_H
#define GUARD_ACT2_STORY_H

// Act 2's story (docs/hack/act2_story.md)
bool32 Act2_TrainersLocked(void);
bool32 Act2_IsStoryTrainer(u32 trainerId);
bool32 Act2_UsesAct2WildPokemon(void);
bool16 Act2_PartyHasType(void);
bool16 Act2_PartyHasProjectPokemon(void);
void Act2_SetNight(void);
void Act2_RaisePartyLevels(void);

#endif // GUARD_ACT2_STORY_H
