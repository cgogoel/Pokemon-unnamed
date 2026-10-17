@ Celadon University's campus (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CeladonUniversity_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CeladonUniversity_OnFrame
	.byte 0

CeladonUniversity_EventScript_Lass0::
	msgbox CeladonUniversity_Text_Lass0, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Youngster1::
	msgbox CeladonUniversity_Text_Youngster1, MSGBOX_NPC
	end

CeladonUniversity_EventScript_CooltrainerM2::
	msgbox CeladonUniversity_Text_CooltrainerM2, MSGBOX_NPC
	end

CeladonUniversity_EventScript_CooltrainerF3::
	msgbox CeladonUniversity_Text_CooltrainerF3, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Scientist4::
	msgbox CeladonUniversity_Text_Scientist4, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Man5::
	msgbox CeladonUniversity_Text_Man5, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Ponyta6::
	lock
	faceplayer
	playmoncry SPECIES_PONYTA, CRY_MODE_NORMAL
	msgbox CeladonUniversity_Text_Ponyta6
	waitmoncry
	release
	end

CeladonUniversity_EventScript_Ponyta7::
	lock
	faceplayer
	playmoncry SPECIES_PONYTA, CRY_MODE_NORMAL
	msgbox CeladonUniversity_Text_Ponyta7
	waitmoncry
	release
	end

CeladonUniversity_EventScript_SwimmerFWater8::
	msgbox CeladonUniversity_Text_SwimmerFWater8, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Psyduck9::
	lock
	faceplayer
	playmoncry SPECIES_PSYDUCK, CRY_MODE_NORMAL
	msgbox CeladonUniversity_Text_Psyduck9
	waitmoncry
	release
	end

CeladonUniversity_EventScript_Beauty10::
	msgbox CeladonUniversity_Text_Beauty10, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Gentleman11::
	msgbox CeladonUniversity_Text_Gentleman11, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Boy12::
	msgbox CeladonUniversity_Text_Boy12, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Woman213::
	msgbox CeladonUniversity_Text_Woman213, MSGBOX_NPC
	end

CeladonUniversity_EventScript_BaldingMan14::
	msgbox CeladonUniversity_Text_BaldingMan14, MSGBOX_NPC
	end

CeladonUniversity_EventScript_Sign0::
	msgbox CeladonUniversity_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign1::
	msgbox CeladonUniversity_Text_Sign1, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign2::
	msgbox CeladonUniversity_Text_Sign2, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign3::
	msgbox CeladonUniversity_Text_Sign3, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign4::
	msgbox CeladonUniversity_Text_Sign4, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign5::
	msgbox CeladonUniversity_Text_Sign5, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign6::
	msgbox CeladonUniversity_Text_Sign6, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign7::
	msgbox CeladonUniversity_Text_Sign7, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign8::
	msgbox CeladonUniversity_Text_Sign8, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign9::
	msgbox CeladonUniversity_Text_Sign9, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign10::
	msgbox CeladonUniversity_Text_Sign10, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign11::
	msgbox CeladonUniversity_Text_Sign11, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign12::
	msgbox CeladonUniversity_Text_Sign12, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign13::
	msgbox CeladonUniversity_Text_Sign13, MSGBOX_SIGN
	end

CeladonUniversity_EventScript_Sign14::
	msgbox CeladonUniversity_Text_Sign14, MSGBOX_SIGN
	end

CeladonUniversity_Text_Lass0::
	.string "When the GYARADOS fountain\n"
	.string "sprays, it's noon.\p"
	.string "That's the signal for lunch!$"

CeladonUniversity_Text_Youngster1::
	.string "First week too?\p"
	.string "The MAIN HALL is the one\n"
	.string "with the clock tower.$"

CeladonUniversity_Text_CooltrainerM2::
	.string "Phys Ed means laps.\n"
	.string "My POKéMON runs them with me!$"

CeladonUniversity_Text_CooltrainerF3::
	.string "Every course ends with a\n"
	.string "practical exam in this stadium.$"

CeladonUniversity_Text_Scientist4::
	.string "The SCIENCE RESEARCH INSTITUTE\n"
	.string "uses the very latest technology.\p"
	.string "They say the strongest MAGIKARP\n"
	.string "ever was developed in there.$"

CeladonUniversity_Text_Man5::
	.string "This is the research ranch.\p"
	.string "We study how POKéMON grow\n"
	.string "when they're raised with care.$"

CeladonUniversity_Text_Ponyta6::
	.string "PONYTA: Neigh!$"

CeladonUniversity_Text_Ponyta7::
	.string "PONYTA: Hrrr...$"

CeladonUniversity_Text_SwimmerFWater8::
	.string "The pool's open to every\n"
	.string "student. WATER types too!$"

CeladonUniversity_Text_Psyduck9::
	.string "PSYDUCK: Psy...?$"

CeladonUniversity_Text_Beauty10::
	.string "Home Economics studies the\n"
	.string "beauty of flowers and tea.\p"
	.string "I heard ERIKA of the CELADON\n"
	.string "GYM drops by sometimes!$"

CeladonUniversity_Text_Gentleman11::
	.string "The Literature department\n"
	.string "puts POKéMON into words.\p"
	.string "The library next door has\n"
	.string "books on every move known.$"

CeladonUniversity_Text_Boy12::
	.string "The dorms are right here.\n"
	.string "Curfew's ten, officially.$"

CeladonUniversity_Text_Woman213::
	.string "Students at the hospital are\n"
	.string "training to be POKéMON doctors.$"

CeladonUniversity_Text_BaldingMan14::
	.string "The school emblem? A MAGIKARP\n"
	.string "and a GYARADOS.\p"
	.string "It means anyone can grow,\n"
	.string "if they put in the effort.$"

CeladonUniversity_Text_Sign0::
	.string "CELADON UNIVERSITY\n"
	.string "Welcome, students!$"

CeladonUniversity_Text_Sign1::
	.string "MAIN HALL\n"
	.string "The clock tower$"

CeladonUniversity_Text_Sign2::
	.string "LECTURE HALL A$"

CeladonUniversity_Text_Sign3::
	.string "AUDIOVISUAL CLASSROOM$"

CeladonUniversity_Text_Sign4::
	.string "CAFETERIA\n"
	.string "Open breakfast to dinner$"

CeladonUniversity_Text_Sign5::
	.string "FACULTY OFFICE$"

CeladonUniversity_Text_Sign6::
	.string "DEPARTMENT OF POKéMON\n"
	.string "LITERATURE$"

CeladonUniversity_Text_Sign7::
	.string "UNIVERSITY LIBRARY\n"
	.string "Please be quiet inside.$"

CeladonUniversity_Text_Sign8::
	.string "POKéMON SCIENCE\n"
	.string "RESEARCH INSTITUTE$"

CeladonUniversity_Text_Sign9::
	.string "CELADON UNIVERSITY\n"
	.string "HOSPITAL$"

CeladonUniversity_Text_Sign10::
	.string "DORMITORIES\n"
	.string "Residents only past the lobby.$"

CeladonUniversity_Text_Sign11::
	.string "DEPARTMENT OF POKéMON\n"
	.string "HOME ECONOMICS$"

CeladonUniversity_Text_Sign12::
	.string "STADIUM\n"
	.string "Physical Education$"

CeladonUniversity_Text_Sign13::
	.string "RESEARCH RANCH\n"
	.string "Please don't feed the PONYTA.$"

CeladonUniversity_Text_Sign14::
	.string "POOL\n"
	.string "No running!$"
