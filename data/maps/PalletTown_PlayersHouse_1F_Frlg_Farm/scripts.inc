@ The farmhouse, where the player grew up. Act 1's story: docs/hack/act1_story.md

PalletTown_PlayersHouse_1F_Frlg_Farm_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Farmhouse1F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Farmhouse1F_OnFrame
	.byte 0

@ Dad is home the morning after Jangmo-o (FLAG_TEMP_1 hides him otherwise)
Farmhouse1F_OnTransition:
	call_if_ne VAR_ACT1_PROGRESS, ACT1_RESTED, Farmhouse1F_EventScript_HideDad
	end

Farmhouse1F_EventScript_HideDad:
	setflag FLAG_TEMP_1
	return

Farmhouse1F_EventScript_Dad::
	msgbox Farmhouse1F_Text_DadShowOak, MSGBOX_NPC
	end

Farmhouse1F_OnFrame:
	map_script_2 VAR_ACT1_WOKE_UP, 1, Farmhouse1F_EventScript_WokeUp
	.2byte 0

@ After blacking out in the Woods (Act1_EventScript_BlackOut): home, next to Mom
Farmhouse1F_EventScript_WokeUp::
	lockall
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Farmhouse1F_Text_MomYoureAwake
	setvar VAR_ACT1_WOKE_UP, 0
	releaseall
	end

Farmhouse1F_EventScript_Mom::
	lock
	faceplayer
	goto_if_ne VAR_STORY_ACT, STORY_ACT_CHILDHOOD, PalletTown_PlayersHouse_1F_EventScript_Mom
	switch VAR_ACT1_PROGRESS
	case ACT1_START, Farmhouse1F_EventScript_MomStart
	case ACT1_GOT_POKEDEX, Farmhouse1F_EventScript_MomGotPokedex
	case ACT1_MOM_SENT_TO_DAD, Farmhouse1F_EventScript_MomDadWaiting
	case ACT1_DAD_SENT_TO_WOODS, Farmhouse1F_EventScript_MomOffToWoods
	case ACT1_BEFRIENDED_BLIPBUG, Farmhouse1F_EventScript_MomDadWantsYou
	case ACT1_DAD_WARNED_JANGMO_O, Farmhouse1F_EventScript_MomStayAway
	case ACT1_BEFRIENDED_THREE, Farmhouse1F_EventScript_MomTellDad
	case ACT1_DAD_SAYS_TOO_DANGEROUS, Farmhouse1F_EventScript_MomFollowYourHeart
	case ACT1_MOM_SAYS_FOLLOW_HEART, Farmhouse1F_EventScript_MomGoTalkToDad
	case ACT1_GOT_HARD_STONE, Farmhouse1F_EventScript_MomBeKind
	case ACT1_RESTED, Farmhouse1F_EventScript_MomShowOak
	msgbox Farmhouse1F_Text_MomKnewYouCould
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomStart::
	msgbox Farmhouse1F_Text_MomSorryPickUpPokedex
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomGotPokedex::
	msgbox Farmhouse1F_Text_MomGoTalkToDad
	setvar VAR_ACT1_PROGRESS, ACT1_MOM_SENT_TO_DAD
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomDadWaiting::
	msgbox Farmhouse1F_Text_MomDadsInTheShed
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomOffToWoods::
	msgbox Farmhouse1F_Text_MomBeCareful
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomDadWantsYou::
	msgbox Farmhouse1F_Text_MomDadWantsToTalkAgain
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomStayAway::
	msgbox Farmhouse1F_Text_MomStayAwayFromJangmoO
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomTellDad::
	msgbox Farmhouse1F_Text_MomGoTellDad
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomFollowYourHeart::
	msgbox Farmhouse1F_Text_MomFollowYourHeart
	setvar VAR_ACT1_PROGRESS, ACT1_MOM_SAYS_FOLLOW_HEART
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomGoTalkToDad::
	msgbox Farmhouse1F_Text_MomGoOnTalkToDad
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomShowOak::
	msgbox Farmhouse1F_Text_MomShowOak
	goto Farmhouse1F_EventScript_MomEnd

Farmhouse1F_EventScript_MomBeKind::
	msgbox Farmhouse1F_Text_MomBeBraveAndKind
Farmhouse1F_EventScript_MomEnd::
	closemessage
	applymovement LOCALID_PLAYERS_HOUSE_1F_MOM, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

Farmhouse1F_Text_MomSorryPickUpPokedex::
	.string "MOM: Oh, {PLAYER}…\p"
	.string "I'm so sorry PROF. OAK didn't pick\n"
	.string "you to become a TRAINER.\p"
	.string "But he said he left a POKéDEX for\n"
	.string "you at his LAB.\p"
	.string "Why don't you go pick it up?\n"
	.string "PALLET TOWN is just east of here.$"

Farmhouse1F_Text_MomGoTalkToDad::
	.string "MOM: Oh! Is that the POKéDEX?\n"
	.string "How exciting!\p"
	.string "Why don't you go show Dad?\n"
	.string "He's out in the storage shed.$"

Farmhouse1F_Text_MomDadsInTheShed::
	.string "MOM: Dad's out in the storage shed.\n"
	.string "Go on, he's waiting for you.$"

Farmhouse1F_Text_MomBeCareful::
	.string "MOM: Off to the woods? Be careful,\n"
	.string "and be home for dinner!$"

Farmhouse1F_Text_MomDadWantsToTalkAgain::
	.string "MOM: Your new friend is adorable!\p"
	.string "Oh, Dad wants to talk to you again.\n"
	.string "He's in the storage shed.$"

Farmhouse1F_Text_MomStayAwayFromJangmoO::
	.string "MOM: Remember what Dad said, and\n"
	.string "stay away from that JANGMO-O!\p"
	.string "Oh, and if you want to make more\n"
	.string "friends in the woods…\p"
	.string "The folks in town might know a thing\n"
	.string "or two about the POKéMON there.\p"
	.string "Why not ask around PALLET TOWN?$"

Farmhouse1F_Text_MomGoTellDad::
	.string "MOM: Look at all your new friends!\p"
	.string "Go show Dad. He'll be so proud.$"

Farmhouse1F_Text_MomFollowYourHeart::
	.string "MOM: Dad told you to stay away from\n"
	.string "JANGMO-O, didn't he?\p"
	.string "He's scared for you, {PLAYER}.\n"
	.string "That's just how dads are.\p"
	.string "But you should follow your heart.\p"
	.string "Your kindness has won over every\n"
	.string "POKéMON you've met so far.\p"
	.string "Maybe it can break through to\n"
	.string "JANGMO-O, too.\p"
	.string "Go talk to Dad again.$"

Farmhouse1F_Text_MomGoOnTalkToDad::
	.string "MOM: Go on, talk to Dad.$"

Farmhouse1F_Text_MomBeBraveAndKind::
	.string "MOM: Be brave, and be kind.\n"
	.string "I know you can do it.$"

Farmhouse1F_Text_MomKnewYouCould::
	.string "MOM: You befriended JANGMO-O?\n"
	.string "I knew you could!$"

Farmhouse1F_Text_MomYoureAwake::
	.string "MOM: {PLAYER}! You're awake!\p"
	.string "Dad found you in the woods and\n"
	.string "carried you all the way home.\p"
	.string "Please be more careful!$"

Farmhouse1F_Text_MomShowOak:
	.string "MOM: Good morning, sleepyhead!\p"
	.string "We still can't believe it. JANGMO-O,\n"
	.string "of all POKéMON!\p"
	.string "You should show your POKéDEX to\n"
	.string "PROF. OAK. He'll want to hear\l"
	.string "about this!$"

Farmhouse1F_Text_DadShowOak:
	.string "DAD: Morning, kiddo!\p"
	.string "Hey, why don't you show PROF. OAK\n"
	.string "your POKéDEX?\p"
	.string "I bet he's never heard of anyone\n"
	.string "calming down a JANGMO-O!$"
