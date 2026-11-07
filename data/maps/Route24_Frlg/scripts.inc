Route24_Frlg_MapScripts::
	.byte 0

@ The NUGGET BRIDGE's host: crossing the bridge wins the prize (VAR_MAP_SCENE_ROUTE24 = 1 once given).
Route24_EventScript_BridgeHost::
	lock
	faceplayer
	goto_if_eq VAR_MAP_SCENE_ROUTE24, 1, Route24_EventScript_BridgeHostPrizeGiven
	msgbox Route24_Text_JustEarnedFabulousPrize
	checkitemspace ITEM_NUGGET
	goto_if_eq VAR_RESULT, FALSE, Route24_EventScript_NoRoomForNugget
	giveitem_msg Route24_Text_ReceivedNuggetFromHost, ITEM_NUGGET
	setvar VAR_MAP_SCENE_ROUTE24, 1
	msgbox Route24_Text_ComeBackNextSeason
	release
	end

Route24_EventScript_NoRoomForNugget::
	msgbox Route24_Text_YouDontHaveAnyRoom
	release
	end

Route24_EventScript_BridgeHostPrizeGiven::
	msgbox Route24_Text_ComeBackNextSeason
	release
	end

Route24_Text_JustEarnedFabulousPrize::
	.string "Congratulations! You crossed the\n"
	.string "NUGGET BRIDGE!\p"
	.string "Our CERULEAN TRAINERS' CLUB runs\n"
	.string "this challenge every weekend.\p"
	.string "Here's your fabulous prize!$"

Route24_Text_ReceivedNuggetFromHost::
	.string "{PLAYER} received a NUGGET\n"
	.string "from the host!$"

Route24_Text_YouDontHaveAnyRoom::
	.string "You don't have any room!$"

Route24_Text_ComeBackNextSeason::
	.string "Sell that NUGGET at a shop and\n"
	.string "treat your POKéMON to something.\p"
	.string "The club funds the prizes with\n"
	.string "a bake sale. I made the cookies!$"

Route24_Text_ShaneIntro::
	.string "I saw your feat from the grass!$"

Route24_Text_ShaneDefeat::
	.string "I thought not!$"

Route24_Text_ShanePostBattle::
	.string "I hid because the crowd on the\n"
	.string "bridge is so loud on weekends.$"

Route24_Text_EthanIntro::
	.string "Okay! I'm No. 5!\n"
	.string "I'll stomp you!$"

Route24_Text_EthanDefeat::
	.string "Whoa!\n"
	.string "Too much!$"

Route24_Text_EthanPostBattle::
	.string "I did my best. I have no regrets!$"

Route24_Text_ReliIntro::
	.string "I'm No. 4!\n"
	.string "Getting tired?$"

Route24_Text_ReliDefeat::
	.string "I lost, too!$"

Route24_Text_ReliPostBattle::
	.string "I did my best, so I've no regrets!$"

Route24_Text_TimmyIntro::
	.string "Here's No. 3!\n"
	.string "I won't be easy!$"

Route24_Text_TimmyDefeat::
	.string "Ow!\n"
	.string "Stomped flat!$"

Route24_Text_TimmyPostBattle::
	.string "I did my best. I have no regrets!$"

Route24_Text_AliIntro::
	.string "I'm second!\n"
	.string "Now it's serious!$"

Route24_Text_AliDefeat::
	.string "How could I lose?$"

Route24_Text_AliPostBattle::
	.string "I did my best. I have no regrets!$"

Route24_Text_CaleIntro::
	.string "People call this the NUGGET\n"
	.string "BRIDGE!\p"
	.string "Beat us five TRAINERS and win\n"
	.string "a fabulous prize!\p"
	.string "Think you got what it takes?$"

Route24_Text_CaleDefeat::
	.string "Whoo!\n"
	.string "Good stuff!$"

Route24_Text_CalePostBattle::
	.string "I did my best. I have no regrets!$"
