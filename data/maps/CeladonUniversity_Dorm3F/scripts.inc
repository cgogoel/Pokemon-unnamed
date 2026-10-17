@ Celadon University: Dorms 3F (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_Dorm3F_MapScripts::
	.byte 0

CeladonUniversity_Dorm3F_EventScript_Rocker0::
	msgbox CeladonUniversity_Dorm3F_Text_Rocker0, MSGBOX_NPC
	end

CeladonUniversity_Dorm3F_EventScript_PokeManiac1::
	msgbox CeladonUniversity_Dorm3F_Text_PokeManiac1, MSGBOX_NPC
	end

CeladonUniversity_Dorm3F_EventScript_FatMan2::
	msgbox CeladonUniversity_Dorm3F_Text_FatMan2, MSGBOX_NPC
	end

CeladonUniversity_Dorm3F_EventScript_Sign0::
	msgbox CeladonUniversity_Dorm3F_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_Dorm3F_EventScript_Blocked0::
	lockall
	msgbox CeladonUniversity_Dorm3F_Text_Blocked0
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

CeladonUniversity_Dorm3F_EventScript_Blocked1::
	lockall
	msgbox CeladonUniversity_Dorm3F_Text_Blocked1
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkDown
	waitmovement 0
	releaseall
	end

CeladonUniversity_Dorm3F_Text_Rocker0::
	.string "Shh! I'm practicing guitar\n"
	.string "for the school festival.$"

CeladonUniversity_Dorm3F_Text_PokeManiac1::
	.string "I've read every textbook\n"
	.string "twice. Finals? Bring it on!$"

CeladonUniversity_Dorm3F_Text_FatMan2::
	.string "Midnight snack run?\n"
	.string "Count me in.$"

CeladonUniversity_Dorm3F_Text_Sign0::
	.string "Someone's essay is half-\n"
	.string "written on the computer.$"

CeladonUniversity_Dorm3F_Text_Blocked0::
	.string "The door to the roof is\n"
	.string "locked.$"

CeladonUniversity_Dorm3F_Text_Blocked1::
	.string "The door to the roof is\n"
	.string "locked.$"
