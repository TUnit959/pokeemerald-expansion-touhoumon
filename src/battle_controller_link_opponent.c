#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_tv.h"
#include "bg.h"
#include "data.h"
#include "link.h"
#include "main.h"
#include "m4a.h"
#include "palette.h"
#include "pokeball.h"
#include "pokemon.h"
#include "recorded_battle.h"
#include "reshow_battle_screen.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "util.h"
#include "window.h"
#include "constants/battle_anim.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "recorded_battle.h"

static void LinkOpponentHandleGetMonData(void);
static void LinkOpponentHandleSetMonData(void);
static void LinkOpponentHandleSetRawMonData(void);
static void LinkOpponentHandleLoadMonSprite(void);
static void LinkOpponentHandleSwitchInAnim(void);
static void LinkOpponentHandleReturnMonToBall(void);
static void LinkOpponentHandleDrawTrainerPic(void);
static void LinkOpponentHandleTrainerSlide(void);
static void LinkOpponentHandleTrainerSlideBack(void);
static void LinkOpponentHandleFaintAnimation(void);
static void LinkOpponentHandleMoveAnimation(void);
static void LinkOpponentHandlePrintString(void);
static void LinkOpponentHandleHealthBarUpdate(void);
static void LinkOpponentHandleStatusIconUpdate(void);
static void LinkOpponentHandleStatusAnimation(void);
static void LinkOpponentHandleClearUnkVar(void);
static void LinkOpponentHandleSetUnkVar(void);
static void LinkOpponentHandleClearUnkFlag(void);
static void LinkOpponentHandleToggleUnkFlag(void);
static void LinkOpponentHandleHitAnimation(void);
static void LinkOpponentHandleIntroSlide(void);
static void LinkOpponentHandleIntroTrainerBallThrow(void);
static void LinkOpponentHandleDrawPartyStatusSummary(void);
static void LinkOpponentHandleHidePartyStatusSummary(void);
static void LinkOpponentHandleSpriteInvisibility(void);
static void LinkOpponentHandleBattleAnimation(void);
static void LinkOpponentHandleLinkStandbyMsg(void);
static void LinkOpponentHandleEndLinkBattle(void);

static void LinkOpponentBufferRunCommand(void);
static void LinkOpponentBufferExecCompleted(void);
static void SwitchIn_HandleSoundAndEnd(void);
static u32 CopyLinkOpponentMonData(u8 monId, u8 *dst);
static void SetLinkOpponentMonData(u8 monId);
static void LinkOpponentDoMoveAnimation(void);
static void Task_StartSendOutAnim(u8 taskId);
static void SpriteCB_FreeOpponentSprite(struct Sprite *sprite);
static void EndDrawPartyStatusSummary(void);

static void (*const sLinkOpponentBufferCommands[CONTROLLER_CMDS_COUNT])(void) =
{
    [CONTROLLER_GETMONDATA]               = LinkOpponentHandleGetMonData,
    [CONTROLLER_GETRAWMONDATA]            = BtlController_Empty,
    [CONTROLLER_SETMONDATA]               = LinkOpponentHandleSetMonData,
    [CONTROLLER_SETRAWMONDATA]            = LinkOpponentHandleSetRawMonData,
    [CONTROLLER_LOADMONSPRITE]            = LinkOpponentHandleLoadMonSprite,
    [CONTROLLER_SWITCHINANIM]             = LinkOpponentHandleSwitchInAnim,
    [CONTROLLER_RETURNMONTOBALL]          = LinkOpponentHandleReturnMonToBall,
    [CONTROLLER_DRAWTRAINERPIC]           = LinkOpponentHandleDrawTrainerPic,
    [CONTROLLER_TRAINERSLIDE]             = LinkOpponentHandleTrainerSlide,
    [CONTROLLER_TRAINERSLIDEBACK]         = LinkOpponentHandleTrainerSlideBack,
    [CONTROLLER_FAINTANIMATION]           = LinkOpponentHandleFaintAnimation,
    [CONTROLLER_PALETTEFADE]              = BtlController_Empty,
    [CONTROLLER_SUCCESSBALLTHROWANIM]     = BtlController_Empty,
    [CONTROLLER_BALLTHROWANIM]            = BtlController_Empty,
    [CONTROLLER_PAUSE]                    = BtlController_Empty,
    [CONTROLLER_MOVEANIMATION]            = LinkOpponentHandleMoveAnimation,
    [CONTROLLER_PRINTSTRING]              = LinkOpponentHandlePrintString,
    [CONTROLLER_PRINTSTRINGPLAYERONLY]    = BtlController_Empty,
    [CONTROLLER_CHOOSEACTION]             = BtlController_Empty,
    [CONTROLLER_YESNOBOX]                 = BtlController_Empty,
    [CONTROLLER_CHOOSEMOVE]               = BtlController_Empty,
    [CONTROLLER_OPENBAG]                  = BtlController_Empty,
    [CONTROLLER_CHOOSEPOKEMON]            = BtlController_Empty,
    [CONTROLLER_23]                       = BtlController_Empty,
    [CONTROLLER_HEALTHBARUPDATE]          = LinkOpponentHandleHealthBarUpdate,
    [CONTROLLER_EXPUPDATE]                = BtlController_Empty,
    [CONTROLLER_STATUSICONUPDATE]         = LinkOpponentHandleStatusIconUpdate,
    [CONTROLLER_STATUSANIMATION]          = LinkOpponentHandleStatusAnimation,
    [CONTROLLER_STATUSXOR]                = BtlController_Empty,
    [CONTROLLER_DATATRANSFER]             = BtlController_Empty,
    [CONTROLLER_DMA3TRANSFER]             = BtlController_Empty,
    [CONTROLLER_PLAYBGM]                  = BtlController_Empty,
    [CONTROLLER_32]                       = BtlController_Empty,
    [CONTROLLER_TWORETURNVALUES]          = BtlController_Empty,
    [CONTROLLER_CHOSENMONRETURNVALUE]     = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE]           = BtlController_Empty,
    [CONTROLLER_ONERETURNVALUE_DUPLICATE] = BtlController_Empty,
    [CONTROLLER_CLEARUNKVAR]              = LinkOpponentHandleClearUnkVar,
    [CONTROLLER_SETUNKVAR]                = LinkOpponentHandleSetUnkVar,
    [CONTROLLER_CLEARUNKFLAG]             = LinkOpponentHandleClearUnkFlag,
    [CONTROLLER_TOGGLEUNKFLAG]            = LinkOpponentHandleToggleUnkFlag,
    [CONTROLLER_HITANIMATION]             = LinkOpponentHandleHitAnimation,
    [CONTROLLER_CANTSWITCH]               = BtlController_Empty,
    [CONTROLLER_PLAYSE]                   = BtlController_HandlePlaySE,
    [CONTROLLER_PLAYFANFAREORBGM]         = BtlController_HandlePlayFanfareOrBGM,
    [CONTROLLER_FAINTINGCRY]              = BtlController_HandleFaintingCry,
    [CONTROLLER_INTROSLIDE]               = LinkOpponentHandleIntroSlide,
    [CONTROLLER_INTROTRAINERBALLTHROW]    = LinkOpponentHandleIntroTrainerBallThrow,
    [CONTROLLER_DRAWPARTYSTATUSSUMMARY]   = LinkOpponentHandleDrawPartyStatusSummary,
    [CONTROLLER_HIDEPARTYSTATUSSUMMARY]   = LinkOpponentHandleHidePartyStatusSummary,
    [CONTROLLER_ENDBOUNCE]                = BtlController_Empty,
    [CONTROLLER_SPRITEINVISIBILITY]       = LinkOpponentHandleSpriteInvisibility,
    [CONTROLLER_BATTLEANIMATION]          = LinkOpponentHandleBattleAnimation,
    [CONTROLLER_LINKSTANDBYMSG]           = LinkOpponentHandleLinkStandbyMsg,
    [CONTROLLER_RESETACTIONMOVESELECTION] = BtlController_Empty,
    [CONTROLLER_ENDLINKBATTLE]            = LinkOpponentHandleEndLinkBattle,
    [CONTROLLER_DEBUGMENU]                = BtlController_Empty,
    [CONTROLLER_TERMINATOR_NOP]           = BtlController_TerminatorNop
};

static void LinkOpponentDummy(void)
{
}

void SetControllerToLinkOpponent(void)
{
    gBattlerControllerEndFuncs[gActiveBattler] = LinkOpponentBufferExecCompleted;
    gBattlerControllerFuncs[gActiveBattler] = LinkOpponentBufferRunCommand;
}

static void LinkOpponentBufferRunCommand(void)
{
    if (gBattleControllerExecFlags & gBitTable[gActiveBattler])
    {
        if (gBattleResources->bufferA[gActiveBattler][0] < ARRAY_COUNT(sLinkOpponentBufferCommands))
            sLinkOpponentBufferCommands[gBattleResources->bufferA[gActiveBattler][0]]();
        else
            LinkOpponentBufferExecCompleted();
    }
}

static void CompleteOnBattlerSpriteCallbackDummy(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
        LinkOpponentBufferExecCompleted();
}

static void CompleteOnBankSpriteCallbackDummy2(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
        LinkOpponentBufferExecCompleted();
}

static void FreeTrainerSpriteAfterSlide(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
    {
        FreeTrainerFrontPicPalette(gSprites[gBattlerSpriteIds[gActiveBattler]].oam.affineParam);
        FreeSpriteOamMatrix(&gSprites[gBattlerSpriteIds[gActiveBattler]]);
        DestroySprite(&gSprites[gBattlerSpriteIds[gActiveBattler]]);
        LinkOpponentBufferExecCompleted();
    }
}

static void Intro_DelayAndEnd(void)
{
    if (--gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].introEndDelay == (u8)-1)
    {
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].introEndDelay = 0;
        LinkOpponentBufferExecCompleted();
    }
}

static void Intro_WaitForShinyAnimAndHealthbox(void)
{
    bool32 healthboxAnimDone = FALSE;
    bool32 twoMons = FALSE;

    if (!IsDoubleBattle() || (IsDoubleBattle() && (gBattleTypeFlags & BATTLE_TYPE_MULTI)))
    {
        if (gSprites[gHealthboxSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
            healthboxAnimDone = TRUE;
    }
    else
    {
        if (gSprites[gHealthboxSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy
         && gSprites[gHealthboxSpriteIds[BATTLE_PARTNER(gActiveBattler)]].callback == SpriteCallbackDummy)
        {
            healthboxAnimDone = TRUE;
        }
        twoMons = TRUE;
    }

    if (healthboxAnimDone)
    {
        if (twoMons || !IsBattlerSpriteVisible(BATTLE_PARTNER(gActiveBattler)))
        {
            if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim)
                return;
            if (!gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].finishedShinyMonAnim)
                return;

            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim = FALSE;

            gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].finishedShinyMonAnim = FALSE;

            FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
            FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
        }
        else
        {
            if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim)
                return;

            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim = FALSE;

            if (GetBattlerPosition(gActiveBattler) == B_POSITION_OPPONENT_RIGHT)
            {
                FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
                FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
            }
        }

        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].introEndDelay = 3;
        gBattlerControllerFuncs[gActiveBattler] = Intro_DelayAndEnd;
    }
}

static void Intro_TryShinyAnimShowHealthbox(void)
{
    bool32 bgmRestored = FALSE;

    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim)
    {
        TryShinyAnimation(gActiveBattler, &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]]);
    }
    if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI)
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].triedShinyMonAnim
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].finishedShinyMonAnim)
    {
        TryShinyAnimation(BATTLE_PARTNER(gActiveBattler), &gEnemyParty[gBattlerPartyIndexes[BATTLE_PARTNER(gActiveBattler)]]);
    }


    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].ballAnimActive && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].ballAnimActive)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].healthboxSlideInStarted)
        {
            if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                UpdateHealthboxAttribute(gHealthboxSpriteIds[BATTLE_PARTNER(gActiveBattler)], &gEnemyParty[gBattlerPartyIndexes[BATTLE_PARTNER(gActiveBattler)]], HEALTHBOX_ALL);
                StartHealthboxSlideIn(BATTLE_PARTNER(gActiveBattler));
                SetHealthboxSpriteVisible(gHealthboxSpriteIds[BATTLE_PARTNER(gActiveBattler)]);
            }
            UpdateHealthboxAttribute(gHealthboxSpriteIds[gActiveBattler], &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], HEALTHBOX_ALL);
            StartHealthboxSlideIn(gActiveBattler);
            SetHealthboxSpriteVisible(gHealthboxSpriteIds[gActiveBattler]);
        }
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].healthboxSlideInStarted = TRUE;
    }

    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].waitForCry
        && gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].healthboxSlideInStarted
        && !gBattleSpritesDataPtr->healthBoxesData[BATTLE_PARTNER(gActiveBattler)].waitForCry
        && !IsCryPlayingOrClearCrySongs())
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].bgmRestored)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI && gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                if (GetBattlerPosition(gActiveBattler) == B_POSITION_OPPONENT_LEFT)
                    m4aMPlayContinue(&gMPlayInfo_BGM);
            }
            else
            {
                m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
            }
        }

        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].bgmRestored = TRUE;
        bgmRestored = TRUE;
    }

    if (bgmRestored)
    {
        if (gSprites[gBattleControllerData[gActiveBattler]].callback == SpriteCallbackDummy
            && gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI && GetBattlerPosition(gActiveBattler) == B_POSITION_OPPONENT_RIGHT)
            {
                if (++gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].introEndDelay == 1)
                    return;
                gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].introEndDelay = 0;
            }

            if (IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            {
                DestroySprite(&gSprites[gBattleControllerData[BATTLE_PARTNER(gActiveBattler)]]);
                SetBattlerShadowSpriteCallback(BATTLE_PARTNER(gActiveBattler), GetMonData(&gEnemyParty[gBattlerPartyIndexes[BATTLE_PARTNER(gActiveBattler)]], MON_DATA_SPECIES));
            }


            DestroySprite(&gSprites[gBattleControllerData[gActiveBattler]]);
            SetBattlerShadowSpriteCallback(gActiveBattler, GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES));

            gBattleSpritesDataPtr->animationData->introAnimActive = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].bgmRestored = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].healthboxSlideInStarted = FALSE;

            gBattlerControllerFuncs[gActiveBattler] = Intro_WaitForShinyAnimAndHealthbox;
        }
    }
}

static void TryShinyAnimAfterMonAnim(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy
     && gSprites[gBattlerSpriteIds[gActiveBattler]].x2 == 0)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim)
        {
            TryShinyAnimation(gActiveBattler, &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]]);
        }
        else if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim)
        {
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim = FALSE;
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim = FALSE;
            FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
            FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);
            LinkOpponentBufferExecCompleted();
        }
    }
}

static void CompleteOnHealthbarDone(void)
{
    s16 hpValue = MoveBattleBar(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], HEALTH_BAR, 0);

    SetHealthboxSpriteVisible(gHealthboxSpriteIds[gActiveBattler]);

    if (hpValue != -1)
        UpdateHpTextInHealthbox(gHealthboxSpriteIds[gActiveBattler], HP_CURRENT, hpValue, gBattleMons[gActiveBattler].maxHP);
    else
        LinkOpponentBufferExecCompleted();
}

static void CompleteOnInactiveTextPrinter(void)
{
    if (!IsTextPrinterActive(B_WIN_MSG))
        LinkOpponentBufferExecCompleted();
}

static void DoHitAnimBlinkSpriteEffect(void)
{
    u8 spriteId = gBattlerSpriteIds[gActiveBattler];

    if (gSprites[spriteId].data[1] == 32)
    {
        gSprites[spriteId].data[1] = 0;
        gSprites[spriteId].invisible = FALSE;
        gDoingBattleAnim = FALSE;
        LinkOpponentBufferExecCompleted();
    }
    else
    {
        if ((gSprites[spriteId].data[1] % 4) == 0)
            gSprites[spriteId].invisible ^= 1;
        gSprites[spriteId].data[1]++;
    }
}

static void SwitchIn_ShowSubstitute(void)
{
    if (gSprites[gHealthboxSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
    {
        if (gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute)
            InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_MON_TO_SUBSTITUTE);

        gBattlerControllerFuncs[gActiveBattler] = SwitchIn_HandleSoundAndEnd;
    }
}

static void SwitchIn_HandleSoundAndEnd(void)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive && !IsCryPlayingOrClearCrySongs())
    {
        if (gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy
            || gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy_2)
        {
            m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
            LinkOpponentBufferExecCompleted();
        }
    }
}

static void SwitchIn_ShowHealthbox(void)
{
    if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim
     && gSprites[gBattlerSpriteIds[gActiveBattler]].callback == SpriteCallbackDummy)
    {
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim = FALSE;
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].finishedShinyMonAnim = FALSE;

        FreeSpriteTilesByTag(ANIM_TAG_GOLD_STARS);
        FreeSpritePaletteByTag(ANIM_TAG_GOLD_STARS);

        StartSpriteAnim(&gSprites[gBattlerSpriteIds[gActiveBattler]], 0);

        UpdateHealthboxAttribute(gHealthboxSpriteIds[gActiveBattler], &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], HEALTHBOX_ALL);
        StartHealthboxSlideIn(gActiveBattler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[gActiveBattler]);
        CopyBattleSpriteInvisibility(gActiveBattler);
        gBattlerControllerFuncs[gActiveBattler] = SwitchIn_ShowSubstitute;
    }
}

static void SwitchIn_TryShinyAnim(void)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].ballAnimActive
        && !gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].triedShinyMonAnim)
    {
        TryShinyAnimation(gActiveBattler, &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]]);
    }

    if (gSprites[gBattleControllerData[gActiveBattler]].callback == SpriteCallbackDummy
        && !gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].ballAnimActive)
    {
        DestroySprite(&gSprites[gBattleControllerData[gActiveBattler]]);
        SetBattlerShadowSpriteCallback(gActiveBattler, GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES));
        gBattlerControllerFuncs[gActiveBattler] = SwitchIn_ShowHealthbox;
    }
}

static void CompleteOnFinishedStatusAnimation(void)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].statusAnimActive)
        LinkOpponentBufferExecCompleted();
}

static void CompleteOnFinishedBattleAnimation(void)
{
    if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animFromTableActive)
        LinkOpponentBufferExecCompleted();
}

static void LinkOpponentBufferExecCompleted(void)
{
    gBattlerControllerFuncs[gActiveBattler] = LinkOpponentBufferRunCommand;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
    {
        u8 playerId = GetMultiplayerId();

        PrepareBufferDataTransferLink(2, 4, &playerId);
        gBattleResources->bufferA[gActiveBattler][0] = CONTROLLER_TERMINATOR_NOP;
    }
    else
    {
        gBattleControllerExecFlags &= ~gBitTable[gActiveBattler];
    }
}

static void LinkOpponentHandleGetMonData(void)
{
    BtlController_HandleGetMonData(gActiveBattler, gEnemyParty);
}

static void LinkOpponentHandleSetMonData(void)
{
    BtlController_HandleSetMonData(gActiveBattler, gEnemyParty);
}

static void LinkOpponentHandleSetRawMonData(void)
{
    BtlController_HandleSetRawMonData(gActiveBattler, gEnemyParty);
}

static void LinkOpponentHandleLoadMonSprite(void)
{
    BtlController_HandleLoadMonSprite(gActiveBattler, gEnemyParty, TryShinyAnimAfterMonAnim);
}

static void LinkOpponentHandleSwitchInAnim(void)
{
    BtlController_HandleSwitchInAnim(gActiveBattler, FALSE, SwitchIn_TryShinyAnim);
}

static void LinkOpponentHandleReturnMonToBall(void)
{
    BtlController_HandleReturnMonToBall(gActiveBattler);
}

#define sSpeedX data[0]

static void LinkOpponentHandleDrawTrainerPic(void)
{
    s16 xPos;
    u32 trainerPicId;

    if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        if ((GetBattlerPosition(gActiveBattler) & BIT_FLANK) != 0) // second mon
            xPos = 152;
        else // first mon
            xPos = 200;

        if (gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
        {
            if (gActiveBattler == B_POSITION_OPPONENT_LEFT)
                trainerPicId = GetFrontierTrainerFrontSpriteId(gTrainerBattleOpponent_A);
            else
                trainerPicId = GetFrontierTrainerFrontSpriteId(gTrainerBattleOpponent_B);
        }
        else
        {
            if ((gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].version & 0xFF) == VERSION_FIRE_RED
            || (gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].version & 0xFF) == VERSION_LEAF_GREEN)
            {
                if (gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].gender != MALE)
                    trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_LEAF];
                else
                    trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RED];
            }
            else if ((gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].version & 0xFF) == VERSION_RUBY
                     || (gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].version & 0xFF) == VERSION_SAPPHIRE)
            {
                if (gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].gender != MALE)
                    trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RS_MAY];
                else
                    trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RS_BRENDAN];
            }
            else
            {
                trainerPicId = PlayerGenderToFrontTrainerPicId(gLinkPlayers[GetBattlerMultiplayerId(gActiveBattler)].gender);
            }
        }
    }
    else
    {
        xPos = 176;
        if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
        {
            trainerPicId = GetUnionRoomTrainerPic();
        }
        else if ((gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].version & 0xFF) == VERSION_FIRE_RED
                 || (gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].version & 0xFF) == VERSION_LEAF_GREEN)
        {
            if (gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].gender != MALE)
                trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_LEAF];
            else
                trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RED];
        }
        else if ((gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].version & 0xFF) == VERSION_RUBY
                 || (gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].version & 0xFF) == VERSION_SAPPHIRE)
        {
            if (gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].gender != MALE)
                trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RS_MAY];
            else
                trainerPicId = gFacilityClassToPicIndex[FACILITY_CLASS_RS_BRENDAN];
        }
        else
        {
            trainerPicId = PlayerGenderToFrontTrainerPicId(gLinkPlayers[GetMultiplayerId() ^ BIT_SIDE].gender);
        }
    }

    DecompressTrainerFrontPic(trainerPicId, gActiveBattler);
    SetMultiuseSpriteTemplateToTrainerBack(trainerPicId, GetBattlerPosition(gActiveBattler));
    gBattlerSpriteIds[gActiveBattler] = CreateSprite(&gMultiuseSpriteTemplate,
                                               xPos,
                                               (8 - gTrainerFrontPicCoords[trainerPicId].size) * 4 + 40,
                                               GetBattlerSpriteSubpriority(gActiveBattler));

    gSprites[gBattlerSpriteIds[gActiveBattler]].x2 = -DISPLAY_WIDTH;
    gSprites[gBattlerSpriteIds[gActiveBattler]].sSpeedX = 2;
    gSprites[gBattlerSpriteIds[gActiveBattler]].oam.paletteNum = IndexOfSpritePaletteTag(gTrainerFrontPicPaletteTable[trainerPicId].tag);
    gSprites[gBattlerSpriteIds[gActiveBattler]].oam.affineParam = trainerPicId;
    gSprites[gBattlerSpriteIds[gActiveBattler]].callback = SpriteCB_TrainerSlideIn;

    gBattlerControllerFuncs[gActiveBattler] = CompleteOnBattlerSpriteCallbackDummy;
}

static void LinkOpponentHandleTrainerSlide(void)
{
    u32 trainerPicId;

    if (gActiveBattler == 1)
        trainerPicId = GetFrontierTrainerFrontSpriteId(gTrainerBattleOpponent_A);
    else
        trainerPicId = GetFrontierTrainerFrontSpriteId(gTrainerBattleOpponent_B);

    DecompressTrainerFrontPic(trainerPicId, gActiveBattler);
    SetMultiuseSpriteTemplateToTrainerBack(trainerPicId, GetBattlerPosition(gActiveBattler));
    gBattlerSpriteIds[gActiveBattler] = CreateSprite(&gMultiuseSpriteTemplate, 176, (8 - gTrainerFrontPicCoords[trainerPicId].size) * 4 + 40, 0x1E);

    gSprites[gBattlerSpriteIds[gActiveBattler]].x2 = 96;
    gSprites[gBattlerSpriteIds[gActiveBattler]].x += 32;
    gSprites[gBattlerSpriteIds[gActiveBattler]].sSpeedX = -2;
    gSprites[gBattlerSpriteIds[gActiveBattler]].oam.paletteNum = IndexOfSpritePaletteTag(gTrainerFrontPicPaletteTable[trainerPicId].tag);
    gSprites[gBattlerSpriteIds[gActiveBattler]].oam.affineParam = trainerPicId;
    gSprites[gBattlerSpriteIds[gActiveBattler]].callback = SpriteCB_TrainerSlideIn;

    gBattlerControllerFuncs[gActiveBattler] = CompleteOnBankSpriteCallbackDummy2; // this line is redundant, because LinkOpponentBufferExecCompleted changes the battle battlerId function
    LinkOpponentBufferExecCompleted();
}

#undef sSpeedX

static void LinkOpponentHandleTrainerSlideBack(void)
{
    SetSpritePrimaryCoordsFromSecondaryCoords(&gSprites[gBattlerSpriteIds[gActiveBattler]]);
    gSprites[gBattlerSpriteIds[gActiveBattler]].data[0] = 35;
    gSprites[gBattlerSpriteIds[gActiveBattler]].data[2] = 280;
    gSprites[gBattlerSpriteIds[gActiveBattler]].data[4] = gSprites[gBattlerSpriteIds[gActiveBattler]].y;
    gSprites[gBattlerSpriteIds[gActiveBattler]].callback = StartAnimLinearTranslation;
    StoreSpriteCallbackInData6(&gSprites[gBattlerSpriteIds[gActiveBattler]], SpriteCallbackDummy);
    gBattlerControllerFuncs[gActiveBattler] = FreeTrainerSpriteAfterSlide;
}

static void LinkOpponentHandleFaintAnimation(void)
{
    BtlController_HandleFaintAnimation(gActiveBattler);
}

static void LinkOpponentHandleMoveAnimation(void)
{
    if (!IsBattleSEPlaying(gActiveBattler))
    {
        u16 move = gBattleResources->bufferA[gActiveBattler][1] | (gBattleResources->bufferA[gActiveBattler][2] << 8);

        gAnimMoveTurn = gBattleResources->bufferA[gActiveBattler][3];
        gAnimMovePower = gBattleResources->bufferA[gActiveBattler][4] | (gBattleResources->bufferA[gActiveBattler][5] << 8);
        gAnimMoveDmg = gBattleResources->bufferA[gActiveBattler][6] | (gBattleResources->bufferA[gActiveBattler][7] << 8) | (gBattleResources->bufferA[gActiveBattler][8] << 16) | (gBattleResources->bufferA[gActiveBattler][9] << 24);
        gAnimFriendship = gBattleResources->bufferA[gActiveBattler][10];
        gWeatherMoveAnim = gBattleResources->bufferA[gActiveBattler][12] | (gBattleResources->bufferA[gActiveBattler][13] << 8);
        gAnimDisableStructPtr = (struct DisableStruct *)&gBattleResources->bufferA[gActiveBattler][16];
        gTransformedPersonalities[gActiveBattler] = gAnimDisableStructPtr->transformedMonPersonality;
        gTransformedOtIds[gActiveBattler] = gAnimDisableStructPtr->transformedMonOtId;
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
        gBattlerControllerFuncs[gActiveBattler] = LinkOpponentDoMoveAnimation;
        BattleTv_SetDataBasedOnMove(move, gWeatherMoveAnim, gAnimDisableStructPtr);
    }
}

static void LinkOpponentDoMoveAnimation(void)
{
    u16 move = gBattleResources->bufferA[gActiveBattler][1] | (gBattleResources->bufferA[gActiveBattler][2] << 8);
    u8 multihit = gBattleResources->bufferA[gActiveBattler][11];

    switch (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState)
    {
    case 0:
        if (gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute
            && !gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8)
        {
            gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8 = 1;
            InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_SUBSTITUTE_TO_MON);
        }
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 1;
        break;
    case 1:
        if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive)
        {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_OFF);
            DoMoveAnim(move);
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 2;
        }
        break;
    case 2:
        gAnimScriptCallback();
        if (!gAnimScriptActive)
        {
            SetBattlerSpriteAffineMode(ST_OAM_AFFINE_NORMAL);
            if (gBattleSpritesDataPtr->battlerData[gActiveBattler].behindSubstitute && multihit < 2)
            {
                InitAndLaunchSpecialAnimation(gActiveBattler, gActiveBattler, gActiveBattler, B_ANIM_MON_TO_SUBSTITUTE);
                gBattleSpritesDataPtr->battlerData[gActiveBattler].flag_x8 = 0;
            }
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 3;
        }
        break;
    case 3:
        if (!gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].specialAnimActive)
        {
            CopyAllBattleSpritesInvisibilities();
            TrySetBehindSubstituteSpriteBit(gActiveBattler, gBattleResources->bufferA[gActiveBattler][1] | (gBattleResources->bufferA[gActiveBattler][2] << 8));
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].animationState = 0;
            LinkOpponentBufferExecCompleted();
        }
        break;
    }
}

static void LinkOpponentHandlePrintString(void)
{
    u16 *stringId;

    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    stringId = (u16 *)(&gBattleResources->bufferA[gActiveBattler][2]);
    BufferStringBattle(*stringId);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
    gBattlerControllerFuncs[gActiveBattler] = CompleteOnInactiveTextPrinter;
    BattleTv_SetDataBasedOnString(*stringId);
}

static void LinkOpponentHandleHealthBarUpdate(void)
{
    s16 hpVal;

    LoadBattleBarGfx(0);
    hpVal = gBattleResources->bufferA[gActiveBattler][2] | (gBattleResources->bufferA[gActiveBattler][3] << 8);

    if (hpVal != INSTANT_HP_BAR_DROP)
    {
        u32 maxHP = GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MAX_HP);
        u32 curHP = GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_HP);

        SetBattleBarStruct(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], maxHP, curHP, hpVal);
    }
    else
    {
        u32 maxHP = GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_MAX_HP);

        SetBattleBarStruct(gActiveBattler, gHealthboxSpriteIds[gActiveBattler], maxHP, 0, hpVal);
    }

    gBattlerControllerFuncs[gActiveBattler] = CompleteOnHealthbarDone;
}

static void LinkOpponentHandleStatusIconUpdate(void)
{
    if (!IsBattleSEPlaying(gActiveBattler))
    {
        u8 battlerId;

        UpdateHealthboxAttribute(gHealthboxSpriteIds[gActiveBattler], &gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], HEALTHBOX_STATUS_ICON);
        battlerId = gActiveBattler;
        gBattleSpritesDataPtr->healthBoxesData[battlerId].statusAnimActive = 0;
        gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedStatusAnimation;
    }
}

static void LinkOpponentHandleStatusAnimation(void)
{
    if (!IsBattleSEPlaying(gActiveBattler))
    {
        InitAndLaunchChosenStatusAnimation(gBattleResources->bufferA[gActiveBattler][1],
                        gBattleResources->bufferA[gActiveBattler][2] | (gBattleResources->bufferA[gActiveBattler][3] << 8) | (gBattleResources->bufferA[gActiveBattler][4] << 16) | (gBattleResources->bufferA[gActiveBattler][5] << 24));
        gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedStatusAnimation;
    }
}

static void LinkOpponentHandleClearUnkVar(void)
{
    gUnusedControllerStruct.unk = 0;
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleSetUnkVar(void)
{
    gUnusedControllerStruct.unk = gBattleResources->bufferA[gActiveBattler][1];
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleClearUnkFlag(void)
{
    gUnusedControllerStruct.flag = 0;
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleToggleUnkFlag(void)
{
    gUnusedControllerStruct.flag ^= 1;
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleHitAnimation(void)
{
    if (gSprites[gBattlerSpriteIds[gActiveBattler]].invisible == TRUE)
    {
        LinkOpponentBufferExecCompleted();
    }
    else
    {
        gDoingBattleAnim = TRUE;
        gSprites[gBattlerSpriteIds[gActiveBattler]].data[1] = 0;
        DoHitAnimHealthboxEffect(gActiveBattler);
        gBattlerControllerFuncs[gActiveBattler] = DoHitAnimBlinkSpriteEffect;
    }
}

static void LinkOpponentHandleIntroSlide(void)
{
    HandleIntroSlide(gBattleResources->bufferA[gActiveBattler][1]);
    gIntroSlideFlags |= 1;
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleIntroTrainerBallThrow(void)
{
    u8 taskId;

    SetSpritePrimaryCoordsFromSecondaryCoords(&gSprites[gBattlerSpriteIds[gActiveBattler]]);

    gSprites[gBattlerSpriteIds[gActiveBattler]].data[0] = 35;
    gSprites[gBattlerSpriteIds[gActiveBattler]].data[2] = 280;
    gSprites[gBattlerSpriteIds[gActiveBattler]].data[4] = gSprites[gBattlerSpriteIds[gActiveBattler]].y;
    gSprites[gBattlerSpriteIds[gActiveBattler]].callback = StartAnimLinearTranslation;

    StoreSpriteCallbackInData6(&gSprites[gBattlerSpriteIds[gActiveBattler]], SpriteCB_FreeOpponentSprite);

    taskId = CreateTask(Task_StartSendOutAnim, 5);
    gTasks[taskId].data[0] = gActiveBattler;

    if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusSummaryShown)
        gTasks[gBattlerStatusSummaryTaskId[gActiveBattler]].func = Task_HidePartyStatusSummary;

    gBattleSpritesDataPtr->animationData->introAnimActive = TRUE;
    gBattlerControllerFuncs[gActiveBattler] = LinkOpponentDummy;
}

static void Task_StartSendOutAnim(u8 taskId)
{
    u8 savedActiveBank = gActiveBattler;

    gActiveBattler = gTasks[taskId].data[0];
    if (!IsDoubleBattle() || (gBattleTypeFlags & BATTLE_TYPE_MULTI))
    {
        gBattleResources->bufferA[gActiveBattler][1] = gBattlerPartyIndexes[gActiveBattler];
        StartSendOutAnim(gActiveBattler, FALSE);
    }
    else
    {
        gBattleResources->bufferA[gActiveBattler][1] = gBattlerPartyIndexes[gActiveBattler];
        StartSendOutAnim(gActiveBattler, FALSE);
        gActiveBattler = BATTLE_PARTNER(gActiveBattler);
        gBattleResources->bufferA[gActiveBattler][1] = gBattlerPartyIndexes[gActiveBattler];
        StartSendOutAnim(gActiveBattler, FALSE);
        gActiveBattler = BATTLE_PARTNER(gActiveBattler);
    }
    gBattlerControllerFuncs[gActiveBattler] = Intro_TryShinyAnimShowHealthbox;
    gActiveBattler = savedActiveBank;
    DestroyTask(taskId);
}

static void SpriteCB_FreeOpponentSprite(struct Sprite *sprite)
{
    FreeTrainerFrontPicPalette(sprite->oam.affineParam);
    FreeSpriteOamMatrix(sprite);
    DestroySprite(sprite);
}

static void LinkOpponentHandleDrawPartyStatusSummary(void)
{
    if (gBattleResources->bufferA[gActiveBattler][1] != 0 && GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
    {
        LinkOpponentBufferExecCompleted();
    }
    else
    {
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusSummaryShown = 1;

        if (gBattleResources->bufferA[gActiveBattler][2] != 0)
        {
            if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].opponentDrawPartyStatusSummaryDelay < 2)
            {
                gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].opponentDrawPartyStatusSummaryDelay++;
                return;
            }
            else
            {
                gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].opponentDrawPartyStatusSummaryDelay = 0;
            }
        }

        gBattlerStatusSummaryTaskId[gActiveBattler] = CreatePartyStatusSummarySprites(gActiveBattler, (struct HpAndStatus *)&gBattleResources->bufferA[gActiveBattler][4], gBattleResources->bufferA[gActiveBattler][1], gBattleResources->bufferA[gActiveBattler][2]);
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusDelayTimer = 0;

        if (gBattleResources->bufferA[gActiveBattler][2] != 0)
            gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusDelayTimer = 93;

        gBattlerControllerFuncs[gActiveBattler] = EndDrawPartyStatusSummary;
    }
}

static void EndDrawPartyStatusSummary(void)
{
    if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusDelayTimer++ > 92)
    {
        gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusDelayTimer = 0;
        LinkOpponentBufferExecCompleted();
    }
}

static void LinkOpponentHandleHidePartyStatusSummary(void)
{
    if (gBattleSpritesDataPtr->healthBoxesData[gActiveBattler].partyStatusSummaryShown)
        gTasks[gBattlerStatusSummaryTaskId[gActiveBattler]].func = Task_HidePartyStatusSummary;
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleSpriteInvisibility(void)
{
    if (IsBattlerSpritePresent(gActiveBattler))
    {
        gSprites[gBattlerSpriteIds[gActiveBattler]].invisible = gBattleResources->bufferA[gActiveBattler][1];
        CopyBattleSpriteInvisibility(gActiveBattler);
    }
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleBattleAnimation(void)
{
    if (!IsBattleSEPlaying(gActiveBattler))
    {
        u8 animationId = gBattleResources->bufferA[gActiveBattler][1];
        u16 argument = gBattleResources->bufferA[gActiveBattler][2] | (gBattleResources->bufferA[gActiveBattler][3] << 8);

        if (TryHandleLaunchBattleTableAnimation(gActiveBattler, gActiveBattler, gActiveBattler, animationId, argument))
            LinkOpponentBufferExecCompleted();
        else
            gBattlerControllerFuncs[gActiveBattler] = CompleteOnFinishedBattleAnimation;

        BattleTv_SetDataBasedOnAnimation(animationId);
    }
}

static void LinkOpponentHandleLinkStandbyMsg(void)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[gActiveBattler][2]);
    LinkOpponentBufferExecCompleted();
}

static void LinkOpponentHandleEndLinkBattle(void)
{
    RecordedBattle_RecordAllBattlerData(&gBattleResources->bufferA[gActiveBattler][4]);

    if (gBattleResources->bufferA[gActiveBattler][1] == B_OUTCOME_DREW)
        gBattleOutcome = gBattleResources->bufferA[gActiveBattler][1];
    else
        gBattleOutcome = gBattleResources->bufferA[gActiveBattler][1] ^ B_OUTCOME_DREW;

    gSaveBlock2Ptr->frontier.disableRecordBattle = gBattleResources->bufferA[gActiveBattler][2];
    FadeOutMapMusic(5);
    BeginFastPaletteFade(3);
    LinkOpponentBufferExecCompleted();
    gBattlerControllerFuncs[gActiveBattler] = SetBattleEndCallbacks;
}
