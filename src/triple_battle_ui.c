#include "global.h"
#include "battle.h"
#include "decompress.h"
#include "sprite.h"
#include "triple_battle.h"
#include "constants/rgb.h"

// Move info indicator for triple battles, shown on the move type line of the move menu (docs/hack/triple_battles.md):
// - 3 squares for the damage each target takes: dark = full, gray = reduced, white = not hit
// - Red outlines for reach: only the first square for FRONT moves, every square for ANY moves
// - A red "!" when the move also hits the user's allies

#define TAG_TRIPLE_INDICATOR 0xD7F4

enum
{
    INDICATOR_TILE_DARK,
    INDICATOR_TILE_GRAY,
    INDICATOR_TILE_WHITE,
    INDICATOR_TILE_OUTLINED, // Added to the three above
    INDICATOR_TILE_ALLY_MARKER = INDICATOR_TILE_OUTLINED + 3,
    INDICATOR_TILE_COUNT,
};

#define INDICATOR_SQUARES 3
#define INDICATOR_SPRITES (INDICATOR_SQUARES + 1)

// Screen position of the first square, at the start of the move type window.
#define INDICATOR_X 172
#define INDICATOR_Y 145

// Palette: 1 red, 2 dark, 3 gray, 4 white, 5 black.
static const u16 sIndicatorPalette[16] =
{
    RGB(0, 0, 0), RGB(31, 0, 0), RGB(7, 7, 7), RGB(19, 19, 19), RGB(31, 31, 31), RGB(0, 0, 0),
};

static const u32 sIndicatorTiles[INDICATOR_TILE_COUNT * 8] =
{
    0x05555555, 0x05222225, 0x05222225, 0x05222225, 0x05222225, 0x05222225, 0x05555555, 0x00000000, // Dark
    0x05555555, 0x05333335, 0x05333335, 0x05333335, 0x05333335, 0x05333335, 0x05555555, 0x00000000, // Gray
    0x05555555, 0x05444445, 0x05444445, 0x05444445, 0x05444445, 0x05444445, 0x05555555, 0x00000000, // White
    0x01111111, 0x01222221, 0x01222221, 0x01222221, 0x01222221, 0x01222221, 0x01111111, 0x00000000, // Dark, red outline
    0x01111111, 0x01333331, 0x01333331, 0x01333331, 0x01333331, 0x01333331, 0x01111111, 0x00000000, // Gray, red outline
    0x01111111, 0x01444441, 0x01444441, 0x01444441, 0x01444441, 0x01444441, 0x01111111, 0x00000000, // White, red outline
    0x00000110, 0x00000110, 0x00000110, 0x00000110, 0x00000110, 0x00000000, 0x00000110, 0x00000000, // Ally marker
};

static const struct SpriteSheet sIndicatorSpriteSheet = { sIndicatorTiles, sizeof(sIndicatorTiles), TAG_TRIPLE_INDICATOR };
static const struct SpritePalette sIndicatorSpritePalette = { sIndicatorPalette, TAG_TRIPLE_INDICATOR };

static const struct OamData sOamData_Indicator =
{
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 0,
};

static const struct SpriteTemplate sIndicatorSpriteTemplate =
{
    .tileTag = TAG_TRIPLE_INDICATOR,
    .paletteTag = TAG_TRIPLE_INDICATOR,
    .oam = &sOamData_Indicator,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static EWRAM_DATA u8 sIndicatorSpriteIds[INDICATOR_SPRITES] = {0};

void ResetTripleMoveIndicator(void)
{
    for (u32 i = 0; i < INDICATOR_SPRITES; i++)
        sIndicatorSpriteIds[i] = MAX_SPRITES;
}

void HideTripleMoveIndicator(void)
{
    if (sIndicatorSpriteIds[0] == MAX_SPRITES)
        return;
    for (u32 i = 0; i < INDICATOR_SPRITES; i++)
    {
        if (sIndicatorSpriteIds[i] != MAX_SPRITES)
            DestroySprite(&gSprites[sIndicatorSpriteIds[i]]);
        sIndicatorSpriteIds[i] = MAX_SPRITES;
    }
    FreeSpriteTilesByTag(TAG_TRIPLE_INDICATOR);
    FreeSpritePaletteByTag(TAG_TRIPLE_INDICATOR);
}

// Returns whether the indicator is shown, i.e. whether the move type line should leave room for it.
bool32 ShowTripleMoveIndicator(enum Move move)
{
    const struct TripleMoveInfo *info = GetTripleMoveInfo(move);
    u32 fills[INDICATOR_SQUARES];

    HideTripleMoveIndicator();
    if (!IsTripleBattle() || info->aoe == 0)
        return FALSE;

    switch (info->aoe)
    {
    case 1:
        fills[0] = INDICATOR_TILE_DARK, fills[1] = INDICATOR_TILE_WHITE, fills[2] = INDICATOR_TILE_WHITE;
        break;
    case 2:
        fills[0] = INDICATOR_TILE_DARK, fills[1] = INDICATOR_TILE_GRAY, fills[2] = INDICATOR_TILE_WHITE;
        break;
    default:
        fills[0] = INDICATOR_TILE_GRAY, fills[1] = INDICATOR_TILE_GRAY, fills[2] = INDICATOR_TILE_GRAY;
        break;
    }

    LoadSpriteSheet(&sIndicatorSpriteSheet);
    LoadSpritePalette(&sIndicatorSpritePalette);
    u32 tileStart = GetSpriteTileStartByTag(TAG_TRIPLE_INDICATOR);
    if (tileStart == 0xFFFF)
        return FALSE;

    for (u32 i = 0; i < INDICATOR_SQUARES; i++)
    {
        bool32 outlined = (info->reach == TRIPLE_REACH_ANY || i == 0);
        u32 tile = fills[i] + (outlined ? INDICATOR_TILE_OUTLINED : 0);
        sIndicatorSpriteIds[i] = CreateSprite(&sIndicatorSpriteTemplate, INDICATOR_X + 8 * i, INDICATOR_Y, 0);
        if (sIndicatorSpriteIds[i] != MAX_SPRITES)
            gSprites[sIndicatorSpriteIds[i]].oam.tileNum = tileStart + tile;
    }

    if (info->hitsAllies)
    {
        sIndicatorSpriteIds[INDICATOR_SQUARES] = CreateSprite(&sIndicatorSpriteTemplate, INDICATOR_X + 8 * INDICATOR_SQUARES, INDICATOR_Y, 0);
        if (sIndicatorSpriteIds[INDICATOR_SQUARES] != MAX_SPRITES)
            gSprites[sIndicatorSpriteIds[INDICATOR_SQUARES]].oam.tileNum = tileStart + INDICATOR_TILE_ALLY_MARKER;
    }
    return TRUE;
}
