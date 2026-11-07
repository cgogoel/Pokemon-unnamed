#ifndef GUARD_CONSTANTS_QUESTS_H
#define GUARD_CONSTANTS_QUESTS_H

// VAR_QUEST_LOG (src/quests.c, set on entering maps): what the story's own vars don't keep

// Act 2's road home: its stops reached on the final project, whichever way home (docs/hack/quests.md)
#define QUEST_LOG_ROAD_SAFFRON      (1 << 0)
#define QUEST_LOG_ROAD_MIDWAY       (1 << 1) // Vermilion City (south) or Cerulean City (north)
#define QUEST_LOG_ROAD_CAVE         (1 << 2) // Diglett's Cave (south) or Pewter City (north)
#define QUEST_LOG_ROAD_VIRIDIAN     (1 << 3)
#define QUEST_LOG_ROAD_PALLET       (1 << 4)
#define QUEST_LOG_ROAD_FARM         (1 << 5)
#define QUEST_LOG_ROAD_STOPS        6
#define QUEST_LOG_ROAD_NORTH        (1 << 6) // The way the player took last: north (Cerulean, Mt. Moon) or south
// Act 2's ranger station
#define QUEST_LOG_STATION_ENTERED   (1 << 7)
#define QUEST_LOG_STATION_B3F       (1 << 8)

#endif // GUARD_CONSTANTS_QUESTS_H
