@ Celadon University: Dorms 2F (Act 2, docs/hack/celadon_university.md). The third room is the player's:
@ Act 2 starts there (ACT2_START_MAP).

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

@ The player's room
CeladonUniversity_Dorm2F_EventScript_LaprasDoll::
	msgbox CeladonUniversity_Dorm2F_Text_LaprasDoll, MSGBOX_NPC
	end

CeladonUniversity_Dorm2F_EventScript_YourBed::
	msgbox CeladonUniversity_Dorm2F_Text_YourBed, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_EventScript_Schedule::
	msgbox CeladonUniversity_Dorm2F_Text_Schedule, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_EventScript_Textbooks::
	msgbox CeladonUniversity_Dorm2F_Text_Textbooks, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_EventScript_Desk::
	msgbox CeladonUniversity_Dorm2F_Text_Desk, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_EventScript_Photo::
	msgbox CeladonUniversity_Dorm2F_Text_Photo, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_Text_LaprasDoll::
	.string "A LAPRAS doll.\n"
	.string "A going-away gift from Mom.$"

CeladonUniversity_Dorm2F_Text_YourBed::
	.string "Your bed. It's not as comfy\n"
	.string "as the one back home.$"

CeladonUniversity_Dorm2F_Text_Schedule::
	.string "This term's course schedule is\n"
	.string "taped to the mirror.$"

CeladonUniversity_Dorm2F_Text_Textbooks::
	.string "Textbooks from four years of\n"
	.string "courses, covered in notes.$"

CeladonUniversity_Dorm2F_Text_Desk::
	.string "A study desk. Nothing on it\n"
	.string "yet but a pencil.$"

CeladonUniversity_Dorm2F_Text_Photo::
	.string "A photo of the farm back\n"
	.string "home, with Mom and Dad.$"

CeladonUniversity_Dorm2F_EventScript_YourRoomPlate::
	msgbox CeladonUniversity_Dorm2F_Text_YourRoomPlate, MSGBOX_SIGN
	end

CeladonUniversity_Dorm2F_Text_YourRoomPlate::
	.string "{PLAYER}'s room.$"
