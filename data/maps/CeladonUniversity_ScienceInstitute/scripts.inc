@ Celadon University: Science Research Institute, 1F of its glass tower (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_ScienceInstitute_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Act2_EventScript_InstituteObjects
	.byte 0

CeladonUniversity_ScienceInstitute_EventScript_Scientist0::
	msgbox CeladonUniversity_ScienceInstitute_Text_Scientist0, MSGBOX_NPC
	end

CeladonUniversity_ScienceInstitute_EventScript_Sign0::
	msgbox CeladonUniversity_ScienceInstitute_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_ScienceInstitute_EventScript_Sign1::
	msgbox CeladonUniversity_ScienceInstitute_Text_Sign1, MSGBOX_SIGN
	end

CeladonUniversity_ScienceInstitute_Text_Scientist0::
	.string "Welcome to the POKéMON SCIENCE\n"
	.string "RESEARCH INSTITUTE.\p"
	.string "We research POKéMON with\n"
	.string "the latest technology.$"

CeladonUniversity_ScienceInstitute_Text_Sign0::
	.string "A plaque: 'Where the strongest\n"
	.string "MAGIKARP was born.'$"

CeladonUniversity_ScienceInstitute_Text_Sign1::
	.string "2F INSTITUTE LOUNGE\n"
	.string "3F RESEARCH ROOM\p"
	.string "4F EXPERIMENT ROOM$"
