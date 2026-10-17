@ Celadon University: Research Room (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_ScienceResearchRoom_MapScripts::
	.byte 0

CeladonUniversity_ScienceResearchRoom_EventScript_PokeManiac0::
	msgbox CeladonUniversity_ScienceResearchRoom_Text_PokeManiac0, MSGBOX_NPC
	end

CeladonUniversity_ScienceResearchRoom_EventScript_Scientist1::
	msgbox CeladonUniversity_ScienceResearchRoom_Text_Scientist1, MSGBOX_NPC
	end

CeladonUniversity_ScienceResearchRoom_EventScript_Sign0::
	msgbox CeladonUniversity_ScienceResearchRoom_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_ScienceResearchRoom_Text_PokeManiac0::
	.string "Look at this data!\n"
	.string "MAGIKARP is amazing!$"

CeladonUniversity_ScienceResearchRoom_Text_Scientist1::
	.string "We're measuring how far\n"
	.string "training can take a POKéMON.$"

CeladonUniversity_ScienceResearchRoom_Text_Sign0::
	.string "Data on MAGIKARP growth\n"
	.string "scrolls across the screen.$"
