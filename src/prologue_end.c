#include "global.h"
#include "act1_story.h"
#include "act2_story.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "prologue_end.h"
#include "sprite.h"
#include "text.h"
#include "window.h"
#include "constants/heal_locations.h"
#include "constants/rgb.h"
#include "constants/story.h"

// The end of an act: title cards on black (3 seconds each, or until A/B), then the next act starts somewhere new.
// The end of Act 1 (docs/hack/act1_story.md) is here; the end of Act 2 is in src/act2_end.c.

#define CARD_FRAMES 180

static const u8 *const sPrologueCards[] =
{
    COMPOUND_STRING("End of Prologue"),
    COMPOUND_STRING("Years Later at Celadon University"),
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
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 8,
        .width = DISPLAY_TILE_WIDTH,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sTextPalette[16] = { RGB_BLACK, RGB_WHITE, RGB(16, 16, 16) };
static const u8 sTextColors[] = { 0, 1, 2 };

static EWRAM_DATA void *sTilemap = NULL;
static EWRAM_DATA u16 sTimer = 0;
static EWRAM_DATA u8 sCard = 0;
static EWRAM_DATA const u8 *const *sCards = NULL;
static EWRAM_DATA u8 sCardCount = 0;
static EWRAM_DATA void (*sStartNextAct)(void) = NULL;

static void VBlankCB_ActEndCards(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void PrintCard(void)
{
    s32 x = (DISPLAY_WIDTH - GetStringWidth(FONT_NORMAL, sCards[sCard], 0)) / 2;

    FillWindowPixelBuffer(0, PIXEL_FILL(0));
    AddTextPrinterParameterized3(0, FONT_NORMAL, x, 8, sTextColors, TEXT_SKIP_DRAW, sCards[sCard]);
    CopyWindowToVram(0, COPYWIN_FULL);
}

static void CB2_ActEndCards(void)
{
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();

    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        DmaFill16(3, 0, VRAM, VRAM_SIZE);
        DmaFill32(3, 0, OAM, OAM_SIZE);
        DmaFill16(3, 0, PLTT, PLTT_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        sTilemap = AllocZeroed(BG_SCREEN_SIZE);
        SetBgTilemapBuffer(0, sTilemap);
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        LoadPalette(sTextPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        LoadPalette(sTextPalette, BG_PLTT_ID(0), PLTT_SIZEOF(1));
        PutWindowTilemap(0);
        CopyBgTilemapBufferToVram(0);
        sCard = 0;
        PrintCard();
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        SetVBlankCallback(VBlankCB_ActEndCards);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0);
        ShowBg(0);
        BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
        gMain.state++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            sTimer = 0;
            gMain.state++;
        }
        break;
    case 2:
        if (++sTimer >= CARD_FRAMES || JOY_NEW(A_BUTTON | B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
            gMain.state++;
        }
        break;
    case 3:
        if (gPaletteFade.active)
            break;
        if (++sCard < sCardCount)
        {
            PrintCard();
            BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            gMain.state = 1;
            break;
        }
        FreeAllWindowBuffers();
        TRY_FREE_AND_SET_NULL(sTilemap);
        sStartNextAct();
        WarpIntoMap();
        gMain.state = 0;
        SetMainCallback2(CB2_LoadMap);
        break;
    }
}

// Shows the cards, then calls startNextAct (which sets the story vars and the warp destination) and warps.
// The screen must already be faded to black.
void ShowActEndCards(const u8 *const *cards, u8 count, void (*startNextAct)(void))
{
    sCards = cards;
    sCardCount = count;
    sStartNextAct = startNextAct;
    gMain.state = 0;
    SetMainCallback2(CB2_ActEndCards);
}

static void StartAct2(void)
{
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    VarSet(VAR_ACT2_PROGRESS, ACT2_START);
    Act2_RaisePartyLevels();
    Act2_GiveStartingKit();
    SetLastHealLocationWarp(HEAL_LOCATION_CELADON_CITY);
    SetWarpDestination(MAP_GROUP(ACT2_START_MAP), MAP_NUM(ACT2_START_MAP), WARP_ID_NONE, ACT2_START_X, ACT2_START_Y);
}

// special Act1_StartPrologueEnd, with the screen already faded to black
void Act1_StartPrologueEnd(void)
{
    ShowActEndCards(sPrologueCards, ARRAY_COUNT(sPrologueCards), StartAct2);
}
