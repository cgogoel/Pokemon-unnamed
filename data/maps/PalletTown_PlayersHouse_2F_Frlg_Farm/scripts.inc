PalletTown_PlayersHouse_2F_Frlg_Farm_MapScripts::
	.byte 0

@ Replaces the vanilla notice about a HELP system this game doesn't have
Farmhouse2F_EventScript_ChoreChart::
	msgbox Farmhouse2F_Text_ChoreChart, MSGBOX_SIGN
	end

Farmhouse2F_Text_ChoreChart::
	.string "It's a chore chart.\p"
	.string "{PLAYER}: Feed the MAREEP.\n"
	.string "DAD: Fix the barn door.\l"
	.string "MOM: Make DAD fix the barn door.$"
