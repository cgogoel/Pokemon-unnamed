Route2_ViridianForest_SouthEntrance_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route2_ViridianForest_SouthEntrance_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Route2_ViridianForest_SouthEntrance_OnFrame
	.byte 0

@ The Grass trip's class meets here (data/scripts/act2_story.inc)
Route2_ViridianForest_SouthEntrance_OnTransition::
	call Route2_ViridianForest_SouthEntrance_EventScript_Act2Objects
	end

Route2_ViridianForest_SouthEntrance_EventScript_Woman1::
	msgbox Route2_ViridianForest_SouthEntrance_Text_ForestIsMaze, MSGBOX_NPC
	end

Route2_ViridianForest_SouthEntrance_EventScript_Woman2::
	msgbox Route2_ViridianForest_SouthEntrance_Text_RattataHasWickedBite, MSGBOX_NPC
	end

Route2_ViridianForest_SouthEntrance_Text_ForestIsMaze::
	.string "Are you going to VIRIDIAN FOREST?\n"
	.string "It's a natural maze in there.\l"
	.string "Be careful you don't get lost.$"

Route2_ViridianForest_SouthEntrance_Text_RattataHasWickedBite::
	.string "RATTATA may be small, but don't\n"
	.string "underestimate its wicked bite.\p"
	.string "Have you caught one already?$"

