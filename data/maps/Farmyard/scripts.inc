@ Act 1 farmyard (docs/hack/befriending.md). Placeholder map: a copy of Pallet Town's layout, to be redrawn in Porymap.
@ Its first FARM_SHOWN_COUNT objects are the farm's Pokémon (graphics VAR_OBJ_GFX_ID_0.., hidden by FLAG_TEMP_1..).

Farmyard_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Farmyard_OnTransition
	.byte 0

Farmyard_OnTransition:
	special SetupFarmyardMons
	end

@ The gates, across each entrance: the following Pokémon is dropped off and starts wandering the farmyard
Farmyard_EventScript_Gate::
	lockall
	special Special_DepositFollowerInFarm
	copyvar VAR_0x8004, VAR_RESULT
	goto_if_eq VAR_0x8004, FARM_DEPOSIT_NONE, Farmyard_EventScript_GateEnd
	msgbox Farmyard_Text_RanOffToPlay, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8004, 0, Farmyard_EventScript_GateEnd
	addobject VAR_0x8004
Farmyard_EventScript_GateEnd:
	releaseall
	end

@ The Woods' edge is still solid, so its side is a warp for now (Pallet Town's side is a map connection)
Farmyard_EventScript_ToWoods::
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_SENT_TO_WOODS, Farmyard_EventScript_WoodsNotYet
	warp MAP_WOODS, 49, 58
	waitstate
	end

@ Act 1: the Woods open once Dad has sent the player there (docs/hack/act1_story.md)
Farmyard_EventScript_WoodsNotYet::
	lockall
	switch VAR_ACT1_PROGRESS
	case ACT1_START, Farmyard_EventScript_WoodsNotYetPokedex
	case ACT1_GOT_POKEDEX, Farmyard_EventScript_WoodsNotYetMom
	msgbox Farmyard_Text_WoodsNotYetDad
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsNotYetPokedex::
	msgbox Farmyard_Text_WoodsNotYetPokedex
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsNotYetMom::
	msgbox Farmyard_Text_WoodsNotYetMom
Farmyard_EventScript_WoodsTurnBack::
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepBack
	waitmovement 0
	releaseall
	end

Farmyard_Movement_StepBack:
	walk_right
	step_end

@ The farm's own POKéMON, there from the start: they can't be befriended, but they'll call back
Farmyard_EventScript_Miltank::
	lock
	faceplayer
	playmoncry SPECIES_MILTANK, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Miltank
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Mareep::
	lock
	faceplayer
	playmoncry SPECIES_MAREEP, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Mareep
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Tauros::
	lock
	faceplayer
	playmoncry SPECIES_TAUROS, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Tauros
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Doduo::
	lock
	faceplayer
	playmoncry SPECIES_DODUO, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Doduo
Farmyard_EventScript_FarmAnimalEnd::
	waitmoncry
	release
	end

Farmyard_EventScript_Sign::
	msgbox Farmyard_Text_Sign, MSGBOX_SIGN
	end

Farmyard_EventScript_FarmMon::
	lock
	faceplayer
	msgbox Farmyard_Text_LooksHappy, MSGBOX_DEFAULT
	release
	end

Farmyard_Text_RanOffToPlay:
	.string "{STR_VAR_1} ran off to play\n"
	.string "in the farmyard!$"

Farmyard_Text_LooksHappy:
	.string "It looks happy here.$"

Farmyard_Text_Sign:
	.string "Quincy Farm$"

Farmyard_Text_WoodsNotYetPokedex:
	.string "I should pick up the POKéDEX from\n"
	.string "PROF. OAK's LAB first.$"

Farmyard_Text_WoodsNotYetMom:
	.string "I should show Mom the POKéDEX\n"
	.string "first.$"

Farmyard_Text_WoodsNotYetDad:
	.string "Mom said Dad wanted to see me in\n"
	.string "the storage shed.$"

Farmyard_Text_Miltank:
	.string "MILTANK: Mooo!$"

Farmyard_Text_Mareep:
	.string "MAREEP: Baaah!$"

Farmyard_Text_Tauros:
	.string "TAUROS snorted and stamped\n"
	.string "its hooves!$"

Farmyard_Text_Doduo:
	.string "Both of DODUO's heads squawked\n"
	.string "at once!$"
