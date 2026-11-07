SilphCo_6F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_6F_OnLoad
	.byte 0

SilphCo_6F_OnLoad::
	call_if_unset FLAG_SILPH_6F_DOOR, EventScript_Close6FDoor
	end

SilphCo_6F_EventScript_WorkerM2::
	msgbox SilphCo_6F_Text_ComeWorkForSilphWhenYoureOlder, MSGBOX_NPC
	end

SilphCo_6F_EventScript_WorkerM3::
	msgbox SilphCo_6F_Text_BetterGetBackToWork, MSGBOX_NPC
	end

SilphCo_6F_EventScript_WorkerM1::
	msgbox SilphCo_6F_Text_WeGotEngaged, MSGBOX_NPC
	end

SilphCo_6F_EventScript_WorkerF1::
	msgbox SilphCo_6F_Text_NeedsMeToLookAfterHim, MSGBOX_NPC
	end

SilphCo_6F_EventScript_WorkerF2::
	msgbox SilphCo_6F_Text_ShippingToEveryMart, MSGBOX_NPC
	end

SilphCo_6F_EventScript_FloorSign::
	msgbox SilphCo_6F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_6F_EventScript_Taylor::
	trainerbattle_single TRAINER_SCIENTIST_TAYLOR, SilphCo_6F_Text_TaylorIntro, SilphCo_6F_Text_TaylorDefeat
	msgbox SilphCo_6F_Text_TaylorPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_EventScript_Grunt1::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_30, SilphCo_6F_Text_Grunt1Intro, SilphCo_6F_Text_Grunt1Defeat
	msgbox SilphCo_6F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_EventScript_Grunt2::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_31, SilphCo_6F_Text_Grunt2Intro, SilphCo_6F_Text_Grunt2Defeat
	msgbox SilphCo_6F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_6F_Text_BetterGetBackToWork::
	.string "Lunch break's over already?\p"
	.string "Well, better get back to work.\n"
	.string "These forms won't file themselves.$"

SilphCo_6F_Text_WeGotEngaged::
	.string "We got engaged last week.\n"
	.string "Heheh!\p"
	.string "We met at the company picnic\n"
	.string "on ROUTE 8!$"

SilphCo_6F_Text_NeedsMeToLookAfterHim::
	.string "He forgot his lunch again today.\p"
	.string "He's so helpless, he needs\n"
	.string "someone like me to look after him.$"

SilphCo_6F_Text_ShippingToEveryMart::
	.string "Our department ships SILPH goods\n"
	.string "to every POKéMON MART in KANTO.\p"
	.string "If a POTION's late, the phone\n"
	.string "on my desk never stops ringing!$"

SilphCo_6F_Text_ComeWorkForSilphWhenYoureOlder::
	.string "Visiting SILPH, are you?\p"
	.string "Study hard, and come work for us\n"
	.string "when you're older.$"

SilphCo_6F_Text_Grunt1Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_6F_Text_Grunt1Defeat::
	.string "Flame out!$"

SilphCo_6F_Text_Grunt1PostBattle::
	.string "No matter!\n"
	.string "My brothers will avenge me!$"

SilphCo_6F_Text_TaylorIntro::
	.string "That rotten PRESIDENT!\p"
	.string "Serves him right for shipping me\n"
	.string "off to the TIKSI BRANCH!\p"
	.string "That's why TEAM ROCKET came after\n"
	.string "us, I'm sure of it!$"

SilphCo_6F_Text_TaylorDefeat::
	.string "Shoot!$"

SilphCo_6F_Text_TaylorPostBattle::
	.string "TIKSI BRANCH?\n"
	.string "It's in Russian no-man's-land!$"

SilphCo_6F_Text_Grunt2Intro::
	.string "You dare betray TEAM ROCKET?$"

SilphCo_6F_Text_Grunt2Defeat::
	.string "You traitor!$"

SilphCo_6F_Text_Grunt2PostBattle::
	.string "If you stand for justice, you\n"
	.string "betray us bad guys!$"

SilphCo_6F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "6F$"

