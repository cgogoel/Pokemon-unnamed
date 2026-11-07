#include "global.h"
#include "battle.h"
#include "battle_bg.h"
#include "battle_environment.h"
#include "decompress.h"
#include "palette.h"
#include "triple_battle.h"
#include "test/test.h"

// The 3v3 floor is drawn on the battle's load and again whenever the background is reloaded: after every move
// animation that changes it (restorebg), on the evolution scene, coming back from the bag or the party menu. It used
// to take ~8 seconds (486 frames) each time, as black screens and freezes after moves like Payback or Spite.
static u32 DrawFloorFrames(u32 environment, u32 flags, u32 *checksum)
{
    const u32 *tiles = (const u32 *)BG_CHAR_ADDR(2);
    const u16 *tilemap = (const u16 *)BG_SCREEN_ADDR(26);
    u32 start, sum = 0, i;

    gBattleTypeFlags = flags;
    gBattlersCount = (flags & BATTLE_TYPE_TRIPLE_2V2) ? 4 : 6;
    DecompressDataWithHeaderVram(gBattleEnvironmentInfo[environment].background.tileset, (void *)(BG_CHAR_ADDR(2)));
    DecompressDataWithHeaderVram(gBattleEnvironmentInfo[environment].background.tilemap, (void *)(BG_SCREEN_ADDR(26)));
    LoadPalette(gBattleEnvironmentInfo[environment].palette, BG_PLTT_ID(2), 3 * PLTT_SIZE_4BPP);
    start = gMain.vblankCounter2;
    DrawTripleBattleFloor();
    start = gMain.vblankCounter2 - start;
    for (i = 0; i < 512 * TILE_SIZE_4BPP / 4; i++)
        sum = sum * 31 + tiles[i];
    for (i = 0; i < 32 * 20; i++)
        sum = sum * 31 + tilemap[i];
    for (i = 0; i < 16; i++)
        sum = sum * 31 + gPlttBufferUnfaded[BG_PLTT_ID(TRIPLE_FLOOR_BG_PALETTE) + i];
    *checksum = sum;
    gBattleTypeFlags = 0;
    return start;
}

TEST("Triple battles: the floor is drawn in a few frames, and again from the cache at once")
{
    u32 environment = 0, flags = 0, frames, cachedFrames, checksum, cachedChecksum;
    for (u32 j = 0; j < BATTLE_ENVIRONMENT_COUNT; j++)
    {
        if (gBattleEnvironmentInfo[j].background.tileset == NULL || gBattleEnvironmentInfo[j].background.tilemap == NULL)
            continue;
        PARAMETRIZE { environment = j; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE; }
        PARAMETRIZE { environment = j; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE | BATTLE_TYPE_TRIPLE_2V2; }
    }
    InvalidateTripleBattleFloorCache();
    frames = DrawFloorFrames(environment, flags, &checksum);
    // The background reloaded (restorebg): the same floor, from the cache
    cachedFrames = DrawFloorFrames(environment, flags, &cachedChecksum);
    Test_MgbaPrintf("floor env %d flags %d: %d frames, checksum %d, cached %d", environment, flags, frames, checksum, cachedFrames);
    EXPECT_LE(frames, 15);
    EXPECT_LE(cachedFrames, 1);
    EXPECT_EQ(cachedChecksum, checksum);
    InvalidateTripleBattleFloorCache();
}
