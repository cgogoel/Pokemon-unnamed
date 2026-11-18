@ The abandoned ranger station, B1F: the clock (Act 2 finale, docs/hack/act2_finale.md). Twelve torches at the hours
@ of a clock face round a stone mosaic, two furnaces under a moon and a sun. A FIRE-type move lights one, a
@ WATER-type move puts it out (VAR_ACT2_CLOCK). At 4:15 a.m. (ACT2_CLOCK_SOLUTION: the torches at 4 and 3 and the
@ moon furnace lit, nothing else) the slab in the mosaic's middle slides away from a ladder down, for good
@ (ACT2_FINALE_CLOCK_SOLVED). Act2Station_DrawFloor sets the clock to ACT2_CLOCK_START on the first visit and draws it
@ (and the open shaft) on every load.
@ Round the clock room, a ring of corridor and rooms (wild Pokémon there, none in the clock room): a lone ranger's
@ post. Their log, five notes from the five stages of their stay, gives the time, and why; the last is in their
@ hands, in the study. Two Pokémon guard the doors of the lab and the study.
@ The lore: the station's founders sealed three Pokémon below (the chamber's trio, B4F) at a quarter past four one
@ morning, the night the ground split, and built the clock to that hour. The trio stirs at that hour every night.

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
	goto_if_eq VAR_TEMP_3, TRUE, RangerStation_B1F_EventScript_OpenMosaic
	releaseall
	end

RangerStation_B1F_EventScript_Toggle:
	copyvar VAR_0x8004, VAR_TEMP_2
	specialvar VAR_TEMP_3, Act2Station_ToggleClockPiece
	special DrawWholeMapView
	return

@ The clock set: a rumble, and the slab in the mosaic's middle grinds aside, halfway, then all the way
RangerStation_B1F_EventScript_OpenMosaic:
	delay 30
	playse SE_UNLOCK
	waitse
	playse SE_M_EARTHQUAKE
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 1  @ horizontal pan
	setvar VAR_0x8006, 8  @ num shakes
	setvar VAR_0x8007, 4  @ shake delay
	special ShakeCamera
	waitstate
	playse SE_M_STRENGTH
	setvar VAR_0x8004, 1
	special Act2Station_SlideMosaic
	special DrawWholeMapView
	delay 24
	setvar VAR_0x8004, 2
	special Act2Station_SlideMosaic
	special DrawWholeMapView
	waitse
	playse SE_M_ROCK_THROW
	waitse
	msgbox RangerStation_B1F_Text_MosaicOpened
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

@ The ranger's five notes, one hint each (no two hint at the same thing): night; morning, not evening; before six;
@ the long hand pointing east (a quarter past); four o'clock. Together: 4:15 a.m. Four lie in the rooms (object
@ events), the fifth in the ranger's hands (RangerStation_B1F_EventScript_Remains); reading one counts it
@ (ACT2_FINALE_NOTE_1-5), and the terminal in the clock room shows the ones found so far.
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

@ The ranger, slumped against the study's wall, the last note in their hands
RangerStation_B1F_EventScript_Remains::
	lockall
	msgbox RangerStation_B1F_Text_Remains
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

RangerStation_B1F_Text_MosaicOpened:
	.string "The stone in the middle of the\n"
	.string "mosaic slid away!\p"
	.string "A ladder leads down into\n"
	.string "the dark…$"

RangerStation_B1F_Text_Note1:
	.string "A page from a ranger's log…\p"
	.string "“DAY 3. Posted down here alone.\n"
	.string "Apparently I couldn't handle a post\l"
	.string "dealing with real visitors. My orders\l"
	.string "are to keep the post. But I haven't\l"
	.string "been able to find out why. Nobody would\l"
	.string "say.”\p"
	.string "“There's a hum under the floor. Only\n"
	.string "when it's dark, when the stars are out\l"
	.string "over the WOODS.”$"

RangerStation_B1F_Text_Note2:
	.string "A page from a ranger's log…\p"
	.string "“DAY 40. I talk to the plants now.\n"
	.string "They're good listeners.”\p"
	.string "“The hum never bugs me when I'm falling\n"
	.string "asleep. It worms into my conscious\l"
	.string "before I wake. Staying in my mind\l"
	.string "throughout the day.”$"

RangerStation_B1F_Text_Note3:
	.string "A page from a ranger's log, the writing\n"
	.string "cramped…\p"
	.string "“DAY 112. Lit every torch. Put them\n"
	.string "out. Fed the furnaces. They won't\l"
	.string "answer me.”\p"
	.string "“A hundred nights I've timed it. It\n"
	.string "starts before the SPEAROW wake and\l"
	.string "fly out for the day.”$"

RangerStation_B1F_Text_Note4:
	.string "A page from a ranger's log, the writing\n"
	.string "all over the place…\p"
	.string "“DAY ???. Found a log behind the\n"
	.string "machines. They SEALED something down\l"
	.string "there.”\p"
	.string "“…when the ground was sealed, a\n"
	.string "clock's long hand lay flat, not angled.\l"
	.string "That's one. Now for the hour hand…”$"

RangerStation_B1F_Text_Note5:
	.string "“They built the clock to the hour they\n"
	.string "shut it in… hands reaching to the\l"
	.string "east.”\p"
	.string "“Set the time and it opens. I know the\n"
	.string "minute, but the hour still eludes me… I\l"
	.string "tried to track my sleep cycles… 2 hours\l"
	.string "each… starting at 10…\p"
	.string "…it must have been 3 or 4 of them\n"
	.string "before the hum drove into my conscious\l"
	.string "and the maddening dreams burrowed\l"
	.string "into my soul.”\p"
	.string "“Can't make the stairs anymore. The\n"
	.string "legs won't. That's all right. The mind\l"
	.string "gave up long ago.\p"
	.string "I realized it wasn't the post. It's me,\n"
	.string "I've never been right. I was right to\l"
	.string "lock myself down here.” -HOLT$"

RangerStation_B1F_Text_Remains:
	.string "Someone is slumped against\n"
	.string "the wall…\p"
	.string "A RANGER, in a uniform faded to\n"
	.string "dust. They've been here a long,\l"
	.string "long time.\p"
	.string "Their hands still hold a note.$"

RangerStation_B1F_Text_NotesFound:
	.string "Notes found: {STR_VAR_1}/5$"

RangerStation_B1F_Text_AllNotes:
	.string "Notes found: 5/5\p"
	.string "That's the whole log. Put together,\n"
	.string "it gives the hour to set the clock.$"

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
