@ Celadon University: Library (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_Library_MapScripts::
	.byte 0

CeladonUniversity_Library_EventScript_Woman20::
	msgbox CeladonUniversity_Library_Text_Woman20, MSGBOX_NPC
	end

CeladonUniversity_Library_EventScript_Gentleman1::
	msgbox CeladonUniversity_Library_Text_Gentleman1, MSGBOX_NPC
	end

CeladonUniversity_Library_EventScript_Sign0::
	msgbox CeladonUniversity_Library_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_Library_Text_Woman20::
	.string "Please keep your voice down\n"
	.string "in the library.$"

CeladonUniversity_Library_Text_Gentleman1::
	.string "This book lists every move\n"
	.string "ever recorded.\p"
	.string "There's a blank page at the\n"
	.string "end. Curious, isn't it?$"

CeladonUniversity_Library_Text_Sign0::
	.string "Books on POKéMON moves and\n"
	.string "where they came from.$"
