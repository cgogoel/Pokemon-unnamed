FourIsland_LoreleisHouse_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, FourIsland_LoreleisHouse_OnTransition
	.byte 0

FourIsland_LoreleisHouse_OnTransition::
	special UpdateLoreleiDollCollection
	end

FourIsland_LoreleisHouse_EventScript_Lorelei::
	lock
	faceplayer
	msgbox FourIsland_LoreleisHouse_Text_HomeBetweenLeagueSeasons
	release
	end

FourIsland_LoreleisHouse_EventScript_Doll::
	lock
	msgbox FourIsland_LoreleisHouse_Text_StuffedMonDollsGalore
	release
	end

FourIsland_LoreleisHouse_Text_HomeBetweenLeagueSeasons::
	.string "LORELEI: Welcome. Between LEAGUE\n"
	.string "seasons, I come home to rest.\p"
	.string "The INDIGO PLATEAU is grand, but\n"
	.string "nothing beats this sea breeze.\p"
	.string "…The dolls? Please don't stare.\n"
	.string "Every visit, the pile grows.$"

FourIsland_LoreleisHouse_Text_StuffedMonDollsGalore::
	.string "Stuffed POKéMON dolls galore!$"

