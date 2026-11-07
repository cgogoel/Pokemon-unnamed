ViridianCity_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, ViridianCity_OnFrame
	.byte 0

ViridianCity_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_VIRIDIAN_CITY
	call ViridianCity_EventScript_SetOldManNormal
	call ViridianCity_EventScript_Act2Objects
	call_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR, 0, ViridianCity_EventScript_TryUnlockGym
	end

ViridianCity_EventScript_SetOldManNormal::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OLD_MAN_1
	return

ViridianCity_EventScript_TryUnlockGym::
	goto_if_unset FLAG_BADGE02_GET, Common_EventScript_NopReturn
	goto_if_unset FLAG_BADGE03_GET, Common_EventScript_NopReturn
	goto_if_unset FLAG_BADGE04_GET, Common_EventScript_NopReturn
	goto_if_unset FLAG_BADGE05_GET, Common_EventScript_NopReturn
	goto_if_unset FLAG_BADGE06_GET, Common_EventScript_NopReturn
	goto_if_unset FLAG_BADGE07_GET, Common_EventScript_NopReturn
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR, 1
	return

ViridianCity_EventScript_GymDoorLocked::
	lockall
	textcolor NPC_TEXT_COLOR_NEUTRAL
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 20
	msgbox ViridianCity_Text_GymDoorsAreLocked
	closemessage
	applymovement LOCALID_PLAYER, ViridianCity_Movement_JumpDownLedge
	waitmovement 0
	releaseall
	end

ViridianCity_Movement_JumpDownLedge::
	jump_2_down
	step_end

ViridianCity_EventScript_CitySign::
	msgbox ViridianCity_Text_CitySign, MSGBOX_SIGN
	end

ViridianCity_EventScript_TrainerTips1::
	msgbox ViridianCity_Text_CatchMonsForEasierBattles, MSGBOX_SIGN
	end

ViridianCity_EventScript_TrainerTips2::
	msgbox ViridianCity_Text_MovesLimitedByPP, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymSign::
	msgbox ViridianCity_Text_GymSign, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymDoor::
	msgbox ViridianCity_Text_GymDoorsAreLocked, MSGBOX_SIGN
	end

ViridianCity_EventScript_Boy::
	msgbox ViridianCity_Text_CanCarryMonsAnywhere, MSGBOX_NPC
	end

@ Other old man, not the tutorial old man
ViridianCity_EventScript_OldMan::
	msgbox ViridianCity_Text_GymClosedWonderWhoLeaderIs, MSGBOX_NPC
	end

ViridianCity_EventScript_TutorialOldMan::
	msgbox ViridianCity_Text_WeakenMonsFirstToCatch, MSGBOX_NPC
	end

ViridianCity_EventScript_Youngster::
	lock
	faceplayer
	msgbox ViridianCity_Text_WantToKnowAboutCaterpillarMons, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, ViridianCity_EventScript_YoungsterExplainCaterpillars
	goto_if_eq VAR_RESULT, NO, ViridianCity_EventScript_YoungsterDeclineExplanation
	end

ViridianCity_EventScript_YoungsterExplainCaterpillars::
	msgbox ViridianCity_Text_ExplainCaterpieWeedle
	release
	end

ViridianCity_EventScript_YoungsterDeclineExplanation::
	msgbox ViridianCity_Text_OhOkayThen
	release
	end

ViridianCity_EventScript_Woman::
	msgbox ViridianCity_Text_GoShoppingInPewterOccasionally, MSGBOX_NPC
	end

ViridianCity_EventScript_DreamEaterTutor::
	goto EventScript_DreamEaterTutor
	end

ViridianCity_Text_CanCarryMonsAnywhere::
	.string "My mom runs the flower shop.\n"
	.string "I deliver the bouquets!\p"
	.string "My PIDGEY carries the small\n"
	.string "ones. It works for seeds.$"

ViridianCity_Text_GymClosedWonderWhoLeaderIs::
	.string "Ah, a fine morning for sitting\n"
	.string "in the sun.\p"
	.string "At my age, that's a full day's\n"
	.string "work. Hohoho!$"

ViridianCity_Text_WantToKnowAboutCaterpillarMons::
	.string "Want to hear about the bugs in\n"
	.string "the forest up north?$"

ViridianCity_Text_OhOkayThen::
	.string "Aw. You'd have liked it.$"

ViridianCity_Text_ExplainCaterpieWeedle::
	.string "CATERPIE are harmless, but\n"
	.string "WEEDLE sting! I've got the\l"
	.string "scar to prove it.\p"
	.string "I'm going to be a bug professor\n"
	.string "when I grow up!$"

ViridianCity_Text_GoShoppingInPewterOccasionally::
	.string "The fields west of town used to\n"
	.string "be so green.\p"
	.string "This dry spell's been hard on\n"
	.string "them. I hope the rain comes soon.$"

ViridianCity_Text_WeakenMonsFirstToCatch::
	.string "I've had my coffee, and now I'm\n"
	.string "ready for my walk!\p"
	.string "Every morning, out to ROUTE 22\n"
	.string "and back. Doctor's orders.$"

ViridianCity_Text_CitySign::
	.string "VIRIDIAN CITY \n"
	.string "The Eternally Green Paradise$"

ViridianCity_Text_CatchMonsForEasierBattles::
	.string "TRAINER TIPS\p"
	.string "Catch POKéMON and expand your\n"
	.string "collection.\p"
	.string "The more you have, the easier it\n"
	.string "is to battle.$"

ViridianCity_Text_MovesLimitedByPP::
	.string "TRAINER TIPS\p"
	.string "The battle moves of POKéMON are\n"
	.string "limited by their POWER POINTS, PP.\p"
	.string "To replenish PP, rest your tired\n"
	.string "POKéMON at a POKéMON CENTER.$"

ViridianCity_Text_GymSign::
	.string "VIRIDIAN CITY POKéMON GYM$"

ViridianCity_Text_GymDoorsAreLocked::
	.string "VIRIDIAN GYM's doors are locked…$"

