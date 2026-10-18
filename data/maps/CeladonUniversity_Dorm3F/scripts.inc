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

CeladonUniversity_Dorm3F_Text_Rocker0::
	.string "Shh! I'm practicing guitar\n"
	.string "for the school festival.$"

CeladonUniversity_Dorm3F_Text_PokeManiac1::
	.string "I've read every textbook\n"
	.string "twice. Finals? Bring it on!$"

CeladonUniversity_Dorm3F_Text_FatMan2::
	.string "Midnight snack run?\n"
	.string "Count me in.$"

CeladonUniversity_Dorm3F_EventScript_Bed::
	msgbox CeladonUniversity_Dorm3F_Text_Bed, MSGBOX_SIGN
	end

CeladonUniversity_Dorm3F_Text_Bed::
	.string "An unmade bed, buried in\nlecture notes.$"
