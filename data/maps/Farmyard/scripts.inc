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
	call_if_ge VAR_STORY_ACT, STORY_ACT_UNIVERSITY, Farmyard_EventScript_FamilyOut
	end

@ Acts 2 and 3: the Pokémon the player grew up with are out in the farmyard the whole time they're home. They never
@ leave the party, so nothing has to put them back - walking off the map is enough.
@ Special_ShowPartyFarmFamily also puts the follower away for the length of the visit - see src/farmyard.c.
Farmyard_EventScript_FamilyOut:
	special Special_ShowPartyFarmFamily
	goto_if_eq VAR_RESULT, 0, Farmyard_EventScript_FamilyOutEnd
	@ Arriving by warp (the farmhouse, the barn, the Woods) they're already out playing as the player steps in, so
	@ there's nothing to announce at the Woods gate. Coming from PALLET TOWN they run out at the gate, which is the
	@ moment the greeting is for: the connection puts the player on the right edge (x 23).
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ge VAR_0x8004, 22, Farmyard_EventScript_FamilyOutEnd
	setvar VAR_TEMP_8, 1
Farmyard_EventScript_FamilyOutEnd:
	return

@ Acts 2 and 3: the farmhouse and the barn are home too, and the player's Pokémon are out in the yard, so none
@ follows them indoors either (FLAG_TEMP_HIDE_FOLLOWER clears itself on the next map). Called from their
@ ON_TRANSITION scripts.
Farmyard_EventScript_HomeNoFollower::
	goto_if_lt VAR_STORY_ACT, STORY_ACT_UNIVERSITY, Farmyard_EventScript_HomeNoFollowerEnd
	setflag FLAG_TEMP_HIDE_FOLLOWER
Farmyard_EventScript_HomeNoFollowerEnd:
	return

@ While the player chooses their two friends, Mom, Dad and Oak wait in the farmyard
Farmyard_EventScript_EveryoneWaits:
	clearflag FLAG_TEMP_A
	clearflag FLAG_TEMP_B
	clearflag FLAG_TEMP_C
	return

@ The gates, across each entrance: the following Pokémon is dropped off and starts wandering the farmyard
Farmyard_EventScript_Gate::
	goto_if_ge VAR_STORY_ACT, STORY_ACT_UNIVERSITY, Farmyard_EventScript_FamilyGate
	lockall
	special Special_DepositFollowerInFarm
	copyvar VAR_0x8004, VAR_RESULT
	goto_if_eq VAR_0x8004, FARM_DEPOSIT_NONE, Farmyard_EventScript_GateEnd
	msgbox Farmyard_Text_RanOffToPlay, MSGBOX_DEFAULT
	call_if_eq VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, Farmyard_EventScript_CheckWithDad
	goto_if_eq VAR_0x8004, 0, Farmyard_EventScript_GateShown
	addobject VAR_0x8004
Farmyard_EventScript_GateShown:
	goto_if_eq VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_JANGMO_O, Farmyard_EventScript_JangmoOHome
Farmyard_EventScript_GateEnd:
	releaseall
	end

@ Acts 2 and 3, at either gate: the player's own Pokémon run out as they come home and are back at their side as
@ they go. Which gate decides which way they're walking - PALLET TOWN is to the right of the farmyard, the Woods to
@ the left - and VAR_TEMP_8 keeps the greeting to once a visit.
Farmyard_EventScript_FamilyGate::
	lockall
	@ Walking in from PALLET TOWN crosses a map connection, which keeps the Pokemon that was following, so it's still
	@ at the player's heel here. Return it to its ball where they can see it, before the rest run out to meet them.
	hidefollower
	specialvar VAR_RESULT, Special_ShowPartyFarmFamily
	goto_if_eq VAR_RESULT, 0, Farmyard_EventScript_GateEnd
	getplayerxy VAR_0x8004, VAR_0x8005
	copyvar VAR_0x8006, VAR_RESULT
	goto_if_eq VAR_0x8004, 1, Farmyard_EventScript_FamilyWoodsGate
	goto_if_eq VAR_FACING, DIR_EAST, Farmyard_EventScript_FamilyComesBack
	goto Farmyard_EventScript_FamilyRanOut

Farmyard_EventScript_FamilyWoodsGate:
	goto_if_eq VAR_FACING, DIR_WEST, Farmyard_EventScript_FamilyComesBack
	goto Farmyard_EventScript_FamilyRanOut

Farmyard_EventScript_FamilyRanOut:
	goto_if_eq VAR_TEMP_8, 1, Farmyard_EventScript_GateEnd
	setvar VAR_TEMP_8, 1
	goto_if_ge VAR_0x8006, 2, Farmyard_EventScript_FamilyRanOutTogether
	msgbox Farmyard_Text_RanOffToPlay, MSGBOX_DEFAULT
	goto Farmyard_EventScript_GateEnd

Farmyard_EventScript_FamilyRanOutTogether:
	msgbox Farmyard_Text_FamilyRanOut, MSGBOX_DEFAULT
	goto Farmyard_EventScript_GateEnd

Farmyard_EventScript_FamilyComesBack::
	setvar VAR_TEMP_8, 0
	goto_if_ge VAR_0x8006, 2, Farmyard_EventScript_FamilyComesBackTogether
	msgbox Farmyard_Text_FamilyComesBack, MSGBOX_DEFAULT
	goto Farmyard_EventScript_GateEnd

Farmyard_EventScript_FamilyComesBackTogether:
	msgbox Farmyard_Text_FamilyComesBackTogether, MSGBOX_DEFAULT
	goto Farmyard_EventScript_GateEnd

@ The Woods are reached by warp, not a map connection: the Woods use another tileset, which the game would draw
@ with the farmyard's tiles (garbled). Pallet Town's side is a map connection.
Farmyard_EventScript_ToWoods::
	goto_if_ge VAR_STORY_ACT, STORY_ACT_UNIVERSITY, Act2_EventScript_FarmyardToWoods
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_DAD_SENT_TO_WOODS, Farmyard_EventScript_WoodsNotYet
	goto_if_ge VAR_ACT1_PROGRESS, ACT1_RESTED, Farmyard_EventScript_WoodsShowOak
	goto_if_ge VAR_ACT1_PROGRESS, ACT1_GOT_HARD_STONE, Farmyard_EventScript_EnterWoods
	goto_if_ge VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_THREE, Farmyard_EventScript_WoodsTalkToParents
	goto_if_eq VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, Farmyard_EventScript_WoodsCheckWithDad
Farmyard_EventScript_EnterWoods:
	warp MAP_WOODS, 49, 58
	waitstate
	end

@ Act 1: the Woods open once Dad has sent the player there (docs/hack/act1_story.md)
Farmyard_EventScript_WoodsNotYet::
	lockall
	switch VAR_ACT1_PROGRESS
	case ACT1_START, Farmyard_EventScript_WoodsNotYetPokedex
	msgbox Farmyard_Text_WoodsNotYetDad
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsNotYetPokedex::
	msgbox Farmyard_Text_WoodsNotYetPokedex
	goto Farmyard_EventScript_WoodsTurnBack

@ After the other three: the Woods wait until Dad and Mom have talked about Jangmo-o and Dad has given the Hard Stone
Farmyard_EventScript_WoodsTalkToParents::
	lockall
	switch VAR_ACT1_PROGRESS
	case ACT1_BEFRIENDED_THREE, Farmyard_EventScript_WoodsTellDad
	case ACT1_DAD_SAYS_TOO_DANGEROUS, Farmyard_EventScript_WoodsSeeMom
	msgbox Farmyard_Text_WoodsSeeDadAgain
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsTellDad::
	msgbox Farmyard_Text_WoodsTellDad
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsSeeMom::
	msgbox Farmyard_Text_WoodsSeeMom
	goto Farmyard_EventScript_WoodsTurnBack

@ After Blipbug: the Woods wait until Dad has had his say (the Jangmo-o warning)
Farmyard_EventScript_CheckWithDad:
	msgbox Farmyard_Text_CheckWithDad
	return

Farmyard_EventScript_WoodsCheckWithDad::
	lockall
	msgbox Farmyard_Text_CheckWithDad
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsShowOak::
	lockall
	msgbox Farmyard_Text_ShowPokedexToOak
	goto Farmyard_EventScript_WoodsTurnBack

Farmyard_EventScript_WoodsTurnBack::
	closemessage
	applymovement LOCALID_PLAYER, Farmyard_Movement_StepBack
	waitmovement 0
	releaseall
	end

Farmyard_Movement_StepBack:
	walk_right
	step_end

@ The farm's own POKéMON, there from the start: they can't be befriended, but they'll call back. Miltank and Mareep
@ live in the barn: more different POKéMON than this on the map run out of sprite memory.
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
	goto_if_ge VAR_STORY_ACT, STORY_ACT_UNIVERSITY, Farmyard_EventScript_TakeAlong
Farmyard_EventScript_FarmMonHappy:
	msgbox Farmyard_Text_LooksHappy, MSGBOX_DEFAULT
	release
	end

@ Acts 2 and 3: the ones left behind at the end of Act 1 are still here, and the player can take one along now
@ (GetFarmMonToTake / TakeFarmMonIntoParty, src/farmyard.c). Shown like the end of Act 1's choice.
Farmyard_EventScript_TakeAlong:
	specialvar VAR_RESULT, Special_GetFarmMonToTake
	goto_if_eq VAR_RESULT, FALSE, Farmyard_EventScript_FarmMonHappy
	goto_if_eq VAR_RESULT, FARM_TAKE_PARTY_FULL, Farmyard_EventScript_TakeAlongTeamFull
	showmonpic VAR_0x8004, 10, 3
	msgbox Farmyard_Text_TakeAlong, MSGBOX_YESNO
	hidemonpic
	goto_if_eq VAR_RESULT, NO, Farmyard_EventScript_TakeAlongStays
	specialvar VAR_RESULT, Special_TakeFarmMonIntoParty
	goto_if_eq VAR_RESULT, FALSE, Farmyard_EventScript_TakeAlongTeamFull
	removeobject VAR_LAST_TALKED
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Farmyard_Text_TakeAlongJoined
	waitmessage
	waitfanfare
	msgbox Farmyard_Text_TakeAlongLookAfterIt, MSGBOX_DEFAULT
	release
	end

Farmyard_EventScript_TakeAlongStays:
	msgbox Farmyard_Text_TakeAlongStays, MSGBOX_DEFAULT
	release
	end

Farmyard_EventScript_TakeAlongTeamFull:
	msgbox Farmyard_Text_TakeAlongTeamFull, MSGBOX_DEFAULT
	release
	end

@ ---------------------------------------------------------------------------------------------------------------------
@ The end of Act 1 (docs/hack/act1_story.md)

@ Fades that come back to the same map use fadescreenswapbuffers: a plain fadescreen there re-applies the time-of-day
@ tint on the way back in, so the scene looked like another time of day.

@ Coming home with Jangmo-o: Mom and Dad can't believe it, then send the player to bed
Farmyard_EventScript_JangmoOHome:
	fadescreenswapbuffers FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_MOM
	setvar VAR_0x8005, LOCALID_FARMYARD_DAD
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_MOM
	addobject LOCALID_FARMYARD_DAD
	applymovement LOCALID_FARMYARD_MOM, Farmyard_Movement_FacePlayer
	applymovement LOCALID_FARMYARD_DAD, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreenswapbuffers FADE_FROM_BLACK
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
	fadescreenswapbuffers FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_MOM
	setvar VAR_0x8005, LOCALID_FARMYARD_DAD
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_MOM
	addobject LOCALID_FARMYARD_DAD
	applymovement LOCALID_FARMYARD_MOM, Farmyard_Movement_FacePlayer
	applymovement LOCALID_FARMYARD_DAD, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreenswapbuffers FADE_FROM_BLACK
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
	fadescreenswapbuffers FADE_TO_BLACK
	setvar VAR_0x8004, LOCALID_FARMYARD_OAK
	setvar VAR_0x8005, 0
	special Act1_PlaceNearPlayer
	addobject LOCALID_FARMYARD_OAK
	applymovement LOCALID_FARMYARD_OAK, Farmyard_Movement_FacePlayer
	waitmovement 0
	fadescreenswapbuffers FADE_FROM_BLACK
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

Farmyard_Text_FamilyRanOut:
	.string "{STR_VAR_1} and the others ran\n"
	.string "off to play in the farmyard!$"

Farmyard_Text_FamilyComesBack:
	.string "{STR_VAR_1} came running back\n"
	.string "to your side.$"

Farmyard_Text_TakeAlong:
	.string "{STR_VAR_1} looks up at you,\n"
	.string "hoping.\p"
	.string "Take {STR_VAR_1} along with you?$"

Farmyard_Text_TakeAlongJoined:
	.string "{STR_VAR_1} joined your team!$"

Farmyard_Text_TakeAlongLookAfterIt:
	.string "It's a long way from the farm.\n"
	.string "Look after it out there!$"

Farmyard_Text_TakeAlongStays:
	.string "{STR_VAR_1} goes back to playing\n"
	.string "in the farmyard.$"

Farmyard_Text_TakeAlongTeamFull:
	.string "Your team is full, so {STR_VAR_1}\n"
	.string "has to stay on the farm for now.$"

Farmyard_Text_FamilyComesBackTogether:
	.string "{STR_VAR_1} and the others came\n"
	.string "running back to your side.$"

Farmyard_Text_LooksHappy:
	.string "It looks happy here.$"

Farmyard_Text_Sign:
	.string "Quincy Farm$"

Farmyard_Text_WoodsNotYetPokedex:
	.string "I should pick up the POKéDEX from\n"
	.string "PROF. OAK's LAB first.$"

Farmyard_Text_WoodsNotYetDad:
	.string "I should go see Dad in the storage\n"
	.string "shed first.$"

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
	.string "I think you have a future studying\n"
	.string "POKéMON behavior.\p"
	.string "I used to teach at CELADON, you\n"
	.string "know. I can get you all set up!$"

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
	.string "OAK: Splendid! Oh, one more thing:\n"
	.string "you should choose two POKéMON to\l"
	.string "come with you.\p"
	.string "…\p"
	.string "What, you expected me to say you\n"
	.string "can only choose one?\p"
	.string "It would get lonely while you're in\n"
	.string "class. Go grab two friends!\p"
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

Farmyard_Text_WoodsTellDad:
	.string "I should go tell Dad about all my\n"
	.string "new friends first.$"

Farmyard_Text_WoodsSeeMom:
	.string "Dad said Mom wanted to see me.$"

Farmyard_Text_WoodsSeeDadAgain:
	.string "Mom said to go talk to Dad again.$"

Farmyard_Text_CheckWithDad:
	.string "I should talk to Dad to make sure\n"
	.string "it's OK.$"
