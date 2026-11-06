@ The abandoned ranger station, 1F (Act 2 finale, docs/hack/act2_finale.md). Entered from the Woods by the story:
@ warp MAP_RANGER_STATION_1F, 6, 9. A wall switch slides the bottom of the left counter away, opening the way to
@ the stair down behind it (ACT2_FINALE_BASEMENT_OPEN, redrawn on load by Act2Station_DrawFloor).

RangerStation_1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RangerStation_1F_OnLoad
	.byte 0

RangerStation_1F_OnLoad:
	special Act2Station_DrawFloor
	end

RangerStation_1F_EventScript_Switch::
	lockall
	setvar VAR_0x8004, ACT2_FINALE_BASEMENT_OPEN
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_1F_EventScript_SwitchPulled
	msgbox RangerStation_1F_Text_Switch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RangerStation_1F_EventScript_Release
	playse SE_SWITCH
	waitse
	special Act2Station_SetFinaleBit
	playse SE_TRUCK_DOOR
	special Act2Station_DrawFloor
	special DrawWholeMapView
	waitse
	msgbox RangerStation_1F_Text_CounterSlid
	releaseall
	end

RangerStation_1F_EventScript_SwitchPulled:
	msgbox RangerStation_1F_Text_SwitchPulled
RangerStation_1F_EventScript_Release:
	releaseall
	end

RangerStation_1F_Text_Switch:
	.string "There's a dusty switch on the wall.\n"
	.string "Pull the switch?$"

RangerStation_1F_Text_CounterSlid:
	.string "Something rumbled…\p"
	.string "The end of the left counter slid\n"
	.string "away!$"

RangerStation_1F_Text_SwitchPulled:
	.string "The switch is pulled down.\n"
	.string "It won't budge.$"
