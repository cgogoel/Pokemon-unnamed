SaffronCity_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SaffronCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

SaffronCity_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_SAFFRON_CITY
	end

SaffronCity_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse_Frlg
	release
	end

SaffronCity_PokemonCenter_1F_EventScript_Gentleman::
	msgbox SaffronCity_PokemonCenter_1F_Text_SilphStockKeepsRising, MSGBOX_NPC
	end

SaffronCity_PokemonCenter_1F_EventScript_Woman::
	msgbox SaffronCity_PokemonCenter_1F_Text_GrowthRatesDifferBySpecies, MSGBOX_NPC
	end

SaffronCity_PokemonCenter_1F_EventScript_Youngster::
	msgbox SaffronCity_PokemonCenter_1F_Text_WaitingForSabrinaToOpenGym, MSGBOX_NPC
	end

SaffronCity_PokemonCenter_1F_Text_GrowthRatesDifferBySpecies::
	.string "POKéMON growth rates differ from\n"
	.string "species to species.$"

SaffronCity_PokemonCenter_1F_Text_SilphStockKeepsRising::
	.string "SILPH CO. is known all over the\n"
	.string "world for its POKé BALLS.\p"
	.string "I bought a few of their shares\n"
	.string "years ago. Best choice I ever made!$"

SaffronCity_PokemonCenter_1F_Text_WaitingForSabrinaToOpenGym::
	.string "I came all the way here to\n"
	.string "challenge SABRINA's GYM.\p"
	.string "But my ABRA keeps teleporting me\n"
	.string "back here whenever I get nervous!$"

