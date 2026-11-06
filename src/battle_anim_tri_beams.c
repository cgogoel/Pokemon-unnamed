#include "global.h"
#include "act2_legends.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_anim_internal.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"
#include "util.h"
#include "constants/battle_anim.h"
#include "constants/rgb.h"

// Three beams at once, Fire (Flamethrower's flames), Water (Hydro Pump's orbs) and Grass (Solar Beam's orbs): Tri-Blast
// from the user to its target, and in Act 2's finale the legendary trio's joint attack, each beam from its own trio
// member (GetTriBeamBattlers, src/act2_legends.c). Scripts: gBattleAnimMove_TriBlast, gBattleAnimGeneral_Act2LegendsTrio.

static void AnimTriBeam(struct Sprite *sprite);
static void AnimTriBeam_Step(struct Sprite *sprite);

#define TRI_BEAM_DURATION 30

// args: x offset, y offset (from the source), enum TriBeamSource, wave amplitude, sprite anim;
// gBattleAnimArgs[7] is the wave's phase (AnimTask_StartSinAnimTimer), as for Flamethrower and Hydro Pump.
const struct SpriteTemplate gTriBeamFireSpriteTemplate =
{
    .tileTag = ANIM_TAG_SMALL_EMBER,
    .paletteTag = ANIM_TAG_SMALL_EMBER,
    .oam = &gOamData_AffineOff_ObjNormal_32x32,
    .anims = gAnims_FlamethrowerFlame,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimTriBeam,
};

const struct SpriteTemplate gTriBeamWaterSpriteTemplate =
{
    .tileTag = ANIM_TAG_WATER_ORB,
    .paletteTag = ANIM_TAG_WATER_ORB,
    .oam = &gOamData_AffineOff_ObjBlend_16x16,
    .anims = gAnims_WaterMudOrb,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimTriBeam,
};

const struct SpriteTemplate gTriBeamGrassSpriteTemplate =
{
    .tileTag = ANIM_TAG_ORBS,
    .paletteTag = ANIM_TAG_ORBS,
    .oam = &gOamData_AffineOff_ObjNormal_8x8,
    .anims = gSolarBeamBigOrbAnimTable,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = AnimTriBeam,
};

static enum TriBeamKind GetTriBeamKind(struct Sprite *sprite)
{
    if (sprite->template == &gTriBeamFireSpriteTemplate)
        return TRI_BEAM_FIRE;
    if (sprite->template == &gTriBeamWaterSpriteTemplate)
        return TRI_BEAM_WATER;
    return TRI_BEAM_GRASS;
}

// Like AnimToTargetInSinWave (src/battle_anim_water.c), but between the battlers the beam's source names.
static void AnimTriBeam(struct Sprite *sprite)
{
    CMD_ARGS(x, y, source, waveAmplitude, animation);
    u32 from, to;
    u16 phase = gBattleAnimArgs[7];
    s16 xOffset = cmd->x;
    s16 fromX, toX;

    GetTriBeamBattlers(cmd->source, GetTriBeamKind(sprite), &from, &to);
    fromX = GetBattlerSpriteCoord(from, BATTLER_COORD_X_2);
    toX = GetBattlerSpriteCoord(to, BATTLER_COORD_X_2);
    if (fromX > toX || (fromX == toX && !IsOnPlayerSide(from)))
        xOffset = -xOffset;

    if (GetTriBeamKind(sprite) == TRI_BEAM_GRASS)
    {
        // Solar Beam's orbs in all their sizes, as Solar Beam mixes them, tinted green. BlendPalette works from the
        // unfaded palette, so doing it for every orb is fine.
        StartSpriteAnim(sprite, (phase / 3) % 7);
        if (!gPaletteFade.active) // Not over the trio's white-out
            BlendPalette(OBJ_PLTT_ID(sprite->oam.paletteNum), 16, 9, RGB(2, 24, 2));
    }
    else
    {
        StartSpriteAnim(sprite, cmd->animation);
    }
    sprite->x = fromX + xOffset;
    sprite->y = GetBattlerSpriteCoord(from, BATTLER_COORD_Y_PIC_OFFSET) + cmd->y;
    sprite->data[0] = TRI_BEAM_DURATION;
    sprite->data[1] = sprite->x;
    sprite->data[2] = toX;
    sprite->data[3] = sprite->y;
    sprite->data[4] = GetBattlerSpriteCoord(to, BATTLER_COORD_Y_PIC_OFFSET);
    InitAnimLinearTranslation(sprite);
    sprite->data[5] = 0xD200 / sprite->data[0];
    sprite->data[7] = cmd->waveAmplitude;
    if (phase > 127)
    {
        sprite->data[6] = (phase - 127) * 256;
        sprite->data[7] = -sprite->data[7];
    }
    else
    {
        sprite->data[6] = phase * 256;
    }
    sprite->callback = AnimTriBeam_Step;
    sprite->callback(sprite);
}

static void AnimTriBeam_Step(struct Sprite *sprite)
{
    if (AnimTranslateLinear(sprite))
    {
        DestroyAnimSprite(sprite);
        return;
    }
    sprite->y2 += Sin(sprite->data[6] >> 8, sprite->data[7]);
    if ((sprite->data[6] + sprite->data[5]) >> 8 > 127)
    {
        sprite->data[6] = 0;
        sprite->data[7] = -sprite->data[7];
    }
    else
    {
        sprite->data[6] += sprite->data[5];
    }
}

// The trio's attack ends in a white-out over the whole screen, which stays until the battle fades out.
// No args.
void AnimTask_TriBeamWhiteOut(u8 taskId)
{
    if (gTasks[taskId].data[0] == 0)
    {
        BeginNormalPaletteFade(PALETTES_ALL, 1, 0, 16, RGB_WHITE);
        gTasks[taskId].data[0] = 1;
    }
    else if (!gPaletteFade.active)
    {
        DestroyAnimVisualTask(taskId);
    }
}
