SilphCo_11F_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, SilphCo_11F_OnLoad
	.byte 0

SilphCo_11F_OnLoad::
	call_if_unset FLAG_SILPH_11F_DOOR, EventScript_Close11FDoor
	end

SilphCo_11F_EventScript_President::
	lock
	faceplayer
	goto_if_set FLAG_GOT_MASTER_BALL_FROM_SILPH, SilphCo_11F_EventScript_AlreadyGotMasterBall
	msgbox SilphCo_11F_Text_PresidentFieldTest
	checkitemspace ITEM_MASTER_BALL
	goto_if_eq VAR_RESULT, FALSE, SilphCo_11F_EventScript_NoRoomForMasterBall
	giveitem_msg SilphCo_11F_Text_ObtainedMasterBallFromPresident, ITEM_MASTER_BALL, 1, MUS_RG_OBTAIN_KEY_ITEM
	msgbox SilphCo_11F_Text_ThatsOurSecretPrototype
	setflag FLAG_GOT_MASTER_BALL_FROM_SILPH
	release
	end

SilphCo_11F_EventScript_NoRoomForMasterBall::
	msgbox SilphCo_11F_Text_YouHaveNoRoomForThis
	release
	end

SilphCo_11F_EventScript_AlreadyGotMasterBall::
	msgbox SilphCo_11F_Text_ThatsOurSecretPrototype
	release
	end

SilphCo_11F_EventScript_Secretary::
	msgbox SilphCo_11F_Text_PresidentLovesVisitors, MSGBOX_NPC
	end

SilphCo_11F_EventScript_FloorSign::
	msgbox SilphCo_11F_Text_FloorSign, MSGBOX_SIGN
	end

SilphCo_11F_EventScript_Grunt1::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_40, SilphCo_11F_Text_Grunt1Intro, SilphCo_11F_Text_Grunt1Defeat
	msgbox SilphCo_11F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_11F_EventScript_Grunt2::
	trainerbattle_single TRAINER_TEAM_ROCKET_GRUNT_41, SilphCo_11F_Text_Grunt2Intro, SilphCo_11F_Text_Grunt2Defeat
	msgbox SilphCo_11F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

SilphCo_11F_Text_PresidentFieldTest::
	.string "PRESIDENT: Oh, a young TRAINER\n"
	.string "made it all the way up here!\p"
	.string "You found your way through our\n"
	.string "maze of doors and warp pads?\p"
	.string "Then you're just who I need.\n"
	.string "Our engineers want field data.\p"
	.string "Here, take this out into the\n"
	.string "world and put it to good use.$"

SilphCo_11F_Text_ObtainedMasterBallFromPresident::
	.string "{PLAYER} obtained a MASTER BALL\n"
	.string "from the PRESIDENT!$"

SilphCo_11F_Text_ThatsOurSecretPrototype::
	.string "PRESIDENT: You can't buy that\n"
	.string "anywhere.\p"
	.string "It's our secret prototype MASTER\n"
	.string "BALL.\p"
	.string "It will catch any POKéMON without\n"
	.string "fail!\p"
	.string "You should be quiet about using it,\n"
	.string "though.$"

SilphCo_11F_Text_YouHaveNoRoomForThis::
	.string "You have no room for this.$"

SilphCo_11F_Text_PresidentLovesVisitors::
	.string "SECRETARY: The PRESIDENT's\n"
	.string "schedule is packed today.\p"
	.string "But he never turns away a TRAINER.\n"
	.string "He says they're his best testers.$"

SilphCo_11F_Text_Grunt2Intro::
	.string "Stop right there!\n"
	.string "Don't you move!$"

SilphCo_11F_Text_Grunt2Defeat::
	.string "Don't…\n"
	.string "Please!$"

SilphCo_11F_Text_Grunt2PostBattle::
	.string "So, you want to see my BOSS?$"

SilphCo_11F_Text_Grunt1Intro::
	.string "Halt! Do you have an appointment\n"
	.string "with my BOSS?$"

SilphCo_11F_Text_Grunt1Defeat::
	.string "Gaah!\n"
	.string "Demolished!$"

SilphCo_11F_Text_Grunt1PostBattle::
	.string "Watch your step…\n"
	.string "My BOSS likes his POKéMON tough!$"

SilphCo_11F_Text_FloorSign::
	.string "SILPH CO. HEAD OFFICE\n"
	.string "11F$"

