SixIsland_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SixIsland_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

SixIsland_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_SIX_ISLAND
	end

SixIsland_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse_Frlg
	release
	end

SixIsland_PokemonCenter_1F_EventScript_Hiker::
	msgbox SixIsland_PokemonCenter_1F_Text_SomethingHiddenOnThisIsland, MSGBOX_NPC
	end

SixIsland_PokemonCenter_1F_EventScript_OldMan::
	msgbox SixIsland_PokemonCenter_1F_Text_SomeMonsEvolveByTradingWithHeldItem, MSGBOX_NPC
	end

SixIsland_PokemonCenter_1F_Text_SomethingHiddenOnThisIsland::
	.string "For ten years, I've searched for\n"
	.string "RUINS.\p"
	.string "No, make that twenty years.\p"
	.string "I get this feeling that there's\n"
	.string "something hidden on this island.$"

SixIsland_PokemonCenter_1F_Text_SomeMonsEvolveByTradingWithHeldItem::
	.string "You know of POKéMON that evolve\n"
	.string "only upon trading, yes?\p"
	.string "But did you know that there are\n"
	.string "still others?\p"
	.string "Some POKéMON evolve only when\n"
	.string "traded while holding specific items.$"
