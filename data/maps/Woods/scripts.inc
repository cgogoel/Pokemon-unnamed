@ Act 1 Woods (docs/hack/befriending.md, docs/hack/act1_story.md): reached only from the farmyard; all the Act 1
@ Pokémon live here. Placeholder map: a copy of Viridian Forest's layout, to be redrawn in Porymap.

Woods_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Woods_OnTransition
	.byte 0

Woods_OnTransition:
	special Act1_RestockLovedItems
	end

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
	call_if_lt VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, Woods_EventScript_SetBefriendedBlipbug
	release
	end

Woods_EventScript_SetBefriendedBlipbug:
	setvar VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG
	return

@ Tynamo, Hatenna and Teddiursa keep their distance until Blipbug is befriended and Dad has warned about Jangmo-o
Woods_EventScript_TooWary:
	goto_if_ge VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, Woods_EventScript_TooWaryAskDad
	msgbox Woods_Text_TooWaryTryBlipbug
	release
	end

Woods_EventScript_TooWaryAskDad:
	msgbox Woods_Text_TooWaryTalkToDad
	release
	end

Woods_EventScript_Tynamo::
	lock
	faceplayer
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_WARNED_JANGMO_O, Woods_EventScript_TooWary
	befriendbattle SPECIES_TYNAMO, 5
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_TynamoBefriended
	release
	end

Woods_EventScript_TynamoBefriended:
	setflag FLAG_BEFRIENDED_TYNAMO
	goto Woods_EventScript_OneOfThreeBefriended

Woods_EventScript_Hatenna::
	lock
	faceplayer
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_WARNED_JANGMO_O, Woods_EventScript_TooWary
	befriendbattle SPECIES_HATENNA, 5
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_HatennaBefriended
	release
	end

Woods_EventScript_HatennaBefriended:
	setflag FLAG_BEFRIENDED_HATENNA
	goto Woods_EventScript_OneOfThreeBefriended

Woods_EventScript_Teddiursa::
	lock
	faceplayer
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_WARNED_JANGMO_O, Woods_EventScript_TooWary
	befriendbattle SPECIES_TEDDIURSA, 6
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_TeddiursaBefriended
	release
	end

Woods_EventScript_TeddiursaBefriended:
	setflag FLAG_BEFRIENDED_TEDDIURSA
Woods_EventScript_OneOfThreeBefriended:
	removeobject VAR_LAST_TALKED
	goto_if_unset FLAG_BEFRIENDED_TYNAMO, Woods_EventScript_BefriendedEnd
	goto_if_unset FLAG_BEFRIENDED_HATENNA, Woods_EventScript_BefriendedEnd
	goto_if_unset FLAG_BEFRIENDED_TEDDIURSA, Woods_EventScript_BefriendedEnd
	call_if_lt VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_THREE, Woods_EventScript_SetBefriendedThree
Woods_EventScript_BefriendedEnd:
	release
	end

Woods_EventScript_SetBefriendedThree:
	setvar VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_THREE
	return

@ Without Dad's Hard Stone, Jangmo-o charges (also when it spots the player, src/act1_story.c). With it, the
@ encounter starts: throwing the stone blacks the player out, offering it wins Jangmo-o over.
Woods_EventScript_JangmoO::
	lock
	faceplayer
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_GOT_HARD_STONE, Woods_EventScript_JangmoOCharges
	befriendbattle SPECIES_JANGMO_O, 10
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, Woods_EventScript_JangmoOBefriended
	goto_if_eq VAR_RESULT, B_OUTCOME_LOST, Act1_EventScript_BlackOut
	release
	end

Woods_EventScript_JangmoOBefriended:
	setflag FLAG_BEFRIENDED_JANGMO_O
	setvar VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_JANGMO_O
	removeobject VAR_LAST_TALKED
	release
	end

Woods_EventScript_JangmoOSpotted::
	lockall
	applymovement LOCALID_WOODS_JANGMO_O, Common_Movement_ExclamationMark
	waitmovement 0
Woods_EventScript_JangmoOCharges:
	playmoncry SPECIES_JANGMO_O, CRY_MODE_ENCOUNTER
	waitmoncry
	msgbox Woods_Text_JangmoOCharged
	goto Act1_EventScript_BlackOut

Woods_EventScript_ItemChestoBerry::
	finditem ITEM_CHESTO_BERRY
	end

Woods_EventScript_ItemHoney::
	finditem ITEM_HONEY
	end

Woods_Text_TooWaryTryBlipbug:
	.string "It's keeping its distance.\n"
	.string "It won't let you anywhere near.\p"
	.string "Maybe start with the BLIPBUG by\n"
	.string "the entrance to the woods.$"

Woods_Text_TooWaryTalkToDad:
	.string "It's keeping its distance.\p"
	.string "Maybe Dad knows something about\n"
	.string "the POKéMON in these woods.$"

Woods_Text_JangmoOCharged:
	.string "JANGMO-O charged at you!\p"
	.string "{PLAYER} blacked out!$"
