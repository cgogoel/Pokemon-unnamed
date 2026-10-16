@ Act 1 Woods (docs/hack/befriending.md, docs/hack/act1_story.md): reached only from the farmyard; all the Act 1
@ Pokémon live here.

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
	msgbox Woods_Text_GetBackToFarmyard
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
	msgbox Woods_Text_GetBackToFarmyard
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
	msgbox Woods_Text_GetBackToFarmyard
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

@ The Chesto Berry for Hatenna is in the mailbox by the Keep Out sign. Offered and lost, it's back next time
@ (special Act1_RestockLovedItems clears FLAG_ACT1_ITEM_CHESTO_BERRY).
Woods_EventScript_Mailbox::
	lockall
	goto_if_set FLAG_ACT1_ITEM_CHESTO_BERRY, Woods_EventScript_MailboxEmpty
	msgbox Woods_Text_SomethingInMailbox
	giveitem ITEM_CHESTO_BERRY
	goto_if_eq VAR_RESULT, FALSE, Woods_EventScript_MailboxEnd
	setflag FLAG_ACT1_ITEM_CHESTO_BERRY
	releaseall
	end

Woods_EventScript_MailboxEmpty::
	msgbox Woods_Text_MailboxEmpty
Woods_EventScript_MailboxEnd::
	releaseall
	end

Woods_EventScript_SignQuincyWoods::
	msgbox Woods_Text_SignQuincyWoods, MSGBOX_SIGN
	end

Woods_EventScript_SignAggressivePokemon::
	msgbox Woods_Text_SignAggressivePokemon, MSGBOX_SIGN
	end

Woods_EventScript_SignKeepOut::
	msgbox Woods_Text_SignKeepOut, MSGBOX_SIGN
	end

@ Teddiursa's Honey is in this tree, with nothing to show it: the player works it out (Daisy's hint: flowers mean
@ bees). Offered and lost, it's back next time (special Act1_RestockLovedItems clears FLAG_ACT1_ITEM_HONEY).
Woods_EventScript_HoneyTree::
	lockall
	goto_if_set FLAG_ACT1_ITEM_HONEY, Woods_EventScript_HoneyTreeEmpty
	msgbox Woods_Text_HoneyDripping
	giveitem ITEM_HONEY
	goto_if_eq VAR_RESULT, FALSE, Woods_EventScript_HoneyTreeEnd
	setflag FLAG_ACT1_ITEM_HONEY
	releaseall
	end

Woods_EventScript_HoneyTreeEmpty::
	msgbox Woods_Text_BeesBuzzing
Woods_EventScript_HoneyTreeEnd::
	releaseall
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

Woods_Text_SomethingInMailbox:
	.string "There's something in the mailbox…$"

Woods_Text_HoneyDripping:
	.string "Bees are buzzing around a hollow in\n"
	.string "the tree… Something sweet is\l"
	.string "dripping out of it!$"

Woods_Text_BeesBuzzing:
	.string "Bees are buzzing around the tree.$"

Woods_Text_MailboxEmpty:
	.string "The mailbox is empty.$"

Woods_Text_SignQuincyWoods:
	.string "Quincy Woods$"

Woods_Text_SignAggressivePokemon:
	.string "Warning:\n"
	.string "Aggressive POKéMON Nearby$"

Woods_Text_SignKeepOut:
	.string "Danger: Keep Out$"

@ After each befriending
Woods_Text_GetBackToFarmyard:
	.string "I should get this back to the\n"
	.string "farmyard.$"
