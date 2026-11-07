#include "global.h"
#include "bg.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "quest_menu.h"
#include "quests.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/region_map_sections.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The start menu's QUESTS (docs/hack/quests.md): the act's quests on the left (the one under way first, then the
// finished ones, newest first), the chosen one's tasks and % on the right, and its next step along the bottom.
// A opens the Town Map with the current quest's place blinking; B goes back to the start menu.

enum
{
    WIN_HEADER,
    WIN_LIST,
    WIN_DETAIL,
    WIN_NEXT,
};

#define FRAME_TILES   0x1C0
#define PAL_BG        13
#define PAL_FRAME     14
#define PAL_TEXT      15

// PAL_TEXT's colours
#define COLOR_WHITE        1
#define COLOR_DARK         2
#define COLOR_DARK_SHADOW  3
#define COLOR_RED          4
#define COLOR_RED_SHADOW   5
#define COLOR_BLUE         6
#define COLOR_BLUE_SHADOW  7
#define COLOR_GRAY         8
#define COLOR_GRAY_SHADOW  9
#define COLOR_BAR          10
#define COLOR_BAR_SHADOW   11
#define COLOR_BAR_EMPTY    12
#define COLOR_HEADER       13
#define COLOR_HEADER_SHADOW 14
#define COLOR_PIN_DARK     15

static const u16 sTextPalette[16] =
{
    RGB(0, 0, 0),
    RGB(31, 31, 31),
    RGB(12, 12, 12),
    RGB(26, 26, 25),
    RGB(28, 1, 1),
    RGB(31, 23, 14),
    RGB(6, 10, 25),
    RGB(20, 24, 30),
    RGB(20, 20, 20),
    RGB(28, 28, 28),
    RGB(11, 25, 15),
    RGB(7, 20, 11),
    RGB(29, 29, 28),
    RGB(6, 8, 13),
    RGB(3, 4, 7),
    RGB(11, 2, 2),
};

// The background (BG1): a checkerboard of two light blues under a dark header bar
static const u16 sBgPalette[16] =
{
    RGB(0, 0, 0),
    RGB(22, 26, 30),
    RGB(20, 24, 29),
    RGB(6, 8, 13),
    RGB(3, 4, 7),
};

enum
{
    BG_TILE_LIGHT,
    BG_TILE_DARK,
    BG_TILE_HEADER,
    BG_TILE_LIGHT_SHADOW, // The header's shadow along the top
    BG_TILE_DARK_SHADOW,
    BG_TILE_COUNT,
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = { .bg = 0, .tilemapLeft = 0,  .tilemapTop = 0,  .width = 30, .height = 2,  .paletteNum = PAL_TEXT, .baseBlock = 1 },
    [WIN_LIST]   = { .bg = 0, .tilemapLeft = 1,  .tilemapTop = 3,  .width = 9,  .height = 12, .paletteNum = PAL_TEXT, .baseBlock = 61 },
    [WIN_DETAIL] = { .bg = 0, .tilemapLeft = 11, .tilemapTop = 3,  .width = 18, .height = 12, .paletteNum = PAL_TEXT, .baseBlock = 169 },
    [WIN_NEXT]   = { .bg = 0, .tilemapLeft = 1,  .tilemapTop = 17, .width = 28, .height = 2,  .paletteNum = PAL_TEXT, .baseBlock = 385 },
    DUMMY_WIN_TEMPLATE
};

// On the white windows the text's background is left as it is, so close rows don't clip each other
static const u8 sColorDark[]   = { TEXT_COLOR_TRANSPARENT, COLOR_DARK, COLOR_DARK_SHADOW };
static const u8 sColorBlue[]   = { TEXT_COLOR_TRANSPARENT, COLOR_BLUE, COLOR_BLUE_SHADOW };
static const u8 sColorRed[]    = { TEXT_COLOR_TRANSPARENT, COLOR_RED, COLOR_RED_SHADOW };
static const u8 sColorGray[]   = { TEXT_COLOR_TRANSPARENT, COLOR_GRAY, COLOR_GRAY_SHADOW };
static const u8 sColorHeader[] = { COLOR_HEADER, COLOR_WHITE, COLOR_HEADER_SHADOW };

static const u8 sText_Quests[] = _("QUESTS");
static const u8 sText_Hints[] = _("{A_BUTTON}MAP  {B_BUTTON}BACK");
static const u8 sText_Act[] = _("ACT ");
static const u8 sText_Percent[] = _("%");
static const u8 sText_Locked[] = _("? ? ?");
static const u8 sText_Cursor[] = _("▶");
static const u8 sText_Next[] = _("NEXT:");
static const u8 sText_Done[] = _("Done!");
static const u8 sText_Of[] = _("/");

static const u8 sText_UpArrow[] = _("{UP_ARROW}");
static const u8 sText_DownArrow[] = _("{DOWN_ARROW}");

#define MAX_ROWS     16
#define VISIBLE_ROWS 5
#define ROW_HEIGHT   12
#define LIST_TOP     21

struct QuestMenu
{
    u8 rows[MAX_ROWS]; // Quest indices in the list's order
    u8 numRows;        // Selectable rows
    bool8 moreLocked;  // A "? ? ?" row after them
    u16 *bgTilemap;
};

static EWRAM_DATA struct QuestMenu *sQuestMenu = NULL;
static EWRAM_DATA u8 sSelectedRow = 0; // Kept while the map is open
static EWRAM_DATA u8 sFirstVisibleRow = 0;

static void CB2_InitQuestMenu(void);
static void CB2_ReturnToQuestMenu(void);
static void Task_QuestMenuInput(u8 taskId);
static void Task_QuestMenuExit(u8 taskId);
static void Task_QuestMenuOpenMap(u8 taskId);

void CB2_OpenQuestMenu(void)
{
    sSelectedRow = 0;
    sFirstVisibleRow = 0;
    gMain.state = 0;
    SetMainCallback2(CB2_InitQuestMenu);
}

static void CB2_ReturnToQuestMenu(void)
{
    gMain.state = 0;
    SetMainCallback2(CB2_InitQuestMenu);
}

static void VBlankCB_QuestMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_QuestMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

// The list's order: the quest under way, then the finished ones (newest first), then "? ? ?" for the rest
static void BuildRows(void)
{
    s32 current = Quests_GetCurrentIndex(), i;
    u32 count = Quests_Count();
    struct QuestInfo info;

    sQuestMenu->numRows = 0;
    sQuestMenu->moreLocked = FALSE;
    if (current >= 0)
        sQuestMenu->rows[sQuestMenu->numRows++] = current;
    for (i = count - 1; i >= 0; i--)
    {
        Quests_Get(i, &info);
        if (info.state == QUEST_DONE && sQuestMenu->numRows < MAX_ROWS)
            sQuestMenu->rows[sQuestMenu->numRows++] = i;
        else if (info.state == QUEST_LOCKED)
            sQuestMenu->moreLocked = TRUE;
    }
    if (sSelectedRow >= sQuestMenu->numRows)
        sSelectedRow = 0;
}

// The list's rows, the "? ? ?" one included
static u32 CountListRows(void)
{
    return sQuestMenu->numRows + (sQuestMenu->moreLocked ? 1 : 0);
}

// Scrolls the list so the selected row shows, with the row after it when there is one
static void ScrollToSelected(void)
{
    u32 total = CountListRows();
    if (sSelectedRow < sFirstVisibleRow)
        sFirstVisibleRow = sSelectedRow;
    while (sSelectedRow + 1 >= sFirstVisibleRow + VISIBLE_ROWS && sFirstVisibleRow + VISIBLE_ROWS < total)
        sFirstVisibleRow++;
    if (sFirstVisibleRow + VISIBLE_ROWS > total)
        sFirstVisibleRow = total > VISIBLE_ROWS ? total - VISIBLE_ROWS : 0;
}

static void DrawBackground(void)
{
    u32 *tiles = AllocZeroed(BG_TILE_COUNT * TILE_SIZE_4BPP);
    u32 x, y, i;

    for (i = 0; i < TILE_SIZE_4BPP / 4; i++)
    {
        tiles[BG_TILE_LIGHT * 8 + i] = 0x11111111;
        tiles[BG_TILE_DARK * 8 + i] = 0x22222222;
        tiles[BG_TILE_HEADER * 8 + i] = 0x33333333;
        tiles[BG_TILE_LIGHT_SHADOW * 8 + i] = i == 0 ? 0x44444444 : 0x11111111;
        tiles[BG_TILE_DARK_SHADOW * 8 + i] = i == 0 ? 0x44444444 : 0x22222222;
    }
    LoadBgTiles(1, tiles, BG_TILE_COUNT * TILE_SIZE_4BPP, 0);
    Free(tiles);

    for (y = 0; y < 20; y++)
    {
        for (x = 0; x < 32; x++)
        {
            u32 tile;
            if (y < 2)
                tile = BG_TILE_HEADER;
            else if (y == 2)
                tile = (x + y) % 2 ? BG_TILE_DARK_SHADOW : BG_TILE_LIGHT_SHADOW;
            else
                tile = (x + y) % 2 ? BG_TILE_DARK : BG_TILE_LIGHT;
            sQuestMenu->bgTilemap[y * 32 + x] = tile | (PAL_BG << 12);
        }
    }
    CopyBgTilemapBufferToVram(1);
}

static void FillPixels(u32 windowId, u32 color, u32 x, u32 y, u32 width, u32 height)
{
    FillWindowPixelRect(windowId, PIXEL_FILL(color), x, y, width, height);
}

static void DrawBar(u32 windowId, u32 x, u32 y, u32 width, u32 height, u32 percent)
{
    u32 fill = (width - 2) * percent / 100;
    FillPixels(windowId, COLOR_DARK, x, y, width, height);
    FillPixels(windowId, COLOR_BAR_EMPTY, x + 1, y + 1, width - 2, height - 2);
    if (fill != 0)
    {
        FillPixels(windowId, COLOR_BAR, x + 1, y + 1, fill, height - 2);
        FillPixels(windowId, COLOR_BAR_SHADOW, x + 1, y + height - 2, fill, 1);
    }
}

static void DrawCheckbox(u32 windowId, u32 x, u32 y, bool32 checked)
{
    static const u8 sCheck[][2] = { {1, 4}, {2, 5}, {3, 6}, {4, 5}, {5, 4}, {6, 3}, {7, 2}, {2, 4}, {3, 5}, {4, 4}, {5, 3}, {6, 2} };
    u32 i;
    FillPixels(windowId, COLOR_DARK, x, y, 9, 9);
    FillPixels(windowId, COLOR_WHITE, x + 1, y + 1, 7, 7);
    if (checked)
    {
        for (i = 0; i < ARRAY_COUNT(sCheck); i++)
            FillPixels(windowId, COLOR_BAR_SHADOW, x + sCheck[i][0], y + sCheck[i][1], 1, 1);
    }
}

// A map pin, 7x9
static void DrawPin(u32 windowId, u32 x, u32 y)
{
    static const char sPin[9][8] = { "..XXX..", ".XoooX.", "XooWooX", "XoWWWoX", "XooWooX", ".XoooX.", "..XoX..", "...X...", "...X..." };
    u32 i, j;
    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < 7; i++)
        {
            if (sPin[j][i] == 'X')
                FillPixels(windowId, COLOR_PIN_DARK, x + i, y + j, 1, 1);
            else if (sPin[j][i] == 'o')
                FillPixels(windowId, COLOR_RED, x + i, y + j, 1, 1);
            else if (sPin[j][i] == 'W')
                FillPixels(windowId, COLOR_RED_SHADOW, x + i, y + j, 1, 1);
        }
    }
}

static u8 *AppendPercent(u8 *dest, u32 percent)
{
    dest = ConvertIntToDecimalStringN(dest, percent, STR_CONV_MODE_LEFT_ALIGN, 3);
    return StringCopy(dest, sText_Percent);
}

static void PrintRightAligned(u32 windowId, u32 fontId, u32 right, u32 y, const u8 *color, const u8 *str)
{
    AddTextPrinterParameterized3(windowId, fontId, right - GetStringWidth(fontId, str, 0), y, color, TEXT_SKIP_DRAW, str);
}

static void DrawHeader(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(COLOR_HEADER));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 4, 1, sColorHeader, TEXT_SKIP_DRAW, sText_Quests);
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 150, 1, sColorHeader, TEXT_SKIP_DRAW, sText_Hints);
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawList(void)
{
    u8 text[16];
    u32 percent = Quests_GetActPercent(), i, y;
    struct QuestInfo info;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(COLOR_WHITE));
    ConvertIntToDecimalStringN(StringCopy(text, sText_Act), Quests_GetActNumber(), STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, 0, sColorDark, TEXT_SKIP_DRAW, text);
    AppendPercent(text, percent);
    PrintRightAligned(WIN_LIST, FONT_SMALL, 72, 0, sColorDark, text);
    DrawBar(WIN_LIST, 0, 11, 72, 4, percent);
    FillPixels(WIN_LIST, COLOR_DARK_SHADOW, 0, 18, 72, 1);

    ScrollToSelected();
    y = LIST_TOP;
    for (i = sFirstVisibleRow; i < sQuestMenu->numRows && i < sFirstVisibleRow + VISIBLE_ROWS; i++, y += ROW_HEIGHT)
    {
        Quests_Get(sQuestMenu->rows[i], &info);
        if (i == sSelectedRow)
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sColorDark, TEXT_SKIP_DRAW, sText_Cursor);
        else if (info.state == QUEST_DONE)
            DrawCheckbox(WIN_LIST, 0, y + 2, TRUE);
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 12, y, info.state == QUEST_CURRENT ? sColorBlue : sColorDark,
                                     TEXT_SKIP_DRAW, info.listName);
    }
    if (sQuestMenu->moreLocked && i < sFirstVisibleRow + VISIBLE_ROWS)
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 12, y, sColorGray, TEXT_SKIP_DRAW, sText_Locked);
    // More rows above or below: arrows on the line after the rows
    if (sFirstVisibleRow > 0)
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 24, LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT, sColorGray, TEXT_SKIP_DRAW, sText_UpArrow);
    if (sFirstVisibleRow + VISIBLE_ROWS < CountListRows())
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 40, LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT, sColorGray, TEXT_SKIP_DRAW, sText_DownArrow);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void DrawTask(const struct QuestTask *task, u32 y)
{
    u8 counter[8];
    if (task->total > 1 && !task->done)
    {
        u8 *end = ConvertIntToDecimalStringN(counter, task->count, STR_CONV_MODE_LEFT_ALIGN, 1);
        end = StringCopy(end, sText_Of);
        ConvertIntToDecimalStringN(end, task->total, STR_CONV_MODE_LEFT_ALIGN, 1);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, y, sColorRed, TEXT_SKIP_DRAW, counter);
    }
    else
    {
        DrawCheckbox(WIN_DETAIL, 2, y + 2, task->done);
    }
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 17, y, task->done ? sColorGray : sColorDark, TEXT_SKIP_DRAW, task->text);
}

static void DrawDetail(const struct QuestInfo *info)
{
    u8 text[24];
    u32 percent = Quests_GetPercent(info), i;

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(COLOR_WHITE));
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 0, 0, sColorBlue, TEXT_SKIP_DRAW, info->title);
    AppendPercent(text, percent);
    PrintRightAligned(WIN_DETAIL, FONT_NORMAL, 144, 0, sColorDark, text);
    if (info->targetMapSec != MAPSEC_NONE)
    {
        DrawPin(WIN_DETAIL, 0, 16);
        GetMapNameGeneric(text, info->targetMapSec);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 10, 14, sColorDark, TEXT_SKIP_DRAW, text);
    }
    DrawBar(WIN_DETAIL, 0, 27, 144, 5, percent);
    for (i = 0; i < info->numTasks; i++)
        DrawTask(&info->tasks[i], 34 + i * 10);
    PutWindowTilemap(WIN_DETAIL);
    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static void DrawNext(const struct QuestInfo *info)
{
    const u8 *next = info->next != NULL ? info->next : sText_Done;
    u32 fontId = GetStringWidth(FONT_NORMAL, next, 0) <= 192 ? FONT_NORMAL : FONT_NARROW;

    FillWindowPixelBuffer(WIN_NEXT, PIXEL_FILL(COLOR_WHITE));
    AddTextPrinterParameterized3(WIN_NEXT, FONT_NORMAL, 0, 1, sColorRed, TEXT_SKIP_DRAW, sText_Next);
    AddTextPrinterParameterized3(WIN_NEXT, fontId, 32, 1, sColorDark, TEXT_SKIP_DRAW, next);
    PutWindowTilemap(WIN_NEXT);
    CopyWindowToVram(WIN_NEXT, COPYWIN_FULL);
}

static void DrawSelected(void)
{
    struct QuestInfo info;
    DrawList();
    if (sQuestMenu->numRows == 0)
        return;
    Quests_Get(sQuestMenu->rows[sSelectedRow], &info);
    DrawDetail(&info);
    DrawNext(&info);
}

static void CB2_InitQuestMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        sQuestMenu = AllocZeroed(sizeof(*sQuestMenu));
        sQuestMenu->bgTilemap = AllocZeroed(BG_SCREEN_SIZE);
        SetBgTilemapBuffer(1, sQuestMenu->bgTilemap);
        LoadPalette(sBgPalette, BG_PLTT_ID(PAL_BG), sizeof(sBgPalette));
        LoadPalette(sTextPalette, BG_PLTT_ID(PAL_TEXT), sizeof(sTextPalette));
        LoadUserWindowBorderGfx(WIN_LIST, FRAME_TILES, BG_PLTT_ID(PAL_FRAME));
        DrawBackground();
        gMain.state++;
        break;
    case 2:
        BuildRows();
        DrawHeader();
        DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, FRAME_TILES, PAL_FRAME);
        DrawStdFrameWithCustomTileAndPalette(WIN_DETAIL, FALSE, FRAME_TILES, PAL_FRAME);
        DrawStdFrameWithCustomTileAndPalette(WIN_NEXT, FALSE, FRAME_TILES, PAL_FRAME);
        DrawSelected();
        CopyBgTilemapBufferToVram(0);
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_QuestMenu);
        CreateTask(Task_QuestMenuInput, 0);
        SetMainCallback2(CB2_QuestMenu);
        break;
    }
}

static void FreeQuestMenu(void)
{
    FreeAllWindowBuffers();
    UnsetBgTilemapBuffer(1);
    TRY_FREE_AND_SET_NULL(sQuestMenu->bgTilemap);
    TRY_FREE_AND_SET_NULL(sQuestMenu);
}

static void Task_QuestMenuInput(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(DPAD_UP) && sSelectedRow > 0)
    {
        PlaySE(SE_SELECT);
        sSelectedRow--;
        DrawSelected();
    }
    else if (JOY_NEW(DPAD_DOWN) && sSelectedRow + 1 < sQuestMenu->numRows)
    {
        PlaySE(SE_SELECT);
        sSelectedRow++;
        DrawSelected();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_QuestMenuOpenMap;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_QuestMenuExit;
    }
}

static void Task_QuestMenuExit(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    FreeQuestMenu();
    DestroyTask(taskId);
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}

static void Task_QuestMenuOpenMap(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    FreeQuestMenu();
    DestroyTask(taskId);
    FieldInitRegionMapFromQuests(CB2_ReturnToQuestMenu);
}
