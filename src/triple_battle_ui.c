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

// Move badge for triple battles, shown on the move type line of the move menu (docs/hack/triple_battles.md): how the
// move reaches its targets, DIRECT (red: only the front foe, which blocks the others), INDIRECT (blue: any foe) or ALL
// (dark: every foe), followed by how many foes it hits ("x1".."x3", printed by the move type window). The move's type
// moves up to the PP line (GetTripleBadgeTextX, MoveSelectionDisplayMoveType). Art: tools/hack/draw_triple_badges.py.

#define TAG_TRIPLE_INDICATOR 0xD7F4

// Screen position of the badge's top left corner, at the start of the move type window
#define INDICATOR_X 167
#define INDICATOR_Y 140
#define BADGE_FRAME_SIZE (64 * 32 / 2)

enum
{
    BADGE_DIRECT,
    BADGE_INDIRECT,
    BADGE_ALL,
};

static const u32 sBadgeGfx[] = INCGFX_U32("graphics/battle_interface/triple_badges.png", ".4bpp");
static const u16 sBadgePalette[] = INCGFX_U16("graphics/battle_interface/triple_badges.pal", ".gbapal");
static const u8 sBadgeWidths[] = { [BADGE_DIRECT] = 34, [BADGE_INDIRECT] = 43, [BADGE_ALL] = 21 }; // From the tool

static const struct SpritePalette sIndicatorSpritePalette = { sBadgePalette, TAG_TRIPLE_INDICATOR };

static const struct OamData sOamData_Indicator =
{
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
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

static EWRAM_DATA u8 sIndicatorSpriteId = 0;

void ResetTripleMoveIndicator(void)
{
    sIndicatorSpriteId = MAX_SPRITES;
}

// Only destroys the indicator itself: the remembered sprite slot can be stale (it's only reset when a 3v3 starts), and
// destroying whatever sprite holds it took out the player's healthbox in single battles, or crashed the game
void HideTripleMoveIndicator(void)
{
    if (sIndicatorSpriteId >= MAX_SPRITES)
        return;
    if (!gSprites[sIndicatorSpriteId].inUse || gSprites[sIndicatorSpriteId].template->tileTag != TAG_TRIPLE_INDICATOR)
    {
        sIndicatorSpriteId = MAX_SPRITES;
        return;
    }
    DestroySprite(&gSprites[sIndicatorSpriteId]);
    sIndicatorSpriteId = MAX_SPRITES;
    FreeSpriteTilesByTag(TAG_TRIPLE_INDICATOR);
    FreeSpritePaletteByTag(TAG_TRIPLE_INDICATOR);
}

static u32 GetTripleBadge(const struct TripleMoveInfo *info)
{
    if (info->aoe >= 3)
        return BADGE_ALL;
    return (info->reach == TRIPLE_REACH_ANY) ? BADGE_INDIRECT : BADGE_DIRECT;
}

// How many foes the move hits, for the "xN" after the badge (2v2 has two foes at most)
u32 GetTripleMoveHitCount(enum Move move)
{
    return min(GetTripleMoveInfo(move)->aoe, GetTripleSideSize());
}

// Where the move type window's text starts, after the badge
u32 GetTripleBadgeTextX(enum Move move)
{
    return sBadgeWidths[GetTripleBadge(GetTripleMoveInfo(move))] + 3;
}

// Whether the move gets a badge (a move in the 3v3 table, in a triple battle)
bool32 HasTripleMoveBadge(enum Move move)
{
    return IsTripleBattle() && GetTripleMoveInfo(move)->aoe != 0;
}

// Returns whether the badge is shown, i.e. whether the move type line should leave room for it.
bool32 ShowTripleMoveIndicator(enum Move move)
{
    HideTripleMoveIndicator();
    if (!HasTripleMoveBadge(move))
        return FALSE;

    struct SpriteSheet sheet = { (const u8 *)sBadgeGfx + GetTripleBadge(GetTripleMoveInfo(move)) * BADGE_FRAME_SIZE, BADGE_FRAME_SIZE, TAG_TRIPLE_INDICATOR };
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&sIndicatorSpritePalette);
    sIndicatorSpriteId = CreateSprite(&sIndicatorSpriteTemplate, INDICATOR_X + 32, INDICATOR_Y + 16, 0);
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

// 2v2: the rear and front spots only, a little bigger than in 3v3. Same indexing.
static const u16 sTripleSpriteScale2v2[NUM_BATTLE_SIDES][2] =
{
    [B_SIDE_PLAYER]   = { [B_POSITION_PLAYER_LEFT / 2] = 256, [B_POSITION_PLAYER_RIGHT / 2] = 200 },
    [B_SIDE_OPPONENT] = { [B_POSITION_PLAYER_LEFT / 2] = 160, [B_POSITION_PLAYER_RIGHT / 2] = 192 },
};

static const u8 sTripleHaze2v2[NUM_BATTLE_SIDES][2] =
{
    [B_SIDE_PLAYER]   = { [B_POSITION_PLAYER_LEFT / 2] = 0, [B_POSITION_PLAYER_RIGHT / 2] = 3 },
    [B_SIDE_OPPONENT] = { [B_POSITION_PLAYER_LEFT / 2] = 4, [B_POSITION_PLAYER_RIGHT / 2] = 1 },
};

static EWRAM_DATA u16 sTripleHazeColor = 0; // Set when the floor is drawn

// Fades the battler's sprite palette by its depth. Called after its palette is loaded.
void ApplyTripleHaze(enum BattlerId battler)
{
    u32 coeff = HasTripleMiddleBattlers() ? sTripleHaze[GetBattlerSide(battler)][GetBattlerLayoutPosition(battler) / 2]
                                          : sTripleHaze2v2[GetBattlerSide(battler)][GetBattlerLayoutPosition(battler) / 2];
    if (coeff == 0)
        return;
    BlendPalette(OBJ_PLTT_ID(battler), 16, coeff, sTripleHazeColor);
    CpuCopy32(&gPlttBufferFaded[OBJ_PLTT_ID(battler)], &gPlttBufferUnfaded[OBJ_PLTT_ID(battler)], PLTT_SIZEOF(16));
}

u32 GetTripleSpriteScale(enum BattlerId battler)
{
    if (!HasTripleMiddleBattlers())
        return sTripleSpriteScale2v2[GetBattlerSide(battler)][GetBattlerLayoutPosition(battler) / 2];
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

static EWRAM_DATA u8 sTripleNameTagBelowBar = 0; // Bit per battler

// Whether the battler's name tag goes under its HP bar, because there's no room above it at the top of the screen.
bool32 IsTripleNameTagBelowBar(enum BattlerId battler)
{
    return (sTripleNameTagBelowBar & (1u << battler)) != 0;
}

// The HP bar floats just above the Pokémon's head; the name tag (the healthbox) sits right above the bar, or right
// under it near the top of the screen. x, y are the healthbox's left half centre (the tag is 64x16, two 32x16 halves).
void GetTripleHealthboxCoords(enum BattlerId battler, s16 *x, s16 *y)
{
    s32 headY = GetBattlerSpriteDefault_Y(battler) - MON_PIC_HEIGHT / 2 + GetTriplePicTopRow(battler);
    s32 barTop = headY - TRIPLE_HEAD_GAP;

    if (barTop < 1)
        barTop = 1;
    *x = GetBattlerSpriteCoord(battler, BATTLER_COORD_X) - 16;
    if (barTop - TRIPLE_HEALTHBAR_OFFSET_Y - 8 >= 0)
    {
        *y = barTop - TRIPLE_HEALTHBAR_OFFSET_Y;
        sTripleNameTagBelowBar &= ~(1u << battler);
    }
    else
    {
        *y = barTop + TRIPLE_NAME_TAG_BELOW_BAR;
        sTripleNameTagBelowBar |= 1u << battler;
    }
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
#define FLOOR_PLAYER_OVAL_PAD 34 // The player's bigger sprites need more: the front one stands clear of the tip
#define FLOOR_PLAYER_OVAL_THICKER 3
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
    u32 count;
    struct FloorOval ovals[TRIPLE_RANK_COUNT]; // Base, middle, outer (2v2: base, outer)
};

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
    // Spots from the rear to the front (see GetBattlerLayoutPosition)
    enum BattlerPosition spots[TRIPLE_RANK_COUNT];
    s32 x[TRIPLE_RANK_COUNT], y[TRIPLE_RANK_COUNT];
    u32 count = GetTripleScreenSpots(side, spots);

    team->count = count;
    for (u32 i = 0; i < count; i++)
    {
        x[i] = GetTripleSpotCoords(spots[i])->x;
        y[i] = GetTripleSpotCoords(spots[i])->y + ((side == B_SIDE_PLAYER) ? FLOOR_PLAYER_GROUND_OFFSET : FLOOR_GROUND_OFFSET);
    }

    s32 dx = x[count - 1] - x[0], dy = y[count - 1] - y[0];
    s32 length = FloorSqrt(dx * dx + dy * dy);
    team->cos = (dx * 256) / length;
    team->sin = (dy * 256) / length;
    team->yMin = (side == B_SIDE_PLAYER) ? FLOOR_TEAM_SPLIT_Y : 0;
    team->yMax = (side == B_SIDE_PLAYER) ? FLOOR_ROWS * 8 : FLOOR_TEAM_SPLIT_Y;

    for (u32 i = 0; i < count; i++)
    {
        // Distance along the diagonal from the rear Pokémon to this one. In 2v2 the base oval reaches partway to the
        // front Pokémon, standing in for the 3v3 middle section.
        s32 distance = ((x[i] - x[0]) * team->cos + (y[i] - y[0]) * team->sin) / 256;
        if (count == 2 && i == 0)
            distance = ((x[1] - x[0]) * team->cos + (y[1] - y[0]) * team->sin) / 256 * 2 / 5;
        team->ovals[i].cx = x[0] + (team->cos * distance) / 512;
        team->ovals[i].cy = y[0] + (team->sin * distance) / 512;
        team->ovals[i].rx = distance / 2 + ((side == B_SIDE_PLAYER) ? FLOOR_PLAYER_OVAL_PAD : FLOOR_OVAL_PAD);
        team->ovals[i].ry = 7 + 4 * i * (TRIPLE_RANK_COUNT - 1) / (count - 1) + ((side == B_SIDE_PLAYER) ? FLOOR_PLAYER_OVAL_THICKER : 0);
    }
}

// Whether a pixel's centre is in an oval (shrunk by shrinkX/shrinkY, in eighths of a pixel) is exact: no rounding
// before the comparison, so the edges step evenly. Along a row it's a quadratic in the pixel's x, so each row is
// worked out once per oval as the span of pixels inside it (testing each pixel took the floor ~8 seconds to draw):
//   (u / 512)^2 / (rx / 8)^2 + (v / 512)^2 / (ry / 8)^2 <= 1, with X, Y in half pixels from the centre and
//   u = X cos + Y sin, v = -X sin + Y cos (Q8), is u^2 ry^2 + v^2 rx^2 - rx^2 ry^2 4096 <= 0,
//   that is a X^2 + (b Y) X + (c Y^2 - k) <= 0, exact in 64 bits.
struct FloorOvalTest
{
    s64 a, b, c, k;
    s64 centerQ16; // Where the row's chord is centred, per Y: -b / 2a, Q16
    s64 cOverAQ16; // c / a, Q16
    s64 kOverA;
    s32 cx, cy;
    s32 yMin, yMax; // The rows it can reach, give or take
    bool32 empty;
};

static void InitFloorOvalTest(struct FloorOvalTest *test, const struct FloorTeam *team, const struct FloorOval *oval, s32 shrinkX, s32 shrinkY)
{
    s64 rx = oval->rx * 8 - shrinkX, ry = oval->ry * 8 - shrinkY; // Eighths of a pixel
    s64 cos = team->cos, sin = team->sin;

    test->cx = oval->cx;
    test->cy = oval->cy;
    test->empty = (rx <= 0 || ry <= 0);
    if (test->empty)
        return;
    rx *= rx;
    ry *= ry;
    test->a = cos * cos * ry + sin * sin * rx;
    test->b = 2 * cos * sin * (ry - rx);
    test->c = sin * sin * ry + cos * cos * rx;
    test->k = rx * ry * 4096;
    test->empty = (test->a <= 0);
    if (test->empty)
        return;
    test->centerQ16 = -(test->b * 65536) / (2 * test->a);
    test->cOverAQ16 = (test->c * 65536) / test->a;
    test->kOverA = test->k / test->a;
    // Half its height in pixels: sqrt((rx sin)^2 + (ry cos)^2), with room for the Q8 direction's rounding
    s32 height = FloorSqrt((u32)min((cos * cos * ry + sin * sin * rx) >> 22, (s64)UINT32_MAX));
    height += height / 16 + 2;
    test->yMin = oval->cy - height;
    test->yMax = oval->cy + height;
}

struct FloorQuadratic
{
    s64 a, b, c;
    s32 cx;
};

static inline bool32 IsInFloorQuadratic(const struct FloorQuadratic *q, s32 px)
{
    s64 x = px * 2 + 1 - q->cx * 2;
    return (q->a * x + q->b) * x + q->c <= 0;
}

// The pixels of a floor row inside an oval: [x0, x1)
struct FloorSpan
{
    s16 x0, x1;
};

static void GetFloorOvalSpan(const struct FloorOvalTest *test, s32 py, struct FloorSpan *span)
{
    s64 y = (py * 2 + 1) - test->cy * 2, center, d;
    struct FloorQuadratic q;
    s32 half, inside = -1, left, right;

    span->x0 = span->x1 = 0;
    if (test->empty || py < test->yMin || py > test->yMax)
        return;
    q.a = test->a;
    q.b = test->b * y;
    q.c = test->c * y * y - test->k;
    q.cx = test->cx;

    // The roots, roughly (X = center +- half), then made exact pixel by pixel
    center = (test->centerQ16 * y) >> 16;
    d = center * center - ((test->cOverAQ16 * y * y) >> 16) + test->kOverA;
    half = (d <= 0) ? 0 : FloorSqrt((d > (s64)UINT32_MAX) ? UINT32_MAX : (u32)d);
    // The lowest pixel of the (convex) quadratic is within a pixel of the rough centre's: outside, the row misses
    for (s32 i = -1; i <= 1 && inside < 0; i++)
    {
        s32 px = (s32)((center - 1 + test->cx * 2) >> 1) + i;
        px = (px < 0) ? 0 : (px >= DISPLAY_WIDTH) ? DISPLAY_WIDTH - 1 : px;
        if (IsInFloorQuadratic(&q, px))
            inside = px;
    }
    if (inside < 0)
        return;
    left = (s32)((center - half - 1 + test->cx * 2) >> 1);
    right = (s32)((center + half - 1 + test->cx * 2) >> 1);
    left = (left < 0) ? 0 : (left > inside) ? inside : left;
    right = (right >= DISPLAY_WIDTH) ? DISPLAY_WIDTH - 1 : (right < inside) ? inside : right;
    if (IsInFloorQuadratic(&q, left))
    {
        while (left > 0 && IsInFloorQuadratic(&q, left - 1))
            left--;
    }
    else
    {
        while (!IsInFloorQuadratic(&q, left))
            left++;
    }
    if (IsInFloorQuadratic(&q, right))
    {
        while (right < DISPLAY_WIDTH - 1 && IsInFloorQuadratic(&q, right + 1))
            right++;
    }
    else
    {
        while (!IsInFloorQuadratic(&q, right))
            right--;
    }
    span->x0 = left;
    span->x1 = right + 1;
}

// A team's ovals, ready to be cut into rows
struct FloorTeamTests
{
    struct FloorOvalTest ovals[TRIPLE_RANK_COUNT]; // Each oval
    struct FloorOvalTest inner[TRIPLE_RANK_COUNT]; // Each oval less its edge line (16x8 for the shading, 8x8 for the platform look)
    struct FloorOvalTest rim, edge;                // The outer oval less its rim, and less the line inside it too
};

#define FLOOR_RIM_Y 13 // Eighths of a pixel

static void InitFloorTeamTests(struct FloorTeamTests *tests, const struct FloorTeam *team, bool32 platformLook)
{
    const struct FloorOval *outer = &team->ovals[team->count - 1];
    s32 rimX = max(24, outer->rx * 16 / 25); // Eighths of a pixel

    for (u32 i = 0; i < team->count; i++)
    {
        InitFloorOvalTest(&tests->ovals[i], team, &team->ovals[i], 0, 0);
        if (platformLook)
            InitFloorOvalTest(&tests->inner[i], team, &team->ovals[i], 8, 8);
        else
            InitFloorOvalTest(&tests->inner[i], team, &team->ovals[i], 16, 8);
    }
    InitFloorOvalTest(&tests->rim, team, outer, rimX, FLOOR_RIM_Y);
    InitFloorOvalTest(&tests->edge, team, outer, rimX + 8, FLOOR_RIM_Y + 6);
}

static void FillFloorRow(u8 *values, s32 x0, s32 x1, u8 value)
{
    x0 = max(x0, 0);
    x1 = min(x1, DISPLAY_WIDTH);
    for (s32 x = x0; x < x1; x++)
        values[x] = value;
}

static void IntersectFloorSpan(struct FloorSpan *span, const struct FloorSpan *other)
{
    span->x0 = max(span->x0, other->x0);
    span->x1 = min(span->x1, other->x1);
    if (span->x1 < span->x0)
        span->x1 = span->x0;
}

enum
{
    FLOOR_ROLE_NONE,
    FLOOR_ROLE_FILL,
    FLOOR_ROLE_EDGE,
    FLOOR_ROLE_RIM,
};

// What each pixel of a row is: a FLOOR_ROLE_* in the platform look, else a FLOOR_* shade
static void GetFloorRowValues(const struct FloorTeam *teams, const struct FloorTeamTests *teamTests, s32 py, bool32 platformLook, u8 *values)
{
    const struct FloorTeam *team = NULL;
    const struct FloorTeamTests *tests = NULL;
    struct FloorSpan span, other;

    FillFloorRow(values, 0, DISPLAY_WIDTH, platformLook ? FLOOR_ROLE_NONE : FLOOR_NONE);
    for (u32 t = 0; t < NUM_BATTLE_SIDES; t++)
    {
        if (py >= teams[t].yMin && py < teams[t].yMax)
        {
            team = &teams[t];
            tests = &teamTests[t];
            break;
        }
    }
    if (team == NULL)
        return;

    if (platformLook)
    {
        // Outside the outer oval: nothing; then its rim, the line inside it, and the fill crossed by the inner ovals' lines
        GetFloorOvalSpan(&tests->ovals[team->count - 1], py, &span);
        FillFloorRow(values, span.x0, span.x1, FLOOR_ROLE_RIM);
        GetFloorOvalSpan(&tests->rim, py, &other);
        IntersectFloorSpan(&span, &other);
        FillFloorRow(values, span.x0, span.x1, FLOOR_ROLE_EDGE);
        GetFloorOvalSpan(&tests->edge, py, &other);
        IntersectFloorSpan(&span, &other);
        FillFloorRow(values, span.x0, span.x1, FLOOR_ROLE_FILL);
        for (s32 i = team->count - 2; i >= 0; i--)
        {
            struct FloorSpan oval = span, inner;
            GetFloorOvalSpan(&tests->ovals[i], py, &other);
            IntersectFloorSpan(&oval, &other);
            GetFloorOvalSpan(&tests->inner[i], py, &inner);
            if (oval.x0 == oval.x1)
                continue;
            if (inner.x0 == inner.x1)
            {
                FillFloorRow(values, oval.x0, oval.x1, FLOOR_ROLE_EDGE);
                continue;
            }
            FillFloorRow(values, oval.x0, min(inner.x0, oval.x1), FLOOR_ROLE_EDGE);
            FillFloorRow(values, max(inner.x1, oval.x0), oval.x1, FLOOR_ROLE_EDGE);
        }
    }
    else
    {
        // Ovals are nested: going in, each one's edge is its outline, and inside it the next one starts
        span.x0 = 0;
        span.x1 = DISPLAY_WIDTH;
        for (s32 i = team->count - 1; i >= 0 && span.x0 < span.x1; i--)
        {
            // Base under the rear Pokémon, the outer section pale; 3v3 has a half shade between them
            u32 shade = (i == 0) ? FLOOR_BASE : (i == (s32)team->count - 1) ? FLOOR_LIGHT : FLOOR_HALF;
            struct FloorSpan inner;
            GetFloorOvalSpan(&tests->ovals[i], py, &other);
            IntersectFloorSpan(&span, &other);
            if (span.x0 == span.x1)
                break;
            FillFloorRow(values, span.x0, span.x1, FLOOR_OUTLINE);
            GetFloorOvalSpan(&tests->inner[i], py, &inner);
            IntersectFloorSpan(&span, &inner);
            FillFloorRow(values, span.x0, span.x1, shade);
        }
    }
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
// slots the floor tiles don't use (TRIPLE_FLOOR_BG_PALETTE).
#define FLOOR_PALETTE TRIPLE_FLOOR_BG_PALETTE

// The oval colors come from the floor's average color: same hue, more saturated, at fixed brightness steps, from a
// dark base oval to a pale outer section. Out of 32: brightness, then saturation (32 = unchanged).
static const u8 sFloorShadeSteps[FLOOR_SHADE_COUNT][2] =
{
    [FLOOR_LIGHT]   = { 27, 56 },
    [FLOOR_HALF]    = { 20, 56 },
    [FLOOR_BASE]    = { 10, 40 },
    [FLOOR_OUTLINE] = { 7, 32 },
};

static u16 GetFloorShadeColor(u32 shade, s32 r, s32 g, s32 b)
{
    s32 mean = (r + g + b) / 3;
    s32 c[3] = { r, g, b };
    for (u32 i = 0; i < 3; i++)
    {
        c[i] = mean + (c[i] - mean) * sFloorShadeSteps[shade][1] / 32;
        c[i] = c[i] * sFloorShadeSteps[shade][0] / 32;
        c[i] = (c[i] < 0) ? 0 : (c[i] > 31) ? 31 : c[i];
    }
    return RGB(c[0], c[1], c[2]);
}

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
    u16 floorColors[FLOOR_SHADE_COUNT] = {0};
    if (count[main] != 0)
    {
        s32 r = sum[main][0] / count[main], g = sum[main][1] / count[main], b = sum[main][2] / count[main];
        sTripleHazeColor = RGB(r, g, b);
        for (u32 shade = FLOOR_LIGHT; shade < FLOOR_SHADE_COUNT; shade++)
            floorColors[shade] = GetFloorShadeColor(shade, r, g, b);
    }

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
            LoadPalette(&floorColors[shade], BG_PLTT_ID(FLOOR_PALETTE) + shades->index[shade], PLTT_SIZEOF(1));
    }

    for (u32 p = 0; p < 3; p++)
    {
        const u16 *colors = &gPlttBufferUnfaded[BG_PLTT_ID(2 + p)];
        for (u32 shade = 0; shade < FLOOR_SHADE_COUNT; shade++)
        {
            for (u32 i = 0; i < 16; i++)
            {
                s32 r = GET_R(floorColors[shade]), g = GET_G(floorColors[shade]), b = GET_B(floorColors[shade]);
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

// The ovals take the look of the environment's own 1v1 platforms: their fill with its dot texture, the rim and the
// line inside it, sampled from the opponent's platform (at the same place in every environment's background) as
// indices of its palette. The outer oval gets the rim and the line; the inner ovals' edges are lines on the fill.
#define PLATFORM_TEX_X   150 // A patch of the platform's middle: fill and dots
#define PLATFORM_TEX_Y   57
#define PLATFORM_TEX_W   48
#define PLATFORM_TEX_H   12
#define PLATFORM_RIM_X   113 // The platform's left rim, a few pixels wide
#define PLATFORM_RIM_W   5
#define PLATFORM_ROW_Y   60  // Rows through the platform's middle
#define PLATFORM_ROWS    6
#define PLATFORM_FLOOR_X 100 // The floor left of the platform

struct FloorStyle
{
    bool8 valid;
    u8 palette;
    u8 rim, edge;
    u8 tex[PLATFORM_TEX_H][PLATFORM_TEX_W];
    u8 map[3][16]; // Each index of the style's palette, as the closest color in environment palettes 2-4
};

static u32 GetFloorBgPixel(const u16 *tilemap, const u8 *tiles, u32 x, u32 y, u32 *palette)
{
    u16 entry = tilemap[(y / 8) * 32 + x / 8];
    u32 sx = x & 7, sy = y & 7;
    if (entry & 0x400)
        sx = 7 - sx;
    if (entry & 0x800)
        sy = 7 - sy;
    *palette = entry >> 12;
    u8 byte = tiles[(entry & 0x3FF) * TILE_SIZE_4BPP + sy * 4 + sx / 2];
    return (sx & 1) ? (byte >> 4) : (byte & 0xF);
}

static u32 GetMostCommonIndex(const u8 *counts)
{
    u32 best = 0;
    for (u32 i = 1; i < 16; i++)
    {
        if (counts[i] > counts[best])
            best = i;
    }
    return best;
}

static void BuildFloorStyle(struct FloorStyle *style, const u16 *tilemap, const u8 *tiles)
{
    u8 counts[16] = {0};
    u32 palette, p, fill, floor;
    bool32 samePalette = TRUE;

    style->valid = FALSE;
    style->palette = 0;

    // The rim: the most common color down the platform's left side
    for (u32 y = PLATFORM_ROW_Y; y < PLATFORM_ROW_Y + PLATFORM_ROWS; y++)
    {
        for (u32 x = PLATFORM_RIM_X; x < PLATFORM_RIM_X + PLATFORM_RIM_W; x++)
        {
            counts[GetFloorBgPixel(tilemap, tiles, x, y, &palette)]++;
            if (style->palette == 0)
                style->palette = palette;
            samePalette &= (palette == style->palette);
        }
    }
    style->rim = GetMostCommonIndex(counts);

    // The line inside it: the first other color going right from the rim, on the middle row
    style->edge = style->rim;
    for (u32 x = PLATFORM_RIM_X; x < PLATFORM_RIM_X + 24; x++)
    {
        u32 index = GetFloorBgPixel(tilemap, tiles, x, PLATFORM_ROW_Y + PLATFORM_ROWS / 2, &palette);
        if (index != style->rim)
        {
            style->edge = index;
            break;
        }
    }

    // The fill with its dots
    for (u32 i = 0; i < 16; i++)
        counts[i] = 0;
    for (u32 y = 0; y < PLATFORM_TEX_H; y++)
    {
        for (u32 x = 0; x < PLATFORM_TEX_W; x++)
        {
            style->tex[y][x] = GetFloorBgPixel(tilemap, tiles, PLATFORM_TEX_X + x, PLATFORM_TEX_Y + y, &palette);
            samePalette &= (palette == style->palette);
            if (counts[style->tex[y][x]] < 255)
                counts[style->tex[y][x]]++;
        }
    }
    fill = GetMostCommonIndex(counts);
    floor = GetFloorBgPixel(tilemap, tiles, PLATFORM_FLOOR_X, PLATFORM_ROW_Y + PLATFORM_ROWS / 2, &palette);

    // No platform where expected (or one made of several palettes): keep the plain shading
    if (!samePalette || style->palette < 2 || style->palette > 4 || style->rim == fill || style->edge == style->rim
     || (fill == floor && palette == style->palette))
        return;

    for (p = 0; p < 3; p++)
    {
        const u16 *from = &gPlttBufferUnfaded[BG_PLTT_ID(style->palette)];
        const u16 *to = &gPlttBufferUnfaded[BG_PLTT_ID(2 + p)];
        for (u32 i = 0; i < 16; i++)
        {
            u32 best = i, bestDistance = 0xFFFFFFFF;
            if (2 + p == style->palette)
            {
                style->map[p][i] = i;
                continue;
            }
            for (u32 j = 1; j < 16; j++)
            {
                s32 dr = GET_R(to[j]) - GET_R(from[i]), dg = GET_G(to[j]) - GET_G(from[i]), db = GET_B(to[j]) - GET_B(from[i]);
                u32 distance = dr * dr + dg * dg + db * db;
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    best = j;
                }
            }
            style->map[p][i] = best;
        }
    }
    style->valid = TRUE;
}

#define FLOOR_TILE_BUCKETS 64
#define FLOOR_NO_TILE 0xFFFF

// The floor's working memory, off the stack
struct FloorWork
{
    struct FloorShades shades;
    struct FloorStyle style;
    struct FloorTeamTests tests[NUM_BATTLE_SIDES];
    u32 hashes[FLOOR_MAX_TILES];          // Of the tiles made so far, to find identical ones quickly:
    u16 nextTile[FLOOR_MAX_TILES];        // chained by hash bucket
    u16 firstTile[FLOOR_TILE_BUCKETS];
    u8 values[8][DISPLAY_WIDTH] ALIGNED(4); // One row of tiles: GetFloorRowValues
};

// The floor is drawn again whenever the background is reloaded: after every move animation that changes it
// (restorebg), on the evolution scene, coming back from the bag or the party menu. When the background is the same and
// the floor's tiles are still in VRAM, the tilemap is all that has to be put back.
struct FloorCache
{
    u32 key;       // GetFloorCacheKey: the background the floor was drawn on
    u32 tilesHash; // The floor's own tiles
    u16 tileCount;
    bool16 hasPalette;
    u16 palette[16]; // FLOOR_PALETTE, when the floor uses it
    u16 hazeColor;
    u16 tilemap[FLOOR_ROWS][FLOOR_COLS];
};

static EWRAM_DATA struct FloorCache sFloorCache = {0};

static u32 HashFloorWords(u32 hash, const u32 *words, u32 count)
{
    for (u32 i = 0; i < count; i++)
        hash = (hash ^ words[i]) * 16777619;
    return hash;
}

static u32 GetFloorCacheKey(const u16 *tilemap, const u8 *tiles)
{
    u32 hash = 2166136261u ^ (HasTripleMiddleBattlers() ? 3 : 2);
    hash = HashFloorWords(hash, (const u32 *)tilemap, FLOOR_ROWS * 32 / 2);
    // The tiles it reads: those of the rows it's drawn on (other tiles may hold an animation's leftovers)
    for (u32 i = 0; i < FLOOR_ROWS * 32; i++)
        hash = HashFloorWords(hash, (const u32 *)(tiles + (tilemap[i] & 0x3FF) * TILE_SIZE_4BPP), TILE_SIZE_4BPP / 4);
    hash = HashFloorWords(hash, (const u32 *)&gPlttBufferUnfaded[BG_PLTT_ID(2)], 3 * 16 / 2);
    return hash;
}

static u32 GetFloorTilesHash(const u8 *tiles, u32 count)
{
    return HashFloorWords(2166136261u, (const u32 *)(tiles + FLOOR_FIRST_TILE * TILE_SIZE_4BPP), count * TILE_SIZE_4BPP / 4);
}

static bool32 TryRestoreFloorFromCache(u16 *tilemap, const u8 *tiles, u32 key)
{
    struct FloorCache *cache = &sFloorCache;
    if (cache->key != key || cache->tileCount == 0 || cache->tilesHash != GetFloorTilesHash(tiles, cache->tileCount))
        return FALSE;
    for (u32 row = 0; row < FLOOR_ROWS; row++)
        CpuCopy16(cache->tilemap[row], &tilemap[row * 32], sizeof(cache->tilemap[row]));
    if (cache->hasPalette)
        LoadPalette(cache->palette, BG_PLTT_ID(FLOOR_PALETTE), PLTT_SIZE_4BPP);
    sTripleHazeColor = cache->hazeColor;
    return TRUE;
}

static void SaveFloorCache(const u16 *tilemap, const u8 *tiles, u32 key, u32 tileCount, bool32 hasPalette)
{
    struct FloorCache *cache = &sFloorCache;
    cache->key = key;
    cache->tileCount = tileCount;
    cache->tilesHash = GetFloorTilesHash(tiles, tileCount);
    cache->hasPalette = hasPalette;
    if (hasPalette)
        CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(FLOOR_PALETTE)], cache->palette, sizeof(cache->palette));
    cache->hazeColor = sTripleHazeColor;
    for (u32 row = 0; row < FLOOR_ROWS; row++)
        CpuCopy16(&tilemap[row * 32], cache->tilemap[row], sizeof(cache->tilemap[row]));
}

void InvalidateTripleBattleFloorCache(void)
{
    sFloorCache.key = 0;
    sFloorCache.tileCount = 0;
}

void DrawTripleBattleFloor(void)
{
    struct FloorTeam teams[NUM_BATTLE_SIDES];
    u16 *tilemap = (u16 *)BG_SCREEN_ADDR(26);
    const u8 *tiles = (const u8 *)BG_CHAR_ADDR(2);
    u32 newTiles = 0, key = GetFloorCacheKey(tilemap, tiles);
    bool32 hasPalette = FALSE;

    if (TryRestoreFloorFromCache(tilemap, tiles, key))
        return;

    struct FloorWork *work = Alloc(sizeof(struct FloorWork));
    if (work == NULL)
        return;
    struct FloorShades *shades = &work->shades;
    struct FloorStyle *style = &work->style;
    InitFloorTeam(&teams[B_SIDE_PLAYER], B_SIDE_PLAYER);
    InitFloorTeam(&teams[B_SIDE_OPPONENT], B_SIDE_OPPONENT);
    BuildFloorShades(shades, tilemap, tiles);
    hasPalette = shades->reserved[0] || shades->reserved[1] || shades->reserved[2]; // It loaded FLOOR_PALETTE
    BuildFloorStyle(style, tilemap, tiles); // Before the tilemap is painted over: it reads the 1v1 platform
    InitFloorTeamTests(&work->tests[B_SIDE_PLAYER], &teams[B_SIDE_PLAYER], style->valid);
    InitFloorTeamTests(&work->tests[B_SIDE_OPPONENT], &teams[B_SIDE_OPPONENT], style->valid);
    for (u32 i = 0; i < FLOOR_TILE_BUCKETS; i++)
        work->firstTile[i] = FLOOR_NO_TILE;

    for (u32 row = 0; row < FLOOR_ROWS; row++)
    {
        // The plain background for this row, away from the 1v1 platforms
        u16 source = GetFloorSourceEntry(tilemap, row);
        u32 sourceTile = source & 0x3FF, palette = source >> 12;
        bool32 hFlip = (source & 0x400) != 0, vFlip = (source & 0x800) != 0;
        const u8 *texRows[8];
        u8 sourcePixels[8][8];

        // Nothing to paint on other palettes
        if (palette < 2 || palette > 4)
        {
            for (u32 col = 0; col < FLOOR_COLS; col++)
                tilemap[row * 32 + col] = source;
            continue;
        }
        for (u32 y = 0; y < 8; y++)
        {
            GetFloorRowValues(teams, work->tests, row * 8 + y, style->valid, work->values[y]);
            texRows[y] = style->tex[(row * 8 + y) % PLATFORM_TEX_H];
            for (u32 x = 0; x < 8; x++)
            {
                u32 sx = hFlip ? 7 - x : x, sy = vFlip ? 7 - y : y;
                u8 byte = tiles[sourceTile * TILE_SIZE_4BPP + sy * 4 + sx / 2];
                sourcePixels[y][x] = (sx & 1) ? (byte >> 4) : (byte & 0xF);
            }
        }

        for (u32 col = 0; col < FLOOR_COLS; col++)
        {
            u32 words[TILE_SIZE_4BPP / 4];
            u8 *pixels = (u8 *)words;
            u32 texCol = (col * 8) % PLATFORM_TEX_W, any = 0, y;

            // A tile with nothing on it (FLOOR_NONE, FLOOR_ROLE_NONE: 0) keeps the background's
            for (y = 0; y < 8; y++)
            {
                const u32 *values = (const u32 *)&work->values[y][col * 8];
                any |= values[0] | values[1];
            }
            if (any == 0 || newTiles == FLOOR_MAX_TILES)
            {
                tilemap[row * 32 + col] = source;
                continue;
            }

            for (y = 0; y < 8; y++)
            {
                const u8 *values = &work->values[y][col * 8];
                for (u32 x = 0; x < 8; x += 2)
                {
                    u32 color[2];
                    for (u32 i = 0; i < 2; i++)
                    {
                        u32 value = values[x + i];
                        color[i] = sourcePixels[y][x + i];
                        if (value == FLOOR_NONE) // FLOOR_ROLE_NONE too
                            continue;
                        if (style->valid)
                        {
                            u32 index;
                            if (value == FLOOR_ROLE_FILL)
                            {
                                u32 tx = texCol + x + i;
                                index = texRows[y][(tx >= PLATFORM_TEX_W) ? tx - PLATFORM_TEX_W : tx];
                            }
                            else if (value == FLOOR_ROLE_EDGE)
                            {
                                index = style->edge;
                            }
                            else
                            {
                                index = style->rim;
                            }
                            color[i] = style->map[palette - 2][index];
                        }
                        else if (shades->reserved[palette - 2])
                        {
                            color[i] = shades->index[value];
                        }
                        else
                        {
                            color[i] = shades->nearest[palette - 2][value][color[i]];
                        }
                    }
                    pixels[y * 4 + x / 2] = color[0] | (color[1] << 4);
                }
            }

            // Reuse an identical tile made earlier
            u32 tile, hash = HashFloorWords(2166136261u, words, ARRAY_COUNT(words)), bucket = hash % FLOOR_TILE_BUCKETS;
            for (tile = work->firstTile[bucket]; tile != FLOOR_NO_TILE; tile = work->nextTile[tile])
            {
                if (work->hashes[tile] != hash)
                    continue;
                const u32 *existing = (const u32 *)(tiles + (FLOOR_FIRST_TILE + tile) * TILE_SIZE_4BPP);
                u32 w;
                for (w = 0; w < ARRAY_COUNT(words) && existing[w] == words[w]; w++)
                    ;
                if (w == ARRAY_COUNT(words))
                    break;
            }
            if (tile == FLOOR_NO_TILE)
            {
                tile = newTiles++;
                CpuCopy32(words, (void *)(tiles + (FLOOR_FIRST_TILE + tile) * TILE_SIZE_4BPP), sizeof(words));
                work->hashes[tile] = hash;
                work->nextTile[tile] = work->firstTile[bucket];
                work->firstTile[bucket] = tile;
            }
            if (!style->valid && shades->reserved[palette - 2])
                tilemap[row * 32 + col] = (FLOOR_FIRST_TILE + tile) | (FLOOR_PALETTE << 12);
            else
                tilemap[row * 32 + col] = (FLOOR_FIRST_TILE + tile) | (palette << 12);
        }
    }
    Free(work);
    SaveFloorCache(tilemap, tiles, key, newTiles, hasPalette);
}
