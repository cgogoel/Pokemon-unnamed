@ The abandoned ranger station, B2F: the flood (Act 2 finale, docs/hack/act2_finale.md). Stepping stones in water too
@ deep to wade; walking at one square of water with a stone behind it jumps it (Act2Station_CanJumpGap). Five dead
@ ends, each with a powerful Water Pokémon that battles the player there (2 levels above the party's average) until
@ beaten or caught (FLAG_ACT2_DEAD_END_1-5). The dead ends' objects come first: their local ids are 1-5, which
@ removeobject VAR_TEMP_1 relies on. A medkit before the ladder down heals the party, any number of times.

RangerStation_B2F_MapScripts::
	.byte 0

RangerStation_B2F_EventScript_DeadEnd1::
	setvar VAR_TEMP_1, 1
	goto RangerStation_B2F_EventScript_DeadEnd

RangerStation_B2F_EventScript_DeadEnd2::
	setvar VAR_TEMP_1, 2
	goto RangerStation_B2F_EventScript_DeadEnd

RangerStation_B2F_EventScript_DeadEnd3::
	setvar VAR_TEMP_1, 3
	goto RangerStation_B2F_EventScript_DeadEnd

RangerStation_B2F_EventScript_DeadEnd4::
	setvar VAR_TEMP_1, 4
	goto RangerStation_B2F_EventScript_DeadEnd

RangerStation_B2F_EventScript_DeadEnd5::
	setvar VAR_TEMP_1, 5
	goto RangerStation_B2F_EventScript_DeadEnd

@ On arriving on the dead end's stone (coord event), or talking to the Pokémon
RangerStation_B2F_EventScript_DeadEnd:
	lockall
	specialvar VAR_RESULT, Act2Station_IsDeadEndBeaten
	goto_if_eq VAR_RESULT, TRUE, RangerStation_B2F_EventScript_Release
	special Act2Station_PrepareDeadEnd
	playse SE_M_SURF
	applymovement VAR_TEMP_1, RangerStation_B2F_Movement_Lunge
	waitmovement 0
	playmoncry VAR_0x8006, CRY_MODE_ENCOUNTER
	msgbox RangerStation_B2F_Text_Lunged
	waitmoncry
	closemessage
	setwildbattle SPECIES_MAGIKARP, 1  @ Replaced by the dead end's Pokémon
	special Act2Station_CreateDeadEndMon
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, RangerStation_B2F_EventScript_Beaten
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, RangerStation_B2F_EventScript_Beaten
RangerStation_B2F_EventScript_Release:
	releaseall
	end

RangerStation_B2F_EventScript_Beaten:
	special Act2Station_SetDeadEndBeaten
	removeobject VAR_TEMP_1
	releaseall
	end

RangerStation_B2F_Movement_Lunge:
	jump_in_place_down
	step_end

RangerStation_B2F_EventScript_Medkit::
	lock
	msgbox RangerStation_B2F_Text_Medkit, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RangerStation_B2F_EventScript_MedkitRelease
	setvar VAR_0x8004, ACT2_FINALE_MEDKIT_USED
	special Act2Station_SetFinaleBit
	fadescreen FADE_TO_BLACK
	playfanfare MUS_RG_HEAL
	waitfanfare
	special HealPlayerParty
	fadescreen FADE_FROM_BLACK
	msgbox RangerStation_B2F_Text_Healed
RangerStation_B2F_EventScript_MedkitRelease:
	release
	end

RangerStation_B2F_Text_Lunged:
	.string "A wild {STR_VAR_1} lunged out of\n"
	.string "the water!$"

RangerStation_B2F_Text_Medkit:
	.string "There's a ranger's MEDKIT here.\n"
	.string "Use it?$"

RangerStation_B2F_Text_Healed:
	.string "Your POKéMON were patched up and\n"
	.string "are good as new!$"
