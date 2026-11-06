@ The abandoned ranger station, B4F: the chamber (Act 2 finale, docs/hack/act2_finale.md). HEATRAN, SUICUNE and
@ VIRIZION stand together at the top; DAD, MOM and MEW are hidden until the story's scene (FLAG_TEMP_1-3). The row
@ of coord events in front of the trio (VAR_ACT2_PROGRESS == ACT2_PARENTS_TALKED) starts the encounter.

RangerStation_B4F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RangerStation_B4F_OnTransition
	.byte 0

RangerStation_B4F_OnTransition:
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
