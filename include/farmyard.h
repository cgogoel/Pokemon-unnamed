#ifndef GUARD_FARMYARD_H
#define GUARD_FARMYARD_H

// Act 1 farmyard: befriended Pokémon live here, shown wandering on the Farmyard map. See docs/hack/befriending.md
#include "constants/farmyard.h"

bool32 IsBefriendAllowed(void);
u32 DepositFollowerInFarm(void);
void SetupFarmyardMons(void);
u32 CountFarmyardMons(void);

#endif // GUARD_FARMYARD_H
