SilphCo_7F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_7F_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, SilphCo_7F_OnTransition
	.byte 0

SilphCo_7F_OnLoad::
	call_if_unset FLAG_SILPH_7F_DOOR_1, EventScript_Close7FDoor1
	call_if_unset FLAG_SILPH_7F_DOOR_2, EventScript_Close7FDoor2
	call_if_unset FLAG_SILPH_7F_DOOR_3, EventScript_Close7FDoor3
	end

SilphCo_7F_OnTransition::
	call_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, SilphCo_7F_EventScript_Act3Calm
	end

@ Act 3 starts on this floor, in the player's lab, before Team Rocket's invasion: no Rockets (they and the Rocket
@ scientist Joshua are hidden on every floor; FLAG_HIDE_SILPH_ROCKETS is also set at new game, so they're never
@ seen before the invasion). The vanilla rival scene here and Giovanni's on 11F are gone. The Act 3 story can bring
@ the Rockets back when the invasion comes (clearflag FLAG_HIDE_SILPH_ROCKETS).
SilphCo_7F_EventScript_Act3Calm::
	setflag FLAG_HIDE_SILPH_ROCKETS
	return

@ Act 3, before the invasion: the floor's staff at work (placeholder lines)
SilphCo_7F_EventScript_Act3Staff::
	msgbox SilphCo_7F_Text_Act3Staff
	release
	end

SilphCo_7F_EventScript_LaprasGuy::
	lock
	faceplayer
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, SilphCo_7F_EventScript_Act3Staff
	msgbox SilphCo_7F_Text_LabPoolIsMyJob
	release
	end

SilphCo_7F_EventScript_WorkerM1::
	lock
	faceplayer
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, SilphCo_7F_EventScript_Act3Staff
	msgbox SilphCo_7F_Text_MasterBallBudget
	release
	end

SilphCo_7F_EventScript_WorkerM2::
	lock
	faceplayer
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, SilphCo_7F_EventScript_Act3Staff
	msgbox SilphCo_7F_Text_StaringAtTheMonitor
	release
	end

SilphCo_7F_EventScript_WorkerF::
	lock
	faceplayer
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, SilphCo_7F_EventScript_Act3Staff
	msgbox SilphCo_7F_Text_LabCoatsInTheWash
	release
	end

SilphCo_7F_EventScript_FloorSign::
	msgbox SilphCo_7F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_7F_EventScript_Joshua::
	trainerbattle_single TRAINER_SCIENTIST_JOSHUA, SilphCo_7F_Text_JoshuaIntro, SilphCo_7F_Text_JoshuaDefeat
	msgbox SilphCo_7F_Text_JoshuaPostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt1::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_33, SilphCo_7F_Text_Grunt1Intro, SilphCo_7F_Text_Grunt1Defeat
	msgbox SilphCo_7F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt2::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_34, SilphCo_7F_Text_Grunt2Intro, SilphCo_7F_Text_Grunt2Defeat
	msgbox SilphCo_7F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_EventScript_Grunt3::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_35, SilphCo_7F_Text_Grunt3Intro, SilphCo_7F_Text_Grunt3Defeat
	msgbox SilphCo_7F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_7F_Text_LabPoolIsMyJob::
	.string "This floor is SILPH's research lab.\p"
	.string "I look after the tank where we\n"
	.string "keep the water POKéMON.\p"
	.string "Scrubbing it out takes all\n"
	.string "morning, but they love it clean!$"

SilphCo_7F_Text_MasterBallBudget::
	.string "We're developing a POKé BALL that\n"
	.string "never fails. The MASTER BALL!\p"
	.string "Well, it never fails in theory.\n"
	.string "The budget keeps failing, though.$"

SilphCo_7F_Text_StaringAtTheMonitor::
	.string "I've been staring at this monitor\n"
	.string "since sunrise.\p"
	.string "If these numbers don't add up\n"
	.string "soon, I'm going home!$"

SilphCo_7F_Text_LabCoatsInTheWash::
	.string "A GRIMER got loose in the lab\n"
	.string "this morning.\p"
	.string "Every lab coat on this floor is\n"
	.string "in the wash now. Phew!$"

SilphCo_7F_Text_Grunt3Intro::
	.string "Uh-oh!\n"
	.string "I smell a little mouse!$"

SilphCo_7F_Text_Grunt3Defeat::
	.string "Lights out!$"

SilphCo_7F_Text_Grunt3PostBattle::
	.string "You won't find my BOSS by just\n"
	.string "scurrying around!$"

SilphCo_7F_Text_JoshuaIntro::
	.string "Heheh!\p"
	.string "You mistook me for a SILPH worker?$"

SilphCo_7F_Text_JoshuaDefeat::
	.string "I'm done!$"

SilphCo_7F_Text_JoshuaPostBattle::
	.string "Despite your age, you are a\n"
	.string "skilled TRAINER!$"

SilphCo_7F_Text_Grunt1Intro::
	.string "I am one of the four ROCKET\n"
	.string "BROTHERS!$"

SilphCo_7F_Text_Grunt1Defeat::
	.string "Aack!\n"
	.string "Brothers, I lost!$"

SilphCo_7F_Text_Grunt1PostBattle::
	.string "Doesn't matter.\n"
	.string "My brothers will repay the favor!$"

SilphCo_7F_Text_Grunt2Intro::
	.string "A child intruder?\n"
	.string "That must be you!$"

SilphCo_7F_Text_Grunt2Defeat::
	.string "Fine!\n"
	.string "I lost!$"

SilphCo_7F_Text_Grunt2PostBattle::
	.string "Go on home before my BOSS gets\n"
	.string "ticked off!$"

SilphCo_7F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "7F$"

SilphCo_7F_Text_Act3Staff::
	.string "Morning! Another busy day\n"
	.string "in the lab.$"
