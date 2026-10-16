@ Act 1 Woods (docs/hack/befriending.md): reached only from the farmyard; all the Act 1 Pokémon live here.
@ Placeholder map: a copy of Viridian Forest's layout, to be redrawn in Porymap.

Woods_MapScripts::
	.byte 0

Woods_EventScript_ToFarmyard::
	warp MAP_FARMYARD, 0, 10
	waitstate
	end

Woods_EventScript_Blipbug::
	lock
	faceplayer
	befriendbattle SPECIES_BLIPBUG, 3
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_BlipbugBefriended
	release
	end

Woods_EventScript_BlipbugBefriended:
	setflag FLAG_BEFRIENDED_BLIPBUG
	removeobject VAR_LAST_TALKED
	release
	end

Woods_EventScript_Tynamo::
	lock
	faceplayer
	befriendbattle SPECIES_TYNAMO, 5
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_TynamoBefriended
	release
	end

Woods_EventScript_TynamoBefriended:
	setflag FLAG_BEFRIENDED_TYNAMO
	removeobject VAR_LAST_TALKED
	release
	end

Woods_EventScript_Hatenna::
	lock
	faceplayer
	befriendbattle SPECIES_HATENNA, 5
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_HatennaBefriended
	release
	end

Woods_EventScript_HatennaBefriended:
	setflag FLAG_BEFRIENDED_HATENNA
	removeobject VAR_LAST_TALKED
	release
	end

Woods_EventScript_Teddiursa::
	lock
	faceplayer
	befriendbattle SPECIES_TEDDIURSA, 6
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_TeddiursaBefriended
	release
	end

Woods_EventScript_TeddiursaBefriended:
	setflag FLAG_BEFRIENDED_TEDDIURSA
	removeobject VAR_LAST_TALKED
	release
	end

Woods_EventScript_JangmoO::
	lock
	faceplayer
	befriendbattle SPECIES_JANGMO_O, 10
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_JangmoOBefriended
	release
	end

Woods_EventScript_JangmoOBefriended:
	setflag FLAG_BEFRIENDED_JANGMO_O
	removeobject VAR_LAST_TALKED
	release
	end
