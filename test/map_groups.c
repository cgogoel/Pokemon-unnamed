#include "global.h"
#include "overworld.h"
#include "test/test.h"
#include "constants/region_map_sections.h"
#include "constants/map_groups.h"
#include "../src/data/map_group_count.h"

// Warps keep a map's number in an s8 (struct WarpData): a group can hold at most 128 maps (0-127). A 129th map's
// number wraps to a negative one, and warping there loads a garbage header (a black screen and a freeze: the night
// bloom's glade, when it was the Dungeons group's 129th map).
TEST("Every map group fits in a warp's map number")
{
    u32 group;
    for (group = 0; group < ARRAY_COUNT(MAP_GROUP_COUNT); group++)
        EXPECT_LE(MAP_GROUP_COUNT[group], 128);
}

TEST("The night bloom's glade can be warped to")
{
    struct WarpData warp = {0};
    warp.mapGroup = MAP_GROUP(MAP_VIRIDIAN_FOREST_GLADE);
    warp.mapNum = MAP_NUM(MAP_VIRIDIAN_FOREST_GLADE);
    EXPECT_EQ((u8)warp.mapNum, MAP_NUM(MAP_VIRIDIAN_FOREST_GLADE));
    EXPECT_GE(warp.mapNum, 0);
    EXPECT_EQ(Overworld_GetMapHeaderByGroupAndId(warp.mapGroup, warp.mapNum)->regionMapSectionId, MAPSEC_VIRIDIAN_FOREST);
}
