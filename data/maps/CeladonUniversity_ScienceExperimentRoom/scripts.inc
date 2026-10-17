@ Celadon University: Experiment Room (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_ScienceExperimentRoom_MapScripts::
	.byte 0

CeladonUniversity_ScienceExperimentRoom_EventScript_Boy0::
	msgbox CeladonUniversity_ScienceExperimentRoom_Text_Boy0, MSGBOX_NPC
	end

CeladonUniversity_ScienceExperimentRoom_EventScript_Scientist1::
	msgbox CeladonUniversity_ScienceExperimentRoom_Text_Scientist1, MSGBOX_NPC
	end

CeladonUniversity_ScienceExperimentRoom_Text_Boy0::
	.string "I'm a student assistant.\n"
	.string "Mostly I clean the tanks.$"

CeladonUniversity_ScienceExperimentRoom_Text_Scientist1::
	.string "The strongest MAGIKARP came\n"
	.string "out of this very room!$"
