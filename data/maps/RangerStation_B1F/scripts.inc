@ The abandoned ranger station, B1F: the clock (Act 2 finale, docs/hack/act2_finale.md). Twelve torches at the hours
@ of a clock face around the caged stair down, two furnaces under a moon and a sun. A FIRE-type move lights one, a
@ WATER-type move puts it out (VAR_ACT2_CLOCK). At 4:15 a.m. (ACT2_CLOCK_SOLUTION: the torches at 4 and 3 and the
@ moon furnace lit, nothing else) the cage's front opens for good (ACT2_FINALE_CLOCK_SOLVED). Act2Station_DrawFloor
@ sets the clock to ACT2_CLOCK_START on the first visit and draws it on every load.

RangerStation_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RangerStation_B1F_OnLoad
	.byte 0

RangerStation_B1F_OnLoad:
	special Act2Station_DrawFloor
	end

@ Every torch and furnace (bg_events): the one the player faces
RangerStation_B1F_EventScript_ClockPiece::
	lockall
	specialvar VAR_RESULT, Act2Station_GetFacedClockPiece
	goto_if_eq VAR_RESULT, 0xFF, RangerStation_B1F_EventScript_Release
	copyvar VAR_TEMP_2, VAR_RESULT
	copyvar VAR_0x8004, VAR_TEMP_2
	specialvar VAR_RESULT, Act2Station_IsClockPieceLit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_LitPiece
	specialvar VAR_RESULT, Act2Station_FindFireMoveUser
	goto_if_eq VAR_RESULT, FALSE, RangerStation_B1F_EventScript_NeedsFire
	msgbox RangerStation_B1F_Text_LightAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RangerStation_B1F_EventScript_Release
	playse SE_M_EMBER
	call RangerStation_B1F_EventScript_Toggle
	msgbox RangerStation_B1F_Text_Lit
	waitse
	goto RangerStation_B1F_EventScript_CheckClock

RangerStation_B1F_EventScript_LitPiece:
	specialvar VAR_RESULT, Act2Station_FindWaterMoveUser
	goto_if_eq VAR_RESULT, FALSE, RangerStation_B1F_EventScript_NeedsWater
	msgbox RangerStation_B1F_Text_PutOutAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RangerStation_B1F_EventScript_Release
	playse SE_M_BUBBLE
	call RangerStation_B1F_EventScript_Toggle
	msgbox RangerStation_B1F_Text_PutOut
	waitse
RangerStation_B1F_EventScript_CheckClock:
	goto_if_eq VAR_TEMP_3, TRUE, RangerStation_B1F_EventScript_OpenGate
	releaseall
	end

RangerStation_B1F_EventScript_Toggle:
	copyvar VAR_0x8004, VAR_TEMP_2
	specialvar VAR_TEMP_3, Act2Station_ToggleClockPiece
	special DrawWholeMapView
	return

RangerStation_B1F_EventScript_OpenGate:
	delay 30
	playse SE_UNLOCK
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 1  @ horizontal pan
	setvar VAR_0x8006, 6  @ num shakes
	setvar VAR_0x8007, 4  @ shake delay
	special ShakeCamera
	waitstate
	special Act2Station_OpenClockGate
	special DrawWholeMapView
	playse SE_BANG
	waitse
	msgbox RangerStation_B1F_Text_GateOpened
	releaseall
	end

RangerStation_B1F_EventScript_NeedsFire:
	msgbox RangerStation_B1F_Text_NeedsFire
	releaseall
	end

RangerStation_B1F_EventScript_NeedsWater:
	msgbox RangerStation_B1F_Text_NeedsWater
	releaseall
	end

RangerStation_B1F_EventScript_Release:
	releaseall
	end

@ Five explorers' notes, one hint each (no two hint at the same thing): night; morning, not evening; before six;
@ a quarter past; four o'clock. Together: 4:15 a.m.
RangerStation_B1F_EventScript_Note1::
	msgbox RangerStation_B1F_Text_Note1, MSGBOX_SIGN
	end

RangerStation_B1F_EventScript_Note2::
	msgbox RangerStation_B1F_Text_Note2, MSGBOX_SIGN
	end

RangerStation_B1F_EventScript_Note3::
	msgbox RangerStation_B1F_Text_Note3, MSGBOX_SIGN
	end

RangerStation_B1F_EventScript_Note4::
	msgbox RangerStation_B1F_Text_Note4, MSGBOX_SIGN
	end

RangerStation_B1F_EventScript_Note5::
	msgbox RangerStation_B1F_Text_Note5, MSGBOX_SIGN
	end

RangerStation_B1F_Text_LightAsk:
	.string "The {STR_VAR_3} is cold.\p"
	.string "Have {STR_VAR_1} use {STR_VAR_2}\n"
	.string "to light it?$"

RangerStation_B1F_Text_Lit:
	.string "{STR_VAR_1} used {STR_VAR_2}!\n"
	.string "The {STR_VAR_3} is lit.$"

RangerStation_B1F_Text_PutOutAsk:
	.string "The {STR_VAR_3} is burning.\p"
	.string "Have {STR_VAR_1} use {STR_VAR_2}\n"
	.string "to put it out?$"

RangerStation_B1F_Text_PutOut:
	.string "{STR_VAR_1} used {STR_VAR_2}!\n"
	.string "The {STR_VAR_3} went out.$"

RangerStation_B1F_Text_NeedsFire:
	.string "The {STR_VAR_3} is cold.\p"
	.string "A FIRE-type move could light it.$"

RangerStation_B1F_Text_NeedsWater:
	.string "The {STR_VAR_3} is burning.\p"
	.string "A WATER-type move could put it\n"
	.string "out.$"

RangerStation_B1F_Text_GateOpened:
	.string "Clank!\p"
	.string "The bars in front of the stairs\n"
	.string "dropped into the floor!$"

RangerStation_B1F_Text_Note1:
	.string "A torn page from a notebook…\p"
	.string "“I couldn't see much when I went\n"
	.string "in. Only the stars lit the trail\l"
	.string "through the WOODS.”\p"
	.string "“My lantern gave out on the\n"
	.string "stairs. Some ranger I'd make.\l"
	.string "-TOM”$"

RangerStation_B1F_Text_Note2:
	.string "A note in neat handwriting…\p"
	.string "“I set out before breakfast.\n"
	.string "The day hadn't even begun.”\p"
	.string "“Mom packed me breakfast AND\n"
	.string "lunch. I ate both on the way.\l"
	.string "-PIP”$"

RangerStation_B1F_Text_Note3:
	.string "A crumpled note…\p"
	.string "“I ran the whole way, sure I'd\n"
	.string "be late. But the town's six\l"
	.string "o'clock bell hadn't rung yet.”\p"
	.string "“Should've walked. My legs!\n"
	.string "-DANA”$"

RangerStation_B1F_Text_Note4:
	.string "A page of a pocket diary…\p"
	.string "“I checked my watch at the door.\n"
	.string "The long hand had gone a quarter\l"
	.string "of the way round.”\p"
	.string "“Then the door shut behind me.\n"
	.string "Why do I write these things?\l"
	.string "-LEX”$"

RangerStation_B1F_Text_Note5:
	.string "A note on the back of a receipt…\p"
	.string "“Back on the farm, the MILTANK\n"
	.string "get milked at four. I heard them\l"
	.string "lowing as I left.”\p"
	.string "“Funny, what you remember when\n"
	.string "you're lost down here. -OLD BRAM”$"
