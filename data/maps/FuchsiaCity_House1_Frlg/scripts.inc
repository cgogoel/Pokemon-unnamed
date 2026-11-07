FuchsiaCity_House1_Frlg_MapScripts::
	.byte 0

FuchsiaCity_House1_EventScript_Woman::
	msgbox FuchsiaCity_House1_Text_WardenIsOldHasFalseTeeth, MSGBOX_NPC
	end

FuchsiaCity_House1_EventScript_OldMan::
	lock
	faceplayer
	famechecker FAMECHECKER_BILL, 2
	msgbox FuchsiaCity_House1_Text_MyGrandsonBill
	release
	end

FuchsiaCity_House1_EventScript_LittleBoy::
	msgbox FuchsiaCity_House1_Text_BillsStorageSystem, MSGBOX_NPC
	end

FuchsiaCity_House1_Text_WardenIsOldHasFalseTeeth::
	.string "The SAFARI ZONE's WARDEN is old, \n"
	.string "but he's still very much active.\p"
	.string "All his teeth are false, though.$"

FuchsiaCity_House1_Text_MyGrandsonBill::
	.string "Hmm?\n"
	.string "You like POKéMON, do you?\p"
	.string "So does my grandson, BILL.\n"
	.string "He lives by the sea past CERULEAN.\p"
	.string "He always liked collecting things,\n"
	.string "even as a child!$"

FuchsiaCity_House1_Text_BillsStorageSystem::
	.string "Cousin BILL keeps POKéMON data on\n"
	.string "his PC. He calls it a “storage\l"
	.string "system.”\p"
	.string "Mom calls it his junk drawer!$"

