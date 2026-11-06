@ Act 2's finale: the legendary trio's battle in the ranger station's B4F chamber (docs/hack/act2_finale.md,
@ src/act2_legends.c). A wild 3v3 against ENTEI, SUICUNE and VIRIZION (Lv. ACT2_LEGENDS_LEVEL): the player acts once,
@ then the trio attacks together and the battle cuts out. No damage, no blackout, no EXP; the player can't run.
@ Afterwards the field music is back and the calling script goes on; VAR_RESULT is B_OUTCOME_PLAYER_TELEPORTED (5).
Act2_EventScript_LegendsBattle::
	special StartAct2LegendsBattle
	return
