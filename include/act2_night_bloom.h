#ifndef GUARD_ACT2_NIGHT_BLOOM_H
#define GUARD_ACT2_NIGHT_BLOOM_H

// The Grass course's night bloom (src/act2_night_bloom.c, docs/hack/act2_story.md)
u32 Act2_GetNightFlashLevel(u32 glowcapsLit);
u32 Act2_GetNightForestFlashLevel(void);
void Act2_SetupForestObjects(void);
void Act2_DrawForestOpening(void);
bool16 Act2_WakeGlowcap(void);
bool16 Act2_IsGlowcapLit(void);
void Act2_BrightenForest(void);
bool16 Act2_FollowOddish(void);
bool16 Act2_FindHoldStillMoveUser(void);
bool16 Act2_FindHedgeMoveUser(void);
void Act2_SetNightBloomBit(void);
bool16 Act2_HasNightBloomBit(void);
void Act2_SetupGladeObjects(void);
void Act2_DrawGladeHedges(void);
u16 Act2_GetGladeScene(void);
u16 Act2_GetNextWave(void);
bool16 Act2_EndWave(void);
bool16 Act2_PlantHedge(void);
u32 Act2_GetBloomWaveSize(u32 wave);
u32 Act2_CreateBloomWaveParty(void);
void Act2_BreakDawn(void);
u16 Act2_GiveGladeMorelull(void);
bool32 Act2_IsGladeMorelull(u32 species, bool32 fateful);

#endif // GUARD_ACT2_NIGHT_BLOOM_H
