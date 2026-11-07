@ Celadon University: Research Room (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_ScienceResearchRoom_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Act2_EventScript_LabObjects
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CeladonUniversity_ScienceResearchRoom_OnFrame
	.byte 0

CeladonUniversity_ScienceResearchRoom_OnFrame:
	map_script_2 VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED, Act2_EventScript_TriBlastReveal
	.2byte 0

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

@ The TM Database (docs/hack/tm_database.md): the researcher explains it from the final project on (and in Act 3),
@ which opens the TM MACHINE beside them and QUESTS' TM RESEARCH tab
CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcher::
	lock
	faceplayer
	goto_if_set FLAG_TM_DATABASE_UNLOCKED, CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcherAgain
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcherExplain
	goto_if_ge VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT, CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcherExplain
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmResearcherEarly
	release
	end

CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcherExplain:
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmResearcherExplain
	special TmResearch_Unlock
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message CeladonUniversity_ScienceResearchRoom_Text_TmDatabaseUnlocked
	waitmessage
	waitfanfare
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmResearcherTryIt
	release
	end

CeladonUniversity_ScienceResearchRoom_EventScript_TmResearcherAgain:
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmResearcherAgain
	release
	end

@ The TM MACHINE: the database's TMs (TmDatabase_OpenFromMachine, src/quest_menu.c)
CeladonUniversity_ScienceResearchRoom_EventScript_TmMachine::
	lockall
	goto_if_unset FLAG_TM_DATABASE_UNLOCKED, CeladonUniversity_ScienceResearchRoom_EventScript_TmMachineLocked
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmMachineOn
	closemessage
	fadescreen FADE_TO_BLACK
	special TmDatabase_OpenFromMachine
	releaseall
	end

CeladonUniversity_ScienceResearchRoom_EventScript_TmMachineLocked:
	msgbox CeladonUniversity_ScienceResearchRoom_Text_TmMachineLocked
	releaseall
	end

CeladonUniversity_ScienceResearchRoom_Text_TmResearcherEarly::
	.string "RESEARCHER: This is the TM MACHINE.\n"
	.string "It's for students on their final\l"
	.string "project. Finish your courses first!$"

CeladonUniversity_ScienceResearchRoom_Text_TmResearcherExplain::
	.string "RESEARCHER: The DIRECTOR sent you?\n"
	.string "Then the TM MACHINE is yours.\p"
	.string "It holds the TM DATABASE: every TM\n"
	.string "we know how to make.\p"
	.string "Pick up to 3 TMs to research, but\n"
	.string "only one of each type at a time.\p"
	.string "For each one, catch different\n"
	.string "POKéMON of its type, and record\l"
	.string "different moves of its type in\l"
	.string "battle. Theirs count, too.\p"
	.string "Stronger moves need more: 2 of\n"
	.string "each for the weakest, up to 5 for\l"
	.string "the most powerful.\p"
	.string "Your progress shows in QUESTS.\n"
	.string "Press L or R there: TM RESEARCH.\p"
	.string "When a TM's research is done,\n"
	.string "craft it right there in QUESTS.\l"
	.string "That frees its slot again.\p"
	.string "Changed your mind? Clear a slot\n"
	.string "any time, but its count is lost.\p"
	.string "Oh, and if you ever catch every\n"
	.string "POKéMON of a type, all its TMs\l"
	.string "are yours to craft. No research!$"

CeladonUniversity_ScienceResearchRoom_Text_TmDatabaseUnlocked::
	.string "{PLAYER} can now use the\n"
	.string "TM DATABASE!$"

CeladonUniversity_ScienceResearchRoom_Text_TmResearcherTryIt::
	.string "RESEARCHER: The machine's right\n"
	.string "beside me. Give it a try!$"

CeladonUniversity_ScienceResearchRoom_Text_TmResearcherAgain::
	.string "RESEARCHER: Pick TMs at the\n"
	.string "machine, then follow them in\l"
	.string "QUESTS under TM RESEARCH.$"

CeladonUniversity_ScienceResearchRoom_Text_TmMachineOn::
	.string "{PLAYER} switched on the\n"
	.string "TM MACHINE.$"

CeladonUniversity_ScienceResearchRoom_Text_TmMachineLocked::
	.string "It's the TM MACHINE.\n"
	.string "Ask the researcher about it first.$"
