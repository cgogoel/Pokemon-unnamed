CinnabarIsland_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CinnabarIsland_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CinnabarIsland_OnFrame
	.byte 0

CinnabarIsland_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_CINNABAR_ISLAND
	call CinnabarIsland_EventScript_CheckUnlockGym
	call CinnabarIsland_EventScript_Act2Objects
	end

CinnabarIsland_EventScript_CheckUnlockGym::
	goto_if_set FLAG_HIDE_POKEMON_MANSION_B1F_SECRET_KEY, CinnabarIsland_EventScript_UnlockGym
	return

CinnabarIsland_EventScript_UnlockGym::
	setvar VAR_TEMP_1, 1
	return

CinnabarIsland_EventScript_GymDoorLocked::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 20
	msgbox CinnabarIsland_Text_DoorIsLocked
	closemessage
	applymovement LOCALID_PLAYER, CinnabarIsland_Movement_ForcePlayerFromDoor
	waitmovement 0
	releaseall
	end

CinnabarIsland_Movement_ForcePlayerFromDoor::
	walk_down
	step_end

CinnabarIsland_EventScript_Woman::
	msgbox CinnabarIsland_Text_BlaineLivedHereSinceBeforeLab, MSGBOX_NPC
	end

CinnabarIsland_EventScript_OldMan::
	msgbox CinnabarIsland_Text_ScientistsExperimentInMansion, MSGBOX_NPC
	end

CinnabarIsland_EventScript_IslandSign::
	msgbox CinnabarIsland_Text_IslandSign, MSGBOX_SIGN
	end

CinnabarIsland_EventScript_PokemonLabSign::
	msgbox CinnabarIsland_Text_PokemonLab, MSGBOX_SIGN
	end

CinnabarIsland_EventScript_GymSign::
	lockall
	famechecker FAMECHECKER_BLAINE, 0
	msgbox CinnabarIsland_Text_GymSign
	releaseall
	end

CinnabarIsland_Text_DoorIsLocked::
	.string "The door is locked…$"

CinnabarIsland_Text_BlaineLivedHereSinceBeforeLab::
	.string "Students again? Every term a new\n"
	.string "class comes over on the ferry.\p"
	.string "The ice cream stand does great\n"
	.string "business on field trip days!$"

CinnabarIsland_Text_ScientistsExperimentInMansion::
	.string "That old mansion's been empty for\n"
	.string "years. Nobody lights the furnaces\l"
	.string "anymore.\p"
	.string "On cold nights, I swear I hear\n"
	.string "something moving around in there.$"

CinnabarIsland_Text_IslandSign::
	.string "CINNABAR ISLAND\n"
	.string "The Fiery Town of Burning Desire$"

CinnabarIsland_Text_PokemonLab::
	.string "POKéMON LAB$"

CinnabarIsland_Text_GymSign::
	.string "CINNABAR ISLAND POKéMON GYM\n"
	.string "LEADER: BLAINE\l"
	.string "The Hotheaded Quiz Master!$"
