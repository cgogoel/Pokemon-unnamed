CeruleanCity_House2_Frlg_MapScripts::
	.byte 0

@ The hole in the back wall: the hiker's DIGLETT dug it. He gives away his DIG TM, since it learned the move anyway.
CeruleanCity_House2_EventScript_Hiker::
	lock
	faceplayer
	goto_if_set FLAG_GOT_TM28_FROM_ROCKET, CeruleanCity_House2_EventScript_HikerGaveTM28
	msgbox CeruleanCity_House2_Text_DiglettDugThroughWall
	checkitemspace ITEM_TM28
	goto_if_eq VAR_RESULT, FALSE, CeruleanCity_House2_EventScript_NoRoomForTM28
	giveitem_msg CeruleanCity_House2_Text_ReceivedTM28, ITEM_TM28
	setflag FLAG_GOT_TM28_FROM_ROCKET
	msgbox CeruleanCity_House2_Text_KeepItAwayFromDiglett
	release
	end

CeruleanCity_House2_EventScript_NoRoomForTM28::
	msgbox CeruleanCity_House2_Text_NoRoomForTM28
	release
	end

CeruleanCity_House2_EventScript_HikerGaveTM28::
	msgbox CeruleanCity_House2_Text_LandlordNotAmused
	release
	end

CeruleanCity_House2_EventScript_Lass::
	msgbox CeruleanCity_House2_Text_ShortcutThroughHouse, MSGBOX_NPC
	end

CeruleanCity_House2_EventScript_WallHole::
	msgbox CeruleanCity_House2_Text_DiglettSizedHole, MSGBOX_SIGN
	end

CeruleanCity_House2_Text_DiglettDugThroughWall::
	.string "See that hole? My DIGLETT did that.\p"
	.string "I bought a TM to teach it DIG,\n"
	.string "but it learned the move by itself\l"
	.string "and went right through the wall!\p"
	.string "I won't be needing the TM now.\n"
	.string "You take it.$"

CeruleanCity_House2_Text_ReceivedTM28::
	.string "{PLAYER} received TM28\n"
	.string "from the HIKER.$"

CeruleanCity_House2_Text_KeepItAwayFromDiglett::
	.string "TM28 is DIG.\n"
	.string "Just keep it away from DIGLETT.\p"
	.string "One back door is plenty!$"

CeruleanCity_House2_Text_NoRoomForTM28::
	.string "Your BAG's stuffed full.\n"
	.string "Make some room and come back.$"

CeruleanCity_House2_Text_LandlordNotAmused::
	.string "The landlord wasn't amused.\p"
	.string "I told him it's a feature.\n"
	.string "Free air conditioning!$"

CeruleanCity_House2_Text_ShortcutThroughHouse::
	.string "The whole street uses our wall\n"
	.string "as a shortcut now.\p"
	.string "I should start charging a toll!$"

CeruleanCity_House2_Text_DiglettSizedHole::
	.string "It started out DIGLETT-sized.\n"
	.string "Now anyone can squeeze through!$"
