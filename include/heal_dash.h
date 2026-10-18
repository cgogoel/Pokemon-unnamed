#ifndef GUARD_HEAL_DASH_H
#define GUARD_HEAL_DASH_H

// The Heal Dash key item, Acts 2 and 3 (src/heal_dash.c)
u32 HealDash_GetDestination(void);
bool32 HealDash_IsAtDestination(u32 healLocation);
void HealDash_Start(u32 healLocation);
bool32 HealDash_TakeWarp(struct WarpData *warp);

#endif // GUARD_HEAL_DASH_H
