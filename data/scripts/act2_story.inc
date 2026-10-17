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
	special Act2_GiveRemoteDesktop
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
	applymovement LOCALID_CAMPUS_KAI, Act2_Movement_KaiToClass
	applymovement LOCALID_CAMPUS_MINA, Act2_Movement_MinaToClass
	applymovement LOCALID_CAMPUS_ROWAN, Act2_Movement_RowanToClass
	waitmovement LOCALID_CAMPUS_ROWAN
	removeobject LOCALID_CAMPUS_KAI
	removeobject LOCALID_CAMPUS_MINA
	removeobject LOCALID_CAMPUS_ROWAN
	setvar VAR_ACT2_PROGRESS, ACT2_MET_FRIENDS
	releaseall
	end

@ Off toward the MAIN HALL: along the path to the avenue (x 33), then up it until well out of sight
Act2_Movement_KaiToClass:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Act2_Movement_MinaToClass:
	delay_16
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Act2_Movement_RowanToClass:
	delay_16
	delay_16
	delay_16
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

@ The friends wait in the classroom for each course to start
Act2_EventScript_ClassroomObjects::
	setflag FLAG_TEMP_1
	call_if_eq VAR_ACT2_PROGRESS, VAR_0x8004, Act2_EventScript_ShowObjectsTemp1
	return

CeladonUniversity_LectureHallA_OnTransition::
	setvar VAR_0x8004, ACT2_MET_FRIENDS
	call Act2_EventScript_ClassroomObjects
	end

CeladonUniversity_LectureHallB_OnTransition::
	setvar VAR_0x8004, ACT2_CLUB_1_DONE
	call Act2_EventScript_ClassroomObjects
	end

CeladonUniversity_HomeEconomics_OnTransition::
	setvar VAR_0x8004, ACT2_CLUB_2_DONE
	call Act2_EventScript_ClassroomObjects
	end

Act2_EventScript_ClassKai::
	msgbox Act2_Text_ClassKai, MSGBOX_NPC
	end

Act2_EventScript_ClassMina::
	msgbox Act2_Text_ClassMina, MSGBOX_NPC
	end

Act2_EventScript_ClassRowan::
	msgbox Act2_Text_ClassRowan, MSGBOX_NPC
	end

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
@ The courses. Each professor starts their course on campus: what to do, Poké Balls, then the field trip. There,
@ the professor's research sheet: catch a Pokémon of the type, use a move of it, record 3 different moves of it,
@ and solve the site's puzzle. With all four done, the professor helps make the course's TM; the exam only counts if
@ the player's Pokémon of the type uses its move (src/act2_story.c tracks all this).

@ Fire: PROF. ASHFORD, Lecture Hall A; field trip to Cinnabar Island
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
	closemessage
	setvar VAR_ACT2_RESEARCH, 0
	setvar VAR_ACT2_MOVES_SEEN, 0
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP
	setrespawn HEAL_LOCATION_CINNABAR_ISLAND
	warp MAP_CINNABAR_ISLAND, 8, 5
	waitstate
	release
	end

Act2_EventScript_ProfAshfordTrip::
	lock
	faceplayer
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_FireExam
	specialvar VAR_RESULT, Act2_IsResearchDone
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_FireMakeTM
	msgbox Act2_Text_FireSheetIntro
	special Act2_BufferResearchSheet
	msgbox gStringVar4
	msgbox Act2_Text_FireHint
	release
	end

Act2_EventScript_FireMakeTM:
	msgbox Act2_Text_FireResearchDone
	giveitem ITEM_TM_FLAMETHROWER
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	special Act2_SetResearchFlag
	msgbox Act2_Text_FireTeachTM
	release
	end

Act2_EventScript_FireExam:
	specialvar VAR_RESULT, Act2_PartyCanUseCourseTM
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_FireTeachTMFirst
	msgbox Act2_Text_ReadyForFireExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_FireExamBegins
	closemessage
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	special Act2_ClearResearchFlag
	cleartrainerflag TRAINER_ACT2_PROF_FIRE
	trainerbattle_no_intro TRAINER_ACT2_PROF_FIRE, Act2_Text_AshfordDefeat
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_FireExamNoMove
	msgbox Act2_Text_FirePassed
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_FireExamNoMove:
	msgbox Act2_Text_FireExamNoMove
	release
	end

Act2_EventScript_FireTeachTMFirst:
	msgbox Act2_Text_FireTeachTM
	release
	end

@ Water: PROF. MARLOWE, the audiovisual classroom; field trip to Vermilion City
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
	setvar VAR_ACT2_RESEARCH, 0
	setvar VAR_ACT2_MOVES_SEEN, 0
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_TRIP
	setrespawn HEAL_LOCATION_VERMILION_CITY
	warp MAP_VERMILION_CITY, 32, 19
	waitstate
	release
	end

Act2_EventScript_ProfMarloweTrip::
	lock
	faceplayer
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_WaterExam
	specialvar VAR_RESULT, Act2_IsResearchDone
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_WaterMakeTM
	msgbox Act2_Text_WaterSheetIntro
	special Act2_BufferResearchSheet
	msgbox gStringVar4
	msgbox Act2_Text_WaterHint
	release
	end

Act2_EventScript_WaterMakeTM:
	msgbox Act2_Text_WaterResearchDone
	giveitem ITEM_TM_BUBBLE_BEAM
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	special Act2_SetResearchFlag
	msgbox Act2_Text_WaterTeachTM
	release
	end

Act2_EventScript_WaterExam:
	specialvar VAR_RESULT, Act2_PartyCanUseCourseTM
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WaterTeachTMFirst
	msgbox Act2_Text_ReadyForWaterExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_WaterExamBegins
	closemessage
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	special Act2_ClearResearchFlag
	cleartrainerflag TRAINER_ACT2_PROF_WATER
	trainerbattle_no_intro TRAINER_ACT2_PROF_WATER, Act2_Text_MarloweDefeat
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WaterExamNoMove
	msgbox Act2_Text_WaterPassed
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_WaterExamNoMove:
	msgbox Act2_Text_WaterExamNoMove
	release
	end

Act2_EventScript_WaterTeachTMFirst:
	msgbox Act2_Text_WaterTeachTM
	release
	end

@ Grass: PROF. FERN, Home Economics; field trip to Viridian City
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
	setvar VAR_ACT2_RESEARCH, 0
	setvar VAR_ACT2_MOVES_SEEN, 0
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_TRIP
	setrespawn HEAL_LOCATION_VIRIDIAN_CITY
	warp MAP_VIRIDIAN_CITY, 4, 17
	waitstate
	release
	end

Act2_EventScript_ProfFernTrip::
	lock
	faceplayer
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_GrassExam
	specialvar VAR_RESULT, Act2_IsResearchDone
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_GrassMakeTM
	msgbox Act2_Text_GrassSheetIntro
	special Act2_BufferResearchSheet
	msgbox gStringVar4
	msgbox Act2_Text_GrassHint
	release
	end

Act2_EventScript_GrassMakeTM:
	msgbox Act2_Text_GrassResearchDone
	giveitem ITEM_TM_GIGA_DRAIN
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	special Act2_SetResearchFlag
	msgbox Act2_Text_GrassTeachTM
	release
	end

Act2_EventScript_GrassExam:
	specialvar VAR_RESULT, Act2_PartyCanUseCourseTM
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_GrassTeachTMFirst
	msgbox Act2_Text_ReadyForGrassExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_GrassExamBegins
	closemessage
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	special Act2_ClearResearchFlag
	cleartrainerflag TRAINER_ACT2_PROF_GRASS
	trainerbattle_no_intro TRAINER_ACT2_PROF_GRASS, Act2_Text_FernDefeat
	setvar VAR_0x8004, ACT2_RESEARCH_USED_TM_MOVE
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_GrassExamNoMove
	msgbox Act2_Text_GrassPassed
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_PASSED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_GrassExamNoMove:
	msgbox Act2_Text_GrassExamNoMove
	release
	end

Act2_EventScript_GrassTeachTMFirst:
	msgbox Act2_Text_GrassTeachTM
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

@ The sites' objects (Act2_SetupSiteObjects)
CinnabarIsland_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_FIRE_TRIP
	special Act2_SetupSiteObjects
	setvar VAR_TEMP_1, 0
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP, Common_EventScript_NopReturn
	setvar VAR_0x8004, ACT2_RESEARCH_BRIEFED
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	setvar VAR_TEMP_1, 1
	return

CinnabarIsland_OnFrame::
	map_script_2 VAR_TEMP_1, 1, Act2_EventScript_FireBriefing
	.2byte 0

@ The class arrives on Cinnabar: PROF. ASHFORD explains the assignment
Act2_EventScript_FireBriefing::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_CINNABAR_PROF, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	msgbox Act2_Text_FireBriefing
	giveitem ITEM_POKE_BALL, 5
	setvar VAR_0x8004, ACT2_RESEARCH_BRIEFED
	special Act2_SetResearchFlag
	setvar VAR_TEMP_1, 0
	releaseall
	end

PokemonMansion_1F_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_FIRE_TRIP
	special Act2_SetupSiteObjects
	return

VermilionCity_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_WATER_TRIP
	special Act2_SetupSiteObjects
	return

ViridianCity_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_GRASS_TRIP
	special Act2_SetupSiteObjects
	return

Route22_OnTransition::
	setvar VAR_0x8004, ACT2_GRASS_TRIP
	special Act2_SetupSiteObjects
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The site puzzles: each part needs a Pokémon of the course's type that knows a move of it (Act2_FindCourseMoveUser)

@ Fire: three cold furnaces in the Pokémon Mansion (1F). Lit, they wake the statue switch, which opens the sealed
@ room with the old research journal.
Act2_EventScript_Furnace1::
	setvar VAR_0x8004, 1
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace2::
	setvar VAR_0x8004, 2
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace3::
	setvar VAR_0x8004, 3
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace:
	lockall
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP, Act2_EventScript_FurnaceCold
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_FurnaceLit
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_FurnaceNeedsFire
	msgbox Act2_Text_FurnaceAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	playse SE_M_EMBER
	msgbox Act2_Text_FurnaceRelit
	waitse
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_FurnacesAllLit
	releaseall
	end

Act2_EventScript_FurnacesAllLit:
	msgbox Act2_Text_FurnacesAllLit
	return

Act2_EventScript_FurnaceCold:
	msgbox Act2_Text_FurnaceCold
	releaseall
	end

Act2_EventScript_FurnaceLit:
	msgbox Act2_Text_FurnaceLit
	releaseall
	end

Act2_EventScript_FurnaceNeedsFire:
	msgbox Act2_Text_FurnaceNeedsFire
	releaseall
	end

Act2_EventScript_PuzzleRelease:
	releaseall
	end

@ The statue switch (PokemonMansion_1F_EventScript_Statue) stays cold until the furnaces are lit
Act2_EventScript_MansionStatue::
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, TRUE, PokemonMansion_1F_EventScript_StatueSwitch
	lockall
	msgbox Act2_Text_StatueCold
	releaseall
	end

Act2_EventScript_MansionJournal::
	lock
	msgbox Act2_Text_MansionJournal
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	removeobject LOCALID_MANSION_JOURNAL
	msgbox Act2_Text_TaskComplete
	release
	end

@ Water: three fires on Vermilion's docks. Put out, a young Lapras that was hiding under the docks surfaces.
Act2_EventScript_DockFire1::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, LOCALID_VERMILION_FIRE_1
	goto Act2_EventScript_DockFire

Act2_EventScript_DockFire2::
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, LOCALID_VERMILION_FIRE_2
	goto Act2_EventScript_DockFire

Act2_EventScript_DockFire3::
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, LOCALID_VERMILION_FIRE_3
	goto Act2_EventScript_DockFire

Act2_EventScript_DockFire:
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireNeedsWater
	msgbox Act2_Text_DockFireAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	playse SE_M_BUBBLE_BEAM
	msgbox Act2_Text_DockFireOut
	waitse
	removeobject VAR_0x8005
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_LaprasSurfaces
	releaseall
	end

Act2_EventScript_DockFireNeedsWater:
	msgbox Act2_Text_DockFireNeedsWater
	releaseall
	end

Act2_EventScript_LaprasSurfaces:
	playse SE_M_SURF
	clearflag FLAG_TEMP_8
	addobject LOCALID_VERMILION_LAPRAS
	waitse
	msgbox Act2_Text_LaprasSurfaces
	return

@ The young Lapras: recording it finishes the research, and then it asks to come along (it won't take no for an
@ answer). In Act 3, it's the Lapras in Silph Co.'s lab.
Act2_EventScript_DockLapras::
	lock
	faceplayer
	playmoncry SPECIES_LAPRAS, CRY_MODE_NORMAL
	msgbox Act2_Text_DockLapras
	waitmoncry
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	msgbox Act2_Text_TaskComplete
Act2_EventScript_LaprasAsks:
	playmoncry SPECIES_LAPRAS, CRY_MODE_NORMAL
	msgbox Act2_Text_LaprasWantsToCome, MSGBOX_YESNO
	waitmoncry
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_LaprasWontLeave
	setvar VAR_TEMP_1, SPECIES_LAPRAS
	givemon SPECIES_LAPRAS, ACT2_LAPRAS_LEVEL
	goto_if_eq VAR_RESULT, 0, Act2_EventScript_LaprasToParty
	goto_if_eq VAR_RESULT, 1, Act2_EventScript_LaprasToPC
	goto Common_EventScript_NoMoreRoomForPokemon

Act2_EventScript_LaprasWontLeave:
	msgbox Act2_Text_LaprasWontLeave
	goto Act2_EventScript_LaprasAsks

Act2_EventScript_LaprasToParty:
	call Act2_EventScript_LaprasJoined
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_LaprasDone
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto Act2_EventScript_LaprasDone

Act2_EventScript_LaprasToPC:
	call Act2_EventScript_LaprasJoined
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_LaprasToPCDone
	call Common_EventScript_NameReceivedBoxMon
Act2_EventScript_LaprasToPCDone:
	call Common_EventScript_TransferredToPC
	goto Act2_EventScript_LaprasDone

Act2_EventScript_LaprasJoined:
	removeobject LOCALID_VERMILION_LAPRAS
	setflag FLAG_ACT2_GOT_LAPRAS
	playfanfare MUS_LEVEL_UP
	message Act2_Text_LaprasJoined
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_LAPRAS
	return

Act2_EventScript_LaprasDone:
	release
	end

@ Grass: three withered patches on Route 22. Regrown, the field blooms.
Act2_EventScript_Withered1::
	setvar VAR_0x8004, 1
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WitheredNeedsGrass
	msgbox Act2_Text_WitheredAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	playse SE_M_GIGA_DRAIN
	msgbox Act2_Text_WitheredRegrown
	waitse
	removeobject LOCALID_ROUTE22_WITHERED_1
	clearflag FLAG_TEMP_5
	addobject LOCALID_ROUTE22_GROWN_1
	goto Act2_EventScript_WitheredSolved

Act2_EventScript_Withered2::
	setvar VAR_0x8004, 2
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WitheredNeedsGrass
	msgbox Act2_Text_WitheredAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	playse SE_M_GIGA_DRAIN
	msgbox Act2_Text_WitheredRegrown
	waitse
	removeobject LOCALID_ROUTE22_WITHERED_2
	clearflag FLAG_TEMP_6
	addobject LOCALID_ROUTE22_GROWN_2
	goto Act2_EventScript_WitheredSolved

Act2_EventScript_Withered3::
	setvar VAR_0x8004, 3
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WitheredNeedsGrass
	msgbox Act2_Text_WitheredAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	playse SE_M_GIGA_DRAIN
	msgbox Act2_Text_WitheredRegrown
	waitse
	removeobject LOCALID_ROUTE22_WITHERED_3
	clearflag FLAG_TEMP_7
	addobject LOCALID_ROUTE22_GROWN_3
	goto Act2_EventScript_WitheredSolved

Act2_EventScript_WitheredSolved:
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_FieldBlooms
	releaseall
	end

Act2_EventScript_FieldBlooms:
	playse SE_M_SWEET_SCENT
	msgbox Act2_Text_FieldBlooms
	waitse
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	msgbox Act2_Text_TaskComplete
	return

Act2_EventScript_WitheredNeedsGrass:
	msgbox Act2_Text_WitheredNeedsGrass
	releaseall
	end

Act2_EventScript_Regrown::
	msgbox Act2_Text_Regrown, MSGBOX_SIGN
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
	.string "course. Hope you brought your\l"
	.string "POKéMON…\p"
	.string "We're going on a field trip!\p"
	.string "In your final year, you'll learn\n"
	.string "by doing. Enough books and theory:\l"
	.string "you'll learn in practice!\p"
	.string "Yes, you're not TRAINERS. But you\n"
	.string "must learn the basics of battling\l"
	.string "and catching POKéMON if you intend\l"
	.string "to reach your full potential as a\l"
	.string "scholar.\p"
	.string "For the FIRE course, we're headed\n"
	.string "to CINNABAR ISLAND.\p"
	.string "I'll explain the assignment when we\n"
	.string "arrive. Let's go!$"

Act2_Text_FireBriefing:
	.string "PROF. ASHFORD: Welcome to CINNABAR\n"
	.string "ISLAND, home of the CINNABAR LAB\l"
	.string "and the POKéMON MANSION.\p"
	.string "Famous for its wild FIRE POKéMON,\n"
	.string "it should be the perfect host for\l"
	.string "our field assignment.\p"
	.string "Catch a FIRE POKéMON. Battle with\n"
	.string "it, and watch how FIRE moves work.\p"
	.string "Inside the mansion, you may also\n"
	.string "find some old research that will\l"
	.string "help with your assignment.\p"
	.string "Find it, and use your POKéMON to\n"
	.string "navigate the mansion.\p"
	.string "Finish the sheet, and you may just\n"
	.string "be able to make a TM based on your\l"
	.string "newfound knowledge.\p"
	.string "Oh, and every student got a REMOTE\n"
	.string "DESKTOP. Check your BAG.\p"
	.string "It reaches the POKéMON Storage\n"
	.string "System from anywhere.\p"
	.string "Catch more than you can carry, and\n"
	.string "they go to your boxes. Swap them\l"
	.string "out any time with it.\p"
	.string "Now, you'll need these.$"

Act2_Text_FireSheetIntro:
	.string "PROF. ASHFORD: Let's see your\n"
	.string "research sheet.$"

Act2_Text_FireHint:
	.string "FIRE POKéMON live in the old\n"
	.string "mansion. So do its cold furnaces.$"

Act2_Text_FireResearchDone:
	.string "PROF. ASHFORD: Your research is\n"
	.string "complete! Wonderful notes.\p"
	.string "From all this, we can make a TM:\n"
	.string "FLAMETHROWER.$"

Act2_Text_FireTeachTM:
	.string "Teach FLAMETHROWER to a FIRE\n"
	.string "POKéMON, then use it in my exam.$"

Act2_Text_ReadyForFireExam:
	.string "PROF. ASHFORD: FLAMETHROWER, ready?\n"
	.string "Shall we begin the exam?$"

Act2_Text_FireExamBegins:
	.string "Let's see how hot it burns!$"

Act2_Text_AshfordDefeat:
	.string "Excellent work!$"

Act2_Text_FireExamNoMove:
	.string "PROF. ASHFORD: You won, but you\n"
	.string "never used FLAMETHROWER!\p"
	.string "The exam is about the move.\n"
	.string "Try again when you're ready.$"

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
	.string "Same as before: a field trip.\p"
	.string "Catch a WATER POKéMON, battle with\n"
	.string "it, record WATER moves, and solve\l"
	.string "the site's puzzle.\p"
	.string "Then we make a TM, and the exam.\n"
	.string "Take these. And this ROD.$"

Act2_Text_FieldTripWater:
	.string "We're off to VERMILION CITY!$"

Act2_Text_WaterSheetIntro:
	.string "PROF. MARLOWE: Your research sheet,\n"
	.string "please.$"

Act2_Text_WaterHint:
	.string "Fish at the harbor, or try the\n"
	.string "grass on ROUTE 6.\p"
	.string "And those docks… I smell smoke.$"

Act2_Text_WaterResearchDone:
	.string "PROF. MARLOWE: Splendid research!\p"
	.string "From your notes, we can make a TM:\n"
	.string "BUBBLEBEAM.$"

Act2_Text_WaterTeachTM:
	.string "Teach BUBBLEBEAM to a WATER\n"
	.string "POKéMON, then use it in my exam.$"

Act2_Text_ReadyForWaterExam:
	.string "PROF. MARLOWE: BUBBLEBEAM, ready?\n"
	.string "Shall we begin the exam?$"

Act2_Text_WaterExamBegins:
	.string "Show me what water can do!$"

Act2_Text_MarloweDefeat:
	.string "Splendid. Simply splendid.$"

Act2_Text_WaterExamNoMove:
	.string "PROF. MARLOWE: A win, yes, but no\n"
	.string "BUBBLEBEAM. Try again.$"

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
	.string "course. You know how this goes.\p"
	.string "Catch a GRASS POKéMON, battle with\n"
	.string "it, record GRASS moves, and solve\l"
	.string "the site's puzzle.\p"
	.string "Then a TM, and my exam.\n"
	.string "Take these.$"

Act2_Text_FieldTripGrass:
	.string "Our field trip is to VIRIDIAN CITY!$"

Act2_Text_GrassSheetIntro:
	.string "PROF. FERN: Let's see your research\n"
	.string "sheet.$"

Act2_Text_GrassHint:
	.string "GRASS POKéMON grow on ROUTE 22,\n"
	.string "west of here. Some of its field\l"
	.string "has withered, though…$"

Act2_Text_GrassResearchDone:
	.string "PROF. FERN: Beautiful research.\p"
	.string "From your notes, we can make a TM:\n"
	.string "GIGA DRAIN.$"

Act2_Text_GrassTeachTM:
	.string "Teach GIGA DRAIN to a GRASS\n"
	.string "POKéMON, then use it in my exam.$"

Act2_Text_ReadyForGrassExam:
	.string "PROF. FERN: GIGA DRAIN, ready?\n"
	.string "Shall we begin the exam?$"

Act2_Text_GrassExamBegins:
	.string "Let it grow!$"

Act2_Text_FernDefeat:
	.string "Beautifully done.$"

Act2_Text_GrassExamNoMove:
	.string "PROF. FERN: You won without GIGA\n"
	.string "DRAIN. Patience. Try again.$"

Act2_Text_GrassPassed:
	.string "PROF. FERN: You pass!\p"
	.string "That's every course. Back to the\n"
	.string "university, everyone!$"

Act2_Text_ProfFernAfter:
	.string "PROF. FERN: Your GRASS POKéMON\n"
	.string "looks healthy. Keep it up!$"

Act2_Text_TaskComplete:
	.string "The last research task is done!\n"
	.string "Time to report to the professor.$"

Act2_Text_FurnaceCold:
	.string "An old furnace. It's stone cold.$"

Act2_Text_FurnaceLit:
	.string "The furnace burns brightly.$"

Act2_Text_FurnaceNeedsFire:
	.string "An old, cold furnace.\p"
	.string "A FIRE POKéMON's move could\n"
	.string "relight it.$"

Act2_Text_FurnaceAsk:
	.string "Have {STR_VAR_1} use {STR_VAR_2}\n"
	.string "to relight the furnace?$"

Act2_Text_FurnaceRelit:
	.string "{STR_VAR_1} used {STR_VAR_2}!\n"
	.string "The furnace roars to life!$"

Act2_Text_FurnacesAllLit:
	.string "Somewhere in the mansion, something\n"
	.string "clicks. The statue's eyes glow red!$"

Act2_Text_StatueCold:
	.string "A statue of a POKéMON. Its eyes\n"
	.string "are dark. It feels cold, like the\l"
	.string "furnaces.$"

Act2_Text_MansionJournal:
	.string "An old research journal.\p"
	.string "“The furnaces kept the lab warm\n"
	.string "for our FIRE POKéMON. Heat builds\l"
	.string "in them before a FLAMETHROWER…”\p"
	.string "{PLAYER} copied the notes onto the\n"
	.string "research sheet.$"

Act2_Text_DockFireNeedsWater:
	.string "The dock is on fire!\p"
	.string "A WATER POKéMON's move could\n"
	.string "put it out.$"

Act2_Text_DockFireAsk:
	.string "The dock is on fire! Have\n"
	.string "{STR_VAR_1} use {STR_VAR_2}?$"

Act2_Text_DockFireOut:
	.string "{STR_VAR_1} used {STR_VAR_2}!\n"
	.string "The fire went out!$"

Act2_Text_LaprasSurfaces:
	.string "Something big surfaced by the dock!\n"
	.string "It's a young LAPRAS!$"

Act2_Text_DockLapras:
	.string "The young LAPRAS was hiding under\n"
	.string "the burning docks.\p"
	.string "{PLAYER} recorded it on the\n"
	.string "research sheet!$"

Act2_Text_LaprasWantsToCome:
	.string "The LAPRAS nuzzles {PLAYER}.\n"
	.string "It seems to want to come along!\p"
	.string "Take the LAPRAS with you?$"

Act2_Text_LaprasWontLeave:
	.string "The LAPRAS won't leave\n"
	.string "{PLAYER}'s side…$"

Act2_Text_LaprasJoined:
	.string "The LAPRAS joined {PLAYER}!$"

Act2_Text_WitheredNeedsGrass:
	.string "A patch of withered plants.\p"
	.string "A GRASS POKéMON's move could\n"
	.string "bring it back.$"

Act2_Text_WitheredAsk:
	.string "A withered patch. Have {STR_VAR_1}\n"
	.string "use {STR_VAR_2} on it?$"

Act2_Text_WitheredRegrown:
	.string "{STR_VAR_1} used {STR_VAR_2}!\n"
	.string "New shoots sprang up!$"

Act2_Text_FieldBlooms:
	.string "The whole field burst into bloom!\n"
	.string "GRASS POKéMON gather all around.\p"
	.string "{PLAYER} recorded the bloom on the\n"
	.string "research sheet!$"

Act2_Text_Regrown:
	.string "A young tree, growing strong.$"

Act2_Text_ComeBackWhenReady:
	.string "Come back when you're ready.$"

Act2_Text_ClassKai:
	.string "KAI: There you are! Class is about\n"
	.string "to start. Talk to the professor!$"

Act2_Text_ClassMina:
	.string "MINA: I saved you a seat!\n"
	.string "Well, kind of. We're all standing.$"

Act2_Text_ClassRowan:
	.string "ROWAN: The professor's waiting\n"
	.string "for you.$"

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
