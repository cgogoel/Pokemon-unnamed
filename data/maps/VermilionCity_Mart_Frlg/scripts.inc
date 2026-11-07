VermilionCity_Mart_Frlg_MapScripts::
	.byte 0

VermilionCity_Mart_EventScript_CooltrainerF::
	msgbox VermilionCity_Mart_Text_MonsGoodOrBadDependingOnTrainer, MSGBOX_NPC
	end

VermilionCity_Mart_EventScript_BaldingMan::
	msgbox VermilionCity_Mart_Text_StockingUpForVoyage, MSGBOX_NPC
	end

VermilionCity_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart VermilionCity_Mart_Items
	msgbox gText_PleaseComeAgain
	release
	end

	.align 2
VermilionCity_Mart_Items::
	.2byte ITEM_POKE_BALL
	.2byte ITEM_SUPER_POTION
	.2byte ITEM_ANTIDOTE
	.2byte ITEM_PARALYZE_HEAL
	.2byte ITEM_AWAKENING
	.2byte ITEM_ICE_HEAL
	.2byte ITEM_REPEL
	.2byte ITEM_NONE
	release
	end

VermilionCity_Mart_Text_StockingUpForVoyage::
	.string "I'm stocking up before my ship\n"
	.string "sails for the SEVII ISLANDS.\p"
	.string "Three weeks at sea with my\n"
	.string "POLIWHIRL and a crate of POTIONS.\p"
	.string "My wife says I pack like a\n"
	.string "nervous TRAINER on his first day!$"

VermilionCity_Mart_Text_MonsGoodOrBadDependingOnTrainer::
	.string "I think POKéMON can be good or\n"
	.string "bad. It depends on the TRAINER.$"

