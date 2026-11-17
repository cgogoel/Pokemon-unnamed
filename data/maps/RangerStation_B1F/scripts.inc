@ The abandoned ranger station, B1F: the clock (Act 2 finale, docs/hack/act2_finale.md). Twelve torches at the hours
@ of a clock face around the caged stair down, two furnaces under a moon and a sun. A FIRE-type move lights one, a
@ WATER-type move puts it out (VAR_ACT2_CLOCK). At 4:15 a.m. (ACT2_CLOCK_SOLUTION: the torches at 4 and 3 and the
@ moon furnace lit, nothing else) the cage's front opens for good (ACT2_FINALE_CLOCK_SOLVED). Act2Station_DrawFloor
@ sets the clock to ACT2_CLOCK_START on the first visit and draws it on every load.
@ Round the clock room, a ring of corridor and rooms (wild Pokémon there, none in the clock room) where five
@ explorers' notes give the time. Two Pokémon guard the doors of the lab and the study.

RangerStation_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RangerStation_B1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, RangerStation_B1F_OnTransition
	.byte 0

RangerStation_B1F_OnTransition:
	special Act2Station_HideBeatenGuards
	end

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
@ the long hand pointing east (a quarter past); four o'clock. Together: 4:15 a.m. Each lies in one of the rooms
@ (object events); reading one counts it (ACT2_FINALE_NOTE_1-5), and the terminal in the clock room shows the ones
@ found so far.
RangerStation_B1F_EventScript_Note1::
	lockall
	msgbox RangerStation_B1F_Text_Note1
	setvar VAR_0x8004, ACT2_FINALE_NOTE_1
	goto RangerStation_B1F_EventScript_NoteRead

RangerStation_B1F_EventScript_Note2::
	lockall
	msgbox RangerStation_B1F_Text_Note2
	setvar VAR_0x8004, ACT2_FINALE_NOTE_2
	goto RangerStation_B1F_EventScript_NoteRead

RangerStation_B1F_EventScript_Note3::
	lockall
	msgbox RangerStation_B1F_Text_Note3
	setvar VAR_0x8004, ACT2_FINALE_NOTE_3
	goto RangerStation_B1F_EventScript_NoteRead

RangerStation_B1F_EventScript_Note4::
	lockall
	msgbox RangerStation_B1F_Text_Note4
	setvar VAR_0x8004, ACT2_FINALE_NOTE_4
	goto RangerStation_B1F_EventScript_NoteRead

RangerStation_B1F_EventScript_Note5::
	lockall
	msgbox RangerStation_B1F_Text_Note5
	setvar VAR_0x8004, ACT2_FINALE_NOTE_5
	goto RangerStation_B1F_EventScript_NoteRead

RangerStation_B1F_EventScript_NoteRead:
	special Act2Station_SetFinaleBit
	specialvar VAR_RESULT, Act2Station_CountNotes
	goto_if_eq VAR_RESULT, ACT2_STATION_NOTE_COUNT, RangerStation_B1F_EventScript_AllNotes
	msgbox RangerStation_B1F_Text_NotesFound
	releaseall
	end

RangerStation_B1F_EventScript_AllNotes:
	msgbox RangerStation_B1F_Text_AllNotes
	releaseall
	end

@ The terminal on the clock room's wall: the notes found so far, read over
RangerStation_B1F_EventScript_Terminal::
	lockall
	specialvar VAR_RESULT, Act2Station_CountNotes
	goto_if_eq VAR_RESULT, 0, RangerStation_B1F_EventScript_TerminalEmpty
	msgbox RangerStation_B1F_Text_TerminalAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RangerStation_B1F_EventScript_Release
	setvar VAR_0x8004, ACT2_FINALE_NOTE_1
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	call_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_ShowNote1
	setvar VAR_0x8004, ACT2_FINALE_NOTE_2
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	call_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_ShowNote2
	setvar VAR_0x8004, ACT2_FINALE_NOTE_3
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	call_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_ShowNote3
	setvar VAR_0x8004, ACT2_FINALE_NOTE_4
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	call_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_ShowNote4
	setvar VAR_0x8004, ACT2_FINALE_NOTE_5
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	call_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_ShowNote5
	specialvar VAR_RESULT, Act2Station_CountNotes
	msgbox RangerStation_B1F_Text_TerminalDone
	releaseall
	end

RangerStation_B1F_EventScript_TerminalEmpty:
	msgbox RangerStation_B1F_Text_TerminalEmpty
	releaseall
	end

RangerStation_B1F_EventScript_ShowNote1:
	msgbox RangerStation_B1F_Text_Note1
	return

RangerStation_B1F_EventScript_ShowNote2:
	msgbox RangerStation_B1F_Text_Note2
	return

RangerStation_B1F_EventScript_ShowNote3:
	msgbox RangerStation_B1F_Text_Note3
	return

RangerStation_B1F_EventScript_ShowNote4:
	msgbox RangerStation_B1F_Text_Note4
	return

RangerStation_B1F_EventScript_ShowNote5:
	msgbox RangerStation_B1F_Text_Note5
	return

@ The Pokémon in the doors of the lab and the study: a battle each, 3 levels above the party's average. Beaten or
@ caught, it's gone for good (Act2Station_HideBeatenGuards).
RangerStation_B1F_EventScript_LabGuard::
	lock
	faceplayer
	setvar VAR_0x8006, SPECIES_MAGNETON
	setvar VAR_TEMP_4, ACT2_FINALE_LAB_GUARD
	setvar VAR_TEMP_5, 1  @ Its local id
	goto RangerStation_B1F_EventScript_Guard

RangerStation_B1F_EventScript_StudyGuard::
	lock
	faceplayer
	setvar VAR_0x8006, SPECIES_HAUNTER
	setvar VAR_TEMP_4, ACT2_FINALE_STUDY_GUARD
	setvar VAR_TEMP_5, 2
	goto RangerStation_B1F_EventScript_Guard

RangerStation_B1F_EventScript_Guard:
	bufferspeciesname STR_VAR_1, VAR_0x8006
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox RangerStation_B1F_Text_Guard
	waitmoncry
	closemessage
	setwildbattle SPECIES_MAGIKARP, 1  @ Replaced by the guard
	special Act2Station_CreateGuardMon
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, RangerStation_B1F_EventScript_GuardBeaten
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, RangerStation_B1F_EventScript_GuardBeaten
	release
	end

RangerStation_B1F_EventScript_GuardBeaten:
	copyvar VAR_0x8004, VAR_TEMP_4
	special Act2Station_SetFinaleBit
	removeobject VAR_TEMP_5
	release
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
	.string "“I held my watch up to the map\n"
	.string "at the door. Its long hand lay\l"
	.string "flat, pointing due east.”\p"
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

RangerStation_B1F_Text_NotesFound:
	.string "Notes found: {STR_VAR_1}/5$"

RangerStation_B1F_Text_AllNotes:
	.string "Notes found: 5/5\p"
	.string "That's all of them. Put together,\n"
	.string "their times could set the clock.$"

RangerStation_B1F_Text_TerminalAsk:
	.string "An old terminal. It still runs!\p"
	.string "Type in the notes you've found\n"
	.string "and read them over?$"

RangerStation_B1F_Text_TerminalDone:
	.string "That's every note found so far.\n"
	.string "Notes found: {STR_VAR_1}/5$"

RangerStation_B1F_Text_TerminalEmpty:
	.string "An old terminal. It still runs!\p"
	.string "But there's nothing to type\n"
	.string "into it yet.$"

RangerStation_B1F_Text_Guard:
	.string "A wild {STR_VAR_1} is blocking\n"
	.string "the doorway!$"
