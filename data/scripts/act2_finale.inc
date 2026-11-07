@ Act 2's finale, the final project (docs/hack/act2_finale.md): the director's brief, the road home, Mom and Dad,
@ the Woods' noise, the ranger station's chamber, and the ending. Progress is VAR_ACT2_PROGRESS from
@ ACT2_GOT_TRAINER_CARD.

@ ---------------------------------------------------------------------------------------------------------------
@ The brief: the morning after the Trainer Card, the friends wait at the Science Research Institute

Act2_EventScript_InstituteObjects::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD, Act2_EventScript_ShowObjectsTemp1
	end

@ Coord events just inside the door (x 3-5, y 8): the player walks up between KAI and MINA, facing the director
Act2_EventScript_FinalBrief::
	lockall
	msgbox Act2_Text_BriefComeIn
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	call_if_eq VAR_0x8004, 3, Act2_EventScript_BriefWalkFrom3
	call_if_eq VAR_0x8004, 4, Act2_EventScript_BriefWalkFrom4
	call_if_eq VAR_0x8004, 5, Act2_EventScript_BriefWalkFrom5
	waitmovement 0
	msgbox Act2_Text_Brief
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Act2_Text_GotTownMap
	waitmessage
	waitfanfare
	additem ITEM_TOWN_MAP
	addmoney 10000
	playse SE_SHOP
	msgbox Act2_Text_GotMoney
	msgbox Act2_Text_BriefMart
	closemessage
	@ The friends come over
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_INSTITUTE_KAI, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_INSTITUTE_MINA, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_INSTITUTE_ROWAN, Act2_Movement_RowanToPlayer
	waitmovement 0
	msgbox Act2_Text_FriendsTMs
	msgbox Act2_Text_FriendsAskPlayer
	msgbox Act2_Text_FriendsWarn
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_INSTITUTE_KAI
	removeobject LOCALID_INSTITUTE_MINA
	removeobject LOCALID_INSTITUTE_ROWAN
	setflag FLAG_TEMP_1
	setflag FLAG_ACT2_ROUTE_OPEN
	setvar VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT
	fadescreen FADE_FROM_BLACK
	releaseall
	end

Act2_EventScript_BriefWalkFrom3:
	applymovement LOCALID_PLAYER, Act2_Movement_BriefFrom3
	return

Act2_EventScript_BriefWalkFrom4:
	applymovement LOCALID_PLAYER, Act2_Movement_BriefFrom4
	return

Act2_EventScript_BriefWalkFrom5:
	applymovement LOCALID_PLAYER, Act2_Movement_BriefFrom5
	return

Act2_Movement_BriefFrom3:
	walk_right
Act2_Movement_BriefFrom4:
	walk_right
Act2_Movement_BriefFrom5:
	walk_right
	walk_right
	walk_up
	step_end

@ From (9,7) to just below the player (7,8)
Act2_Movement_RowanToPlayer:
	walk_down
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

Act2_EventScript_InstituteKai::
	msgbox Act2_Text_InstituteKai, MSGBOX_NPC
	end

Act2_EventScript_InstituteMina::
	msgbox Act2_Text_InstituteMina, MSGBOX_NPC
	end

Act2_EventScript_InstituteRowan::
	msgbox Act2_Text_InstituteRowan, MSGBOX_NPC
	end

@ The director, at the institute
Act2_EventScript_Director::
	lock
	faceplayer
	goto_if_ge VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT, Act2_EventScript_DirectorProject
	msgbox Act2_Text_DirectorEarly
	release
	end

Act2_EventScript_DirectorProject:
	msgbox Act2_Text_DirectorProject
	release
	end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_BriefComeIn:
	.string "DIRECTOR: Ah, there you are.\n"
	.string "Come in, come in.$"

Act2_Text_Brief:
	.string "DIRECTOR: Good morning, all four\n"
	.string "of you. Your final project.\p"
	.string "Make a TM. Any move you like,\n"
	.string "as long as it has 120 power or\l"
	.string "more.\p"
	.string "No research sheet this time.\n"
	.string "No field trip. Follow your passion.\p"
	.string "Remember what got you interested\n"
	.string "in POKéMON in the first place.\l"
	.string "That's where your move is.\p"
	.string "This exam is unguided, so you'll\n"
	.string "need these.$"

Act2_Text_GotTownMap:
	.string "Each of you got a TOWN MAP!$"

Act2_Text_GotMoney:
	.string "And ¥10,000 for the project!$"

Act2_Text_BriefMart:
	.string "DIRECTOR: That's your budget.\n"
	.string "Spend it well.\p"
	.string "Stock up at the DEPT. STORE in\n"
	.string "CELADON before you head out.\p"
	.string "Come back when your TM is ready.\n"
	.string "Good luck.$"

Act2_Text_FriendsTMs:
	.string "KAI: FIRE, obviously. I'm making\n"
	.string "FLARE BLITZ. All power, all in.\p"
	.string "MINA: WATER for me. WAVE CRASH,\n"
	.string "a wave that hits like a wall.\p"
	.string "ROWAN: And GRASS for me.\n"
	.string "SOLAR BEAM, charged by the sun.$"

Act2_Text_FriendsAskPlayer:
	.string "KAI: What about you, {PLAYER}?\n"
	.string "What's your type?\p"
	.string "{PLAYER}: …\p"
	.string "……\p"
	.string "MINA: You don't have one, do you?\p"
	.string "ROWAN: You never did. You liked\n"
	.string "every POKéMON you met.\p"
	.string "{PLAYER}: …\p"
	.string "KAI: So you want to find\n"
	.string "something new!\p"
	.string "MINA: Then do what the DIRECTOR\n"
	.string "said. Remember how it started.\p"
	.string "ROWAN: You grew up by some woods,\n"
	.string "right? Maybe go back there.$"

Act2_Text_FriendsWarn:
	.string "ROWAN: It's a long road, though.\n"
	.string "Check your TOWN MAP to find\l"
	.string "your way home.\p"
	.string "Take it town by town. TRAINERS\n"
	.string "and wild POKéMON will be waiting\l"
	.string "all along the way.\p"
	.string "MINA: There are two ways, I think.\n"
	.string "South through VERMILION and\l"
	.string "DIGLETT'S CAVE…\p"
	.string "Or north through CERULEAN,\n"
	.string "MT. MOON and PEWTER.\p"
	.string "Your call, {PLAYER}!\p"
	.string "KAI: And you've got a TRAINER CARD\n"
	.string "now. Every TRAINER out there will\l"
	.string "want a shot at you!\p"
	.string "MINA: See you back here with\n"
	.string "your TM!$"

Act2_Text_InstituteKai:
	.string "KAI: The DIRECTOR's waiting!$"

Act2_Text_InstituteMina:
	.string "MINA: Morning! Come on in.$"

Act2_Text_InstituteRowan:
	.string "ROWAN: The final project…\n"
	.string "I wonder what it is.$"

Act2_Text_DirectorEarly:
	.string "DIRECTOR: Welcome to the SCIENCE\n"
	.string "RESEARCH INSTITUTE.\p"
	.string "Finish your courses, then come see\n"
	.string "me about your final project.$"

Act2_Text_DirectorProject:
	.string "DIRECTOR: Follow your passion.\n"
	.string "Your TM will tell me the rest.$"

@ ---------------------------------------------------------------------------------------------------------------
@ Home: Mom and Dad in the farmhouse (from Act 2 on). On the final project, they send the player to the Woods.

Act2_EventScript_FarmhouseParents::
	lock
	faceplayer
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, Act2_EventScript_ParentsLater
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT, Act2_EventScript_ParentsMeditate
	goto_if_ge VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED, Act2_EventScript_ParentsAfterChamber
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED, Act2_EventScript_ParentsGoToWoods
	msgbox Act2_Text_ParentsVisit
	release
	end

Act2_EventScript_ParentsMeditate:
	msgbox Act2_Text_ParentsMeditate
	setvar VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED
	release
	end

Act2_EventScript_ParentsGoToWoods:
	msgbox Act2_Text_ParentsGoToWoods
	release
	end

Act2_EventScript_ParentsAfterChamber:
	msgbox Act2_Text_ParentsAfterChamber
	release
	end

Act2_EventScript_ParentsLater:
	msgbox Act2_Text_ParentsLater
	release
	end

@ The Woods' edge in the farmyard: open once Mom and Dad have talked (the final project)
Act2_EventScript_FarmyardToWoods::
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED, Act2_EventScript_FarmyardWoodsNotYet
	warp MAP_WOODS, 49, 58
	waitstate
	end

Act2_EventScript_FarmyardWoodsNotYet:
	lockall
	msgbox Act2_Text_WoodsTalkToParents
	goto Farmyard_EventScript_WoodsTurnBack

@ ---------------------------------------------------------------------------------------------------------------
@ The Woods (Act 2 on): the Act 1 Pokémon have all gone to the farm. On the final project, the noise: the first line
@ on arrival (VAR_TEMP_1 1 → 2), then Act2_TryStartWoodsNoise after each step.

Act2_EventScript_WoodsSetup::
	setflag FLAG_BEFRIENDED_BLIPBUG
	setflag FLAG_BEFRIENDED_TYNAMO
	setflag FLAG_BEFRIENDED_HATENNA
	setflag FLAG_BEFRIENDED_TEDDIURSA
	setflag FLAG_BEFRIENDED_JANGMO_O
	call_if_eq VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED, Act2_EventScript_WoodsNoiseOn
	return

Act2_EventScript_WoodsNoiseOn:
	setvar VAR_TEMP_1, 1
	return

Act2_EventScript_WoodsNoiseFaint::
	lockall
	setvar VAR_TEMP_1, 2
	special Act2_ResetWoodsNoise
	msgbox Act2_Text_WoodsNoiseFaint
	releaseall
	end

Act2_EventScript_WoodsNoiseLouder::
	lockall
	msgbox Act2_Text_WoodsNoiseLouder
	releaseall
	end

Act2_EventScript_WoodsNoiseNear::
	lockall
	msgbox Act2_Text_WoodsNoiseNear
	releaseall
	end

Act2_EventScript_WoodsNoiseQuieter::
	lockall
	msgbox Act2_Text_WoodsNoiseQuieter
	releaseall
	end

@ The ranger station's door (coord events on it, Act 2 on)
Act2_EventScript_StationDoor::
	lockall
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_PARENTS_TALKED, Act2_EventScript_StationDoorShut
	goto_if_ge VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED, Act2_EventScript_StationDoorShut
	msgbox Act2_Text_StationReady, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_StationStepBack
	closemessage
	goto Act2_EventScript_EnterStation

Act2_EventScript_StationDoorShut:
	msgbox Act2_Text_StationShut
Act2_EventScript_StationStepBack:
	closemessage
	applymovement LOCALID_PLAYER, Woods_Movement_StepBackDown
	waitmovement 0
	releaseall
	end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_ParentsVisit:
	.string "MOM: {PLAYER}! A surprise visit?\n"
	.string "How's university treating you?\p"
	.string "DAD: Don't let us keep you. Your\n"
	.string "friends are waiting, I bet.$"

Act2_Text_ParentsMeditate:
	.string "MOM: {PLAYER}! You're home!\n"
	.string "Look at you, all grown up.\p"
	.string "DAD: A final project, huh?\n"
	.string "A move with 120 power…\p"
	.string "{PLAYER}: …\p"
	.string "MOM: You don't know where\n"
	.string "to start?\p"
	.string "DAD: When I don't know what to do,\n"
	.string "I go sit in the WOODS. I just sit,\l"
	.string "and listen.\p"
	.string "MOM: That's where it all started\n"
	.string "for you. BLIPBUG, JANGMO-O, then\l"
	.string "PROF. OAK came knocking.\p"
	.string "DAD: Go back to the WOODS, where\n"
	.string "your university journey started.\l"
	.string "Meditate. Clear your head.\p"
	.string "The answer usually finds you.$"

Act2_Text_ParentsGoToWoods:
	.string "DAD: The WOODS are past the\n"
	.string "farmyard. Listen to them.\p"
	.string "MOM: Come home for dinner!$"

Act2_Text_ParentsAfterChamber:
	.string "MOM: No more sneaking into old\n"
	.string "ranger stations, OK?\p"
	.string "DAD: Heh. It runs in the family.$"

Act2_Text_ParentsLater:
	.string "MOM: {PLAYER}! Working hard at\n"
	.string "SILPH CO.? Don't forget to eat!$"

Act2_Text_WoodsTalkToParents:
	.string "I should say hi to MOM and DAD\n"
	.string "first. They're in the house.$"

Act2_Text_WoodsNoiseFaint:
	.string "…\p"
	.string "An eerie noise is faintly heard…$"

Act2_Text_WoodsNoiseLouder:
	.string "The noise is getting louder…$"

Act2_Text_WoodsNoiseNear:
	.string "The noise is loud now.\n"
	.string "It's coming from the old ranger\l"
	.string "station!$"

Act2_Text_WoodsNoiseQuieter:
	.string "The noise is getting quieter…$"

Act2_Text_StationReady:
	.string "The noise is coming from inside.\p"
	.string "Am I ready to go in?$"

Act2_Text_StationShut:
	.string "The old ranger station.\n"
	.string "The door won't budge.$"

@ Into the ranger station (its 1F door mat)
Act2_EventScript_EnterStation:
	warp MAP_RANGER_STATION_1F, 6, 9
	waitstate
	releaseall
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The ending. After the chamber, the player is home in their room (2F); the friends come up the stairs (10,2) one
@ by one and line up beside the player at (3,3). Then the lab (the institute's Research Room): Tri-Blast.

Act2_EventScript_HomeFriend::
	msgbox Act2_Text_HomeFriend, MSGBOX_NPC
	end

Act2_EventScript_FriendsVisit::
	lockall
	msgbox Act2_Text_MomFriendsHere
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	clearflag FLAG_TEMP_1
	addobject LOCALID_HOME_KAI
	applymovement LOCALID_HOME_KAI, Act2_Movement_HomeKai
	delay 32
	clearflag FLAG_TEMP_2
	addobject LOCALID_HOME_MINA
	applymovement LOCALID_HOME_MINA, Act2_Movement_HomeMina
	delay 32
	clearflag FLAG_TEMP_3
	addobject LOCALID_HOME_ROWAN
	applymovement LOCALID_HOME_ROWAN, Act2_Movement_HomeRowan
	waitmovement LOCALID_HOME_KAI
	waitmovement LOCALID_HOME_MINA
	waitmovement LOCALID_HOME_ROWAN
	msgbox Act2_Text_FriendsVisit
	closemessage
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	msgbox Act2_Text_Epiphany
	closemessage
	fadescreen FADE_TO_BLACK
	warp MAP_CELADON_UNIVERSITY_SCIENCE_RESEARCH_ROOM, 5, 2
	waitstate
	releaseall
	end

@ From the stairs (10,2): down to (10,4), left to (7,4), up to (7,3), then left to their spot, facing the player
Act2_Movement_HomeKai:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
Act2_Movement_HomeMinaEnd:
	walk_left
Act2_Movement_HomeRowanEnd:
	walk_left
	walk_in_place_faster_left
	step_end

Act2_Movement_HomeMina:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_left
	walk_in_place_faster_left
	step_end

Act2_Movement_HomeRowan:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_up
	walk_left
	walk_in_place_faster_left
	step_end

@ The lab: the friends stand behind the player, who's at the machine (5,2) facing the wall
Act2_EventScript_LabObjects::
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED, Act2_EventScript_LabObjectsEnd
	clearflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_4
	setflag FLAG_TEMP_5
	end

Act2_EventScript_LabObjectsEnd:
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_4
	end

Act2_EventScript_TriBlastReveal::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox Act2_Text_LabWorking
	closemessage
	playse SE_PC_ON
	waitse
	delay 30
	playse SE_PC_LOGIN
	waitse
	delay 30
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox Act2_Text_LabDone
	playfanfare MUS_OBTAIN_TMHM
	message Act2_Text_MadeTriBlast
	waitmessage
	waitfanfare
	additem ITEM_TM_TRI_BLAST
	msgbox Act2_Text_FriendsReactTriBlast
	closemessage
	clearflag FLAG_TEMP_2
	addobject LOCALID_LAB_ASHFORD
	applymovement LOCALID_LAB_ASHFORD, Act2_Movement_LabAshford
	delay 32
	clearflag FLAG_TEMP_3
	addobject LOCALID_LAB_MARLOWE
	applymovement LOCALID_LAB_MARLOWE, Act2_Movement_LabMarlowe
	delay 32
	clearflag FLAG_TEMP_4
	addobject LOCALID_LAB_FERN
	applymovement LOCALID_LAB_FERN, Act2_Movement_LabFern
	waitmovement LOCALID_LAB_ASHFORD
	waitmovement LOCALID_LAB_MARLOWE
	waitmovement LOCALID_LAB_FERN
	applymovement LOCALID_LAB_KAI, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_LAB_MINA, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_LAB_ROWAN, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox Act2_Text_ProfessorsMarvel
	closemessage
	fadescreen FADE_TO_BLACK
	special Act2_EndAct2
	waitstate
	releaseall
	end

@ From the door (7,9): up to (7,7), left to their column, up to row 5, behind the friends
Act2_Movement_LabAshford:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_in_place_faster_up
	step_end

Act2_Movement_LabMarlowe:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_up
	walk_up
	walk_in_place_faster_up
	step_end

Act2_Movement_LabFern:
	walk_up
	walk_up
	walk_left
	walk_up
	walk_up
	walk_in_place_faster_up
	step_end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_HomeFriend:
	.string "Whoa, {PLAYER}!$"

Act2_Text_MomFriendsHere:
	.string "MOM: {PLAYER}! Your friends are\n"
	.string "here! Go on up!$"

Act2_Text_FriendsVisit:
	.string "KAI: {PLAYER}! We came all\n"
	.string "the way out here to find you.\p"
	.string "MINA: Did you finish your exam?\n"
	.string "My WAVE CRASH is done!\p"
	.string "ROWAN: You look like you saw\n"
	.string "a ghost. What happened?\p"
	.string "{PLAYER}: …\p"
	.string "FIRE, WATER and GRASS…\n"
	.string "all at the same time…$"

Act2_Text_Epiphany:
	.string "{PLAYER}: …!\p"
	.string "Come back to the lab with me!\p"
	.string "KAI: Huh? Wait up!$"

Act2_Text_LabWorking:
	.string "{PLAYER} worked through the night…$"

Act2_Text_LabDone:
	.string "{PLAYER}: Done!$"

Act2_Text_MadeTriBlast:
	.string "{PLAYER} made TM TRI-BLAST!$"

Act2_Text_FriendsReactTriBlast:
	.string "KAI: TRI-BLAST?\p"
	.string "{PLAYER}: FIRE, WATER and GRASS\n"
	.string "in one move. 120 power.\p"
	.string "MINA: All three at once… So\n"
	.string "anything weak to one of them…\p"
	.string "ROWAN: …takes super-effective\n"
	.string "damage. And nothing resists it!\p"
	.string "KAI: No way. That's amazing!$"

Act2_Text_ProfessorsMarvel:
	.string "PROF. ASHFORD: What's all the\n"
	.string "noise in here? Is that…\p"
	.string "PROF. MARLOWE: FIRE, WATER and\n"
	.string "GRASS in a single move.\l"
	.string "In all my years…\p"
	.string "PROF. FERN: All three courses in\n"
	.string "one TM. You really listened.\p"
	.string "PROF. ASHFORD: {PLAYER}, this\n"
	.string "isn't just a passing grade.\l"
	.string "It's a discovery.\p"
	.string "Congratulations, graduate!$"

@ ---------------------------------------------------------------------------------------------------------------
@ The chamber (RangerStation_B4F): coord events on row 12 in front of the trio (HEATRAN (5,6), SUICUNE (7,6),
@ VIRIZION (9,6)). The player lines up at (7,11); the battle; then, in a flash, MEW (7,9) between them with DAD
@ (6,10) and MOM (8,10).

Act2_EventScript_ChamberScene::
	lockall
Act2_EventScript_ChamberLineUp:
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_lt VAR_0x8004, 7, Act2_EventScript_ChamberStepRight
	goto_if_gt VAR_0x8004, 7, Act2_EventScript_ChamberStepLeft
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_CHAMBER_HEATRAN, Common_Movement_ExclamationMark
	applymovement LOCALID_CHAMBER_SUICUNE, Common_Movement_ExclamationMark
	applymovement LOCALID_CHAMBER_VIRIZION, Common_Movement_ExclamationMark
	waitmovement 0
	playmoncry SPECIES_HEATRAN, CRY_MODE_ENCOUNTER
	waitmoncry
	playmoncry SPECIES_SUICUNE, CRY_MODE_ENCOUNTER
	waitmoncry
	playmoncry SPECIES_VIRIZION, CRY_MODE_ENCOUNTER
	waitmoncry
	msgbox Act2_Text_ChamberTrio
	closemessage
	call Act2_EventScript_LegendsBattle
	@ The blast: a white flash, and MEW, MOM and DAD are there
	fadescreen FADE_TO_WHITE
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	addobject LOCALID_CHAMBER_DAD
	addobject LOCALID_CHAMBER_MEW
	addobject LOCALID_CHAMBER_MOM
	fadescreen FADE_FROM_WHITE
	playmoncry SPECIES_MEW, CRY_MODE_NORMAL
	waitmoncry
	msgbox Act2_Text_MewAbsorbed
	closemessage
	applymovement LOCALID_CHAMBER_HEATRAN, Common_Movement_WalkInPlaceDown
	applymovement LOCALID_CHAMBER_SUICUNE, Common_Movement_WalkInPlaceDown
	applymovement LOCALID_CHAMBER_VIRIZION, Common_Movement_WalkInPlaceDown
	waitmovement 0
	msgbox Act2_Text_TrioCalmed
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_CHAMBER_HEATRAN
	removeobject LOCALID_CHAMBER_SUICUNE
	removeobject LOCALID_CHAMBER_VIRIZION
	fadescreen FADE_FROM_BLACK
	applymovement LOCALID_CHAMBER_DAD, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_CHAMBER_MOM, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_CHAMBER_MEW, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox Act2_Text_ParentsInChamber
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	setvar VAR_ACT2_PROGRESS, ACT2_LEGENDS_CALMED
	warp MAP_PALLET_TOWN_PLAYERS_HOUSE_2F_FRLG_FARM, 3, 3
	waitstate
	releaseall
	end

Act2_EventScript_ChamberStepRight:
	applymovement LOCALID_PLAYER, Common_Movement_WalkRight
	waitmovement 0
	goto Act2_EventScript_ChamberLineUp

Act2_EventScript_ChamberStepLeft:
	applymovement LOCALID_PLAYER, Common_Movement_WalkLeft
	waitmovement 0
	goto Act2_EventScript_ChamberLineUp

Act2_Text_ChamberTrio:
	.string "HEATRAN, SUICUNE and VIRIZION…\n"
	.string "The noise was coming from them!\p"
	.string "They're not happy to see you!$"

Act2_Text_MewAbsorbed:
	.string "A pink POKéMON took the blast!\p"
	.string "MEW absorbed the attack!$"

Act2_Text_TrioCalmed:
	.string "HEATRAN, SUICUNE and VIRIZION\n"
	.string "calmed down…$"

Act2_Text_ParentsInChamber:
	.string "DAD: {PLAYER}! Are you hurt?\p"
	.string "MOM: What were you thinking?!\n"
	.string "Those three guard the WOODS.\l"
	.string "They don't hold back!\p"
	.string "DAD: That was dangerous. If MEW\n"
	.string "hadn't come…\p"
	.string "{PLAYER}: …\p"
	.string "……\p"
	.string "MOM: MEW? Heh. You want to know\n"
	.string "how we know MEW.\p"
	.string "DAD: We met it in these very\n"
	.string "WOODS, when we were young.\p"
	.string "MOM: We befriended it together.\n"
	.string "We never caught it.\p"
	.string "DAD: It comes to visit us when\n"
	.string "we need it. Like today.\p"
	.string "MEW: Mew!\p"
	.string "MOM: Come on, you. Let's go home.$"
