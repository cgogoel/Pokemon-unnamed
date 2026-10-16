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
	warp MAP_WOODS, 49, 58
	waitstate
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
