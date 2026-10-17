@ Celadon University: Faculty Office (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_FacultyOffice_MapScripts::
	.byte 0

CeladonUniversity_FacultyOffice_EventScript_BaldingMan0::
	msgbox CeladonUniversity_FacultyOffice_Text_BaldingMan0, MSGBOX_NPC
	end

CeladonUniversity_FacultyOffice_EventScript_Slowpoke1::
	lock
	faceplayer
	playmoncry SPECIES_SLOWPOKE, CRY_MODE_NORMAL
	msgbox CeladonUniversity_FacultyOffice_Text_Slowpoke1
	waitmoncry
	release
	end

CeladonUniversity_FacultyOffice_Text_BaldingMan0::
	.string "Grading papers...\n"
	.string "So many papers...$"

CeladonUniversity_FacultyOffice_Text_Slowpoke1::
	.string "SLOWPOKE: ...Yawn?$"
