#ifndef GUARD_CONSTANTS_FARMYARD_H
#define GUARD_CONSTANTS_FARMYARD_H

// The last PC box holds the farmyard's Pokémon (see src/farmyard.c)
#define FARM_BOX            (TOTAL_BOXES_COUNT - 1)
// How many of them are shown wandering on the Farmyard map. Its first FARM_SHOWN_COUNT object events are these, with
// graphics VAR_OBJ_GFX_ID_0.. and hidden by FLAG_TEMP_1.. while their spot is empty.
#define FARM_SHOWN_COUNT    8
// The spot in the farmyard pond (LOCALID_FARMYARD_MON_3): only Tynamo and its evolutions go there, and they go nowhere
// else. Everyone else fills the other spots in farm box order.
#define FARM_WATER_SPOT     2
#define FARM_SPOT_NONE      0xFF
// DepositFollowerInFarm: nothing was deposited
#define FARM_DEPOSIT_NONE   0xFF
// befriendbattle: VAR_RESULT when the encounter didn't start because a Pokémon is still following (Act 1)
#define BEFRIEND_RESULT_BLOCKED 0xFE

// New games start here on the Farmyard map, in front of the farmhouse door
#define FARMYARD_START_X    6
#define FARMYARD_START_Y    8

#endif // GUARD_CONSTANTS_FARMYARD_H
