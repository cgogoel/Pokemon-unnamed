LavenderTown_PokemonCenter_1F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LavenderTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

LavenderTown_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_LAVENDER_TOWN
	end

LavenderTown_PokemonCenter_1F_EventScript_Nurse::
	lock
	faceplayer
	call EventScript_PkmnCenterNurse_Frlg
	release
	end

LavenderTown_PokemonCenter_1F_EventScript_Gentleman::
	msgbox LavenderTown_PokemonCenter_1F_Text_RadioTowerPlans, MSGBOX_NPC
	end

LavenderTown_PokemonCenter_1F_EventScript_Lass::
	msgbox LavenderTown_PokemonCenter_1F_Text_NurseKnitsScarves, MSGBOX_NPC
	end

LavenderTown_PokemonCenter_1F_EventScript_Youngster::
	msgbox LavenderTown_PokemonCenter_1F_Text_CuboneSkullsAreHeirlooms, MSGBOX_NPC
	end

LavenderTown_PokemonCenter_1F_Text_RadioTowerPlans::
	.string "The town council wants to build\n"
	.string "a radio tower one day.\p"
	.string "Imagine LAVENDER's quiet nights\n"
	.string "full of music! I'm not sure I'd\l"
	.string "like it.$"

LavenderTown_PokemonCenter_1F_Text_NurseKnitsScarves::
	.string "The NURSE here knits little scarves\n"
	.string "for the POKéMON in MR. FUJI's care.\p"
	.string "She gave me one for my ODDISH.\n"
	.string "It won't take it off!$"

LavenderTown_PokemonCenter_1F_Text_CuboneSkullsAreHeirlooms::
	.string "You know how the CUBONE species\n"
	.string "wears skulls, right?\p"
	.string "They never take them off, not\n"
	.string "even to sleep. Tough little guys.$"

