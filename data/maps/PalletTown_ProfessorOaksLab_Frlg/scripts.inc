.equ PLAYER_STARTER_NUM,      VAR_TEMP_1
.equ PLAYER_STARTER_SPECIES,  VAR_TEMP_2
.equ RIVAL_STARTER_SPECIES,   VAR_TEMP_3
.equ RIVAL_STARTER_ID,        VAR_TEMP_4

.equ SHOWED_OAK_COMPLETE_DEX, FLAG_TEMP_2

@ The base game's Oak scenes (starter choice, rival battle, Pokédex, National Dex) are stripped: this hack tells its
@ own story (docs/hack/act1_story.md). So are Oak and BLUE, who were only here for them.
PalletTown_ProfessorOaksLab_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_ProfessorOaksLab_OnTransition
	.byte 0

PalletTown_ProfessorOaksLab_OnTransition::
	setflag FLAG_VISITED_OAKS_LAB
	end

PalletTown_ProfessorOaksLab_EventScript_BulbasaurBall::
	msgbox PalletTown_ProfessorOaksLab_Text_StarterBallsReserved, MSGBOX_SIGN
	end

PalletTown_ProfessorOaksLab_EventScript_SquirtleBall::
	msgbox PalletTown_ProfessorOaksLab_Text_StarterBallsReserved, MSGBOX_SIGN
	end

PalletTown_ProfessorOaksLab_EventScript_CharmanderBall::
	msgbox PalletTown_ProfessorOaksLab_Text_StarterBallsReserved, MSGBOX_SIGN
	end

PalletTown_ProfessorOaksLab_EventScript_Aide1::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, PalletTown_ProfessorOaksLab_EventScript_Aide1GameClear
	goto_if_eq VAR_STORY_ACT, STORY_ACT_CHILDHOOD, PalletTown_ProfessorOaksLab_EventScript_Aide1Act1
	msgbox PalletTown_ProfessorOaksLab_Text_StudyAsOaksAide
	release
	end

@ Act 1: the POKéDEX PROF. OAK promised in the intro, then some advice (docs/hack/act1_story.md)
PalletTown_ProfessorOaksLab_EventScript_Aide1Act1::
	goto_if_set FLAG_SYS_POKEDEX_GET, PalletTown_ProfessorOaksLab_EventScript_Aide1OtherWays
	msgbox PalletTown_ProfessorOaksLab_Text_Aide1OakLeftPokedex
	textcolor NPC_TEXT_COLOR_NEUTRAL
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message PalletTown_ProfessorOaksLab_Text_ReceivedPokedexFromAide
	waitmessage
	waitfanfare
	call EventScript_RestorePrevTextColor
	setflag FLAG_SYS_POKEDEX_GET
	special SetUnlockedPokedexFlags
	call_if_lt VAR_ACT1_PROGRESS, ACT1_GOT_POKEDEX, PalletTown_ProfessorOaksLab_EventScript_SetGotPokedex
	msgbox PalletTown_ProfessorOaksLab_Text_Aide1PokedexRecordsMons
PalletTown_ProfessorOaksLab_EventScript_Aide1OtherWays::
	msgbox PalletTown_ProfessorOaksLab_Text_Aide1OtherWaysToKnowMons
	release
	end

PalletTown_ProfessorOaksLab_EventScript_SetGotPokedex::
	setvar VAR_ACT1_PROGRESS, ACT1_GOT_POKEDEX
	return

PalletTown_ProfessorOaksLab_EventScript_Aide1GameClear::
	famechecker FAMECHECKER_OAK, 5
	msgbox PalletTown_ProfessorOaksLab_Text_OakIsGoingToHaveRadioShow
	release
	end

PalletTown_ProfessorOaksLab_EventScript_Aide2::
	lock
	faceplayer
	goto_if_set FLAG_SYS_GAME_CLEAR, PalletTown_ProfessorOaksLab_EventScript_Aide2GameClear
	goto_if_ne VAR_STORY_ACT, STORY_ACT_CHILDHOOD, PalletTown_ProfessorOaksLab_EventScript_Aide2Default
	goto_if_lt VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG, PalletTown_ProfessorOaksLab_EventScript_Aide2Default
	goto_if_set FLAG_BEFRIENDED_HATENNA, PalletTown_ProfessorOaksLab_EventScript_Aide2Default
	msgbox PalletTown_ProfessorOaksLab_Text_Aide2HatennaHint
	release
	end

PalletTown_ProfessorOaksLab_EventScript_Aide2Default::
	msgbox PalletTown_ProfessorOaksLab_Text_StudyAsOaksAide
	release
	end

PalletTown_ProfessorOaksLab_EventScript_Aide2GameClear::
	famechecker FAMECHECKER_DAISY, 0
	msgbox PalletTown_ProfessorOaksLab_Text_DaisyWillGroomMons
	release
	end

PalletTown_ProfessorOaksLab_EventScript_Aide3::
	lock
	faceplayer
	famechecker FAMECHECKER_OAK, 2
	msgbox PalletTown_ProfessorOaksLab_Text_OakIsAuthorityOnMons
	release
	end

PalletTown_ProfessorOaksLab_EventScript_Pokedex::
	msgbox PalletTown_ProfessorOaksLab_Text_BlankEncyclopedia, MSGBOX_NPC
	end

PalletTown_ProfessorOaksLab_EventScript_Computer::
	msgbox PalletTown_ProfessorOaksLab_Text_EmailMessage, MSGBOX_SIGN
	end

PalletTown_ProfessorOaksLab_EventScript_LeftSign::
	msgbox PalletTown_ProfessorOaksLab_Text_PressStartToOpenMenu, MSGBOX_SIGN
	end

PalletTown_ProfessorOaksLab_EventScript_RightSign::
	lockall
	goto_if_ge VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, PalletTown_ProfessorOaksLab_EventScript_RightSignAlt
	msgbox PalletTown_ProfessorOaksLab_Text_SaveOptionInMenu
	releaseall
	end

PalletTown_ProfessorOaksLab_EventScript_RightSignAlt::
	msgbox PalletTown_ProfessorOaksLab_Text_AllMonTypesHaveStrongAndWeakPoints
	releaseall
	end

Text_RivalVictory::
	.string "{RIVAL}: Yeah!\n"
	.string "Am I great or what?$"

PalletTown_ProfessorOaksLab_Text_BlankEncyclopedia::
	.string "It's like an encyclopedia, but the\n"
	.string "pages are blank.$"

PalletTown_ProfessorOaksLab_Text_PressStartToOpenMenu::
	.string "Press START to open the MENU!$"

PalletTown_ProfessorOaksLab_Text_SaveOptionInMenu::
	.string "The SAVE option is on the MENU.\n"
	.string "Use it regularly.$"

PalletTown_ProfessorOaksLab_Text_AllMonTypesHaveStrongAndWeakPoints::
	.string "All POKéMON types have strong and\n"
	.string "weak points against others.$"

PalletTown_ProfessorOaksLab_Text_EmailMessage::
	.string "There's an e-mail message here.\p"
	.string "…\p"
	.string "Finally!\n"
	.string "The ultimate TRAINERS of the\l"
	.string "POKéMON LEAGUE are ready to\l"
	.string "take on all comers!\p"
	.string "Bring your best POKéMON and see\n"
	.string "how you rate as a TRAINER!\p"
	.string "POKéMON LEAGUE HQ\n"
	.string "INDIGO PLATEAU\p"
	.string "PROF. OAK, please visit us!\n"
	.string "…$"

PalletTown_ProfessorOaksLab_Text_Aide1OakLeftPokedex::
	.string "Oh, you must be {PLAYER}!\p"
	.string "PROF. OAK asked me to give you\n"
	.string "this. Here you go!$"

PalletTown_ProfessorOaksLab_Text_ReceivedPokedexFromAide::
	.string "{PLAYER} received the POKéDEX\n"
	.string "from PROF. OAK's AIDE.$"

PalletTown_ProfessorOaksLab_Text_Aide1PokedexRecordsMons::
	.string "It records data on every POKéMON\n"
	.string "you meet.$"

PalletTown_ProfessorOaksLab_Text_Aide1OtherWaysToKnowMons::
	.string "You know, battling isn't the only\n"
	.string "way to get to know POKéMON.\p"
	.string "Maybe try getting to know your\n"
	.string "local POKéMON a bit better…\p"
	.string "…and see what happens!$"

@ Act 1 hint, after BLIPBUG (docs/hack/act1_story.md)
PalletTown_ProfessorOaksLab_Text_Aide2HatennaHint::
	.string "A HATENNA in the woods? They can\n"
	.string "sense how people are feeling.\p"
	.string "Anything loud or sudden upsets\n"
	.string "them terribly.\p"
	.string "I've read that a CHESTO BERRY helps\n"
	.string "them settle down.\p"
	.string "Some grow deep in the woods,\n"
	.string "I believe.$"

@ The POKéMON on the table are for the TRAINERS PROF. OAK chose
PalletTown_ProfessorOaksLab_Text_StarterBallsReserved::
	.string "These POKé BALLS hold the POKéMON\n"
	.string "for PROF. OAK's new TRAINERS.$"

PalletTown_ProfessorOaksLab_Text_StudyAsOaksAide::
	.string "I study POKéMON as PROF. OAK's\n"
	.string "AIDE.$"

PalletTown_ProfessorOaksLab_Text_DaisyWillGroomMons::
	.string "Hi, {PLAYER}. I bet you've become\n"
	.string "good friends with your POKéMON.\p"
	.string "By the way, did you know about\n"
	.string "DAISY?\p"
	.string "If you show DAISY your POKéMON,\n"
	.string "she can tell how much it likes you.\p"
	.string "Occasionally, she will even groom\n"
	.string "a POKéMON for you.\p"
	.string "This is a secret, so please don't\n"
	.string "tell anyone.$"

PalletTown_ProfessorOaksLab_Text_OakIsGoingToHaveRadioShow::
	.string "PROF. OAK is going to have his own\n"
	.string "radio show soon.\p"
	.string "The program will be called PROF.\n"
	.string "OAK's POKéMON SEMINAR.$"

PalletTown_ProfessorOaksLab_Text_OakIsAuthorityOnMons::
	.string "PROF. OAK may not look like much,\n"
	.string "but he's the authority on POKéMON.\p"
	.string "Many POKéMON TRAINERS hold him in\n"
	.string "high regard.$"
