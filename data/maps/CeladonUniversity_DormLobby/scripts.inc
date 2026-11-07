@ Celadon University: Dorm Lobby (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_DormLobby_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CeladonUniversity_DormLobby_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CeladonUniversity_DormLobby_OnFrame
	.byte 0

CeladonUniversity_DormLobby_EventScript_OldWoman0::
	msgbox CeladonUniversity_DormLobby_Text_OldWoman0, MSGBOX_NPC
	end

CeladonUniversity_DormLobby_EventScript_Meowth1::
	lock
	faceplayer
	playmoncry SPECIES_MEOWTH, CRY_MODE_NORMAL
	msgbox CeladonUniversity_DormLobby_Text_Meowth1
	waitmoncry
	release
	end

CeladonUniversity_DormLobby_EventScript_Clefairy2::
	lock
	faceplayer
	playmoncry SPECIES_CLEFAIRY, CRY_MODE_NORMAL
	msgbox CeladonUniversity_DormLobby_Text_Clefairy2
	waitmoncry
	release
	end

CeladonUniversity_DormLobby_EventScript_Sign0::
	msgbox CeladonUniversity_DormLobby_Text_Sign0, MSGBOX_SIGN
	end

CeladonUniversity_DormLobby_Text_OldWoman0::
	.string "Welcome home, dear! Your last year\n"
	.string "here already. I'll miss you.\p"
	.string "No battling in the halls!$"

CeladonUniversity_DormLobby_Text_Meowth1::
	.string "MEOWTH: Meowth!$"

CeladonUniversity_DormLobby_Text_Clefairy2::
	.string "CLEFAIRY: Pippi!$"

CeladonUniversity_DormLobby_Text_Sign0::
	.string "DORM RULES\n"
	.string "No battling in the halls!$"
