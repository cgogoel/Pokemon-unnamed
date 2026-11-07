FiveIsland_Meadow_Frlg_MapScripts::
	.byte 0

FiveIsland_Meadow_EventScript_WarehouseDoor::
	msgbox FiveIsland_Meadow_Text_WarehouseLocked, MSGBOX_SIGN
	end

FiveIsland_Meadow_Text_WarehouseLocked::
	.string "The door is shut tight with a\n"
	.string "heavy padlock.\p"
	.string "A rusty sign reads: “PRIVATE\n"
	.string "STORAGE. NO ENTRY.”$"
