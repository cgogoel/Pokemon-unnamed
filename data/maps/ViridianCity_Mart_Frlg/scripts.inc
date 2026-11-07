ViridianCity_Mart_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, ViridianCity_Mart_OnLoad
	.byte 0

ViridianCity_Mart_OnLoad::
	goto_if_unset FLAG_SYS_POKEDEX_GET, ViridianCity_Mart_EventScript_HideQuestionnaire
	end

ViridianCity_Mart_EventScript_HideQuestionnaire::
	setmetatile 1, 3, METATILE_Mart_CounterMid_Top, 1
	setmetatile 1, 4, METATILE_Mart_CounterMid_Bottom, 1
	end

ViridianCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart ViridianCity_Mart_Items
	msgbox gText_PleaseComeAgain
	release
	end

	.align 2
ViridianCity_Mart_Items::
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_NONE
	release
	end

ViridianCity_Mart_EventScript_Woman::
	msgbox ViridianCity_Mart_Text_ShopDoesGoodBusinessInAntidotes, MSGBOX_NPC
	end

ViridianCity_Mart_EventScript_Youngster::
	msgbox ViridianCity_Mart_Text_GotToBuySomePotions, MSGBOX_NPC
	end

ViridianCity_Mart_Text_ShopDoesGoodBusinessInAntidotes::
	.string "This shop does good business in\n"
	.string "ANTIDOTES, I've heard.$"

ViridianCity_Mart_Text_GotToBuySomePotions::
	.string "I've got to buy some POTIONS.\p"
	.string "You never know when your POKéMON\n"
	.string "will need quick healing.$"

