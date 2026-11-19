@ The abandoned ranger station, B1F: the clock (Act 2 finale, docs/hack/act2_finale.md). Twelve torches at the hours
@ of a clock face round a stone mosaic, two furnaces under a moon and a sun. A FIRE-type move lights one, a
@ WATER-type move puts it out (VAR_ACT2_CLOCK). At 4:15 a.m. (ACT2_CLOCK_SOLUTION: the torches at 4 and 3 and the
@ moon furnace lit, nothing else) the slab in the mosaic's middle slides away from a ladder down, for good
@ (ACT2_FINALE_CLOCK_SOLVED). Act2Station_DrawFloor sets the clock to ACT2_CLOCK_START on the first visit and draws it
@ (and the open shaft) on every load.
@ Round the clock room, a ring of corridor and rooms (wild Pokémon there, none in the clock room): a lone ranger's
@ post. Their log, five notes from the five stages of their stay, gives the time, and why; the last is in their
@ hands, in the study. Two Pokémon hide in the doorways of the lab and the study.
@ The lore: the station's founders sealed three Pokémon below (the chamber's trio, B4F) at a quarter past four one
@ morning, the night the ground split, and built the clock to that hour. The trio stirs at that hour every night.

RangerStation_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, RangerStation_B1F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, RangerStation_B1F_OnTransition
	.byte 0

@ The lab's Pokémon stays hidden until the player steps into its doorway. The study's is hidden too until the four
@ notes outside wake it (ACT2_FINALE_STUDY_AWOKE); from then on it's standing in the doorway in plain sight until
@ it's beaten, so the way past it is a battle rather than a surprise.
RangerStation_B1F_OnTransition:
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setvar VAR_0x8004, ACT2_FINALE_STUDY_AWOKE
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, FALSE, RangerStation_B1F_EventScript_TransitionEnd
	setvar VAR_0x8004, ACT2_FINALE_STUDY_GUARD
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_TransitionEnd
	clearflag FLAG_TEMP_2
RangerStation_B1F_EventScript_TransitionEnd:
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
	call_if_eq VAR_RESULT, ACT2_STATION_NOTES_TO_WAKE, RangerStation_B1F_EventScript_WakeStudy
	msgbox RangerStation_B1F_Text_NotesFound
	releaseall
	end

@ The last note outside the study: whatever has been keeping the player out of it stirs, and from here on it's
@ standing in the doorway to be fought. Reading a note again doesn't say it twice (ACT2_FINALE_STUDY_AWOKE).
RangerStation_B1F_EventScript_WakeStudy:
	setvar VAR_0x8004, ACT2_FINALE_STUDY_AWOKE
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_WakeStudyEnd
	special Act2Station_SetFinaleBit
	clearflag FLAG_TEMP_2
	addobject 2
	playse SE_EXIT
	msgbox RangerStation_B1F_Text_StudyAwoke
RangerStation_B1F_EventScript_WakeStudyEnd:
	return

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

@ The Pokémon hiding in the lab's doorway: stepping into it (coord event), it jumps out from the room's side and
@ battles, 3 levels above the station's level (Act2Road_GetAreaLevel: an area of its own). Beaten or caught, it's
@ gone for good (ACT2_FINALE_LAB_GUARD). Run from, it stays out, blocking the way; talk to it to battle again.
@ The study's keeps the same bit (_STUDY_GUARD) but is met head on instead - see below. Their objects come first:
@ local ids 1 and 2 (VAR_TEMP_5), hidden by FLAG_TEMP_1/2.
RangerStation_B1F_EventScript_LabGuard::
	lockall
	setvar VAR_0x8006, SPECIES_MAGNETON
	setvar VAR_TEMP_4, ACT2_FINALE_LAB_GUARD
	setvar VAR_TEMP_5, 1  @ Its local id
	goto RangerStation_B1F_EventScript_Guard

@ The study's, once the fourth note has woken it: it's standing in the doorway where the player can see it, so
@ there's no ambush - talking to it is what starts the battle, and it stays in the way until it's beaten.
RangerStation_B1F_EventScript_StudyGuard::
	lockall
	setvar VAR_0x8006, SPECIES_HAUNTER
	setvar VAR_TEMP_4, ACT2_FINALE_STUDY_GUARD
	setvar VAR_TEMP_5, 2
	copyvar VAR_0x8004, VAR_TEMP_4
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_Release
	bufferspeciesname STR_VAR_1, VAR_0x8006
	playse SE_LEDGE
	applymovement VAR_TEMP_5, RangerStation_B1F_Movement_Lunge
	waitmovement 0
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox RangerStation_B1F_Text_StudyGuard
	goto RangerStation_B1F_EventScript_GuardBattle

@ The study's doorway before its Pokémon is awake: nothing is there to see, and the player is turned back. Once it
@ is awake the Pokémon itself stands on this square, so this can only run before that, or after it's beaten.
RangerStation_B1F_EventScript_StudyDoor::
	lockall
	setvar VAR_0x8004, ACT2_FINALE_STUDY_AWOKE
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_Release
	playse SE_M_CONFUSE_RAY
	msgbox RangerStation_B1F_Text_UnseenPresence
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, RangerStation_B1F_Movement_TurnedBack
	waitmovement 0
	releaseall
	end

RangerStation_B1F_EventScript_Guard:
	copyvar VAR_0x8004, VAR_TEMP_4
	specialvar VAR_RESULT, Act2Station_HasFinaleBit
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B1F_EventScript_Release
	bufferspeciesname STR_VAR_1, VAR_0x8006
	addobject VAR_TEMP_5
	playse SE_LEDGE
	call RangerStation_B1F_EventScript_JumpOutDown
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox RangerStation_B1F_Text_Guard
RangerStation_B1F_EventScript_GuardBattle:
	waitmoncry
	closemessage
	setwildbattle SPECIES_MAGIKARP, 1  @ Replaced by the guard
	special Act2Station_CreateGuardMon
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, RangerStation_B1F_EventScript_GuardBeaten
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, RangerStation_B1F_EventScript_GuardBeaten
	releaseall
	end

RangerStation_B1F_EventScript_GuardBeaten:
	copyvar VAR_0x8004, VAR_TEMP_4
	special Act2Station_SetFinaleBit
	removeobject VAR_TEMP_5
	releaseall
	end

@ Out at the player: the lab's comes down at them from inside, the study's up
RangerStation_B1F_EventScript_JumpOutDown:
	applymovement VAR_TEMP_5, RangerStation_B1F_Movement_JumpOutDown
	waitmovement 0
	return

RangerStation_B1F_Movement_Lunge:
	jump_in_place_up
	jump_in_place_up
	step_end

RangerStation_B1F_Movement_TurnedBack:
	walk_up
	step_end

RangerStation_B1F_Movement_JumpOutDown:
	jump_in_place_down
	jump_in_place_down
	step_end

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
	.string "A wild {STR_VAR_1} jumped out of\n"
	.string "the dark!$"

RangerStation_B1F_Text_StudyGuard:
	.string "The {STR_VAR_1} in the doorway\n"
	.string "rises to bar the way!$"

RangerStation_B1F_Text_UnseenPresence:
	.string "An unseen presence blocks your way.$"

RangerStation_B1F_Text_StudyAwoke:
	.string "You sense something has awoken.$"
