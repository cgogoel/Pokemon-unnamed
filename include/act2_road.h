#ifndef GUARD_ACT2_ROAD_H
#define GUARD_ACT2_ROAD_H

// Act 2's final project: the open world and the road home (docs/hack/act2_finale.md, "The road home")
bool32 Act2Road_IsActive(void);
bool32 Act2Road_IsRouteOpen(void);
bool16 Act2Road_IsRouteOpenSpecial(void);
bool32 Act2Road_IsOnRouteMap(u32 mapGroup, u32 mapNum);
bool32 Act2Road_IsOnRoute(void);
u32 Act2Road_GetPartyAverageLevel(void);
u32 Act2Road_GetAreaLevel(void);
void Act2Road_OnMapEntered(void);
u32 Act2Road_GetWildLevel(void);
u32 Act2Road_GetTrainerLevel(u32 trainerId);
bool32 Act2Road_IsAreaFinalTrainer(u32 trainerId);
bool32 Act2Road_IsScaledTrainer(u32 trainerId);
u32 Act2Road_EvolveSpeciesToLevel(u32 species, u32 level, u32 personality);
u32 Act2Road_GetEvoStage(u32 species);
u32 Act2Road_PickOtherRegionSpecies(u32 species);
void Act2Road_OnWildMonCreated(struct Pokemon *mon);
void Act2Road_ScaleTrainerParty(struct Pokemon *party, u32 trainerId);

#endif // GUARD_ACT2_ROAD_H
