@ The gate between Celadon City's north edge and Celadon University (Act 2, docs/hack/celadon_university.md).

CeladonCity_UniversityGate_MapScripts::
	.byte 0

CeladonCity_UniversityGate_EventScript_Guard::
	msgbox CeladonCity_UniversityGate_Text_Guard, MSGBOX_NPC
	end

CeladonCity_UniversityGate_Text_Guard::
	.string "Welcome to Celadon University!\n"
	.string "The campus is just through there.$"
