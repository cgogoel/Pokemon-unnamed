@ Act 2's story (docs/hack/act2_story.md). Progress is VAR_ACT2_PROGRESS (ACT2_* in include/constants/story.h).
@ Three courses (Fire, Water, Grass), each a field trip: catch a Pokémon of the course's type, then pass the
@ professor's exam. After each, the friends (KAI, MINA, ROWAN) meet the player at the dorms and invite them to the
@ secret battle club at the stadium that night. The third night is the Trainer Card exam; then the final project.

@ Without a Trainer Card, trainers turn the player down (ConfigureTrainerBattle, src/battle_setup.c)
Act2_EventScript_NoTrainerCard::
	lock
	faceplayer
	msgbox Act2_Text_NoTrainerCard
	release
	end

Act2_EventScript_ShowObjectsTemp1::
	clearflag FLAG_TEMP_1
	return

@ ---------------------------------------------------------------------------------------------------------------
@ The campus: meeting the friends, and the club nights at the stadium

CeladonUniversity_OnTransition::
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	call_if_eq VAR_ACT2_PROGRESS, ACT2_START, Act2_EventScript_ShowObjectsTemp1
	end

CeladonUniversity_OnFrame::
	map_script_2 VAR_ACT2_PROGRESS, ACT2_START, Act2_EventScript_MeetFriends
	.2byte 0

@ Stepping out of the dorm room for the first time
Act2_EventScript_MeetFriends::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox Act2_Text_MeetFriends
	closemessage
	applymovement LOCALID_CAMPUS_KAI, Act2_Movement_LeaveLeft
	applymovement LOCALID_CAMPUS_MINA, Act2_Movement_LeaveLeft
	applymovement LOCALID_CAMPUS_ROWAN, Act2_Movement_LeaveLeft
	waitmovement LOCALID_CAMPUS_ROWAN
	removeobject LOCALID_CAMPUS_KAI
	removeobject LOCALID_CAMPUS_MINA
	removeobject LOCALID_CAMPUS_ROWAN
	setvar VAR_ACT2_PROGRESS, ACT2_MET_FRIENDS
	releaseall
	end

Act2_Movement_LeaveLeft:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Act2_EventScript_CampusKai::
	msgbox Act2_Text_MeetFriends, MSGBOX_NPC
	end

Act2_EventScript_CampusMina::
	msgbox Act2_Text_MeetFriends, MSGBOX_NPC
	end

Act2_EventScript_CampusRowan::
	msgbox Act2_Text_MeetFriends, MSGBOX_NPC
	end

@ Walking into the stadium on a club night (coord events at its gate). It's late; the friends are waiting.
Act2_EventScript_ClubArrive:
	msgbox Act2_Text_SneakOut
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	special Act2_SetNight
	removeobject LOCALID_CAMPUS_RUNNER
	removeobject LOCALID_CAMPUS_STADIUM_STUDENT
	clearflag FLAG_TEMP_2
	addobject LOCALID_CAMPUS_NIGHT_KAI
	addobject LOCALID_CAMPUS_NIGHT_MINA
	addobject LOCALID_CAMPUS_NIGHT_ROWAN
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp2
	waitmovement 0
	fadescreenswapbuffers FADE_FROM_BLACK
	return

@ After a club night: back to bed, and it's morning
Act2_EventScript_ClubMorning:
	closemessage
	fadescreen FADE_TO_BLACK
	special HealPlayerParty
	warp MAP_CELADON_UNIVERSITY_DORM_ROOM, 2, 3
	waitstate
	releaseall
	end

Act2_EventScript_ClubNight1::
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night1
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_KAI, Act2_Text_KaiDefeat
	msgbox Act2_Text_Night1After
	setvar VAR_ACT2_PROGRESS, ACT2_CLUB_1_DONE
	goto Act2_EventScript_ClubMorning

Act2_EventScript_ClubNight2::
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night2
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_MINA, Act2_Text_MinaDefeat
	msgbox Act2_Text_Night2After
	setvar VAR_ACT2_PROGRESS, ACT2_CLUB_2_DONE
	goto Act2_EventScript_ClubMorning

@ The third night: an examiner from the POKéMON LEAGUE, and a real Trainer Card
Act2_EventScript_ClubNight3::
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night3
	closemessage
	fadescreenswapbuffers FADE_TO_BLACK
	clearflag FLAG_TEMP_3
	addobject LOCALID_CAMPUS_DALE
	fadescreenswapbuffers FADE_FROM_BLACK
	msgbox Act2_Text_DaleIntro
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_EXAMINER, Act2_Text_DaleDefeat
	msgbox Act2_Text_DalePasses
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Act2_Text_GotTrainerCard
	waitmessage
	waitfanfare
	setvar VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD
	msgbox Act2_Text_Night3After
	goto Act2_EventScript_ClubMorning

@ ---------------------------------------------------------------------------------------------------------------
@ The dorm lobby: the friends after each course

CeladonUniversity_DormLobby_OnTransition::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_FIRE_PASSED, Act2_EventScript_ShowObjectsTemp1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_PASSED, Act2_EventScript_ShowObjectsTemp1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_PASSED, Act2_EventScript_ShowObjectsTemp1
	end

CeladonUniversity_DormLobby_OnFrame::
	map_script_2 VAR_ACT2_PROGRESS, ACT2_FIRE_PASSED, Act2_EventScript_BackFromFire
	map_script_2 VAR_ACT2_PROGRESS, ACT2_WATER_PASSED, Act2_EventScript_BackFromWater
	map_script_2 VAR_ACT2_PROGRESS, ACT2_GRASS_PASSED, Act2_EventScript_BackFromGrass
	.2byte 0

Act2_EventScript_BackFromFire::
	lockall
	msgbox Act2_Text_BackFromFire
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED
	releaseall
	end

Act2_EventScript_BackFromWater::
	lockall
	msgbox Act2_Text_BackFromWater
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_INVITED
	releaseall
	end

Act2_EventScript_BackFromGrass::
	lockall
	msgbox Act2_Text_BackFromGrass
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED
	releaseall
	end

Act2_EventScript_LobbyKai::
	msgbox Act2_Text_SeeYouTonight, MSGBOX_NPC
	end

Act2_EventScript_LobbyMina::
	msgbox Act2_Text_SeeYouTonight, MSGBOX_NPC
	end

Act2_EventScript_LobbyRowan::
	msgbox Act2_Text_SeeYouTonight, MSGBOX_NPC
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The courses. Each professor starts their course on campus (giving Poké Balls), and the class goes on a field
@ trip. There, the professor gives the exam once the player has caught a Pokémon of the course's type.

@ Fire: PROF. ASHFORD, Lecture Hall A, field trip to Cinnabar Island (the Pokémon Mansion)
Act2_EventScript_ProfAshford::
	lock
	faceplayer
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_MET_FRIENDS, Act2_EventScript_StartFire
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_MET_FRIENDS, Act2_EventScript_ProfAshfordEarly
	msgbox Act2_Text_ProfAshfordAfter
	release
	end

Act2_EventScript_ProfAshfordEarly:
	msgbox Act2_Text_ProfAshfordEarly
	release
	end

Act2_EventScript_StartFire:
	msgbox Act2_Text_StartFire
	giveitem ITEM_POKE_BALL, 5
	msgbox Act2_Text_FieldTripFire
	closemessage
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP
	setrespawn HEAL_LOCATION_CINNABAR_ISLAND
	warp MAP_CINNABAR_ISLAND, 8, 5
	waitstate
	release
	end

CinnabarIsland_EventScript_Act2Objects::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP, Act2_EventScript_ShowObjectsTemp1
	return

Act2_EventScript_ProfAshfordTrip::
	lock
	faceplayer
	setvar VAR_0x8004, TYPE_FIRE
	specialvar VAR_RESULT, Act2_PartyHasType
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_NoFireYet
	msgbox Act2_Text_ReadyForFireExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_FireExamBegins
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_PROF_FIRE, Act2_Text_AshfordDefeat
	msgbox Act2_Text_FirePassed
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_NoFireYet:
	msgbox Act2_Text_NoFireYet
	release
	end

@ Water: PROF. MARLOWE, the audiovisual classroom, field trip to Vermilion City (fishing at the harbor, Route 6's grass)
Act2_EventScript_ProfMarlowe::
	lock
	faceplayer
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_CLUB_1_DONE, Act2_EventScript_StartWater
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_CLUB_1_DONE, Act2_EventScript_ProfMarloweEarly
	msgbox Act2_Text_ProfMarloweAfter
	release
	end

Act2_EventScript_ProfMarloweEarly:
	msgbox Act2_Text_ProfMarloweEarly
	release
	end

Act2_EventScript_StartWater:
	msgbox Act2_Text_StartWater
	giveitem ITEM_POKE_BALL, 5
	giveitem ITEM_GOOD_ROD
	msgbox Act2_Text_FieldTripWater
	closemessage
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_TRIP
	setrespawn HEAL_LOCATION_VERMILION_CITY
	warp MAP_VERMILION_CITY, 32, 19
	waitstate
	release
	end

VermilionCity_EventScript_Act2Objects::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_TRIP, Act2_EventScript_ShowObjectsTemp1
	return

Act2_EventScript_ProfMarloweTrip::
	lock
	faceplayer
	setvar VAR_0x8004, TYPE_WATER
	specialvar VAR_RESULT, Act2_PartyHasType
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_NoWaterYet
	msgbox Act2_Text_ReadyForWaterExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_WaterExamBegins
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_PROF_WATER, Act2_Text_MarloweDefeat
	msgbox Act2_Text_WaterPassed
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_NoWaterYet:
	msgbox Act2_Text_NoWaterYet
	release
	end

@ Grass: PROF. FERN, Home Economics, field trip to Viridian City (Route 22's grass)
Act2_EventScript_ProfFern::
	lock
	faceplayer
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_CLUB_2_DONE, Act2_EventScript_StartGrass
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_CLUB_2_DONE, Act2_EventScript_ProfFernEarly
	msgbox Act2_Text_ProfFernAfter
	release
	end

Act2_EventScript_ProfFernEarly:
	msgbox Act2_Text_ProfFernEarly
	release
	end

Act2_EventScript_StartGrass:
	msgbox Act2_Text_StartGrass
	giveitem ITEM_POKE_BALL, 5
	msgbox Act2_Text_FieldTripGrass
	closemessage
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP
	setrespawn HEAL_LOCATION_VIRIDIAN_CITY
	warp MAP_VIRIDIAN_CITY, 4, 17
	waitstate
	release
	end

ViridianCity_EventScript_Act2Objects::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP, Act2_EventScript_ShowObjectsTemp1
	return

Act2_EventScript_ProfFernTrip::
	lock
	faceplayer
	setvar VAR_0x8004, TYPE_GRASS
	specialvar VAR_RESULT, Act2_PartyHasType
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_NoGrassYet
	msgbox Act2_Text_ReadyForGrassExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_GrassExamBegins
	closemessage
	trainerbattle_no_intro TRAINER_ACT2_PROF_GRASS, Act2_Text_FernDefeat
	msgbox Act2_Text_GrassPassed
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_NoGrassYet:
	msgbox Act2_Text_NoGrassYet
	release
	end

Act2_EventScript_ComeBackWhenReady:
	msgbox Act2_Text_ComeBackWhenReady
	release
	end

@ A passed exam: the class heads home, to the dorm lobby, where the friends talk about tonight
Act2_EventScript_BackToDorms:
	closemessage
	setrespawn HEAL_LOCATION_CELADON_CITY
	special HealPlayerParty
	warp MAP_CELADON_UNIVERSITY_DORM_LOBBY, 12, 14
	waitstate
	release
	end

@ The friends on the field trips
Act2_EventScript_TripKai::
	msgbox Act2_Text_TripKai, MSGBOX_NPC
	end

Act2_EventScript_TripMina::
	msgbox Act2_Text_TripMina, MSGBOX_NPC
	end

Act2_EventScript_TripRowan::
	msgbox Act2_Text_TripRowan, MSGBOX_NPC
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The final project: the director of the Science Research Institute

Act2_EventScript_Director::
	lock
	faceplayer
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD, Act2_EventScript_AssignProject
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT, Act2_EventScript_CheckProject
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_PROJECT_DONE, Act2_EventScript_DirectorAfter
	msgbox Act2_Text_DirectorEarly
	release
	end

Act2_EventScript_AssignProject:
	msgbox Act2_Text_AssignProject
	giveitem ITEM_POKE_BALL, 10
	msgbox Act2_Text_ProjectHints
	setvar VAR_ACT2_PROGRESS, ACT2_FINAL_PROJECT
	release
	end

Act2_EventScript_CheckProject:
	specialvar VAR_RESULT, Act2_PartyHasProjectPokemon
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_ProjectNotYet
	msgbox Act2_Text_ProjectDone
	setvar VAR_ACT2_PROGRESS, ACT2_PROJECT_DONE
	msgbox Act2_Text_EndOfAct2
	release
	end

Act2_EventScript_ProjectNotYet:
	msgbox Act2_Text_ProjectHints
	release
	end

Act2_EventScript_DirectorAfter:
	msgbox Act2_Text_EndOfAct2
	release
	end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_NoTrainerCard:
	.string "You want to battle?\n"
	.string "Let me see your TRAINER CARD.\p"
	.string "…You don't have one?\n"
	.string "Sorry, I only battle TRAINERS.$"

Act2_Text_MeetFriends:
	.string "KAI: Hey! You're the new student\n"
	.string "from the farm, right?\p"
	.string "MINA: We're in the FIRE course,\n"
	.string "too! I'm MINA. This is KAI, and\l"
	.string "that's ROWAN.\p"
	.string "ROWAN: PROF. ASHFORD's class is in\n"
	.string "LECTURE HALL A, the MAIN HALL's\l"
	.string "west wing. See you there!$"

Act2_Text_SneakOut:
	.string "It's late. Everyone's asleep…\n"
	.string "Time to sneak into the stadium.$"

Act2_Text_Night1:
	.string "KAI: You came! Welcome to the\n"
	.string "club. No professors, no rules.\p"
	.string "Officially, we're not TRAINERS.\n"
	.string "Unofficially… let's battle!$"

Act2_Text_KaiDefeat:
	.string "Whoa, you're a natural!$"

Act2_Text_Night1After:
	.string "MINA: Not bad for your first night!\p"
	.string "ROWAN: Next is the WATER course.\n"
	.string "PROF. MARLOWE teaches it, in the\l"
	.string "AUDIOVISUAL CLASSROOM.\p"
	.string "KAI: Let's get some sleep before\n"
	.string "anyone notices we're gone.$"

Act2_Text_Night2:
	.string "MINA: My turn tonight!\n"
	.string "My WATER POKéMON are ready!$"

Act2_Text_MinaDefeat:
	.string "You've gotten so good!$"

Act2_Text_Night2After:
	.string "ROWAN: One course left: GRASS,\n"
	.string "with PROF. FERN in HOME ECONOMICS.\p"
	.string "KAI: And then… we have a surprise\n"
	.string "for you. Get some sleep!$"

Act2_Text_Night3:
	.string "ROWAN: Here's the surprise.\p"
	.string "We asked someone from the POKéMON\n"
	.string "LEAGUE to come see you battle.\p"
	.string "If you win, you get a real\n"
	.string "TRAINER CARD!$"

Act2_Text_DaleIntro:
	.string "DALE: I'm a LEAGUE examiner.\n"
	.string "Your friends talk you up a lot.\p"
	.string "Show me what you've learned.\n"
	.string "No holding back!$"

Act2_Text_DaleDefeat:
	.string "That settles it.$"

Act2_Text_DalePasses:
	.string "DALE: You passed. Congratulations.\n"
	.string "This is yours now.$"

Act2_Text_GotTrainerCard:
	.string "{PLAYER} received a\n"
	.string "TRAINER CARD!$"

Act2_Text_Night3After:
	.string "DALE: With that, any TRAINER in\n"
	.string "KANTO will battle you.\p"
	.string "MINA: You did it!\p"
	.string "KAI: Oh, and the director of the\n"
	.string "SCIENCE RESEARCH INSTITUTE wants\l"
	.string "to see you. Your final project!$"

Act2_Text_BackFromFire:
	.string "KAI: We all passed the FIRE course!\p"
	.string "MINA: Listen… we have a club.\n"
	.string "A secret one. We battle at night.\p"
	.string "ROWAN: Meet us at the STADIUM\n"
	.string "tonight. Don't tell anyone!$"

Act2_Text_BackFromWater:
	.string "MINA: Two courses down!\p"
	.string "KAI: The club meets again tonight.\n"
	.string "Same place, the STADIUM.$"

Act2_Text_BackFromGrass:
	.string "ROWAN: That's all three courses!\p"
	.string "KAI: Meet us at the STADIUM one\n"
	.string "more time tonight.\p"
	.string "MINA: Tonight you battle for a\n"
	.string "real TRAINER CARD!$"

Act2_Text_SeeYouTonight:
	.string "See you at the STADIUM tonight!$"

Act2_Text_ProfAshfordEarly:
	.string "PROF. ASHFORD: Classes start soon.$"

Act2_Text_StartFire:
	.string "PROF. ASHFORD: Welcome to the FIRE\n"
	.string "course. I'm PROF. ASHFORD.\p"
	.string "You won't learn FIRE POKéMON from\n"
	.string "a book. You'll catch one, train it,\l"
	.string "and battle me with it.\p"
	.string "Here, you'll need these.$"

Act2_Text_FieldTripFire:
	.string "Our field trip leaves now, for\n"
	.string "CINNABAR ISLAND!$"

Act2_Text_NoFireYet:
	.string "PROF. ASHFORD: FIRE POKéMON live\n"
	.string "in the old mansion, right there.\p"
	.string "Catch one, train it, and come back\n"
	.string "for your exam.$"

Act2_Text_ReadyForFireExam:
	.string "PROF. ASHFORD: You caught a FIRE\n"
	.string "POKéMON! Ready for your exam?$"

Act2_Text_FireExamBegins:
	.string "Let's see how hot it burns!$"

Act2_Text_AshfordDefeat:
	.string "Excellent work!$"

Act2_Text_FirePassed:
	.string "PROF. ASHFORD: You pass!\p"
	.string "That's all for the field trip.\n"
	.string "Back to the university, everyone!$"

Act2_Text_ProfAshfordAfter:
	.string "PROF. ASHFORD: How's your FIRE\n"
	.string "POKéMON doing? Keep training it!$"

Act2_Text_ProfMarloweEarly:
	.string "PROF. MARLOWE: Ah, a FIRE student.\n"
	.string "Come see me after that course.$"

Act2_Text_StartWater:
	.string "PROF. MARLOWE: The WATER course!\n"
	.string "Water shapes everything it touches.\p"
	.string "Catch a WATER POKéMON, train it,\n"
	.string "and pass my exam. Take these.$"

Act2_Text_FieldTripWater:
	.string "We're off to VERMILION CITY!$"

Act2_Text_NoWaterYet:
	.string "PROF. MARLOWE: WATER POKéMON swim\n"
	.string "all around the harbor.\p"
	.string "Fish with the GOOD ROD, or look in\n"
	.string "the grass on ROUTE 6, to the north.$"

Act2_Text_ReadyForWaterExam:
	.string "PROF. MARLOWE: A fine WATER\n"
	.string "POKéMON! Shall we begin the exam?$"

Act2_Text_WaterExamBegins:
	.string "Show me what water can do!$"

Act2_Text_MarloweDefeat:
	.string "Splendid. Simply splendid.$"

Act2_Text_WaterPassed:
	.string "PROF. MARLOWE: You pass!\p"
	.string "Back to the university, everyone!$"

Act2_Text_ProfMarloweAfter:
	.string "PROF. MARLOWE: Keep that GOOD ROD.\n"
	.string "You never know what's out there.$"

Act2_Text_ProfFernEarly:
	.string "PROF. FERN: The GRASS course is\n"
	.string "your third. I'll see you then.$"

Act2_Text_StartGrass:
	.string "PROF. FERN: Welcome to the GRASS\n"
	.string "course. Patience is everything.\p"
	.string "Catch a GRASS POKéMON, raise it,\n"
	.string "and pass my exam. Take these.$"

Act2_Text_FieldTripGrass:
	.string "Our field trip is to VIRIDIAN CITY!$"

Act2_Text_NoGrassYet:
	.string "PROF. FERN: Head west, to ROUTE 22.\n"
	.string "GRASS POKéMON grow in its fields.\p"
	.string "Catch one, raise it, and come back\n"
	.string "for your exam.$"

Act2_Text_ReadyForGrassExam:
	.string "PROF. FERN: What a lovely GRASS\n"
	.string "POKéMON. Ready for the exam?$"

Act2_Text_GrassExamBegins:
	.string "Let it grow!$"

Act2_Text_FernDefeat:
	.string "Beautifully done.$"

Act2_Text_GrassPassed:
	.string "PROF. FERN: You pass!\p"
	.string "That's every course. Back to the\n"
	.string "university, everyone!$"

Act2_Text_ProfFernAfter:
	.string "PROF. FERN: Your GRASS POKéMON\n"
	.string "looks healthy. Keep it up!$"

Act2_Text_ComeBackWhenReady:
	.string "Come back when you're ready.$"

Act2_Text_TripKai:
	.string "KAI: I'm going to catch the\n"
	.string "strongest one out there!$"

Act2_Text_TripMina:
	.string "MINA: Good luck! Remember to weaken\n"
	.string "them before you throw a BALL.$"

Act2_Text_TripRowan:
	.string "ROWAN: Take your time.\n"
	.string "Rushing never works with POKéMON.$"

Act2_Text_DirectorEarly:
	.string "DIRECTOR: Welcome to the SCIENCE\n"
	.string "RESEARCH INSTITUTE.\p"
	.string "Finish your courses, then come see\n"
	.string "me about your final project.$"

Act2_Text_AssignProject:
	.string "DIRECTOR: Ah, the student with the\n"
	.string "new TRAINER CARD. Congratulations.\p"
	.string "Your final project: find and catch\n"
	.string "a POKéMON that's two of the types\l"
	.string "you've studied.\p"
	.string "FIRE and WATER, WATER and GRASS,\n"
	.string "or GRASS and FIRE. Take these.$"

Act2_Text_ProjectHints:
	.string "DIRECTOR: There are reports of a\n"
	.string "WATER and GRASS POKéMON in the\l"
	.string "pond in CELADON CITY. Try fishing.\p"
	.string "And on ROUTE 7, there's a little\n"
	.string "GRASS POKéMON that a FIRE STONE\l"
	.string "would change…$"

Act2_Text_ProjectDone:
	.string "DIRECTOR: That's it! Two types in\n"
	.string "one POKéMON. Remarkable work.\p"
	.string "Your project passes.\n"
	.string "Congratulations, graduate!$"

Act2_Text_EndOfAct2:
	.string "(End of Act 2.)$"
