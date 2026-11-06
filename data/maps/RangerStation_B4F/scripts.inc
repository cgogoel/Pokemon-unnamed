@ The abandoned ranger station, B4F: the chamber (Act 2 finale, docs/hack/act2_finale.md). HEATRAN, SUICUNE and
@ VIRIZION stand together at the top; DAD, MOM and MEW are hidden until the story's scene (FLAG_TEMP_1-3). The row
@ of coord events in front of the trio (VAR_ACT2_PROGRESS == ACT2_PARENTS_TALKED) starts the encounter.

RangerStation_B4F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RangerStation_B4F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, RangerStation_B4F_OnFrame
	.byte 0

@ The trio's battle is always a 3v3: on arrival, a party of fewer than three is filled up from the PC
@ (Act2_FillChamberParty, then a line about it), and the party is healed
RangerStation_B4F_OnTransition:
	specialvar VAR_RESULT, Act2_FillChamberParty
	call_if_ne VAR_RESULT, 0, RangerStation_B4F_EventScript_CameAlong
	special HealPlayerParty
	setflag FLAG_TEMP_1  @ DAD
	setflag FLAG_TEMP_2  @ MEW
	setflag FLAG_TEMP_3  @ MOM
	end

@ The story's scene (Act2_EventScript_ChamberScene, act2_finale.inc): the battle, then Dad, Mom and Mew
RangerStation_B4F_EventScript_Encounter::
	goto Act2_EventScript_ChamberScene

RangerStation_B4F_EventScript_Legend::
	msgbox RangerStation_B4F_Text_Legend, MSGBOX_SIGN
	end

RangerStation_B4F_Text_Legend:
	.string "It's staring at you in silence…$"

RangerStation_B4F_EventScript_CameAlong:
	setvar VAR_TEMP_4, 1
	return

RangerStation_B4F_OnFrame:
	map_script_2 VAR_TEMP_4, 1, RangerStation_B4F_EventScript_CameAlongMessage
	.2byte 0

RangerStation_B4F_EventScript_CameAlongMessage:
	lockall
	setvar VAR_TEMP_4, 0
	msgbox RangerStation_B4F_Text_CameAlong
	releaseall
	end

RangerStation_B4F_Text_CameAlong:
	.string "POKéMON from the PC came along\n"
	.string "to help! Your team is ready.$"
