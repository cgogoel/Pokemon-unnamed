PalletTown_PlayersHouse_2F_Frlg_Farm_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Farmhouse2F_OnFrame
	.byte 0

Farmhouse2F_OnFrame:
	map_script_2 VAR_ACT1_WOKE_UP, 2, Farmhouse2F_EventScript_WokeUp
	.2byte 0

@ The morning after bringing Jangmo-o home (docs/hack/act1_story.md)
Farmhouse2F_EventScript_WokeUp::
	lockall
	msgbox Farmhouse2F_Text_WokeUp
	setvar VAR_ACT1_WOKE_UP, 0
	releaseall
	end

Farmhouse2F_Text_WokeUp:
	.string "{PLAYER} woke up feeling refreshed.$"

@ Replaces the vanilla notice about a HELP system this game doesn't have
Farmhouse2F_EventScript_ChoreChart::
	msgbox Farmhouse2F_Text_ChoreChart, MSGBOX_SIGN
	end

Farmhouse2F_Text_ChoreChart::
	.string "It's a chore chart.\p"
	.string "{PLAYER}: Feed the MAREEP.\n"
	.string "DAD: Fix the barn door.\l"
	.string "MOM: Make DAD fix the barn door.$"
