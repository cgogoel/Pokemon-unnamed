#ifndef GUARD_FISHING_H
#define GUARD_FISHING_H

void StartFishing(u8 rod);
u16 GetBestFishingRodItem(void);
bool32 CanFishFromWaterInFront(void);
void StartFishingWithBestRod(void);
void UpdateChainFishingStreak();
u32 CalculateChainFishingShinyRolls(void);
bool32 ShouldUseFishingEnvironmentInBattle();

#endif // GUARD_FISHING_H