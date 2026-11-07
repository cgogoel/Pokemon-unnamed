PalletTown_PlayersHouse_2F_Frlg_Farm_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Farmhouse2F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Farmhouse2F_OnFrame
	.byte 0

@ KAI, MINA and ROWAN (on the stairs, (10,2)) only come up the stairs in Act 2's ending (Act2_EventScript_FriendsVisit
@ shows them one by one); otherwise they're hidden, or they'd stand in the way at the top of the stairs
Farmhouse2F_OnTransition:
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	end

Farmhouse2F_OnFrame:
	map_script_2 VAR_ACT1_WOKE_UP, 2, Farmhouse2F_EventScript_WokeUp
	map_script_2 VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED, Act2_EventScript_FriendsVisit
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
