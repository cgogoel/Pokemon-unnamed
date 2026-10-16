.equ SIGN_LADY_READY, VAR_TEMP_2

PalletTown_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_OnTransition
	.byte 0

PalletTown_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_PALLET_TOWN
	special Act1_RestockLovedItems
	call_if_set FLAG_PALLET_LADY_NOT_BLOCKING_SIGN, PalletTown_EventScript_TryReadySignLady
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 0, PalletTown_EventScript_SetSignLadyPos
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, PalletTown_EventScript_SetSignLadyDone
	end

PalletTown_EventScript_TryReadySignLady::
	goto_if_unset FLAG_OPENED_START_MENU, Common_EventScript_NopReturn
	goto_if_ge VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, Common_EventScript_NopReturn
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1
	return

PalletTown_EventScript_SetSignLadyDone::
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 2
	return

PalletTown_EventScript_SetSignLadyPos::
	goto_if_set FLAG_PALLET_LADY_NOT_BLOCKING_SIGN, PalletTown_EventScript_MoveSignLadyToRouteEntrance
	setobjectxyperm LOCALID_PALLET_SIGN_LADY, 5, 15
	setobjectmovementtype LOCALID_PALLET_SIGN_LADY, MOVEMENT_TYPE_FACE_UP
	return

PalletTown_EventScript_MoveSignLadyToRouteEntrance::
	setobjectxyperm LOCALID_PALLET_SIGN_LADY, 12, 2
	setobjectmovementtype LOCALID_PALLET_SIGN_LADY, MOVEMENT_TYPE_FACE_DOWN
	setvar SIGN_LADY_READY, TRUE
	return

PalletTown_EventScript_SignLady::
	lock
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 2, PalletTown_EventScript_SignLadyDone
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, PalletTown_EventScript_SignLadyJustShowedSign
	goto_if_eq SIGN_LADY_READY, TRUE, PalletTown_EventScript_SignLadyStartShowSign
	goto_if_set FLAG_TEMP_2, PalletTown_EventScript_SignLadyGoReadSign
	msgbox PalletTown_Text_HmmIsThatRight
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_Delay48
	waitmovement 0
	msgbox PalletTown_Text_OhLookLook
	closemessage
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_EventScript_SignLadyMoveOutOfWayRight
	call_if_ne VAR_FACING, DIR_EAST, PalletTown_EventScript_SignLadyMoveOutOfWayLeft
	copyobjectxytoperm LOCALID_PALLET_SIGN_LADY
	setflag FLAG_TEMP_2
	release
	end

PalletTown_EventScript_SignLadyMoveOutOfWayRight::
	applymovement LOCALID_PALLET_SIGN_LADY, PalletTown_Movement_SignLadyMoveOutOfWayRight
	waitmovement 0
	return

PalletTown_EventScript_SignLadyMoveOutOfWayLeft::
	applymovement LOCALID_PALLET_SIGN_LADY, PalletTown_Movement_SignLadyMoveOutOfWayLeft
	waitmovement 0
	return

PalletTown_EventScript_SignLadyDone::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_RaisingMonsToo
	release
	end

PalletTown_EventScript_SignLadyGoReadSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_ReadItReadIt
	release
	end

PalletTown_EventScript_SignLadyJustShowedSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_SignsAreUsefulArentThey
	release
	end

PalletTown_Movement_SignLadyMoveOutOfWayRight::
	walk_right
	walk_in_place_faster_left
	step_end

PalletTown_Movement_SignLadyMoveOutOfWayLeft::
	walk_left
	walk_in_place_faster_right
	step_end

PalletTown_EventScript_FatMan::
	goto_if_eq VAR_STORY_ACT, STORY_ACT_CHILDHOOD, PalletTown_EventScript_FatManAct1
	msgbox PalletTown_Text_CanStoreItemsAndMonsInPC, MSGBOX_NPC
	end

@ Act 1: after BLIPBUG he has a hint for TYNAMO (docs/hack/act1_story.md)
PalletTown_EventScript_FatManAct1::
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, PalletTown_EventScript_FatManAct1Default
	goto_if_set FLAG_BEFRIENDED_TYNAMO, PalletTown_EventScript_FatManAct1Default
	msgbox PalletTown_Text_FatManTynamoHint, MSGBOX_NPC
	end

PalletTown_EventScript_FatManAct1Default::
	msgbox PalletTown_Text_FatManChinUp, MSGBOX_NPC
	end

PalletTown_EventScript_ItemCheriBerry::
	finditem ITEM_CHERI_BERRY
	end

PalletTown_Text_FatManChinUp::
	.string "Hey, {PLAYER}! I heard PROF. OAK\n"
	.string "picked his new TRAINERS.\p"
	.string "Chin up! There's more to life than\n"
	.string "POKéMON battles, you know.$"

PalletTown_Text_FatManTynamoHint::
	.string "A TYNAMO? Tiny thing, but it's\n"
	.string "full of spark!\p"
	.string "Electric POKéMON love spicy food.\n"
	.string "A CHERI BERRY would do it.\p"
	.string "There's a CHERI BERRY bush down by\n"
	.string "the water, south of town.$"

PalletTown_EventScript_OaksLabSign::
	lockall
	famechecker FAMECHECKER_OAK, 0
	msgbox PalletTown_Text_OakPokemonResearchLab
	releaseall
	end

PalletTown_EventScript_PlayersHouseSign::
	msgbox PalletTown_Text_PlayersHouse, MSGBOX_SIGN
	end

PalletTown_EventScript_RivalsHouseSign::
	msgbox PalletTown_Text_RivalsHouse, MSGBOX_SIGN
	end

PalletTown_EventScript_TownSign::
	msgbox PalletTown_Text_TownSign, MSGBOX_SIGN
	end

PalletTown_EventScript_TrainerTips::
	lockall
	msgbox PalletTown_Text_PressStartToOpenMenu
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1
	releaseall
	end

PalletTown_EventScript_SignLadyTrigger::
	lockall
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	call PalletTown_EventScript_SignLadyShowSign
	releaseall
	end

PalletTown_EventScript_SignLadyShowSign::
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox PalletTown_Text_LookCopiedTrainerTipsSign
	closemessage
	delay 20
	textcolor NPC_TEXT_COLOR_NEUTRAL
	setflag FLAG_OPENED_START_MENU
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1
	setvar SIGN_LADY_READY, FALSE
	special SetWalkingIntoSignVars
	special DisableMsgBoxWalkaway
	signmsg
	msgbox PalletTown_Text_PressStartToOpenMenuCopy
	normalmsg
	return

PalletTown_EventScript_SignLadyStartShowSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	call PalletTown_EventScript_SignLadyShowSign
	release
	end

PalletTown_Text_RaisingMonsToo::
	.string "I'm raising POKéMON, too.\p"
	.string "When they get strong, they can\n"
	.string "protect me.$"

PalletTown_Text_CanStoreItemsAndMonsInPC::
	.string "Technology is incredible!\p"
	.string "You can now store and recall items\n"
	.string "and POKéMON as data via PC.$"

PalletTown_Text_OakPokemonResearchLab::
	.string "OAK POKéMON RESEARCH LAB$"

@ Act 1-2: the original player's house is for sale (Act 3 has its own Pallet Town and house maps)
PalletTown_Text_PlayersHouse::
	.string "For Sale:\n"
	.string "Move-in Ready Small Town Home$"

PalletTown_Text_RivalsHouse::
	.string "{RIVAL}'s house$"

PalletTown_Text_TownSign::
	.string "PALLET TOWN\n"
	.string "Shades of your journey await!$"

PalletTown_Text_HmmIsThatRight::
	.string "Hmm…\n"
	.string "Is that right…$"

PalletTown_Text_OhLookLook::
	.string "Oh!\n"
	.string "Look, look!$"

PalletTown_Text_ReadItReadIt::
	.string "Read it, read it!$"

@ Used by sign
PalletTown_Text_PressStartToOpenMenu::
	.string "TRAINER TIPS\p"
	.string "Press START to open the MENU!$"

PalletTown_Text_SignsAreUsefulArentThey::
	.string "Signs are useful, aren't they?$"

PalletTown_Text_LookCopiedTrainerTipsSign::
	.string "Look, look!\p"
	.string "I copied what it said on one of\n"
	.string "those TRAINER TIPS signs!$"

@ Used by NPC
PalletTown_Text_PressStartToOpenMenuCopy::
	.string "TRAINER TIPS!\p"
	.string "Press START to open the MENU!$"

@ Act 1: the player can't leave Pallet Town yet (VAR_STORY_ACT is STORY_ACT_CHILDHOOD)
PalletTown_EventScript_Act1Boundary::
	lockall
	msgbox PalletTown_Text_Act1TooFar, MSGBOX_DEFAULT
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

PalletTown_Text_Act1TooFar:
	.string "It's too far to go on my own.\n"
	.string "Better stay close to home.$"
