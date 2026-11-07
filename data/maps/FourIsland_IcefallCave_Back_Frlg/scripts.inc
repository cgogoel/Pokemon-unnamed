FourIsland_IcefallCave_Back_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FourIsland_IcefallCave_Back_OnTransition
	.byte 0

FourIsland_IcefallCave_Back_OnTransition::
	call_if_eq VAR_MAP_SCENE_ICEFALL_CAVE_BACK, 1, FourIsland_IcefallCave_Back_EventScript_HideLorelei
	end

FourIsland_IcefallCave_Back_EventScript_HideLorelei::
	setflag FLAG_HIDE_ICEFALL_CAVE_LORELEI
	return

FourIsland_IcefallCave_Back_EventScript_Lorelei::
	lock
	faceplayer
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox FourIsland_IcefallCave_Back_Text_LoreleiChecksOnCave
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_ICEFALL_LORELEI
	fadescreenswapbuffers FADE_FROM_BLACK
	setflag FLAG_HIDE_RUIN_VALLEY_SCIENTIST
	clearflag FLAG_HIDE_LORELEI_IN_HER_HOUSE
	setvar VAR_MAP_SCENE_ICEFALL_CAVE_BACK, 1
	release
	end

FourIsland_IcefallCave_Back_Text_LoreleiChecksOnCave::
	.string "LORELEI: Oh! I didn't expect\n"
	.string "anyone back here.\p"
	.string "I grew up on FOUR ISLAND. When\n"
	.string "I'm home, I check on this cave.\p"
	.string "My LAPRAS was a baby here once.\n"
	.string "Its old friends are all well.\p"
	.string "I'll be heading home now.\n"
	.string "Mind the ice on your way out.$"
