@ Celadon University: Dorms 2F (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_Dorm2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CeladonUniversity_Dorm2F_OnTransition
	.byte 0

CeladonUniversity_Dorm2F_EventScript_CooltrainerM0::
	msgbox CeladonUniversity_Dorm2F_Text_CooltrainerM0, MSGBOX_NPC
	end

CeladonUniversity_Dorm2F_EventScript_Lass1::
	msgbox CeladonUniversity_Dorm2F_Text_Lass1, MSGBOX_NPC
	end

CeladonUniversity_Dorm2F_EventScript_Sign0::
	msgbox CeladonUniversity_Dorm2F_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_Text_CooltrainerM0::
	.string "My roommate snores like\n"
	.string "a SNORLAX.$"

CeladonUniversity_Dorm2F_Text_Lass1::
	.string "Study group in the lounge\n"
	.string "tonight. Want to join?$"

CeladonUniversity_Dorm2F_Text_Sign0::
	.string "A message board covered in\n"
	.string "notes about study groups.$"

CeladonUniversity_Dorm2F_EventScript_Bed::
	msgbox CeladonUniversity_Dorm2F_Text_Bed, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_Text_Bed::
	.string "A neatly made bed. Someone here\nstudies late, by the look of it.$"
