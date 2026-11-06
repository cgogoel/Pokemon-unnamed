#ifndef GUARD_ACT2_STATION_H
#define GUARD_ACT2_STATION_H

// The abandoned ranger station, the Act 2 finale's dungeon (docs/hack/act2_finale.md)

// Its custom metatiles (tools/hack/draw_station_tiles.py)
#define METATILE_RangerStation_TorchUnlit   0x384 // gTileset_PokemonMansion
#define METATILE_RangerStation_TorchLit     0x385
#define METATILE_RangerStation_Bars         0x388
#define METATILE_RangerStation_Stone        0x306 // gTileset_CeruleanCave

// Engine hooks
bool32 Act2Station_CanJumpGap(enum Direction direction);
bool32 Act2Station_TryOvergrowthStep(void);

// Specials
void Act2Station_DrawFloor(void);
bool16 Act2Station_HasFinaleBit(void);
void Act2Station_SetFinaleBit(void);
bool16 Act2Station_FindFireMoveUser(void);
bool16 Act2Station_FindWaterMoveUser(void);
u16 Act2Station_GetFacedClockPiece(void);
bool16 Act2Station_IsClockPieceLit(void);
bool16 Act2Station_ToggleClockPiece(void);
void Act2Station_OpenClockGate(void);
bool16 Act2Station_IsDeadEndBeaten(void);
void Act2Station_PrepareDeadEnd(void);
void Act2Station_CreateDeadEndMon(void);
void Act2Station_SetDeadEndBeaten(void);
void Act2Station_GetPathStart(void);

#endif // GUARD_ACT2_STATION_H
