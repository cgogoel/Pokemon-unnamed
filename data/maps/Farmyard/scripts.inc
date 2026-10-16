@ Act 1 farmyard (docs/hack/befriending.md, docs/hack/act1_story.md).
@ Its first FARM_SHOWN_COUNT objects are the farm's Pokémon (graphics VAR_OBJ_GFX_ID_0.., hidden by FLAG_TEMP_1..).
@ Mom, Dad and Oak (FLAG_TEMP_A..C) only show up for the end of Act 1.

Farmyard_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Farmyard_OnTransition
	.byte 0

Farmyard_OnTransition:
	special SetupFarmyardMons
	setflag FLAG_TEMP_A
	setflag FLAG_TEMP_B
	setflag FLAG_TEMP_C
	call_if_eq VAR_ACT1_PROGRESS, ACT1_CHOOSING_FRIENDS, Farmyard_EventScript_EveryoneWaits
	end

@ While the player chooses their two friends, Mom, Dad and Oak wait in the farmyard
Farmyard_EventScript_EveryoneWaits:
	clearflag FLAG_TEMP_A
	clearflag FLAG_TEMP_B
	clearflag FLAG_TEMP_C
	return

@ The gates, across each entrance: the following Pokémon is dropped off and starts wandering the farmyard
Farmyard_EventScript_Gate::
	lockall
	special Special_DepositFollowerInFarm
	copyvar VAR_0x8004, VAR_RESULT
	goto_if_eq VAR_0x8004, FARM_DEPOSIT_NONE, Farmyard_EventScript_GateEnd
	msgbox Farmyard_Text_RanOffToPlay, MSGBOX_DEFAULT
	goto_if_eq VAR_0x8004, 0, Farmyard_EventScript_GateShown
	addobject VAR_0x8004
Farmyard_EventScript_GateShown:
	goto_if_eq VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_JANGMO_O, Farmyard_EventScript_JangmoOHome
Farmyard_EventScript_GateEnd:
	releaseall
	end

@ The Woods are reached by warp, not a map connection: the Woods use another tileset, which the game would draw
@ with the farmyard's tiles (garbled). Pallet Town's side is a map connection.
Farmyard_EventScript_ToWoods::
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_SENT_TO_WOODS, Farmyard_EventScript_WoodsNotYet
	goto_if_ge VAR_ACT1_PROGRESS, ACT1_RESTED, Farmyard_EventScript_WoodsShowOak
	warp MAP_WOODS, 49, 58
	waitstate
	end

@ Act 1: the Woods open once Dad has sent the player there (docs/hack/act1_story.md)
Farmyard_EventScript_WoodsNotYet::
	lockall
	switch VAR_ACT1_PROGRESS
	case ACT1_START, Farmyard_EventScript_WoodsNotYetPokedex
	case ACT1_GOT_POKEDEX, Farmyard_EventScript_WoodsNotYetMom
	msgbox Farmyard_Text_WoodsNotYetDad
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsNotYetPokedex::
	msgbox Farmyard_Text_WoodsNotYetPokedex
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsShowOak::
	lockall
	msgbox Farmyard_Text_ShowPokedexToOak
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsNotYetMom::
	msgbox Farmyard_Text_WoodsNotYetMom
Farmyard_EventScript_WoodsTurnBack::
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepBack
	waitmovement 0
	releaseall
	end

Farmyard_Movement_StepBack:
	walk_right
	step_end

@ The farm's own POKéMON, there from the start: they can't be befriended, but they'll call back
Farmyard_EventScript_Miltank::
	lock
	faceplayer
	playmoncry SPECIES_MILTANK, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Miltank
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Mareep::
	lock
	faceplayer
	playmoncry SPECIES_MAREEP, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Mareep
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Tauros::
	lock
	faceplayer
	playmoncry SPECIES_TAUROS, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Tauros
	goto Farmyard_EventScript_FarmAnimalEnd

Farmyard_EventScript_Doduo::
	lock
	faceplayer
	playmoncry SPECIES_DODUO, CRY_MODE_NORMAL
	msgbox Farmyard_Text_Doduo
Farmyard_EventScript_FarmAnimalEnd::
	waitmoncry
	release
	end

Farmyard_EventScript_Sign::
	msgbox Farmyard_Text_Sign, MSGBOX_SIGN
	end

Farmyard_EventScript_FarmMon::
	lock
	faceplayer
	goto_if_eq VAR_ACT1_PROGRESS, ACT1_CHOOSING_FRIENDS, Farmyard_EventScript_ChooseFriend
Farmyard_EventScript_FarmMonHappy:
	msgbox Farmyard_Text_LooksHappy, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------------------------------------------------------------
@ The end of Act 1 (docs/hack/act1_story.md)

@ Coming home with Jangmo-o: Mom and Dad can't believe it, then send the player to bed
Farmyard_EventScript_JangmoOHome:
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_MOM
	setvar VAR_0x8005, LOCALID_FARMYARD_DAD
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_MOM
	addobject LOCALID_FARMYARD_DAD
	applymovement LOCALID_FARMYARD_MOM, Farmyard_Movement_FacePlayer
	applymovement LOCALID_FARMYARD_DAD, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreen FADE_FROM_BLACK
	msgbox Farmyard_Text_MomWasThatJangmoO
	msgbox Farmyard_Text_DadItFollowedYouHome
	msgbox Farmyard_Text_MomSpecialUnderstanding
	msgbox Farmyard_Text_DadGoRest
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	setvar VAR_ACT1_PROGRESS, ACT1_RESTED
	setvar VAR_ACT1_WOKE_UP, 2
	warpsilent MAP_PALLET_TOWN_PLAYERS_HOUSE_2F_FRLG_FARM, 3, 3
	waitstate
	end

@ Leaving the farmyard after a night's rest: Oak arrives with news
Farmyard_EventScript_OakArrives::
	lockall
	special Act1_PlaceOakForArrival
	addobject LOCALID_FARMYARD_OAK
	applymovement LOCALID_FARMYARD_OAK, Farmyard_Movement_OakWalksIn
	waitmovement 0
	applymovement LOCALID_FARMYARD_OAK, Common_Movement_ExclamationMark
	applymovement LOCALID_PLAYER, Common_Movement_FaceRight
	waitmovement 0
	msgbox Farmyard_Text_OakHeardRumors
	msgbox Farmyard_Text_PlayerExplains
	msgbox Farmyard_Text_OakCeladonUniversity
	closemessage
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_MOM
	setvar VAR_0x8005, LOCALID_FARMYARD_DAD
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_MOM
	addobject LOCALID_FARMYARD_DAD
	applymovement LOCALID_FARMYARD_MOM, Farmyard_Movement_FacePlayer
	applymovement LOCALID_FARMYARD_DAD, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreen FADE_FROM_BLACK
	msgbox Farmyard_Text_ParentsSurprised
Farmyard_EventScript_AskReallyWant:
	msgbox Farmyard_Text_MomReallyWant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Farmyard_EventScript_WantsToGo
	msgbox Farmyard_Text_DadTakeYourTime
	goto Farmyard_EventScript_AskReallyWant

Farmyard_EventScript_WantsToGo:
	msgbox Farmyard_Text_MomBehindYou
	msgbox Farmyard_Text_OakBringTwoFriends
	giveitem ITEM_POKE_BALL, 2
	msgbox Farmyard_Text_OakGoChoose
	setvar VAR_ACT1_PROGRESS, ACT1_CHOOSING_FRIENDS
	setvar VAR_ACT1_CHOSEN_FRIENDS, 0
	releaseall
	end

@ Choosing: like picking a starter, a look at the Pokémon and "are you sure?"
Farmyard_EventScript_ChooseFriend:
	special Act1_GetFarmMonForChoice
	goto_if_eq VAR_RESULT, FALSE, Farmyard_EventScript_FarmMonHappy
	showmonpic VAR_0x8004, 10, 3
	msgbox Farmyard_Text_BringWithYou, MSGBOX_YESNO
	hidemonpic
	goto_if_eq VAR_RESULT, NO, Farmyard_EventScript_ChooseFriendEnd
	special Act1_TakeFarmMonWithYou
	copyvar VAR_0x8006, VAR_RESULT
	removeobject VAR_LAST_TALKED
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Farmyard_Text_PutInPokeBall
	waitmessage
	waitfanfare
	goto_if_ge VAR_0x8006, 2, Farmyard_EventScript_PrologueEnds
	msgbox Farmyard_Text_ChooseOneMore
Farmyard_EventScript_ChooseFriendEnd:
	release
	end

@ Both chosen: Oak's last words, then the prologue ends (src/prologue_end.c)
Farmyard_EventScript_PrologueEnds:
	closemessage
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_OAK
	setvar VAR_0x8005, 0
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_OAK
	applymovement LOCALID_FARMYARD_OAK, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreen FADE_FROM_BLACK
	msgbox Farmyard_Text_OakNewPhase
	closemessage
	special Act1_FinishChoosing
	fadescreen FADE_TO_BLACK
	special Act1_StartPrologueEnd
	waitstate
	end

@ While choosing: stay in the farmyard (the exits and the doors turn the player back)
Farmyard_EventScript_StayChoosingLeft::
	lockall
	msgbox Farmyard_Text_ChooseFriendsFirst
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepBack
	waitmovement 0
	releaseall
	end

Farmyard_EventScript_StayChoosingRight::
	lockall
	msgbox Farmyard_Text_ChooseFriendsFirst
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepLeft
	waitmovement 0
	releaseall
	end

Farmyard_EventScript_StayChoosingDoor::
	lockall
	msgbox Farmyard_Text_ChooseFriendsFirst
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepDown
	waitmovement 0
	releaseall
	end

Farmyard_EventScript_MomWaiting::
	msgbox Farmyard_Text_MomChooseCarefully, MSGBOX_NPC
	end

Farmyard_EventScript_DadWaiting::
	msgbox Farmyard_Text_DadChooseCarefully, MSGBOX_NPC
	end

Farmyard_EventScript_OakWaiting::
	msgbox Farmyard_Text_OakGoChoose, MSGBOX_NPC
	end

Farmyard_Movement_FacePlayer:
	face_player
	step_end

Farmyard_Movement_OakWalksIn:
	walk_left
	step_end

Farmyard_Movement_StepLeft:
	walk_left
	step_end

Farmyard_Movement_StepDown:
	walk_down
	step_end

Farmyard_Text_RanOffToPlay:
	.string "{STR_VAR_1} ran off to play\n"
	.string "in the farmyard!$"

Farmyard_Text_LooksHappy:
	.string "It looks happy here.$"

Farmyard_Text_Sign:
	.string "Quincy Farm$"

Farmyard_Text_WoodsNotYetPokedex:
	.string "I should pick up the POKéDEX from\n"
	.string "PROF. OAK's LAB first.$"

Farmyard_Text_WoodsNotYetMom:
	.string "I should show Mom the POKéDEX\n"
	.string "first.$"

Farmyard_Text_WoodsNotYetDad:
	.string "Mom said Dad wanted to see me in\n"
	.string "the storage shed.$"

Farmyard_Text_Miltank:
	.string "MILTANK: Mooo!$"

Farmyard_Text_Mareep:
	.string "MAREEP: Baaah!$"

Farmyard_Text_Tauros:
	.string "TAUROS snorted and stamped\n"
	.string "its hooves!$"

Farmyard_Text_Doduo:
	.string "Both of DODUO's heads squawked\n"
	.string "at once!$"

Farmyard_Text_ShowPokedexToOak:
	.string "I should show my POKéDEX to\n"
	.string "PROF. OAK first.$"

Farmyard_Text_MomWasThatJangmoO:
	.string "MOM: {PLAYER}! Was that…\n"
	.string "JANGMO-O?!$"

Farmyard_Text_DadItFollowedYouHome:
	.string "DAD: It followed you all the way\n"
	.string "home… calm as can be.\p"
	.string "I've never seen anything like it.$"

Farmyard_Text_MomSpecialUnderstanding:
	.string "MOM: You really understand POKéMON,\n"
	.string "don't you? How they feel, how they\l"
	.string "behave…\p"
	.string "That's something special, {PLAYER}.$"

Farmyard_Text_DadGoRest:
	.string "DAD: Look at you, though. You're\n"
	.string "exhausted.\p"
	.string "Go on up to bed and get some rest,\n"
	.string "kiddo.$"

Farmyard_Text_OakHeardRumors:
	.string "OAK: Ah, {PLAYER}! Just the person\n"
	.string "I was looking for!\p"
	.string "I've heard rumors that the JANGMO-O\n"
	.string "in the woods has been pacified…\p"
	.string "…and that you had something to do\n"
	.string "with it!\p"
	.string "Tell me, how did you do it?\n"
	.string "Did you battle it?$"

Farmyard_Text_PlayerExplains:
	.string "{PLAYER} told PROF. OAK all about\n"
	.string "befriending JANGMO-O.$"

Farmyard_Text_OakCeladonUniversity:
	.string "OAK: You didn't battle it at all?\n"
	.string "You befriended it?!\p"
	.string "Remarkable… Truly remarkable!\p"
	.string "{PLAYER}, you may not be ready to be\n"
	.string "a TRAINER…\p"
	.string "…but you should be admitted to\n"
	.string "CELADON UNIVERSITY!\p"
	.string "As a distinguished youth scholar in\n"
	.string "POKéMON behavior!\p"
	.string "I used to teach at CELADON, you\n"
	.string "know. I'll get you all set up!$"

Farmyard_Text_ParentsSurprised:
	.string "MOM: CELADON UNIVERSITY?!\p"
	.string "DAD: Our kid, a university\n"
	.string "scholar?$"

Farmyard_Text_MomReallyWant:
	.string "MOM: {PLAYER}, is this really what\n"
	.string "you want?$"

Farmyard_Text_DadTakeYourTime:
	.string "DAD: Take your time, kiddo.\n"
	.string "It's a big decision.$"

Farmyard_Text_MomBehindYou:
	.string "MOM: Then we're behind you all the\n"
	.string "way!$"

Farmyard_Text_OakBringTwoFriends:
	.string "OAK: Splendid! Oh, one more thing.\p"
	.string "You should bring two of the friends\n"
	.string "you've made along with you.\p"
	.string "Just one would get lonely while\n"
	.string "you're in class!\p"
	.string "To travel with them, you'll need\n"
	.string "POKé BALLS.\p"
	.string "Here, take two POKé BALLS to bring\n"
	.string "your friends in.$"

Farmyard_Text_OakGoChoose:
	.string "OAK: Now go and choose the two\n"
	.string "friends you'll bring along!$"

Farmyard_Text_BringWithYou:
	.string "Are you sure you want to bring\n"
	.string "{STR_VAR_1} with you?$"

Farmyard_Text_PutInPokeBall:
	.string "{PLAYER} brought {STR_VAR_1} into\n"
	.string "a POKé BALL!$"

Farmyard_Text_ChooseOneMore:
	.string "One more friend to choose!$"

Farmyard_Text_OakNewPhase:
	.string "OAK: So you've chosen your friends!\n"
	.string "Excellent.\p"
	.string "{PLAYER}, a whole new chapter of your\n"
	.string "life is about to begin.\p"
	.string "At CELADON UNIVERSITY you'll learn\n"
	.string "more than you can imagine.\p"
	.string "Make the most of every opportunity!\p"
	.string "And who knows… The students there\n"
	.string "love to battle in their free time.\p"
	.string "Maybe you'll even start to master\n"
	.string "battling yourself! Ho ho!$"

Farmyard_Text_ChooseFriendsFirst:
	.string "I should choose the two friends\n"
	.string "I'm bringing with me first.$"

Farmyard_Text_MomChooseCarefully:
	.string "MOM: Choose carefully, sweetie.\n"
	.string "We'll take good care of the rest.$"

Farmyard_Text_DadChooseCarefully:
	.string "DAD: The farm will be quiet without\n"
	.string "you, kiddo.$"
