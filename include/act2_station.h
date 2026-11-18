#ifndef GUARD_ACT2_STATION_H
#define GUARD_ACT2_STATION_H

// The abandoned ranger station, the Act 2 finale's dungeon (docs/hack/act2_finale.md)

// Its custom metatiles (tools/hack/draw_station_tiles.py)
#define METATILE_RangerStation_TorchUnlit   0x384 // gTileset_PokemonMansion
#define METATILE_RangerStation_TorchLit     0x385
#define METATILE_RangerStation_MosaicHalf   0x3A1 // The mosaic's middle slab, slid halfway
#define METATILE_RangerStation_MosaicOpen   0x3A2 // ...slid away: the shaft, a ladder down (MB_LADDER)
#define METATILE_RangerStation_Stone        0x306 // gTileset_CeruleanCave

// Engine hooks
bool32 Act2Station_CanJumpGap(enum Direction direction);
bool32 Act2Station_TryOvergrowthStep(void);
bool32 Act2Station_IsFollowerGap(s16 x, s16 y);
u32 Act2Station_GetFloodFlashLevel(void);
bool32 Act2Station_IsEncounterFree(void);

// Specials
void Act2Station_DrawFloor(void);
bool16 Act2Station_HasFinaleBit(void);
void Act2Station_SetFinaleBit(void);
bool16 Act2Station_FindFireMoveUser(void);
bool16 Act2Station_FindWaterMoveUser(void);
u16 Act2Station_GetFacedClockPiece(void);
bool16 Act2Station_IsClockPieceLit(void);
bool16 Act2Station_ToggleClockPiece(void);
void Act2Station_SlideMosaic(void);
void Act2Station_CreateGuardMon(void);
u16 Act2Station_CountNotes(void);
bool16 Act2Station_IsDeadEndBeaten(void);
void Act2Station_PrepareDeadEnd(void);
void Act2Station_CreateDeadEndMon(void);
void Act2Station_SetDeadEndBeaten(void);
void Act2Station_GetPathStart(void);

#endif // GUARD_ACT2_STATION_H
