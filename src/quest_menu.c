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
#include "tm_research.h"
#include "item.h"
#include "move.h"
#include "data.h"
#include "battle_main.h"
#include "window.h"
#include "constants/region_map_sections.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The start menu's QUESTS (docs/hack/quests.md): the act's quests on the left (the one under way first, then the
// finished ones, newest first), the chosen one's tasks and % on the right, and its next step along the bottom.
// A opens the Town Map with the current quest's place blinking; B goes back to the start menu.
// Once the TM Database is open (docs/hack/tm_database.md), L/R switch to the TM RESEARCH tab: the 3 research slots
// and the database's TMs, in the same layout. The institute's TM MACHINE opens that tab directly
// (TmDatabase_OpenFromMachine).

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

static const u8 sText_TabQuests[] = _("QUESTS");
static const u8 sText_TabTm[] = _("TM RESEARCH");
static const u8 sText_TmDatabase[] = _("TM DATABASE");
static const u8 sText_TabKeys[] = _("{L_BUTTON}{R_BUTTON}");
static const u8 sText_HintsTm[] = _("{A_BUTTON}OK  {B_BUTTON}BACK");

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
    u8 tms[NUM_TECHNICAL_MACHINES]; // The browse list (TM numbers), for the filter
    u8 numTms;
};

static EWRAM_DATA struct QuestMenu *sQuestMenu = NULL;
static EWRAM_DATA u8 sSelectedRow = 0; // Kept while the map is open
static EWRAM_DATA u8 sFirstVisibleRow = 0;

enum { TAB_QUESTS, TAB_TM };
enum { TM_VIEW_SLOTS, TM_VIEW_BROWSE };
enum
{
    CONFIRM_NONE,
    CONFIRM_START,       // Research the chosen TM
    CONFIRM_CRAFT,       // Craft a ready slot's TM
    CONFIRM_CLEAR,       // Stop a slot's research
    CONFIRM_CRAFT_READY, // Craft a TM of a type whose Pokémon are all caught
};

#define TM_ROW_ALL       TM_RESEARCH_SLOTS // The slots view's last row: browse every TM
#define TM_ROWS          (TM_RESEARCH_SLOTS + 1)
#define NO_SLOT          0xFF
#define FILTER_ALL       NUMBER_OF_MON_TYPES

static EWRAM_DATA u8 sTab = 0;
static EWRAM_DATA u8 sTmView = 0;
static EWRAM_DATA u8 sTmRow = 0;          // The slots view's row
static EWRAM_DATA u8 sBrowseRow = 0;
static EWRAM_DATA u8 sBrowseFirst = 0;    // The browse list's first visible row
static EWRAM_DATA u8 sBrowseFilter = 0;   // A type, or FILTER_ALL
static EWRAM_DATA u8 sBrowseSlot = 0;     // The slot a TM picked in the browse list goes to (NO_SLOT: the first free one)
static EWRAM_DATA u8 sConfirm = 0;
static EWRAM_DATA bool8 sFromMachine = FALSE;
static EWRAM_DATA const u8 *sNotice = NULL; // Shown along the bottom until the next key

static void CB2_InitQuestMenu(void);
static void CB2_ReturnToQuestMenu(void);
static void Task_QuestMenuInput(u8 taskId);
static void Task_QuestMenuExit(u8 taskId);
static void Task_QuestMenuOpenMap(u8 taskId);

void CB2_OpenQuestMenu(void)
{
    sSelectedRow = 0;
    sFirstVisibleRow = 0;
    sTab = TAB_QUESTS;
    sFromMachine = FALSE;
    sConfirm = CONFIRM_NONE;
    sNotice = NULL;
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

static const u8 sColorHeaderDim[] = { COLOR_HEADER, COLOR_GRAY, COLOR_HEADER_SHADOW };

// The header: QUESTS and TM RESEARCH as tabs once the database is open (L/R switch), or TM DATABASE from the machine
// and while browsing
static void DrawHeader(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(COLOR_HEADER));
    if (sFromMachine || (sTab == TAB_TM && sTmView == TM_VIEW_BROWSE))
    {
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 4, 1, sColorHeader, TEXT_SKIP_DRAW, sText_TmDatabase);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 160, 1, sColorHeader, TEXT_SKIP_DRAW, sText_HintsTm);
    }
    else if (TmResearch_IsUnlocked())
    {
        AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 2, 2, sColorHeaderDim, TEXT_SKIP_DRAW, sText_TabKeys);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 38, 1, sTab == TAB_QUESTS ? sColorHeader : sColorHeaderDim,
                                     TEXT_SKIP_DRAW, sText_TabQuests);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 84, 1, sTab == TAB_TM ? sColorHeader : sColorHeaderDim,
                                     TEXT_SKIP_DRAW, sText_TabTm);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 160, 1, sColorHeader, TEXT_SKIP_DRAW,
                                     sTab == TAB_QUESTS ? sText_Hints : sText_HintsTm);
    }
    else
    {
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 4, 1, sColorHeader, TEXT_SKIP_DRAW, sText_Quests);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 150, 1, sColorHeader, TEXT_SKIP_DRAW, sText_Hints);
    }
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

static void DrawTmSelected(void);

static void DrawSelected(void)
{
    struct QuestInfo info;
    if (sTab == TAB_TM)
    {
        DrawTmSelected();
        return;
    }
    DrawList();
    if (sQuestMenu->numRows == 0)
        return;
    Quests_Get(sQuestMenu->rows[sSelectedRow], &info);
    DrawDetail(&info);
    DrawNext(&info);
}

// ---------------------------------------------------------------------------------------------------------------
// TM RESEARCH (docs/hack/tm_database.md): the slots view (the 3 slots and ALL TMs) and the browse list

static const u8 sText_TmCount[] = _("TMs");
static const u8 sText_EmptySlot[] = _("Empty slot");
static const u8 sText_AllTms[] = _("ALL TMs");
static const u8 sText_AllTypes[] = _("ALL");
static const u8 sText_Tm[] = _("TM");
static const u8 sText_Tier[] = _("TIER ");
static const u8 sText_Pow[] = _("POW ");
static const u8 sText_PowVaries[] = _("POW ---");
static const u8 sText_Status[] = _("STATUS");
static const u8 sText_Gap[] = _("  ");
static const u8 sText_Catch[] = _("Catch ");
static const u8 sText_Record[] = _("Record ");
static const u8 sText_PokemonSuffix[] = _(" POKéMON");
static const u8 sText_MovesSuffix[] = _(" moves");
static const u8 sText_CaughtList[] = _("Caught: ");
static const u8 sText_MovesList[] = _("Moves: ");
static const u8 sText_ListSep[] = _(", ");
static const u8 sText_More[] = _(" +");
static const u8 sText_TypeCaught[] = _(": ");
static const u8 sText_AllCaughtSuffix[] = _(" TMs unlocked!");
static const u8 sText_All[] = _("All ");
static const u8 sText_Owned[] = _("Owned.");
static const u8 sText_Tracked[] = _("Being researched.");
static const u8 sText_ReadyAllCaught[] = _("Ready to craft!");
static const u8 sText_TypeTaken[] = _("Its type's slot is taken.");
static const u8 sText_NoSlot[] = _("All 3 slots are in use.");
static const u8 sText_ReadyToCraft[] = _("Ready to craft!");
static const u8 sText_EmptyHelp1[] = _("Pick a TM to research, then");
static const u8 sText_EmptyHelp2[] = _("catch POKéMON and record");
static const u8 sText_EmptyHelp3[] = _("moves of its type in battle.");
static const u8 sText_EmptyHelp4[] = _("One TM per type at a time.");
static const u8 sText_SlotsUsed[] = _("Slots in use: ");
static const u8 sText_OwnedOf[] = _("TMs owned: ");
static const u8 sText_DatabaseHelp[] = _("Stronger moves need more.");
static const u8 sText_CatchAllHelp1[] = _("Catch 25 POKéMON of a type:");
static const u8 sText_CatchAllHelp2[] = _("all its TMs unlock!");

static const u8 sText_KeysEmpty[] = _("{A_BUTTON} Browse TMs");
static const u8 sText_KeysSlot[] = _("{SELECT_BUTTON} Clear slot");
static const u8 sText_KeysReady[] = _("{A_BUTTON} Craft TM   {SELECT_BUTTON} Clear slot");
static const u8 sText_KeysAll[] = _("{A_BUTTON} Browse every TM");
static const u8 sText_KeysBrowse[] = _("{A_BUTTON} Research   {L_BUTTON}{R_BUTTON} Type");
static const u8 sText_KeysBrowseCraft[] = _("{A_BUTTON} Craft TM   {L_BUTTON}{R_BUTTON} Type");
static const u8 sText_KeysBrowseView[] = _("{L_BUTTON}{R_BUTTON} Type");
static const u8 sText_ConfirmStart[] = _("Research {STR_VAR_1}?  {A_BUTTON}YES  {B_BUTTON}NO");
static const u8 sText_ConfirmCraft[] = _("Craft {STR_VAR_1}?  {A_BUTTON}YES  {B_BUTTON}NO");
static const u8 sText_ConfirmClear[] = _("Clear {STR_VAR_1}? Progress is lost. {A_BUTTON}YES {B_BUTTON}NO");
static const u8 sText_NoticeStarted[] = _("Research started! Good luck.");
static const u8 sText_NoticeCrafted[] = _("You made the TM! It's in your BAG.");
static const u8 sText_NoticeCleared[] = _("The slot is free again.");
static const u8 sText_NoticeNotReady[] = _("Not done yet: keep researching.");
static const u8 sText_NoticeOwned[] = _("You already have that TM.");
static const u8 sText_NoticeTaken[] = _("Clear its type's slot first.");
static const u8 sText_NoticeNoSlot[] = _("Clear a slot in TM RESEARCH first.");
static const u8 sText_NoticeBagFull[] = _("There's no room in the BAG.");

static const u8 sColorReady[] = { TEXT_COLOR_TRANSPARENT, COLOR_RED, COLOR_RED_SHADOW };

// "TM35 FLAMETHROWER"
static u8 *AppendTmName(u8 *dest, u32 tm)
{
    dest = StringCopy(dest, sText_Tm);
    dest = ConvertIntToDecimalStringN(dest, tm, STR_CONV_MODE_LEADING_ZEROS, 2);
    *dest++ = CHAR_SPACE;
    return StringCopy(dest, GetMoveName(TmResearch_GetMove(tm)));
}

// The narrow font when the normal one doesn't fit
static u32 FitFont(u32 fontId, u32 narrowId, const u8 *str, u32 width)
{
    return GetStringWidth(fontId, str, 0) <= width ? fontId : narrowId;
}

static void BuildTmList(void)
{
    u32 tm;
    sQuestMenu->numTms = 0;
    for (tm = 1; tm <= NUM_TECHNICAL_MACHINES; tm++)
    {
        if (TmResearch_IsResearchable(tm) && (sBrowseFilter == FILTER_ALL || TmResearch_GetType(tm) == sBrowseFilter))
            sQuestMenu->tms[sQuestMenu->numTms++] = tm;
    }
    if (sBrowseRow >= sQuestMenu->numTms)
        sBrowseRow = 0;
}

static bool32 TypeHasTms(u32 type)
{
    u32 tm;
    for (tm = 1; tm <= NUM_TECHNICAL_MACHINES; tm++)
    {
        if (TmResearch_IsResearchable(tm) && TmResearch_GetType(tm) == type)
            return TRUE;
    }
    return FALSE;
}

// L/R in the browse list: ALL, then each type that has TMs
static void StepFilter(s32 step)
{
    u32 filter = sBrowseFilter, i;
    for (i = 0; i <= NUMBER_OF_MON_TYPES; i++)
    {
        filter = (filter + NUMBER_OF_MON_TYPES + 1 + step) % (NUMBER_OF_MON_TYPES + 1);
        if (filter == FILTER_ALL || TypeHasTms(filter))
            break;
    }
    sBrowseFilter = filter;
    sBrowseRow = 0;
    sBrowseFirst = 0;
    BuildTmList();
}

static void ScrollBrowseToSelected(void)
{
    if (sBrowseRow < sBrowseFirst)
        sBrowseFirst = sBrowseRow;
    while (sBrowseRow + 1 >= sBrowseFirst + VISIBLE_ROWS && sBrowseFirst + VISIBLE_ROWS < sQuestMenu->numTms)
        sBrowseFirst++;
    if (sBrowseFirst + VISIBLE_ROWS > sQuestMenu->numTms)
        sBrowseFirst = sQuestMenu->numTms > VISIBLE_ROWS ? sQuestMenu->numTms - VISIBLE_ROWS : 0;
}

// The list's top: a label, "owned/total" and a bar
static void DrawListTop(const u8 *label, u32 owned, u32 total)
{
    u8 text[16], *end;
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(COLOR_WHITE));
    AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, 0, sColorDark, TEXT_SKIP_DRAW, label);
    end = ConvertIntToDecimalStringN(text, owned, STR_CONV_MODE_LEFT_ALIGN, 2);
    end = StringCopy(end, sText_Of);
    ConvertIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 2);
    PrintRightAligned(WIN_LIST, FONT_SMALL, 71, 0, sColorDark, text);
    DrawBar(WIN_LIST, 0, 11, 72, 4, total == 0 ? 0 : owned * 100 / total);
    FillPixels(WIN_LIST, COLOR_DARK_SHADOW, 0, 18, 72, 1);
}

static void PrintListRow(u32 y, bool32 selected, const u8 *color, const u8 *text)
{
    if (selected)
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 0, y, sColorDark, TEXT_SKIP_DRAW, sText_Cursor);
    AddTextPrinterParameterized3(WIN_LIST, FitFont(FONT_SMALL, FONT_SMALL_NARROW, text, 60), 12, y, color, TEXT_SKIP_DRAW, text);
}

static void DrawSlotsList(void)
{
    u32 i, y = LIST_TOP;
    DrawListTop(sText_TmCount, TmResearch_CountOwned(), TmResearch_CountResearchable());
    for (i = 0; i < TM_RESEARCH_SLOTS; i++, y += ROW_HEIGHT)
    {
        u32 tm = TmResearch_GetSlotTm(i);
        if (tm == 0)
            PrintListRow(y, sTmRow == i, sColorGray, sText_EmptySlot);
        else
            PrintListRow(y, sTmRow == i, TmResearch_IsSlotReady(i) ? sColorReady : sColorBlue, GetMoveName(TmResearch_GetMove(tm)));
    }
    FillPixels(WIN_LIST, COLOR_GRAY_SHADOW, 0, y + 1, 72, 1);
    PrintListRow(y + 4, sTmRow == TM_ROW_ALL, sColorDark, sText_AllTms);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void DrawBrowseList(void)
{
    u8 label[16];
    u32 i, y = LIST_TOP, owned = 0;
    for (i = 0; i < sQuestMenu->numTms; i++)
        owned += TmResearch_IsOwned(sQuestMenu->tms[i]);
    if (sBrowseFilter == FILTER_ALL)
        StringCopy(label, sText_AllTypes);
    else
        TmResearch_GetTypeName(label, sBrowseFilter);
    DrawListTop(label, owned, sQuestMenu->numTms);
    ScrollBrowseToSelected();
    for (i = sBrowseFirst; i < sQuestMenu->numTms && i < sBrowseFirst + VISIBLE_ROWS; i++, y += ROW_HEIGHT)
    {
        u32 tm = sQuestMenu->tms[i];
        const u8 *color;
        switch (TmResearch_GetStatus(tm))
        {
        case TM_STATUS_OWNED:
            if (i != sBrowseRow)
                DrawCheckbox(WIN_LIST, 0, y + 2, TRUE);
            color = sColorGray;
            break;
        case TM_STATUS_TRACKED:
            color = sColorBlue;
            break;
        case TM_STATUS_READY:
            color = sColorReady;
            break;
        case TM_STATUS_AVAILABLE:
            color = sColorDark;
            break;
        default:
            color = sColorGray;
            break;
        }
        PrintListRow(y, i == sBrowseRow, color, GetMoveName(TmResearch_GetMove(tm)));
    }
    if (sBrowseFirst > 0)
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 24, LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT, sColorGray, TEXT_SKIP_DRAW, sText_UpArrow);
    if (sBrowseFirst + VISIBLE_ROWS < sQuestMenu->numTms)
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 40, LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT, sColorGray, TEXT_SKIP_DRAW, sText_DownArrow);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

// "FIRE  POW 90  TIER 2"
static void DrawTmFacts(u32 tm, u32 y)
{
    u8 text[40], *end;
    u32 move = TmResearch_GetMove(tm);
    end = TmResearch_GetTypeName(text, TmResearch_GetType(tm));
    end = StringCopy(end, sText_Gap);
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        end = StringCopy(end, sText_Status);
    else if (GetMovePower(move) <= 1)
        end = StringCopy(end, sText_PowVaries);
    else
        end = ConvertIntToDecimalStringN(StringCopy(end, sText_Pow), GetMovePower(move), STR_CONV_MODE_LEFT_ALIGN, 3);
    end = StringCopy(end, sText_Gap);
    ConvertIntToDecimalStringN(StringCopy(end, sText_Tier), TmResearch_GetTier(tm), STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, y, sColorDark, TEXT_SKIP_DRAW, text);
}

// "Catch 3 FIRE POKéMON" / "Record 3 FIRE moves"
static void BufferRequirement(u8 *dest, u32 tm, const u8 *verb, const u8 *suffix)
{
    dest = StringCopy(dest, verb);
    dest = ConvertIntToDecimalStringN(dest, TmResearch_GetRequirement(tm), STR_CONV_MODE_LEFT_ALIGN, 1);
    *dest++ = CHAR_SPACE;
    dest = TmResearch_GetTypeName(dest, TmResearch_GetType(tm));
    StringCopy(dest, suffix);
}

// "All FIRE POKéMON: 12/64" (in the POKéDEX): the way to every FIRE TM at once
static void DrawTypeCaught(enum Type type, u32 y)
{
    u8 text[48], *end;
    u32 total, caught = TmResearch_CountTypeCaught(type, &total);
    if (caught >= total && total != 0)
    {
        end = StringCopy(text, sText_All);
        end = TmResearch_GetTypeName(end, type);
        StringCopy(end, sText_AllCaughtSuffix);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, y, sColorReady, TEXT_SKIP_DRAW, text);
        return;
    }
    end = StringCopy(text, sText_All);
    end = TmResearch_GetTypeName(end, type);
    end = StringCopy(end, sText_PokemonSuffix);
    end = StringCopy(end, sText_TypeCaught);
    end = ConvertIntToDecimalStringN(end, caught, STR_CONV_MODE_LEFT_ALIGN, 3);
    end = StringCopy(end, sText_Of);
    ConvertIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_DETAIL, FitFont(FONT_SMALL, FONT_SMALL_NARROW, text, 144), 0, y, sColorGray, TEXT_SKIP_DRAW, text);
}

// "Caught: VULPIX, PONYTA +1": the names that fit in the pane
static void DrawNameList(const u8 *label, const u16 *list, bool32 species, u32 y)
{
    u8 text[96], *end = StringCopy(text, label);
    u32 i, count = 0, shown = 0;
    for (i = 0; i < TM_RESEARCH_MAX_COUNT; i++)
        count += list[i] != 0;
    if (count == 0)
        return;
    for (i = 0; i < TM_RESEARCH_MAX_COUNT && list[i] != 0; i++)
    {
        u8 *before = end;
        if (shown > 0)
            end = StringCopy(end, sText_ListSep);
        end = StringCopy(end, species ? GetSpeciesName(list[i]) : GetMoveName(list[i]));
        if (GetStringWidth(FONT_SMALL_NARROW, text, 0) > (count > shown + 1 ? 130 : 144))
        {
            end = before;
            *end = EOS;
            break;
        }
        shown++;
    }
    if (shown < count)
        ConvertIntToDecimalStringN(StringCopy(end, sText_More), count - shown, STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL_NARROW, 0, y, sColorGray, TEXT_SKIP_DRAW, text);
}

// A requirement as a task: a counter while research runs, else a plain box
static void DrawRequirementTask(u8 *buffer, u32 tm, const u8 *verb, const u8 *suffix, u32 count, bool32 counting, u32 y)
{
    struct QuestTask task;
    u32 need = TmResearch_GetRequirement(tm);
    BufferRequirement(buffer, tm, verb, suffix);
    task.text = buffer;
    task.count = counting ? min(count, need) : 0;
    task.total = counting ? need : 1;
    task.done = counting && count >= need;
    DrawTask(&task, y);
}

static void DrawSlotDetail(u32 slot)
{
    u8 title[32], catchText[40], recordText[40];
    u32 tm = TmResearch_GetSlotTm(slot), need, caught, moves, percent;
    bool32 ready;
    const struct TmResearchSlot *data = &gSaveBlock3Ptr->tmResearch[slot];

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(COLOR_WHITE));
    if (tm == 0)
    {
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 0, 0, sColorGray, TEXT_SKIP_DRAW, sText_EmptySlot);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 20, sColorDark, TEXT_SKIP_DRAW, sText_EmptyHelp1);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 32, sColorDark, TEXT_SKIP_DRAW, sText_EmptyHelp2);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 44, sColorDark, TEXT_SKIP_DRAW, sText_EmptyHelp3);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 62, sColorGray, TEXT_SKIP_DRAW, sText_EmptyHelp4);
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 74, sColorGray, TEXT_SKIP_DRAW, sText_DatabaseHelp);
        PutWindowTilemap(WIN_DETAIL);
        CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
        return;
    }
    need = TmResearch_GetRequirement(tm);
    ready = TmResearch_IsSlotReady(slot);
    caught = TmResearch_GetCaughtCount(slot);
    moves = TmResearch_GetMoveCount(slot);
    percent = ready ? 100 : (min(caught, need) + min(moves, need)) * 100 / (need * 2);

    AppendTmName(title, tm);
    AddTextPrinterParameterized3(WIN_DETAIL, FitFont(FONT_NORMAL, FONT_NARROW, title, 116), 0, 0, sColorBlue, TEXT_SKIP_DRAW, title);
    AppendPercent(catchText, percent);
    PrintRightAligned(WIN_DETAIL, FONT_NORMAL, 144, 0, sColorDark, catchText);
    DrawTmFacts(tm, 14);
    DrawBar(WIN_DETAIL, 0, 27, 144, 5, percent);
    DrawRequirementTask(catchText, tm, sText_Catch, sText_PokemonSuffix, ready ? need : caught, TRUE, 34);
    DrawRequirementTask(recordText, tm, sText_Record, sText_MovesSuffix, ready ? need : moves, TRUE, 44);
    DrawNameList(sText_CaughtList, data->species, TRUE, 56);
    DrawNameList(sText_MovesList, data->moves, FALSE, 66);
    if (ready)
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 0, 80, sColorReady, TEXT_SKIP_DRAW, sText_ReadyToCraft);
    else
        DrawTypeCaught(TmResearch_GetType(tm), 82);
    PutWindowTilemap(WIN_DETAIL);
    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static void DrawDatabaseDetail(void)
{
    u8 text[32], *end;
    u32 owned = TmResearch_CountOwned(), total = TmResearch_CountResearchable(), used = 0, i;

    for (i = 0; i < TM_RESEARCH_SLOTS; i++)
        used += TmResearch_GetSlotTm(i) != 0;
    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(COLOR_WHITE));
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 0, 0, sColorBlue, TEXT_SKIP_DRAW, sText_TmDatabase);
    AppendPercent(text, owned * 100 / total);
    PrintRightAligned(WIN_DETAIL, FONT_NORMAL, 144, 0, sColorDark, text);
    end = ConvertIntToDecimalStringN(StringCopy(text, sText_OwnedOf), owned, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(StringCopy(end, sText_Of), total, STR_CONV_MODE_LEFT_ALIGN, 2);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 14, sColorDark, TEXT_SKIP_DRAW, text);
    DrawBar(WIN_DETAIL, 0, 27, 144, 5, owned * 100 / total);
    end = ConvertIntToDecimalStringN(StringCopy(text, sText_SlotsUsed), used, STR_CONV_MODE_LEFT_ALIGN, 1);
    ConvertIntToDecimalStringN(StringCopy(end, sText_Of), TM_RESEARCH_SLOTS, STR_CONV_MODE_LEFT_ALIGN, 1);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 36, sColorDark, TEXT_SKIP_DRAW, text);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 50, sColorGray, TEXT_SKIP_DRAW, sText_EmptyHelp4);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 62, sColorGray, TEXT_SKIP_DRAW, sText_DatabaseHelp);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 74, sColorGray, TEXT_SKIP_DRAW, sText_CatchAllHelp1);
    AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 0, 85, sColorGray, TEXT_SKIP_DRAW, sText_CatchAllHelp2);
    PutWindowTilemap(WIN_DETAIL);
    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static void DrawBrowseDetail(u32 tm)
{
    u8 title[32], catchText[40], recordText[40];
    enum TmResearchStatus status = TmResearch_GetStatus(tm);
    s32 slot = TmResearch_FindSlotOfType(TmResearch_GetType(tm));
    bool32 tracked = status == TM_STATUS_TRACKED;
    const u8 *line = NULL, *color = sColorGray;

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(COLOR_WHITE));
    AppendTmName(title, tm);
    AddTextPrinterParameterized3(WIN_DETAIL, FitFont(FONT_NORMAL, FONT_NARROW, title, 144), 0, 0,
                                 status == TM_STATUS_OWNED ? sColorGray : sColorBlue, TEXT_SKIP_DRAW, title);
    DrawTmFacts(tm, 14);
    FillPixels(WIN_DETAIL, COLOR_GRAY_SHADOW, 0, 28, 144, 1);
    DrawRequirementTask(catchText, tm, sText_Catch, sText_PokemonSuffix, tracked ? TmResearch_GetCaughtCount(slot) : 0, tracked, 32);
    DrawRequirementTask(recordText, tm, sText_Record, sText_MovesSuffix, tracked ? TmResearch_GetMoveCount(slot) : 0, tracked, 42);
    DrawTypeCaught(TmResearch_GetType(tm), 56);
    switch (status)
    {
    case TM_STATUS_OWNED:
        line = sText_Owned;
        break;
    case TM_STATUS_TRACKED:
        if (TmResearch_IsSlotReady(slot))
            line = sText_ReadyToCraft, color = sColorReady;
        else
            line = sText_Tracked, color = sColorBlue;
        break;
    case TM_STATUS_READY:
        line = sText_ReadyAllCaught, color = sColorReady;
        break;
    case TM_STATUS_TYPE_TAKEN:
        line = sText_TypeTaken;
        break;
    case TM_STATUS_NO_SLOT:
        line = sText_NoSlot;
        break;
    default:
        break;
    }
    if (line != NULL)
        AddTextPrinterParameterized3(WIN_DETAIL, FitFont(FONT_SMALL, FONT_SMALL_NARROW, line, 144), 0, 72, color, TEXT_SKIP_DRAW, line);
    PutWindowTilemap(WIN_DETAIL);
    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static u32 GetBrowsedTm(void)
{
    return sQuestMenu->numTms == 0 ? 0 : sQuestMenu->tms[sBrowseRow];
}

// The slot a confirmation is about
static s32 GetConfirmSlot(void)
{
    if (sTmView == TM_VIEW_BROWSE)
        return TmResearch_FindSlotOfType(TmResearch_GetType(GetBrowsedTm()));
    return sTmRow;
}

// The bottom line: a notice, a question, or the keys
static void DrawTmBottom(void)
{
    const u8 *text = sNotice, *color = sColorRed;
    if (sConfirm != CONFIRM_NONE)
    {
        u32 tm = (sConfirm == CONFIRM_START || sConfirm == CONFIRM_CRAFT_READY) ? GetBrowsedTm() : TmResearch_GetSlotTm(GetConfirmSlot());
        StringCopy(gStringVar1, GetMoveName(TmResearch_GetMove(tm)));
        StringExpandPlaceholders(gStringVar4, sConfirm == CONFIRM_START ? sText_ConfirmStart
                                            : sConfirm == CONFIRM_CLEAR ? sText_ConfirmClear : sText_ConfirmCraft);
        text = gStringVar4;
    }
    else if (text == NULL)
    {
        color = sColorDark;
        if (sTmView == TM_VIEW_BROWSE)
        {
            switch (TmResearch_GetStatus(GetBrowsedTm()))
            {
            case TM_STATUS_AVAILABLE:
                text = sText_KeysBrowse;
                break;
            case TM_STATUS_READY:
                text = sText_KeysBrowseCraft;
                break;
            case TM_STATUS_TRACKED:
                text = TmResearch_IsSlotReady(GetConfirmSlot()) ? sText_KeysBrowseCraft : sText_KeysBrowseView;
                break;
            default:
                text = sText_KeysBrowseView;
                break;
            }
        }
        else if (sTmRow == TM_ROW_ALL)
            text = sText_KeysAll;
        else if (TmResearch_GetSlotTm(sTmRow) == 0)
            text = sText_KeysEmpty;
        else
            text = TmResearch_IsSlotReady(sTmRow) ? sText_KeysReady : sText_KeysSlot;
    }
    FillWindowPixelBuffer(WIN_NEXT, PIXEL_FILL(COLOR_WHITE));
    AddTextPrinterParameterized3(WIN_NEXT, FitFont(FONT_NORMAL, FONT_NARROW, text, 224), 0, 1, color, TEXT_SKIP_DRAW, text);
    PutWindowTilemap(WIN_NEXT);
    CopyWindowToVram(WIN_NEXT, COPYWIN_FULL);
}

static void DrawTmSelected(void)
{
    if (sTmView == TM_VIEW_BROWSE)
    {
        BuildTmList();
        DrawBrowseList();
        if (sQuestMenu->numTms != 0)
            DrawBrowseDetail(GetBrowsedTm());
    }
    else
    {
        DrawSlotsList();
        if (sTmRow == TM_ROW_ALL)
            DrawDatabaseDetail();
        else
            DrawSlotDetail(sTmRow);
    }
    DrawTmBottom();
}

static void OpenBrowse(u32 slot)
{
    sTmView = TM_VIEW_BROWSE;
    sBrowseSlot = slot;
    sBrowseFilter = FILTER_ALL;
    sBrowseRow = 0;
    sBrowseFirst = 0;
    BuildTmList();
}

// The slot a TM picked in the browse list goes to
static s32 GetTargetSlot(void)
{
    if (sBrowseSlot < TM_RESEARCH_SLOTS && TmResearch_GetSlotTm(sBrowseSlot) == 0)
        return sBrowseSlot;
    return TmResearch_FindFreeSlot();
}

static void Crafted(bool32 success)
{
    if (success)
    {
        PlayFanfare(MUS_OBTAIN_TMHM);
        sNotice = sText_NoticeCrafted;
    }
    else
    {
        PlaySE(SE_FAILURE);
        sNotice = sText_NoticeBagFull;
    }
}

static void DoConfirmed(void)
{
    s32 slot;
    switch (sConfirm)
    {
    case CONFIRM_START:
        slot = GetTargetSlot();
        if (slot >= 0 && TmResearch_Start(slot, GetBrowsedTm()))
        {
            PlaySE(SE_SUCCESS);
            sNotice = sText_NoticeStarted;
            sTmView = TM_VIEW_SLOTS;
            sTmRow = slot;
            if (!sFromMachine)
                DrawHeader();
        }
        break;
    case CONFIRM_CRAFT:
        slot = GetConfirmSlot();
        Crafted(slot >= 0 && TmResearch_CraftSlot(slot));
        break;
    case CONFIRM_CRAFT_READY:
        Crafted(TmResearch_CraftReadyTm(GetBrowsedTm()));
        break;
    case CONFIRM_CLEAR:
        TmResearch_ClearSlot(sTmRow);
        PlaySE(SE_SELECT);
        sNotice = sText_NoticeCleared;
        break;
    }
    sConfirm = CONFIRM_NONE;
}

static void Notice(const u8 *text)
{
    PlaySE(SE_FAILURE);
    sNotice = text;
}

static void Confirm(u32 what)
{
    PlaySE(SE_SELECT);
    sConfirm = what;
}

static void BrowsePressA(void)
{
    switch (TmResearch_GetStatus(GetBrowsedTm()))
    {
    case TM_STATUS_AVAILABLE:
        Confirm(CONFIRM_START);
        break;
    case TM_STATUS_READY:
        Confirm(CONFIRM_CRAFT_READY);
        break;
    case TM_STATUS_TRACKED:
        if (TmResearch_IsSlotReady(GetConfirmSlot()))
            Confirm(CONFIRM_CRAFT);
        else
            Notice(sText_NoticeNotReady);
        break;
    case TM_STATUS_OWNED:
        Notice(sText_NoticeOwned);
        break;
    case TM_STATUS_TYPE_TAKEN:
        Notice(sText_NoticeTaken);
        break;
    default:
        Notice(sText_NoticeNoSlot);
        break;
    }
}

static void SlotsPressA(void)
{
    if (sTmRow == TM_ROW_ALL || TmResearch_GetSlotTm(sTmRow) == 0)
    {
        PlaySE(SE_SELECT);
        OpenBrowse(sTmRow == TM_ROW_ALL ? NO_SLOT : sTmRow);
        DrawHeader();
    }
    else if (TmResearch_IsSlotReady(sTmRow))
    {
        Confirm(CONFIRM_CRAFT);
    }
    else
    {
        Notice(sText_NoticeNotReady);
    }
}

static void CloseQuestMenu(u8 taskId)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_QuestMenuExit;
}

#define TM_KEYS (A_BUTTON | B_BUTTON | SELECT_BUTTON | DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON)

static void HandleTmInput(u8 taskId)
{
    if (!JOY_NEW(TM_KEYS) && !JOY_REPEAT(DPAD_UP | DPAD_DOWN))
        return;
    sNotice = NULL;
    if (sConfirm != CONFIRM_NONE)
    {
        if (JOY_NEW(A_BUTTON))
            DoConfirmed();
        else if (JOY_NEW(B_BUTTON))
            PlaySE(SE_SELECT), sConfirm = CONFIRM_NONE;
    }
    else if (sTmView == TM_VIEW_BROWSE)
    {
        if (JOY_REPEAT(DPAD_UP) && sBrowseRow > 0)
            PlaySE(SE_SELECT), sBrowseRow--;
        else if (JOY_REPEAT(DPAD_DOWN) && sBrowseRow + 1 < sQuestMenu->numTms)
            PlaySE(SE_SELECT), sBrowseRow++;
        else if (JOY_NEW(L_BUTTON | DPAD_LEFT))
            PlaySE(SE_SELECT), StepFilter(-1);
        else if (JOY_NEW(R_BUTTON | DPAD_RIGHT))
            PlaySE(SE_SELECT), StepFilter(1);
        else if (JOY_NEW(A_BUTTON) && sQuestMenu->numTms != 0)
            BrowsePressA();
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sTmView = TM_VIEW_SLOTS;
            DrawHeader();
        }
    }
    else
    {
        if (JOY_REPEAT(DPAD_UP) && sTmRow > 0)
            PlaySE(SE_SELECT), sTmRow--;
        else if (JOY_REPEAT(DPAD_DOWN) && sTmRow + 1 < TM_ROWS)
            PlaySE(SE_SELECT), sTmRow++;
        else if (JOY_NEW(A_BUTTON))
            SlotsPressA();
        else if (JOY_NEW(SELECT_BUTTON) && sTmRow < TM_RESEARCH_SLOTS && TmResearch_GetSlotTm(sTmRow) != 0)
            Confirm(CONFIRM_CLEAR);
        else if (JOY_NEW(L_BUTTON | R_BUTTON) && !sFromMachine)
        {
            PlaySE(SE_SELECT);
            sTab = TAB_QUESTS;
            DrawHeader();
            DrawSelected();
            return;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            CloseQuestMenu(taskId);
            return;
        }
    }
    DrawTmSelected();
}

// special TmDatabase_OpenFromMachine (the institute's TM MACHINE, after fadescreen): the database's TMs
void TmDatabase_OpenFromMachine(void)
{
    sSelectedRow = 0;
    sFirstVisibleRow = 0;
    sTab = TAB_TM;
    sTmRow = 0;
    sFromMachine = TRUE;
    sConfirm = CONFIRM_NONE;
    sNotice = NULL;
    sTmView = TM_VIEW_BROWSE;
    sBrowseSlot = NO_SLOT;
    sBrowseFilter = FILTER_ALL;
    sBrowseRow = 0;
    sBrowseFirst = 0;
    gMain.state = 0;
    SetMainCallback2(CB2_InitQuestMenu);
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
        if (sTab == TAB_TM && sTmView == TM_VIEW_BROWSE)
            BuildTmList();
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
    if (sTab == TAB_TM)
    {
        HandleTmInput(taskId);
        return;
    }
    if (JOY_NEW(L_BUTTON | R_BUTTON) && TmResearch_IsUnlocked())
    {
        PlaySE(SE_SELECT);
        sTab = TAB_TM;
        sTmView = TM_VIEW_SLOTS;
        sConfirm = CONFIRM_NONE;
        sNotice = NULL;
        DrawHeader();
        DrawSelected();
    }
    else if (JOY_NEW(DPAD_UP) && sSelectedRow > 0)
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
    SetMainCallback2(sFromMachine ? CB2_ReturnToFieldContinueScript : CB2_ReturnToFieldWithOpenMenu);
}

static void Task_QuestMenuOpenMap(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    FreeQuestMenu();
    DestroyTask(taskId);
    FieldInitRegionMapFromQuests(CB2_ReturnToQuestMenu);
}
