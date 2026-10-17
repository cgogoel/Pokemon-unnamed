@ The farm's storage shed, where Dad works. Act 1's story: docs/hack/act1_story.md

PalletTown_Farm_Barn_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Barn_OnTransition
	.byte 0

@ From the morning after Jangmo-o, Dad is in the house (FLAG_TEMP_1 hides him here)
Barn_OnTransition:
	call_if_ge VAR_ACT1_PROGRESS, ACT1_RESTED, Barn_EventScript_HideDad
	end

Barn_EventScript_HideDad:
	setflag FLAG_TEMP_1
	return

Barn_EventScript_Dad::
	lock
	faceplayer
	switch VAR_ACT1_PROGRESS
	case ACT1_START, Barn_EventScript_DadSeeMom
	case ACT1_GOT_POKEDEX, Barn_EventScript_DadAnotherWay
	case ACT1_MOM_SENT_TO_DAD, Barn_EventScript_DadAnotherWay
	case ACT1_DAD_SENT_TO_WOODS, Barn_EventScript_DadBlipbugReminder
	case ACT1_BEFRIENDED_BLIPBUG, Barn_EventScript_DadWarnJangmoO
	case ACT1_DAD_WARNED_JANGMO_O, Barn_EventScript_DadStayOutOfSight
	case ACT1_BEFRIENDED_THREE, Barn_EventScript_DadTooDangerous
	case ACT1_DAD_SAYS_TOO_DANGEROUS, Barn_EventScript_DadGoSeeMom
	case ACT1_MOM_SAYS_FOLLOW_HEART, Barn_EventScript_DadHardStone
	case ACT1_GOT_HARD_STONE, Barn_EventScript_DadThrowIt
	msgbox Barn_Text_DadWellIllBe
	release
	end

Barn_EventScript_DadSeeMom::
	msgbox Barn_Text_DadSeeMom
	release
	end

Barn_EventScript_DadAnotherWay::
	msgbox Barn_Text_DadAnotherWay
	giveitem ITEM_ORAN_BERRY
	msgbox Barn_Text_DadWoodsPastTheGate
	setvar VAR_ACT1_PROGRESS, ACT1_DAD_SENT_TO_WOODS
	release
	end

Barn_EventScript_DadBlipbugReminder::
	msgbox Barn_Text_DadBlipbugReminder
	release
	end

Barn_EventScript_DadWarnJangmoO::
	msgbox Barn_Text_DadWarnJangmoO
	setvar VAR_ACT1_PROGRESS, ACT1_DAD_WARNED_JANGMO_O
	release
	end

Barn_EventScript_DadStayOutOfSight::
	msgbox Barn_Text_DadStayOutOfSight
	release
	end

Barn_EventScript_DadTooDangerous::
	msgbox Barn_Text_DadTooDangerous
	setvar VAR_ACT1_PROGRESS, ACT1_DAD_SAYS_TOO_DANGEROUS
	release
	end

Barn_EventScript_DadGoSeeMom::
	msgbox Barn_Text_DadGoSeeMom
	release
	end

Barn_EventScript_DadHardStone::
	msgbox Barn_Text_DadHardStone
	giveitem ITEM_HARD_STONE
	msgbox Barn_Text_DadComeHomeSafe
	setvar VAR_ACT1_PROGRESS, ACT1_GOT_HARD_STONE
	release
	end

@ Offering the stone uses it up, so Dad has another if it's gone
Barn_EventScript_DadThrowIt::
	goto_if_unset FLAG_BEFRIENDED_JANGMO_O, Barn_EventScript_DadTryAnotherStone
	msgbox Barn_Text_DadThrowIt
	release
	end

Barn_EventScript_DadTryAnotherStone::
	checkitem ITEM_HARD_STONE
	goto_if_eq VAR_RESULT, FALSE, Barn_EventScript_DadAnotherStone
	msgbox Barn_Text_DadThrowIt
	release
	end

Barn_EventScript_DadAnotherStone::
	msgbox Barn_Text_DadAnotherStone
	giveitem ITEM_HARD_STONE
	release
	end

Barn_Text_DadSeeMom::
	.string "DAD: Hey, kiddo. Your mom wants to\n"
	.string "see you. She's in the house.$"

Barn_Text_DadAnotherWay::
	.string "DAD: Hey, kiddo. I heard about\n"
	.string "PROF. OAK. I'm sorry.\p"
	.string "You know, I never caught a single\n"
	.string "one of the POKéMON on this farm.\p"
	.string "I wasn't selected to be a TRAINER\n"
	.string "either. I found another way.\p"
	.string "Being kind, and being patient.\n"
	.string "That's all it took.\p"
	.string "Why don't you go to the woods and\n"
	.string "get to know the POKéMON there?\p"
	.string "There's been a BLIPBUG hanging\n"
	.string "around the entrance to the woods.\p"
	.string "Here, take this.\n"
	.string "BLIPBUG love these.$"

Barn_Text_DadWoodsPastTheGate::
	.string "DAD: The woods are past the gate on\n"
	.string "the west side of the farm.\p"
	.string "Be kind and be patient, and see\n"
	.string "what happens.$"

Barn_Text_DadBlipbugReminder::
	.string "DAD: That BLIPBUG hangs around the\n"
	.string "woods entrance. Kind and patient!$"

Barn_Text_DadWarnJangmoO::
	.string "DAD: You want to know if BLIPBUG\n"
	.string "can stay? Of course!\p"
	.string "There may be others in the woods\n"
	.string "who'd like to meet you.\p"
	.string "We have plenty of room for any new\n"
	.string "friends you make.\p"
	.string "If you have trouble making friends\n"
	.string "with them, maybe someone in town\l"
	.string "will have some ideas.\p"
	.string "But be careful. There's an angry\n"
	.string "JANGMO-O that roams the woods.\p"
	.string "If it spots you, it'll charge.\n"
	.string "Keep out of its sight!$"

Barn_Text_DadStayOutOfSight::
	.string "DAD: Steer clear of that JANGMO-O.\n"
	.string "Don't let it see you!$"

Barn_Text_DadTooDangerous::
	.string "DAD: Look at you! You've made friends\n"
	.string "with the POKéMON in the woods…\p"
	.string "…except one.\p"
	.string "I know you're getting to know\n"
	.string "POKéMON, {PLAYER}.\p"
	.string "But that JANGMO-O is still too\n"
	.string "dangerous. Leave it alone.\p"
	.string "Go on, your mom's been wanting to\n"
	.string "see you.$"

Barn_Text_DadGoSeeMom::
	.string "DAD: Go see your mom, kiddo.$"

Barn_Text_DadHardStone::
	.string "DAD: So you're set on meeting that\n"
	.string "JANGMO-O, huh?\p"
	.string "…You're as stubborn as your mom.\p"
	.string "Fine. Take this HARD STONE.\n"
	.string "If JANGMO-O attacks, throw it!$"

Barn_Text_DadComeHomeSafe::
	.string "DAD: And come home safe,\n"
	.string "you hear me?$"

Barn_Text_DadThrowIt::
	.string "DAD: Got that HARD STONE?\n"
	.string "If it attacks, throw it!$"

Barn_Text_DadAnotherStone::
	.string "DAD: You lost the HARD STONE?\n"
	.string "Here, I've got another one.$"

Barn_Text_DadWellIllBe::
	.string "DAD: You befriended JANGMO-O?!\n"
	.string "…Well, I'll be.$"

@ The farm's Miltank and Mareep live in the barn (not the farmyard: too many different POKéMON sprites there)
Barn_EventScript_Miltank::
	lock
	faceplayer
	playmoncry SPECIES_MILTANK, CRY_MODE_NORMAL
	msgbox Barn_Text_Miltank
	waitmoncry
	release
	end

Barn_EventScript_Mareep::
	lock
	faceplayer
	playmoncry SPECIES_MAREEP, CRY_MODE_NORMAL
	msgbox Barn_Text_Mareep
	waitmoncry
	release
	end

Barn_Text_Miltank:
	.string "MILTANK: Mooo!$"

Barn_Text_Mareep:
	.string "MAREEP: Baaah!$"
