#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "palette.h"
#include "util.h"
#include "decompress.h"
#include "sprite.h"
#include "triple_battle.h"
#include "constants/rgb.h"
#include "constants/pokemon.h"
#include "malloc.h"

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

// Pokémon sprites are shrunk by depth so the three ranks of each team read front to back (docs/hack/triple_battles.md).
// Indexed by side, then layout position / 2 (left = rear, right = front, middle). 256 = full size.
static const u16 sTripleSpriteScale[NUM_BATTLE_SIDES][3] =
{
    [B_SIDE_PLAYER]   = { [B_POSITION_PLAYER_LEFT / 2] = 256, [B_POSITION_PLAYER_RIGHT / 2] = 180, [B_POSITION_PLAYER_MIDDLE / 2] = 215 },
    [B_SIDE_OPPONENT] = { [B_POSITION_PLAYER_LEFT / 2] = 148, [B_POSITION_PLAYER_RIGHT / 2] = 180, [B_POSITION_PLAYER_MIDDLE / 2] = 164 },
};

// Haze: farther Pokémon fade toward the floor color (out of 16). Same indexing as sTripleSpriteScale.
static const u8 sTripleHaze[NUM_BATTLE_SIDES][3] =
{
    [B_SIDE_PLAYER]   = { [B_POSITION_PLAYER_LEFT / 2] = 0, [B_POSITION_PLAYER_RIGHT / 2] = 4, [B_POSITION_PLAYER_MIDDLE / 2] = 2 },
    [B_SIDE_OPPONENT] = { [B_POSITION_PLAYER_LEFT / 2] = 5, [B_POSITION_PLAYER_RIGHT / 2] = 1, [B_POSITION_PLAYER_MIDDLE / 2] = 3 },
};

static EWRAM_DATA u16 sTripleHazeColor = 0; // Set when the floor is drawn

// Fades the battler's sprite palette by its depth. Called after its palette is loaded.
void ApplyTripleHaze(enum BattlerId battler)
{
    u32 coeff = sTripleHaze[GetBattlerSide(battler)][GetBattlerLayoutPosition(battler) / 2];
    if (coeff == 0)
        return;
    BlendPalette(OBJ_PLTT_ID(battler), 16, coeff, sTripleHazeColor);
    CpuCopy32(&gPlttBufferFaded[OBJ_PLTT_ID(battler)], &gPlttBufferUnfaded[OBJ_PLTT_ID(battler)], PLTT_SIZEOF(16));
}

u32 GetTripleSpriteScale(enum BattlerId battler)
{
    return sTripleSpriteScale[GetBattlerSide(battler)][GetBattlerLayoutPosition(battler) / 2];
}

#define PIC_TILES_WIDE (MON_PIC_WIDTH / 8)

static inline u32 GetPicPixel(const u8 *pic, u32 x, u32 y)
{
    u8 byte = pic[((y / 8) * PIC_TILES_WIDE + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2];
    return (x & 1) ? (byte >> 4) : (byte & 0xF);
}

static inline void SetPicPixel(u8 *pic, u32 x, u32 y, u32 color)
{
    u8 *byte = &pic[((y / 8) * PIC_TILES_WIDE + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2];
    if (x & 1)
        *byte = (*byte & 0x0F) | (color << 4);
    else
        *byte = (*byte & 0xF0) | color;
}

// Shrinks the battler's decompressed pic frames in place (nearest neighbour), toward the middle of its lowest row
// so the Pokémon keeps standing on the same spot. Returns how many rows of empty space are now above it.
u32 ScaleTripleMonPic(u8 *pic, enum BattlerId battler)
{
    u32 scale = GetTripleSpriteScale(battler);
    u8 *copy;
    u32 top = MON_PIC_HEIGHT;

    if (scale >= 256 || (copy = Alloc(MON_PIC_SIZE)) == NULL)
        return 0;

    for (u32 frame = 0; frame < MAX_MON_PIC_FRAMES; frame++)
    {
        u8 *framePic = pic + frame * MON_PIC_SIZE;
        s32 anchorY = -1;

        memcpy(copy, framePic, MON_PIC_SIZE);
        for (s32 y = MON_PIC_HEIGHT - 1; y >= 0 && anchorY < 0; y--)
        {
            for (u32 x = 0; x < MON_PIC_WIDTH; x++)
            {
                if (GetPicPixel(copy, x, y))
                {
                    anchorY = y;
                    break;
                }
            }
        }
        if (anchorY < 0)
            continue; // Empty frame

        for (s32 y = 0; y < MON_PIC_HEIGHT; y++)
        {
            s32 srcY = anchorY + ((y - anchorY) * 256) / (s32)scale;
            for (s32 x = 0; x < MON_PIC_WIDTH; x++)
            {
                s32 srcX = MON_PIC_WIDTH / 2 + ((x - MON_PIC_WIDTH / 2) * 256) / (s32)scale;
                u32 color = 0;
                if (srcX >= 0 && srcX < MON_PIC_WIDTH && srcY >= 0 && srcY < MON_PIC_HEIGHT)
                    color = GetPicPixel(copy, srcX, srcY);
                SetPicPixel(framePic, x, y, color);
                if (color && frame == 0 && (u32)y < top)
                    top = y;
            }
        }
    }
    Free(copy);
    return (top == MON_PIC_HEIGHT) ? 0 : top;
}

// Topmost row with pixels in the battler's (scaled) pic, or MON_PIC_HEIGHT / 2 when it isn't loaded yet.
static u32 GetTriplePicTopRow(enum BattlerId battler)
{
    const u8 *pic = gMonSpritesGfxPtr->spritesGfx[GetBattlerPosition(battler)];
    if (pic == NULL)
        return MON_PIC_HEIGHT / 2;
    for (u32 y = 0; y < MON_PIC_HEIGHT; y++)
    {
        for (u32 x = 0; x < MON_PIC_WIDTH; x++)
        {
            if (GetPicPixel(pic, x, y))
                return y;
        }
    }
    return MON_PIC_HEIGHT / 2;
}

// The HP bar floats just above the Pokémon's head; the name tag (the healthbox) sits right above the bar.
// x, y are the healthbox's left half centre (the tag is 64 pixels wide, two 32x16 halves).
void GetTripleHealthboxCoords(enum BattlerId battler, s16 *x, s16 *y)
{
    s32 headY = GetBattlerSpriteDefault_Y(battler) - MON_PIC_HEIGHT / 2 + GetTriplePicTopRow(battler);

    *x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X) - 16;
    *y = headY - TRIPLE_HEAD_GAP - TRIPLE_HEALTHBAR_OFFSET_Y;
    if (*y < TRIPLE_HEALTHBOX_MIN_Y)
        *y = TRIPLE_HEALTHBOX_MIN_Y;
}

// Battle floor for triple battles: each team stands on nested ovals along its diagonal. The base oval (solid) is under
// the rear Pokémon, the next one (half shade) reaches the middle Pokémon and the outer one (outline only) the front one.
// The ovals are painted at load time onto copies of the environment's background tiles, in the unused upper half of
// BG3's char block, and the 1v1 platforms are painted out. Colors are darker shades picked from the environment palette.
#define FLOOR_FIRST_TILE 256 // Environment tilesets use tiles 0-255
#define FLOOR_MAX_TILES 256
#define FLOOR_ROWS 14        // Rows above the text box
#define FLOOR_COLS (DISPLAY_WIDTH / 8)
#define FLOOR_SPLIT_ROW 10   // Rows above take their background from column 0, rows below from the last column
#define FLOOR_GROUND_OFFSET 28        // From a foe's sprite centre to its feet
#define FLOOR_PLAYER_GROUND_OFFSET 20 // The player's back sprites are cut off by the text box: their ovals sit just above it
#define FLOOR_OVAL_PAD 22    // How far an oval reaches past the Pokémon at its ends
#define FLOOR_TEAM_SPLIT_Y 68 // Each team's ovals stay on its own side of this line

enum
{
    FLOOR_NONE,
    FLOOR_LIGHT,   // Outer section
    FLOOR_HALF,    // Middle section
    FLOOR_BASE,    // Base oval
    FLOOR_OUTLINE,
    FLOOR_SHADE_COUNT,
};


struct FloorOval
{
    s32 cx, cy, rx, ry;
};

struct FloorTeam
{
    s32 cos, sin; // Q8 direction of the team's diagonal
    s32 yMin, yMax;
    struct FloorOval ovals[TRIPLE_RANK_COUNT]; // Base, middle, outer
};

extern const struct UCoords8 sBattlerCoords[][MAX_BATTLERS_COUNT];

static u32 FloorSqrt(u32 n)
{
    u32 root = 0, bit = 1u << 30;
    while (bit > n)
        bit >>= 2;
    while (bit != 0)
    {
        if (n >= root + bit)
        {
            n -= root + bit;
            root = (root >> 1) + bit;
        }
        else
        {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

static void InitFloorTeam(struct FloorTeam *team, enum BattleSide side)
{
    // Rear, middle and front spots (see GetBattlerLayoutPosition)
    const enum BattlerPosition spots[TRIPLE_RANK_COUNT] = { B_POSITION_PLAYER_LEFT | side, B_POSITION_PLAYER_MIDDLE | side, B_POSITION_PLAYER_RIGHT | side };
    s32 x[TRIPLE_RANK_COUNT], y[TRIPLE_RANK_COUNT];

    for (u32 i = 0; i < TRIPLE_RANK_COUNT; i++)
    {
        x[i] = sBattlerCoords[BATTLE_COORDS_TRIPLES][spots[i]].x;
        y[i] = sBattlerCoords[BATTLE_COORDS_TRIPLES][spots[i]].y + ((side == B_SIDE_PLAYER) ? FLOOR_PLAYER_GROUND_OFFSET : FLOOR_GROUND_OFFSET);
    }

    s32 dx = x[2] - x[0], dy = y[2] - y[0];
    s32 length = FloorSqrt(dx * dx + dy * dy);
    team->cos = (dx * 256) / length;
    team->sin = (dy * 256) / length;
    team->yMin = (side == B_SIDE_PLAYER) ? FLOOR_TEAM_SPLIT_Y : 0;
    team->yMax = (side == B_SIDE_PLAYER) ? FLOOR_ROWS * 8 : FLOOR_TEAM_SPLIT_Y;

    for (u32 i = 0; i < TRIPLE_RANK_COUNT; i++)
    {
        // Distance along the diagonal from the rear Pokémon to this one
        s32 distance = ((x[i] - x[0]) * team->cos + (y[i] - y[0]) * team->sin) / 256;
        team->ovals[i].cx = x[0] + (team->cos * distance) / 512;
        team->ovals[i].cy = y[0] + (team->sin * distance) / 512;
        team->ovals[i].rx = distance / 2 + FLOOR_OVAL_PAD;
        team->ovals[i].ry = 7 + 4 * i;
    }
}

static bool32 IsInFloorOval(const struct FloorTeam *team, const struct FloorOval *oval, s32 shrinkX, s32 shrinkY, s32 px, s32 py)
{
    s32 rx = oval->rx - shrinkX, ry = oval->ry - shrinkY;
    s32 u = ((px - oval->cx) * team->cos + (py - oval->cy) * team->sin) / 256;
    s32 v = (-(px - oval->cx) * team->sin + (py - oval->cy) * team->cos) / 256;
    return u * u * ry * ry + v * v * rx * rx <= rx * rx * ry * ry;
}

static u32 GetFloorShade(const struct FloorTeam *teams, s32 px, s32 py)
{
    u32 shade = FLOOR_NONE;

    for (u32 t = 0; t < NUM_BATTLE_SIDES; t++)
    {
        const struct FloorTeam *team = &teams[t];
        if (py < team->yMin || py >= team->yMax)
            continue;
        for (s32 i = TRIPLE_RANK_COUNT - 1; i >= 0; i--)
        {
            if (!IsInFloorOval(team, &team->ovals[i], 0, 0, px, py))
                break; // Ovals are nested: outside this one means outside the smaller ones
            if (!IsInFloorOval(team, &team->ovals[i], 2, 1, px, py))
                return FLOOR_OUTLINE;
            shade = (i == 0) ? FLOOR_BASE : (i == 1) ? FLOOR_HALF : FLOOR_LIGHT;
        }
    }
    return shade;
}

// The ovals are drawn with sFloorColors written into palette slots the environment background doesn't use. If a palette
// has no 3 free slots, they take the closest colors it has.
struct FloorShades
{
    bool8 reserved[3];                     // Painted tiles of this environment palette use FLOOR_PALETTE
    u8 index[FLOOR_SHADE_COUNT];           // Slots of FLOOR_PALETTE holding sFloorColors
    u8 nearest[3][FLOOR_SHADE_COUNT][16];  // Fallback: closest colors of the environment palette
};

// The oval tiles use their own BG palette: a copy of the floor's environment palette with the oval colors added in
// slots the floor tiles don't use. Palette 6 is only used by the link battle VS screen.
#define FLOOR_PALETTE 6

// Colors of the ovals: a dark gray base oval and outlines, a green middle section and a pale green outer section
static const u16 sFloorColors[FLOOR_SHADE_COUNT] =
{
    [FLOOR_LIGHT]   = RGB(24, 28, 22),
    [FLOOR_HALF]    = RGB(15, 21, 13),
    [FLOOR_BASE]    = RGB(9, 10, 10),
    [FLOOR_OUTLINE] = RGB(7, 8, 8),
};

static u16 GetFloorSourceEntry(const u16 *tilemap, u32 row)
{
    return tilemap[row * 32 + ((row < FLOOR_SPLIT_ROW) ? 0 : FLOOR_COLS - 1)];
}

static void BuildFloorShades(struct FloorShades *shades, const u16 *tilemap, const u8 *tiles)
{
    u16 usedColors[3] = {0};
    u32 sum[3][3] = {0}, count[3] = {0}, rows[3] = {0};

    // Colors of the floor tiles (the backgrounds the ovals are painted on) and their average, per palette
    for (u32 row = 0; row < FLOOR_ROWS; row++)
    {
        u32 entry = GetFloorSourceEntry(tilemap, row), palette = entry >> 12, tile = entry & 0x3FF;
        if (palette < 2 || palette > 4)
            continue;
        rows[palette - 2]++;
        for (u32 b = 0; b < TILE_SIZE_4BPP * 2; b++)
        {
            u8 byte = tiles[tile * TILE_SIZE_4BPP + b / 2];
            u32 index = (b & 1) ? (byte >> 4) : (byte & 0xF);
            u16 color = gPlttBufferUnfaded[BG_PLTT_ID(palette) + index];
            usedColors[palette - 2] |= 1 << index;
            sum[palette - 2][0] += GET_R(color);
            sum[palette - 2][1] += GET_G(color);
            sum[palette - 2][2] += GET_B(color);
            count[palette - 2]++;
        }
    }

    // The palette most floor rows use gets the copy with the exact oval colors
    u32 main = (rows[1] > rows[0]) ? 1 : 0;
    if (rows[2] > rows[main])
        main = 2;
    if (count[main] != 0)
        sTripleHazeColor = RGB(sum[main][0] / count[main], sum[main][1] / count[main], sum[main][2] / count[main]);

    for (u32 p = 0; p < 3; p++)
        shades->reserved[p] = FALSE;
    u32 free = 0;
    for (u32 i = 1; i < 16 && free < FLOOR_SHADE_COUNT - 1; i++)
    {
        if (!(usedColors[main] & (1 << i)))
            shades->index[FLOOR_LIGHT + free++] = i;
    }
    if (free == FLOOR_SHADE_COUNT - 1 && rows[main] != 0)
    {
        shades->reserved[main] = TRUE;
        LoadPalette(&gPlttBufferUnfaded[BG_PLTT_ID(2 + main)], BG_PLTT_ID(FLOOR_PALETTE), PLTT_SIZE_4BPP);
        for (u32 shade = FLOOR_LIGHT; shade < FLOOR_SHADE_COUNT; shade++)
            LoadPalette(&sFloorColors[shade], BG_PLTT_ID(FLOOR_PALETTE) + shades->index[shade], PLTT_SIZEOF(1));
    }

    for (u32 p = 0; p < 3; p++)
    {
        const u16 *colors = &gPlttBufferUnfaded[BG_PLTT_ID(2 + p)];
        for (u32 shade = 0; shade < FLOOR_SHADE_COUNT; shade++)
        {
            for (u32 i = 0; i < 16; i++)
            {
                s32 r = GET_R(sFloorColors[shade]), g = GET_G(sFloorColors[shade]), b = GET_B(sFloorColors[shade]);
                u32 best = i, bestDistance = 0xFFFFFFFF;
                if (shade == FLOOR_NONE)
                {
                    shades->nearest[p][shade][i] = i;
                    continue;
                }
                for (u32 j = 1; j < 16; j++)
                {
                    s32 dr = GET_R(colors[j]) - r, dg = GET_G(colors[j]) - g, db = GET_B(colors[j]) - b;
                    u32 distance = dr * dr + dg * dg + db * db;
                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        best = j;
                    }
                }
                shades->nearest[p][shade][i] = best;
            }
        }
    }
}

void DrawTripleBattleFloor(void)
{
    struct FloorTeam teams[NUM_BATTLE_SIDES];
    struct FloorShades *shades = Alloc(sizeof(struct FloorShades));
    u16 *tilemap = (u16 *)BG_SCREEN_ADDR(26);
    const u8 *tiles = (const u8 *)BG_CHAR_ADDR(2);
    u32 newTiles = 0;

    if (shades == NULL)
        return;
    InitFloorTeam(&teams[B_SIDE_PLAYER], B_SIDE_PLAYER);
    InitFloorTeam(&teams[B_SIDE_OPPONENT], B_SIDE_OPPONENT);
    BuildFloorShades(shades, tilemap, tiles);

    for (u32 row = 0; row < FLOOR_ROWS; row++)
    {
        // The plain background for this row, away from the 1v1 platforms
        u16 source = GetFloorSourceEntry(tilemap, row);
        u32 sourceTile = source & 0x3FF, palette = source >> 12;
        bool32 hFlip = (source & 0x400) != 0, vFlip = (source & 0x800) != 0;

        for (u32 col = 0; col < FLOOR_COLS; col++)
        {
            u32 words[TILE_SIZE_4BPP / 4];
            u8 *pixels = (u8 *)words;
            bool32 painted = FALSE;

            for (u32 y = 0; y < 8; y++)
            {
                for (u32 x = 0; x < 8; x++)
                {
                    u32 sx = hFlip ? 7 - x : x, sy = vFlip ? 7 - y : y;
                    u8 byte = tiles[sourceTile * TILE_SIZE_4BPP + sy * 4 + sx / 2];
                    u32 color = (sx & 1) ? (byte >> 4) : (byte & 0xF);
                    u32 shade = GetFloorShade(teams, col * 8 + x, row * 8 + y);

                    if (shade != FLOOR_NONE && palette >= 2 && palette <= 4)
                    {
                        if (shades->reserved[palette - 2])
                            color = shades->index[shade];
                        else
                            color = shades->nearest[palette - 2][shade][color];
                        painted = TRUE;
                    }
                    if (x & 1)
                        pixels[y * 4 + x / 2] = (pixels[y * 4 + x / 2] & 0x0F) | (color << 4);
                    else
                        pixels[y * 4 + x / 2] = color;
                }
            }

            if (!painted || newTiles == FLOOR_MAX_TILES)
            {
                tilemap[row * 32 + col] = source;
                continue;
            }

            // Reuse an identical tile made earlier
            u32 tile;
            for (tile = 0; tile < newTiles; tile++)
            {
                const u32 *existing = (const u32 *)(tiles + (FLOOR_FIRST_TILE + tile) * TILE_SIZE_4BPP);
                u32 w;
                for (w = 0; w < ARRAY_COUNT(words) && existing[w] == words[w]; w++)
                    ;
                if (w == ARRAY_COUNT(words))
                    break;
            }
            if (tile == newTiles)
            {
                CpuCopy32(words, (void *)(tiles + (FLOOR_FIRST_TILE + tile) * TILE_SIZE_4BPP), sizeof(words));
                newTiles++;
            }
            if (palette >= 2 && palette <= 4 && shades->reserved[palette - 2])
                tilemap[row * 32 + col] = (FLOOR_FIRST_TILE + tile) | (FLOOR_PALETTE << 12);
            else
                tilemap[row * 32 + col] = (FLOOR_FIRST_TILE + tile) | (palette << 12);
        }
    }
    Free(shades);
}
