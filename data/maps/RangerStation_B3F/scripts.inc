@ The abandoned ranger station, B3F: the overgrowth (Act 2 finale, docs/hack/act2_finale.md). A plank-floored
@ basement grown over with trees and grass. The stone tablet near the entrance shows, small, the only safe path
@ through the 9x9 field of tall grass. A step off it (Act2Station_TryOvergrowthStep, against the table generated with
@ the tablet) is an ambush: back to the path's start. Reaching the far side sets ACT2_FINALE_GRASS_CROSSED.

RangerStation_B3F_MapScripts::
	.byte 0

RangerStation_B3F_EventScript_Ambush::
	lockall
	playse SE_M_RAZOR_WIND
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 1  @ horizontal pan
	setvar VAR_0x8006, 4  @ num shakes
	setvar VAR_0x8007, 3  @ shake delay
	special ShakeCamera
	waitstate
	msgbox RangerStation_B3F_Text_Ambush
	closemessage
	special Act2Station_GetPathStart
	warp MAP_RANGER_STATION_B3F, VAR_0x8004, VAR_0x8005
	waitstate
	releaseall
	end

RangerStation_B3F_EventScript_Tablet::
	msgbox RangerStation_B3F_Text_Tablet, MSGBOX_SIGN
	end

RangerStation_B3F_Text_Ambush:
	.string "The grass rustled…\p"
	.string "GRASS POKéMON ambushed you!\n"
	.string "You fled back to the start.$"

RangerStation_B3F_Text_Tablet:
	.string "A stone tablet. A map is carved\n"
	.string "into it, nine squares by nine.\p"
	.string "RANGER TRAINING FIELD\n"
	.string "Start at red. Leave at yellow.\p"
	.string "Keep to the path, or the grass\n"
	.string "will bite.$"
