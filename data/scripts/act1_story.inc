@ Act 1's story (docs/hack/act1_story.md)

@ Blacking out in Act 1 (Jangmo-o): the player wakes up at home next to Mom (Farmhouse1F_EventScript_WokeUp).
@ A Woods Pokémon that was still following runs back to where it was befriended.
Act1_EventScript_BlackOut::
	lockall
	special Act1_ReleaseFollowerToWoods
	goto_if_eq VAR_RESULT, FALSE, Act1_EventScript_BlackOutFade
	msgbox Act1_Text_FollowerRanBack
Act1_EventScript_BlackOutFade::
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	setvar VAR_ACT1_WOKE_UP, 1
	warpsilent MAP_PALLET_TOWN_PLAYERS_HOUSE_1F_FRLG_FARM, 10, 6
	waitstate
	releaseall
	end

Act1_Text_FollowerRanBack:
	.string "{STR_VAR_1} got scared and ran\n"
	.string "back into the woods!$"

@ The Woods' Pokémon aren't from Kanto, and the Kanto POKéDEX only lists Kanto's: the player's POKéDEX is the
@ National one from the start (given by Oak's aide; saves from before that get it on entering the Woods).
@ Only when it isn't already: enabling it also resets the POKéDEX's order and scroll position.
Act1_EventScript_EnsureNationalDex::
	specialvar VAR_RESULT, IsNationalPokedexEnabled
	call_if_eq VAR_RESULT, FALSE, Act1_EventScript_EnableNationalDex
	return

Act1_EventScript_EnableNationalDex:
	special EnableNationalPokedex
	return
