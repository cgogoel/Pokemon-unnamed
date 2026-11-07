Route25_SeaCottage_Frlg_MapScripts::
	.byte 0

@ BILL at home, a POKéMANIAC tinkering with his TELEPORTER. The base game's accident (BILL fused with a POKéMON) is
@ gone: he just gives away the S.S. ANNE party ticket he doesn't want.
Route25_SeaCottage_EventScript_Bill::
	lock
	faceplayer
	goto_if_set FLAG_GOT_SS_TICKET, Route25_SeaCottage_EventScript_BillCheckOutPC
	msgbox Route25_SeaCottage_Text_ImBill
	checkitemspace ITEM_SS_TICKET
	goto_if_eq VAR_RESULT, FALSE, Route25_SeaCottage_EventScript_NoRoomForSSTicket
	giveitem_msg Route25_SeaCottage_Text_ReceivedSSTicketFromBill, ITEM_SS_TICKET, 1, MUS_RG_OBTAIN_KEY_ITEM
	setflag FLAG_GOT_SS_TICKET_DUP
	setflag FLAG_GOT_SS_TICKET
	setflag FLAG_SYS_PC_LANETTE
	msgbox Route25_SeaCottage_Text_SSAnnePartyYouGoInstead
	release
	end

Route25_SeaCottage_EventScript_NoRoomForSSTicket::
	msgbox Route25_SeaCottage_Text_YouveGotTooMuchStuff
	release
	end

Route25_SeaCottage_EventScript_BillCheckOutPC::
	msgbox Route25_SeaCottage_Text_CheckOutRareMonsOnPC
	release
	end

Route25_SeaCottage_EventScript_Computer::
	lockall
	msgbox Route25_SeaCottage_Text_BillsFavoriteMonList
	goto Route25_SeaCottage_EventScript_BillsMonList
	end

Route25_SeaCottage_EventScript_BillsMonList::
	message Route25_SeaCottage_Text_SeeWhichMon
	waitmessage
	multichoice 0, 0, MULTI_EEVEELUTIONS, FALSE
	switch VAR_RESULT
	case 0, Route25_SeaCottage_EventScript_ViewEevee
	case 1, Route25_SeaCottage_EventScript_ViewFlareon
	case 2, Route25_SeaCottage_EventScript_ViewJolteon
	case 3, Route25_SeaCottage_EventScript_ViewVaporeon
	case 4, Route25_SeaCottage_EventScript_ExitBillsMonList
	case 127, Route25_SeaCottage_EventScript_ExitBillsMonList
	end

Route25_SeaCottage_EventScript_ViewEevee::
	showmonpic SPECIES_EEVEE, 10, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_EEVEE
	special SetSeenMon
	goto Route25_SeaCottage_EventScript_BillsMonList
	end

Route25_SeaCottage_EventScript_ViewFlareon::
	showmonpic SPECIES_FLAREON, 10, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_FLAREON
	special SetSeenMon
	goto Route25_SeaCottage_EventScript_BillsMonList
	end

Route25_SeaCottage_EventScript_ViewJolteon::
	showmonpic SPECIES_JOLTEON, 10, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_JOLTEON
	special SetSeenMon
	goto Route25_SeaCottage_EventScript_BillsMonList
	end

Route25_SeaCottage_EventScript_ViewVaporeon::
	showmonpic SPECIES_VAPOREON, 10, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_VAPOREON
	special SetSeenMon
	goto Route25_SeaCottage_EventScript_BillsMonList
	end

Route25_SeaCottage_EventScript_ExitBillsMonList::
	releaseall
	end

Route25_SeaCottage_Text_ImBill::
	.string "Hiya! Call me BILL!\n"
	.string "I'm a true-blue POKéMANIAC!\p"
	.string "I built the PC Storage System\n"
	.string "everyone uses. Neat, huh?\p"
	.string "These days I'm tinkering with\n"
	.string "that TELEPORTER over there.\p"
	.string "It's not safe to test yet, so\n"
	.string "don't go poking at it!\p"
	.string "Oh, say, are you going anywhere\n"
	.string "near VERMILION? Take this!$"

Route25_SeaCottage_Text_ReceivedSSTicketFromBill::
	.string "{PLAYER} received an S.S. TICKET\n"
	.string "from BILL.$"

Route25_SeaCottage_Text_YouveGotTooMuchStuff::
	.string "You've got too much stuff!$"

Route25_SeaCottage_Text_SSAnnePartyYouGoInstead::
	.string "That cruise ship S.S. ANNE is in\n"
	.string "VERMILION CITY.\p"
	.string "I hear there are lots of TRAINERS\n"
	.string "on board, too.\p"
	.string "They invited me to their party, but\n"
	.string "I can't stand fancy do's.\p"
	.string "Why don't you go instead of me?\n"
	.string "Go on and have a good time.$"

Route25_SeaCottage_Text_CheckOutRareMonsOnPC::
	.string "BILL: Feel like checking out some\n"
	.string "of my rare POKéMON on my PC?\p"
	.string "Go on, check out my PC.$"

Route25_SeaCottage_Text_BillsFavoriteMonList::
	.string "BILL's favorite POKéMON list!$"

Route25_SeaCottage_Text_SeeWhichMon::
	.string "Which POKéMON do you want to see?$"
