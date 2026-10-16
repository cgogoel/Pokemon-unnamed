@ Act 1's story (docs/hack/act1_story.md)

@ Blacking out in Act 1 (Jangmo-o): the player wakes up at home next to Mom (Farmhouse1F_EventScript_WokeUp).
@ Nothing is lost.
Act1_EventScript_BlackOut::
	lockall
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	setvar VAR_ACT1_WOKE_UP, 1
	warpsilent MAP_PALLET_TOWN_PLAYERS_HOUSE_1F_FRLG_FARM, 10, 6
	waitstate
	releaseall
	end
