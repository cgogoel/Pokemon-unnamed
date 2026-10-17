@ Celadon University: University Hospital (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_Hospital_MapScripts::
	.byte 0

CeladonUniversity_Hospital_EventScript_Nurse0::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse_Frlg
	release
	end

CeladonUniversity_Hospital_EventScript_Chansey1::
	lock
	faceplayer
	playmoncry SPECIES_CHANSEY, CRY_MODE_NORMAL
	msgbox CeladonUniversity_Hospital_Text_Chansey1
	waitmoncry
	release
	end

CeladonUniversity_Hospital_EventScript_Gentleman2::
	msgbox CeladonUniversity_Hospital_Text_Gentleman2, MSGBOX_NPC
	end

CeladonUniversity_Hospital_EventScript_CooltrainerF3::
	msgbox CeladonUniversity_Hospital_Text_CooltrainerF3, MSGBOX_NPC
	end

CeladonUniversity_Hospital_EventScript_Youngster4::
	msgbox CeladonUniversity_Hospital_Text_Youngster4, MSGBOX_NPC
	end

CeladonUniversity_Hospital_Text_Chansey1::
	.string "CHANSEY: Chaaan-sey!$"

CeladonUniversity_Hospital_Text_Gentleman2::
	.string "My GROWLITHE is in for a\n"
	.string "checkup. He hates needles.$"

CeladonUniversity_Hospital_Text_CooltrainerF3::
	.string "I'm an intern. Someday I'll\n"
	.string "be a POKéMON doctor!$"

CeladonUniversity_Hospital_Text_Youngster4::
	.string "The ward upstairs is for\n"
	.string "POKéMON that need rest.$"
