#include "global.h"
#include "overworld.h"
#include "test/test.h"
#include "constants/region_map_sections.h"
#include "constants/map_groups.h"
#include "../src/data/map_group_count.h"

// Warps keep a map's number in an s8 (struct WarpData), and a warp event whose map number is MAP_NUM(MAP_DYNAMIC)
// (127) goes to the dynamic warp instead, whatever its group. So a group can hold at most 127 maps (0-126):
// - a 129th map's number wraps to a negative one, and warping there loads a garbage header (a black screen and a
//   freeze: the night bloom's glade, when it was the Dungeons group's 129th map);
// - the 128th map can't be warped to at all (a crash: the ranger station's B4F, down its ladder, when it was the
//   Dungeons group's 128th map).
TEST("Every map group fits in a warp's map number")
{
    u32 group;
    for (group = 0; group < ARRAY_COUNT(MAP_GROUP_COUNT); group++)
        EXPECT_LE(MAP_GROUP_COUNT[group], MAP_NUM(MAP_DYNAMIC));
}

TEST("The ranger station's floors can be warped to")
{
    EXPECT_NE(MAP_NUM(MAP_RANGER_STATION_1F), MAP_NUM(MAP_DYNAMIC));
    EXPECT_NE(MAP_NUM(MAP_RANGER_STATION_B1F), MAP_NUM(MAP_DYNAMIC));
    EXPECT_NE(MAP_NUM(MAP_RANGER_STATION_B2F), MAP_NUM(MAP_DYNAMIC));
    EXPECT_NE(MAP_NUM(MAP_RANGER_STATION_B3F), MAP_NUM(MAP_DYNAMIC));
    EXPECT_NE(MAP_NUM(MAP_RANGER_STATION_B4F), MAP_NUM(MAP_DYNAMIC));
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
