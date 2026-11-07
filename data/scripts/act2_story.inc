@ Act 2's story (docs/hack/act2_story.md). Progress is VAR_ACT2_PROGRESS (ACT2_* in include/constants/story.h).
@ Three courses (Fire, Water, Grass), each a field trip: catch 3 Pokémon of the course's type, then pass the
@ professor's exam. After each, the friends (KAI, MINA, ROWAN) meet the player at the dorms and invite them to the
@ secret battle club at the stadium that night. The third night is the Trainer Card exam; then the final project.

@ Without a Trainer Card, trainers turn the player down (ConfigureTrainerBattle, src/battle_setup.c)
Act2_EventScript_NoTrainerCard::
	lock
	faceplayer
	msgbox Act2_Text_NoTrainerCard
	release
	end

@ The start menu's player entry, before the player has a Trainer Card (StartMenuPlayerNameCallback)
EventScript_NoTrainerCardYet::
	lockall
	msgbox Act2_Text_NoTrainerCardYet
	releaseall
	end

Act2_Text_NoTrainerCardYet:
	.string "This is where a TRAINER CARD\n"
	.string "would go.$"

Act2_EventScript_ShowObjectsTemp1::
	clearflag FLAG_TEMP_1
	return

@ ---------------------------------------------------------------------------------------------------------------
@ The campus: meeting the friends, and the club nights at the stadium

@ FLAG_TEMP_1: the friends at Act 2's start. FLAG_TEMP_2: the friends waiting at the Sports Complex on a club night,
@ FLAG_TEMP_3: DALE on the third. FLAG_TEMP_4: the students there by day (gone at night).
CeladonUniversity_OnTransition::
	special Act2_GiveStartingKit
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	clearflag FLAG_TEMP_4
	call_if_eq VAR_ACT2_PROGRESS, ACT2_START, Act2_EventScript_ShowObjectsTemp1
	call_if_eq VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED, Act2_EventScript_ShowClubFriends
	call_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_INVITED, Act2_EventScript_ShowClubFriends
	call_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED, Act2_EventScript_ShowClubFriends
	call_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED, Act2_EventScript_ShowExaminer
	specialvar VAR_RESULT, Act2_IsClubNight
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_HideDayStudents
	end

Act2_EventScript_ShowClubFriends:
	clearflag FLAG_TEMP_2
	return

Act2_EventScript_ShowExaminer:
	clearflag FLAG_TEMP_3
	return

Act2_EventScript_HideDayStudents:
	setflag FLAG_TEMP_4
	return

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
	@ Until ROWAN, the last, is out of sight (11 steps after its 2-step wait), not the whole walk
	delay 212
	removeobject LOCALID_CAMPUS_KAI
	removeobject LOCALID_CAMPUS_MINA
	removeobject LOCALID_CAMPUS_ROWAN
	setvar VAR_ACT2_PROGRESS, ACT2_MET_FRIENDS
	releaseall
	end

@ Off toward the MAIN HALL: from outside the annex (the player at (56,58)) west along the dorms' path to x 45,
@ then up it toward the avenue, until well out of sight. ROWAN, east of the player, goes round below.
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
	walk_up
	walk_up
	step_end

Act2_Movement_MinaToClass:
	delay_8
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
	walk_up
	walk_up
	step_end

Act2_Movement_RowanToClass:
	delay_16
	delay_16
	walk_down
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

@ Walking into the Sports Complex on a club night (coord events at its gate): the friends are waiting
Act2_EventScript_ClubArrive:
	applymovement LOCALID_PLAYER, Common_Movement_WalkUp2
	waitmovement 0
	return

@ After a club night, with the next story stage in VAR_0x8008: "The next day…" on black, then the player wakes up in
@ their room. The stage only moves on once the screen is black: it's night until then (Act2_IsClubNight).
Act2_EventScript_ClubMorning:
	closemessage
	fadescreen FADE_TO_BLACK
	copyvar VAR_ACT2_PROGRESS, VAR_0x8008
	special HealPlayerParty
	special Act2_ShowNextDayCard
	waitstate
	releaseall
	end

@ Walking into the stadium (coord events across its entrance, VAR_TEMP_1 0): on a club night, that night's
@ arrival; otherwise nothing
Act2_EventScript_ClubGate::
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED, Act2_EventScript_ClubNight1
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_INVITED, Act2_EventScript_ClubNight2
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED, Act2_EventScript_ClubNight3
	end

@ Arriving on a club night (coord events at the gate): the friends explain tonight's battle, then the player can line
@ up their team and talk to the night's opponent (or any friend) when ready. ACT2_RESEARCH_CLUB_ARRIVED: explained.
Act2_EventScript_ClubNight1::
	call Act2_EventScript_CheckClubArrived
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night1
	msgbox Act2_Text_ClubRules
	goto Act2_EventScript_ClubArrived

Act2_EventScript_ClubNight2::
	call Act2_EventScript_CheckClubArrived
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night2
	msgbox Act2_Text_ClubRulesReminder
	goto Act2_EventScript_ClubArrived

@ The third night: an examiner from the POKéMON LEAGUE, and a real Trainer Card
Act2_EventScript_ClubNight3::
	call Act2_EventScript_CheckClubArrived
	lockall
	call Act2_EventScript_ClubArrive
	msgbox Act2_Text_Night3
	msgbox Act2_Text_DaleIntro
	msgbox Act2_Text_ClubRulesExaminer
	goto Act2_EventScript_ClubArrived

Act2_EventScript_CheckClubArrived:
	setvar VAR_0x8004, ACT2_RESEARCH_CLUB_ARRIVED
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_ClubEnd
	return

Act2_EventScript_ClubArrived:
	setvar VAR_0x8004, ACT2_RESEARCH_CLUB_ARRIVED
	special Act2_SetResearchFlag
	releaseall
Act2_EventScript_ClubEnd:
	end

@ Talking to a friend (or DALE) at the Sports Complex on a club night
Act2_EventScript_ClubTalk::
	lock
	faceplayer
	setvar VAR_0x8004, ACT2_RESEARCH_CLUB_ARRIVED
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_ClubComeIn
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED, Act2_EventScript_ClubBattle1
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_INVITED, Act2_EventScript_ClubBattle2
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED, Act2_EventScript_ClubBattle3
	release
	end

Act2_EventScript_ClubComeIn:
	msgbox Act2_Text_ClubComeIn
	release
	end

@ The team: the first three in the party able to battle. YES in VAR_RESULT to battle.
Act2_EventScript_ClubConfirmTeam:
	specialvar VAR_RESULT, Act2_BufferClubTeam
	msgbox gStringVar4
	goto_if_eq VAR_RESULT, 0, Act2_EventScript_ClubNoTeam
	msgbox Act2_Text_ClubReady, MSGBOX_YESNO
	return

@ No one able to battle: as if the player said not yet
Act2_EventScript_ClubNoTeam:
	setvar VAR_RESULT, NO
	return

Act2_EventScript_ClubNotYet:
	msgbox Act2_Text_ClubNotYet
	release
	end

@ A loss doesn't stop the story: the player can go again, or call it a night
Act2_EventScript_ClubGoAgain:
	msgbox Act2_Text_ClubGoAgain, MSGBOX_YESNO
	return

Act2_EventScript_ClubBattle1:
	call Act2_EventScript_ClubConfirmTeam
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ClubNotYet
	msgbox Act2_Text_KaiStrategy
	closemessage
	special Act2_SetAsideClubBench
	trainerbattle_earlyrival TRAINER_ACT2_KAI, RIVAL_BATTLE_HEAL_AFTER, Act2_Text_KaiDefeat, Act2_Text_KaiWins
	special Act2_RestoreClubBench
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_ClubWon1
	msgbox Act2_Text_KaiAfterLoss
	call Act2_EventScript_ClubGoAgain
	goto_if_eq VAR_RESULT, YES, Act2_EventScript_ClubBattle1
Act2_EventScript_ClubWon1:
	msgbox Act2_Text_Night1After
	setvar VAR_0x8008, ACT2_CLUB_1_DONE
	goto Act2_EventScript_ClubMorning

Act2_EventScript_ClubBattle2:
	call Act2_EventScript_ClubConfirmTeam
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ClubNotYet
	msgbox Act2_Text_MinaStrategy
	closemessage
	special Act2_SetAsideClubBench
	trainerbattle_earlyrival TRAINER_ACT2_MINA, RIVAL_BATTLE_HEAL_AFTER, Act2_Text_MinaDefeat, Act2_Text_MinaWins
	special Act2_RestoreClubBench
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_ClubWon2
	msgbox Act2_Text_MinaAfterLoss
	call Act2_EventScript_ClubGoAgain
	goto_if_eq VAR_RESULT, YES, Act2_EventScript_ClubBattle2
Act2_EventScript_ClubWon2:
	msgbox Act2_Text_Night2After
	setvar VAR_0x8008, ACT2_CLUB_2_DONE
	goto Act2_EventScript_ClubMorning

Act2_EventScript_ClubBattle3:
	call Act2_EventScript_ClubConfirmTeam
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ClubNotYet
	msgbox Act2_Text_DaleStrategy
	closemessage
	special Act2_SetAsideClubBench
	trainerbattle_earlyrival TRAINER_ACT2_EXAMINER, RIVAL_BATTLE_HEAL_AFTER, Act2_Text_DaleDefeat, Act2_Text_DaleWins
	special Act2_RestoreClubBench
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_ClubWon3
	msgbox Act2_Text_DaleAfterLoss
	call Act2_EventScript_ClubGoAgain
	goto_if_eq VAR_RESULT, YES, Act2_EventScript_ClubBattle3
	msgbox Act2_Text_DalePassesAnyway
	goto Act2_EventScript_ClubTrainerCard
Act2_EventScript_ClubWon3:
	msgbox Act2_Text_DalePasses
Act2_EventScript_ClubTrainerCard:
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Act2_Text_GotTrainerCard
	waitmessage
	waitfanfare
	msgbox Act2_Text_Night3After
	setvar VAR_0x8008, ACT2_GOT_TRAINER_CARD
	goto Act2_EventScript_ClubMorning

@ ---------------------------------------------------------------------------------------------------------------
@ The friends' dorm rooms (KAI and MINA on 2F, ROWAN on 3F). Back from the Fire course it's night: KAI asked the
@ player to come by, says where the club meets, then heads there. After the other courses, the friends just say
@ they'll see the player there (ACT2_WATER_INVITED, ACT2_GRASS_INVITED). Night lasts until the club battle
@ (Act2_IsClubNight). ACT2_WATER_SEE_MINA and ACT2_GRASS_SEE_ROWAN are only for saves already at them.

CeladonUniversity_Dorm2F_OnTransition::
	setflag FLAG_TEMP_5
	setflag FLAG_TEMP_6
	setvar VAR_0x8004, ACT2_FIRE_SEE_KAI
	call Act2_EventScript_CheckFriendInRoom
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_ShowTemp5
	setvar VAR_0x8004, ACT2_WATER_SEE_MINA
	call Act2_EventScript_CheckFriendInRoom
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_ShowTemp6
	end

CeladonUniversity_Dorm3F_OnTransition::
	setflag FLAG_TEMP_5
	setvar VAR_0x8004, ACT2_GRASS_SEE_ROWAN
	call Act2_EventScript_CheckFriendInRoom
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_ShowTemp5
	end

@ A friend is in their room on the night they asked the player over (VAR_0x8004), and once the courses are done
Act2_EventScript_CheckFriendInRoom:
	setvar VAR_RESULT, FALSE
	call_if_eq VAR_ACT2_PROGRESS, VAR_0x8004, Act2_EventScript_SetResultTrue
	call_if_ge VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD, Act2_EventScript_SetResultTrue
	return

Act2_EventScript_SetResultTrue:
	setvar VAR_RESULT, TRUE
	return

Act2_EventScript_ShowTemp5:
	clearflag FLAG_TEMP_5
	return

Act2_EventScript_ShowTemp6:
	clearflag FLAG_TEMP_6
	return

Act2_EventScript_RoomKai::
	lock
	faceplayer
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_FIRE_SEE_KAI, Act2_EventScript_RoomKaiDay
	msgbox Act2_Text_RoomKaiInvite
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_INVITED
	setvar VAR_0x8005, LOCALID_DORM_KAI
	goto Act2_EventScript_FriendHeadsOut

Act2_EventScript_RoomKaiDay:
	msgbox Act2_Text_RoomKai
	release
	end

Act2_EventScript_RoomMina::
	lock
	faceplayer
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_WATER_SEE_MINA, Act2_EventScript_RoomMinaDay
	msgbox Act2_Text_RoomMinaInvite
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_INVITED
	setvar VAR_0x8005, LOCALID_DORM_MINA
	goto Act2_EventScript_FriendHeadsOut

Act2_EventScript_RoomMinaDay:
	msgbox Act2_Text_RoomMina
	release
	end

Act2_EventScript_RoomRowan::
	lock
	faceplayer
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_GRASS_SEE_ROWAN, Act2_EventScript_RoomRowanDay
	msgbox Act2_Text_RoomRowanInvite
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED
	setvar VAR_0x8005, LOCALID_DORM_ROWAN
	goto Act2_EventScript_FriendHeadsOut

Act2_EventScript_RoomRowanDay:
	msgbox Act2_Text_RoomRowan
	release
	end

@ The friend (VAR_0x8005) heads off to the Sports Complex
Act2_EventScript_FriendHeadsOut:
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_0x8005
	fadescreen FADE_FROM_BLACK
	msgbox Act2_Text_FriendHeadedOut
	release
	end

Act2_EventScript_RoomPlateKai::
	msgbox Act2_Text_RoomPlateKai, MSGBOX_SIGN
	end

Act2_EventScript_RoomPlateMina::
	msgbox Act2_Text_RoomPlateMina, MSGBOX_SIGN
	end

Act2_EventScript_RoomPlateRowan::
	msgbox Act2_Text_RoomPlateRowan, MSGBOX_SIGN
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The courses. Each professor starts their course on campus: what to do, Poké Balls, then the field trip. There,
@ the professor's research sheet: catch 3 different Pokémon of the type, use a move of it, record 3 different moves of it,
@ and solve the site's puzzle. With all four done, the professor helps make the course's TM, then the exam
@ (src/act2_story.c tracks all this).

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
	setvar VAR_ACT2_MOVE_SEEN_1, MOVE_NONE
	setvar VAR_ACT2_MOVE_SEEN_2, MOVE_NONE
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
	call Act2_EventScript_RefillBalls
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
	msgbox Act2_Text_ReadyForFireExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_FireExamBegins
	closemessage
	cleartrainerflag TRAINER_ACT2_PROF_FIRE
	trainerbattle_no_intro TRAINER_ACT2_PROF_FIRE, Act2_Text_AshfordDefeat
	msgbox Act2_Text_FirePassed
	msgbox Act2_Text_FireComeSeeKai
	setvar VAR_ACT2_PROGRESS, ACT2_FIRE_SEE_KAI
	goto Act2_EventScript_BackToDorms

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
	closemessage
	setvar VAR_ACT2_RESEARCH, 0
	setvar VAR_ACT2_MOVE_SEEN_1, MOVE_NONE
	setvar VAR_ACT2_MOVE_SEEN_2, MOVE_NONE
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
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_WaterAfterTM
	specialvar VAR_RESULT, Act2_IsReadyForTM
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_WaterMakeTM
	msgbox Act2_Text_WaterSheetIntro
	special Act2_BufferResearchSheet
	msgbox gStringVar4
	setvar VAR_0x8004, ACT2_RESEARCH_DOCK_FIRE
	specialvar VAR_RESULT, Act2_HasResearchFlag
	call_if_eq VAR_RESULT, FALSE, Act2_EventScript_WaterHint
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_WaterHintFire
	call Act2_EventScript_RefillBalls
	release
	end

Act2_EventScript_WaterHint:
	msgbox Act2_Text_WaterHint
	return

Act2_EventScript_WaterHintFire:
	msgbox Act2_Text_WaterHintFire
	return

@ The TM is made from the first three tasks; it's what puts out the big dock fire, the last one
Act2_EventScript_WaterMakeTM:
	msgbox Act2_Text_WaterResearchDone
	giveitem ITEM_TM_BUBBLE_BEAM
	setvar VAR_0x8004, ACT2_RESEARCH_GOT_TM
	special Act2_SetResearchFlag
	msgbox Act2_Text_WaterTeachTM
	goto Act2_EventScript_WaterAfterTM

@ The exam only once every task is done, the big dock fire included
Act2_EventScript_WaterAfterTM:
	specialvar VAR_RESULT, Act2_IsResearchDone
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_WaterLastFire
	goto_if_unset FLAG_ACT2_GOT_LAPRAS, Act2_EventScript_WaterSeeLapras
	goto Act2_EventScript_WaterExam

Act2_EventScript_WaterLastFire:
	msgbox Act2_Text_WaterLastFire
	release
	end

@ The fires are out, but the young Lapras is still by the pier: no exam until it's been looked after
Act2_EventScript_WaterSeeLapras:
	msgbox Act2_Text_WaterSeeLapras
	release
	end

Act2_EventScript_WaterExam:
	msgbox Act2_Text_WaterAllDone
	msgbox Act2_Text_ReadyForWaterExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_WaterExamBegins
	closemessage
	cleartrainerflag TRAINER_ACT2_PROF_WATER
	trainerbattle_no_intro TRAINER_ACT2_PROF_WATER, Act2_Text_MarloweDefeat
	msgbox Act2_Text_WaterPassed
	msgbox Act2_Text_WaterSeeYouTonight
	setvar VAR_ACT2_PROGRESS, ACT2_WATER_INVITED
	goto Act2_EventScript_BackToDorms

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
	closemessage
	setvar VAR_ACT2_RESEARCH, 0
	setvar VAR_ACT2_MOVE_SEEN_1, MOVE_NONE
	setvar VAR_ACT2_MOVE_SEEN_2, MOVE_NONE
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
	call Act2_EventScript_RefillBalls
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
	msgbox Act2_Text_ReadyForGrassExam, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_ComeBackWhenReady
	msgbox Act2_Text_GrassExamBegins
	closemessage
	cleartrainerflag TRAINER_ACT2_PROF_GRASS
	trainerbattle_no_intro TRAINER_ACT2_PROF_GRASS, Act2_Text_FernDefeat
	msgbox Act2_Text_GrassPassed
	msgbox Act2_Text_GrassSeeYouTonight
	setvar VAR_ACT2_PROGRESS, ACT2_GRASS_INVITED
	goto Act2_EventScript_BackToDorms

Act2_EventScript_ComeBackWhenReady:
	msgbox Act2_Text_ComeBackWhenReady
	release
	end

@ A passed exam: the class heads home, and the player ends up back in their room. It's night now (Act2_IsClubNight).
Act2_EventScript_BackToDorms:
	closemessage
	setrespawn HEAL_LOCATION_CELADON_CITY
	special HealPlayerParty
	warp MAP_CELADON_UNIVERSITY_DORM_ROOM, 2, 3
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
@ On arriving at the site, the professor explains the assignment (once: ACT2_RESEARCH_BRIEFED). With the trip stage
@ in VAR_0x8004: VAR_TEMP_1 is 1 while the briefing is due, for the site's frame script.
Act2_EventScript_CheckBriefing:
	setvar VAR_TEMP_1, 0
	goto_if_ne VAR_ACT2_PROGRESS, VAR_0x8004, Common_EventScript_NopReturn
	setvar VAR_0x8004, ACT2_RESEARCH_BRIEFED
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_NopReturn
	setvar VAR_TEMP_1, 1
	return

@ A puzzle's task just got done: whether that finishes the research sheet
Act2_EventScript_TaskDone:
	specialvar VAR_RESULT, Act2_IsResearchDone
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_TaskDoneMoreToGo
	msgbox Act2_Text_TaskComplete
	return

Act2_EventScript_TaskDoneMoreToGo:
	msgbox Act2_Text_TaskDoneMoreToGo
	return

@ Out of POKé BALLS on a field trip: the professor hands out more
Act2_EventScript_RefillBalls:
	checkitem ITEM_POKE_BALL
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_RefillBallsEnd
	msgbox Act2_Text_OutOfBalls
	giveitem ITEM_POKE_BALL, 5
Act2_EventScript_RefillBallsEnd:
	return

Act2_EventScript_Briefed:
	setvar VAR_0x8004, ACT2_RESEARCH_BRIEFED
	special Act2_SetResearchFlag
	setvar VAR_TEMP_1, 0
	return

CinnabarIsland_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_FIRE_TRIP
	special Act2_SetupSiteObjects
	setvar VAR_0x8004, ACT2_FIRE_TRIP
	goto Act2_EventScript_CheckBriefing

CinnabarIsland_OnFrame::
	map_script_2 VAR_TEMP_1, 1, Act2_EventScript_FireBriefing
	.2byte 0

VermilionCity_OnFrame::
	map_script_2 VAR_TEMP_1, 1, Act2_EventScript_WaterBriefing
	map_script_2 VAR_TEMP_2, 1, VermilionCity_EventScript_DockFireAlarm
	map_script_2 VAR_TEMP_3, 1, VermilionCity_EventScript_DockFireShow
	.2byte 0

Route6_OnFrame::
	map_script_2 VAR_TEMP_2, 1, Route6_EventScript_DockFireAlarm
	.2byte 0

ViridianCity_OnFrame::
	map_script_2 VAR_TEMP_1, 1, Act2_EventScript_GrassBriefing
	.2byte 0

@ The class arrives on Cinnabar: PROF. ASHFORD explains the assignment
Act2_EventScript_FireBriefing::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_CINNABAR_PROF, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	msgbox Act2_Text_FireBriefing
	giveitem ITEM_POKE_BALL, 5
	call Act2_EventScript_Briefed
	releaseall
	end

@ Vermilion: PROF. MARLOWE stands left of the arrival spot, Viridian: PROF. FERN too
Act2_EventScript_WaterBriefing::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_VERMILION_PROF, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox Act2_Text_WaterBriefing
	giveitem ITEM_POKE_BALL, 5
	giveitem ITEM_GOOD_ROD
	msgbox Act2_Text_WaterBriefingEnd
	call Act2_EventScript_Briefed
	releaseall
	end

Act2_EventScript_GrassBriefing::
	lockall
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_VIRIDIAN_PROF, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox Act2_Text_GrassBriefing
	giveitem ITEM_POKE_BALL, 5
	call Act2_EventScript_Briefed
	releaseall
	end

PokemonMansion_1F_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_FIRE_TRIP
	special Act2_SetupSiteObjects
	return

VermilionCity_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_WATER_TRIP
	special Act2_SetupSiteObjects
	setvar VAR_0x8004, ACT2_WATER_TRIP
	goto Act2_EventScript_CheckBriefing

ViridianCity_EventScript_Act2Objects::
	setvar VAR_0x8004, ACT2_GRASS_TRIP
	special Act2_SetupSiteObjects
	setvar VAR_0x8004, ACT2_GRASS_TRIP
	goto Act2_EventScript_CheckBriefing

Route22_OnTransition::
	setvar VAR_0x8004, ACT2_GRASS_TRIP
	special Act2_SetupSiteObjects
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The site puzzles: each part needs a Pokémon of the course's type that knows a move of it (Act2_FindCourseMoveUser)

@ Fire: three cold furnaces in the Pokémon Mansion (1F). Lit, they wake the statue switch, which opens the sealed
@ room with the old research journal.
Act2_EventScript_Furnace1::
	call Act2_EventScript_Furnace1Spot
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace1Spot:
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 6
	setvar VAR_0x8006, 2
	return

Act2_EventScript_Furnace2::
	call Act2_EventScript_Furnace2Spot
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace2Spot:
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 31
	setvar VAR_0x8006, 2
	return

Act2_EventScript_Furnace3::
	call Act2_EventScript_Furnace3Spot
	goto Act2_EventScript_Furnace

Act2_EventScript_Furnace3Spot:
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 4
	setvar VAR_0x8006, 12
	return

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
	setmetatile VAR_0x8005, VAR_0x8006, METATILE_PokemonMansion_Furnace_Lit, TRUE
	special DrawWholeMapView
	msgbox Act2_Text_FurnaceRelit
	waitse
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_FurnacesAllLit
	releaseall
	end

@ On the Mansion's load (1F): a furnace relit on this trip still burns
Act2_EventScript_DrawLitFurnaces::
	goto_if_ne VAR_ACT2_PROGRESS, ACT2_FIRE_TRIP, Act2_EventScript_DrawLitFurnacesEnd
	call Act2_EventScript_Furnace1Spot
	call Act2_EventScript_DrawFurnaceIfLit
	call Act2_EventScript_Furnace2Spot
	call Act2_EventScript_DrawFurnaceIfLit
	call Act2_EventScript_Furnace3Spot
	call Act2_EventScript_DrawFurnaceIfLit
Act2_EventScript_DrawLitFurnacesEnd:
	return

Act2_EventScript_DrawFurnaceIfLit:
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DrawLitFurnacesEnd
	setmetatile VAR_0x8005, VAR_0x8006, METATILE_PokemonMansion_Furnace_Lit, TRUE
	return

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

@ It's in the sealed room from the start of the trip (Act2_SetupSiteObjects). The other floors' switches can open
@ that room's gate too, so before the furnaces are lit its pages are still stuck together.
Act2_EventScript_MansionJournal::
	lock
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_MansionJournalCold
	msgbox Act2_Text_MansionJournal
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	removeobject LOCALID_MANSION_JOURNAL
	call Act2_EventScript_TaskDone
	release
	end

Act2_EventScript_MansionJournalCold:
	msgbox Act2_Text_MansionJournalCold
	release
	end

@ Water: the docks catch fire once the player catches a WATER POKéMON on the trip (Act2_OnCaughtMon). MINA finds
@ the player and takes them to see it. The two small fires go out with any WATER move and finish the puzzle task; the
@ big one, which cuts off the west end of the pier, only with the course's TM move. Behind it, a young Lapras.
@ The fires are metatiles (Act2_DrawDockFires); each square has a sign event for its fire.
Act2_EventScript_DockFire1::
	setvar VAR_0x8004, 1
	goto Act2_EventScript_SmallDockFire

Act2_EventScript_DockFire2::
	setvar VAR_0x8004, 2
	goto Act2_EventScript_SmallDockFire

Act2_EventScript_SmallDockFire:
	specialvar VAR_RESULT, Act2_IsDockFireBurningSpecial
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireEnd
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireNeedsWater
	msgbox Act2_Text_DockFireAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	call Act2_EventScript_DouseDockFire
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	goto Act2_EventScript_DockFiresCheckOut

@ The big fire: only the TM move puts it out
Act2_EventScript_DockFire3::
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, Act2_IsDockFireBurningSpecial
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireEnd
	lockall
	specialvar VAR_RESULT, Act2_FindTmMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_BigDockFireTooStrong
	msgbox Act2_Text_DockFireAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_PuzzleRelease
	call Act2_EventScript_DouseDockFire
	specialvar VAR_RESULT, Act2_SolvePuzzlePart
	call Act2_EventScript_LaprasSurfaces
	goto Act2_EventScript_DockFiresCheckOut

@ After a fire goes out: with the small ones out, the big one still burning or, all out, the task is done. The young
@ Lapras comes first: the player is sent to it, not the professor, until it has come along.
Act2_EventScript_DockFiresCheckOut:
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_PuzzleRelease
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_PuzzleRelease
	setvar VAR_0x8004, 3
	specialvar VAR_RESULT, Act2_IsPuzzlePartSolved
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_SmallDockFiresOut
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	goto_if_set FLAG_ACT2_GOT_LAPRAS, Act2_EventScript_DockFiresAllOut
	msgbox Act2_Text_DockFiresOutSeeLapras
	releaseall
	end

Act2_EventScript_DockFiresAllOut:
	call Act2_EventScript_TaskDone
	releaseall
	end

Act2_EventScript_SmallDockFiresOut:
	msgbox Act2_Text_SmallDockFiresOut
	releaseall
	end

@ With the fire in VAR_0x8004
Act2_EventScript_DouseDockFire:
	playse SE_M_BUBBLE_BEAM
	special Act2_PutOutDockFire
	special DrawWholeMapView
	msgbox Act2_Text_DockFireOut
	waitse
	return

Act2_EventScript_DockFireNeedsWater:
	msgbox Act2_Text_DockFireNeedsWater
	releaseall
	end

Act2_EventScript_BigDockFireTooStrong:
	msgbox Act2_Text_BigDockFireTooStrong
	releaseall
	end

Act2_EventScript_DockFireEnd:
	end

Act2_EventScript_LaprasSurfaces:
	playse SE_M_SURF
	clearflag FLAG_TEMP_8
	addobject LOCALID_VERMILION_LAPRAS
	waitse
	msgbox Act2_Text_LaprasSurfaces
	return

Act2_EventScript_OnResumeDockFire::
	call Act2_EventScript_CheckDockFireAlarm
	end

@ After a battle (map resume) on ROUTE 6 or in VERMILION: the docks are burning and no one has shown the player yet.
@ VAR_TEMP_2 is 1 for the map's frame script.
Act2_EventScript_CheckDockFireAlarm::
	setvar VAR_TEMP_2, 0
	specialvar VAR_RESULT, Act2_IsDockFireAlarmDue
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	setvar VAR_TEMP_2, 1
	return

@ MINA runs up to the player from out of view (on ROUTE 6, her object waits hidden; in VERMILION she's already on the
@ map). She ends on a land square beside them (Act2_FindSpotBesidePlayer: behind, left, right, then in front; never
@ water), facing them; with no free square, the player is just told and taken to the docks.
Route6_EventScript_DockFireAlarm::
	lockall
	setvar VAR_TEMP_2, 0
	specialvar VAR_RESULT, Act2_FindSpotBesidePlayer
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireAlarm
	setobjectxyperm LOCALID_ROUTE6_MINA, VAR_0x8005, VAR_0x8006
	clearflag FLAG_TEMP_2
	addobject LOCALID_ROUTE6_MINA
	setvar VAR_0x8009, LOCALID_ROUTE6_MINA
	call Act2_EventScript_FriendRunsUp
	goto Act2_EventScript_DockFireAlarm

VermilionCity_EventScript_DockFireAlarm::
	lockall
	setvar VAR_TEMP_2, 0
	specialvar VAR_RESULT, Act2_FindSpotBesidePlayer
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_DockFireAlarm
	setobjectxy LOCALID_VERMILION_MINA, VAR_0x8005, VAR_0x8006
	setvar VAR_0x8009, LOCALID_VERMILION_MINA
	call Act2_EventScript_FriendRunsUp
	goto Act2_EventScript_DockFireAlarm

@ The friend in VAR_0x8009 runs up beside the player, who turns to them
Act2_EventScript_FriendRunsUp:
	playse SE_PIN
	special Act2_WalkFriendToPlayer
	waitmovement VAR_0x8009
	call Act2_EventScript_FriendFacesPlayer
	return

@ The friend in VAR_0x8009 faces the way in VAR_0x8007, and the player faces back
Act2_EventScript_FriendFacesPlayer:
	call_if_eq VAR_0x8007, DIR_NORTH, Act2_EventScript_FriendFacesUp
	call_if_eq VAR_0x8007, DIR_SOUTH, Act2_EventScript_FriendFacesDown
	call_if_eq VAR_0x8007, DIR_WEST, Act2_EventScript_FriendFacesLeft
	call_if_eq VAR_0x8007, DIR_EAST, Act2_EventScript_FriendFacesRight
	waitmovement 0
	return

Act2_EventScript_FriendFacesUp:
	applymovement VAR_0x8009, Common_Movement_WalkInPlaceFasterUp
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	return

Act2_EventScript_FriendFacesDown:
	applymovement VAR_0x8009, Common_Movement_WalkInPlaceFasterDown
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	return

Act2_EventScript_FriendFacesLeft:
	applymovement VAR_0x8009, Common_Movement_WalkInPlaceFasterLeft
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	return

Act2_EventScript_FriendFacesRight:
	applymovement VAR_0x8009, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	return

Act2_EventScript_DockFireAlarm:
	msgbox Act2_Text_DockFireAlarm
	closemessage
	setvar VAR_0x8004, ACT2_RESEARCH_DOCK_FIRE_SEEN | ACT2_RESEARCH_DOCK_FIRE_ARRIVING
	special Act2_SetResearchFlag
	fadescreen FADE_TO_BLACK
	warp MAP_VERMILION_CITY, 32, 29
	waitstate
	releaseall
	end

@ Arriving at the docks with MINA (VERMILION's transition): she stands beside the player; VAR_TEMP_3 is 1 for the
@ frame script that shows them the fire
VermilionCity_EventScript_DockFireArrival::
	setvar VAR_TEMP_3, 0
	setvar VAR_0x8004, ACT2_RESEARCH_DOCK_FIRE_ARRIVING
	specialvar VAR_RESULT, Act2_HasResearchFlag
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	setobjectxyperm LOCALID_VERMILION_MINA, 33, 29
	setobjectmovementtype LOCALID_VERMILION_MINA, MOVEMENT_TYPE_FACE_LEFT
	setvar VAR_TEMP_3, 1
	return

VermilionCity_EventScript_DockFireShow::
	lockall
	setvar VAR_TEMP_3, 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	msgbox Act2_Text_DockFireShow
	setvar VAR_0x8004, ACT2_RESEARCH_DOCK_FIRE_ARRIVING
	special Act2_ClearResearchFlag
	releaseall
	end

@ The dock workers (and the sailors of the harbor) on the Water trip, once the fires have started
Act2_EventScript_DockWorker::
	specialvar VAR_RESULT, Act2_GetDockFireState
	goto_if_eq VAR_RESULT, DOCK_FIRE_BURNING, Act2_EventScript_DockWorkerFire
	goto_if_eq VAR_RESULT, DOCK_FIRE_OUT, Act2_EventScript_DockWorkerThanks
	msgbox Act2_Text_DockWorkerQuiet, MSGBOX_NPC
	end

Act2_EventScript_DockWorkerFire:
	msgbox Act2_Text_DockWorkerFire, MSGBOX_NPC
	end

Act2_EventScript_DockWorkerThanks:
	msgbox Act2_Text_DockWorkerThanks, MSGBOX_NPC
	end

@ The young Lapras: once the big fire is out, it asks to come along (it won't take no for an
@ answer). In Act 3, it's the Lapras in Silph Co.'s lab.
@ The Lapras floats on the water, a lower elevation than the pier, so the pier's player talks to it through a sign
@ event on its square (elevation 0) instead, while it's there
Act2_EventScript_DockLaprasFromPier::
	goto_if_set FLAG_TEMP_8, Act2_EventScript_DockFireEnd
	goto_if_set FLAG_ACT2_GOT_LAPRAS, Act2_EventScript_DockFireEnd
	setvar VAR_LAST_TALKED, LOCALID_VERMILION_LAPRAS
	goto Act2_EventScript_DockLapras

Act2_EventScript_DockLapras::
	lock
	faceplayer
	playmoncry SPECIES_LAPRAS, CRY_MODE_NORMAL
	msgbox Act2_Text_DockLapras
	waitmoncry
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

@ On the trip, with the fires out, the research is done: now it's time for the professor
Act2_EventScript_LaprasDone:
	specialvar VAR_RESULT, Act2_IsResearchDone
	call_if_eq VAR_RESULT, TRUE, Act2_EventScript_LaprasReportToProf
	release
	end

Act2_EventScript_LaprasReportToProf:
	msgbox Act2_Text_LaprasReportToProf
	return

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
	call Act2_EventScript_TaskDone
	return

Act2_EventScript_WitheredNeedsGrass:
	msgbox Act2_Text_WitheredNeedsGrass
	releaseall
	end

Act2_EventScript_Regrown::
	msgbox Act2_Text_Regrown, MSGBOX_SIGN
	end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_NoTrainerCard:
	.string "You want to battle?\n"
	.string "Let me see your TRAINER CARD.\p"
	.string "…You don't have one?\n"
	.string "Sorry, I only battle TRAINERS.$"

Act2_Text_MeetFriends:
	.string "KAI: Morning! Can you believe it's\n"
	.string "our final year?\p"
	.string "MINA: And we're all in the FIRE\n"
	.string "course together. The field\l"
	.string "assignments start today!\p"
	.string "ROWAN: PROF. ASHFORD's class is in\n"
	.string "LECTURE HALL A, the MAIN HALL's\l"
	.string "west wing. Let's go!$"

Act2_Text_Night1:
	.string "KAI: Welcome to the club!\n"
	.string "No professors allowed.\p"
	.string "Officially, we're not TRAINERS.\n"
	.string "Unofficially… let's battle!$"

Act2_Text_FireComeSeeKai:
	.string "KAI: We passed! Hey, come see me\n"
	.string "in my dorm room when we get back.\p"
	.string "2F, the first room. Don't forget!$"

Act2_Text_WaterSeeYouTonight:
	.string "MINA: Two courses down!\p"
	.string "See you at the meeting spot\n"
	.string "tonight. You know where it is!$"

Act2_Text_GrassSeeYouTonight:
	.string "ROWAN: That's all three courses.\p"
	.string "See you at the meeting spot\n"
	.string "tonight. You know where it is.\p"
	.string "And come ready. We've invited\n"
	.string "someone special.$"

Act2_Text_RoomKaiInvite:
	.string "KAI: You came! Okay, here's the\n"
	.string "secret: some of us have a club.\p"
	.string "We battle at night, at the SPORTS\n"
	.string "COMPLEX. No professors, no rules.\p"
	.string "The others are already there.\n"
	.string "Come on, let's go!$"

Act2_Text_RoomMinaInvite:
	.string "MINA: Club night! Everyone's\n"
	.string "meeting at the SPORTS COMPLEX.\p"
	.string "I'll see you there!$"

Act2_Text_RoomRowanInvite:
	.string "ROWAN: Tonight's the big one.\n"
	.string "We invited someone from the\l"
	.string "POKéMON LEAGUE.\p"
	.string "Win, and you get a real TRAINER\n"
	.string "CARD. Meet us at the SPORTS\l"
	.string "COMPLEX!$"

Act2_Text_FriendHeadedOut:
	.string "Off to the SPORTS COMPLEX…$"

Act2_Text_RoomKai:
	.string "KAI: Welcome to my room!\n"
	.string "Don't mind the mess.\p"
	.string "My dads live across town, in the\n"
	.string "CONDOMINIUMS. They still drop off\l"
	.string "food every week. Too much food.$"

Act2_Text_RoomMina:
	.string "MINA: My room! That poster is a\n"
	.string "STARYU. My favorite.\p"
	.string "Back home in CERULEAN, Mom and I\n"
	.string "used to count them in the river.$"

Act2_Text_RoomRowan:
	.string "ROWAN: Shh, I'm reading.\n"
	.string "Kidding. Come in!\p"
	.string "My parents live across town, by\n"
	.string "the GAME CORNER. Mom keeps\l"
	.string "sending me seedlings.$"

Act2_Text_RoomPlateKai:
	.string "KAI's room.$"

Act2_Text_RoomPlateMina:
	.string "MINA's room.$"

Act2_Text_RoomPlateRowan:
	.string "ROWAN's room.$"

Act2_Text_ClubRules:
	.string "KAI: None of us have our TRAINER\n"
	.string "CARD, so we had to figure out\l"
	.string "our own rules.\p"
	.string "Three POKéMON a side, all on the\n"
	.string "field at once. Three on three!\p"
	.string "MINA: You'll have to be smart.\n"
	.string "Only one POKéMON gets to act\l"
	.string "before your opponent responds.\p"
	.string "ROWAN: I recommend setting your\n"
	.string "team up in formation: front,\l"
	.string "middle and back.\p"
	.string "Defense in front, offense in back.\n"
	.string "That way, direct attacks hit your\l"
	.string "strong defense first.\p"
	.string "MINA: That's why I always win,\n"
	.string "ROWAN. My indirect attacks bypass\l"
	.string "your front line and hit your\l"
	.string "damage dealers in back!\p"
	.string "Remember, you can change the order\n"
	.string "of your team at any time without\l"
	.string "losing your chance to act.\p"
	.string "KAI: {PLAYER}, don't forget to study\n"
	.string "your moves before you act.\p"
	.string "Direct attacks like TACKLE always\n"
	.string "hit the front POKéMON, blocking\l"
	.string "the others.\p"
	.string "Indirect attacks, like electric,\n"
	.string "psychic and flying ones, can hit\l"
	.string "any POKéMON on the field.\p"
	.string "ROWAN: If you forget how they work\n"
	.string "like I do, every move is labeled.\p"
	.string "Each one says DIRECT or INDIRECT,\n"
	.string "and how many POKéMON it hits.\p"
	.string "KAI: Before we start, set your\n"
	.string "order in the POKéMON menu.\p"
	.string "First in your party goes in FRONT,\n"
	.string "second in MIDDLE, and third in\l"
	.string "BACK. Talk to me when you're ready!$"

Act2_Text_ClubRulesReminder:
	.string "MINA: Same rules as last time.\n"
	.string "Your first three, front to back.\p"
	.string "Check your order, then talk to me\n"
	.string "when you're ready!$"

Act2_Text_ClubRulesExaminer:
	.string "ROWAN: He said our rules are fine.\n"
	.string "Your first three, front to back.\p"
	.string "Check your order, then talk to\n"
	.string "DALE when you're ready!$"

Act2_Text_ClubComeIn:
	.string "Come on in!$"

Act2_Text_ClubReady:
	.string "Ready to battle?$"

Act2_Text_ClubNotYet:
	.string "Take your time. Talk to me when\n"
	.string "you're ready.$"

Act2_Text_ClubGoAgain:
	.string "Want to go again?$"

@ Before each club battle, the opponent says how they fight (the AI's strategy is in the battle code)
Act2_Text_KaiStrategy:
	.string "KAI: Fair warning, I don't do\n"
	.string "fancy.\p"
	.string "I hit your front POKéMON with\n"
	.string "direct attacks, again and again,\l"
	.string "until its defense cracks.\p"
	.string "Then your whole team's open.\n"
	.string "Let's go!$"

Act2_Text_MinaStrategy:
	.string "MINA: Want to know my secret?\n"
	.string "I'll tell you anyway.\p"
	.string "I don't bother with your front\n"
	.string "line. My indirect attacks go\l"
	.string "right over it, at your back line.\p"
	.string "Protect them if you can!$"

Act2_Text_DaleStrategy:
	.string "DALE: KAI hits the front line.\n"
	.string "MINA goes for the back.\p"
	.string "A real TRAINER does both. Direct\n"
	.string "attacks, indirect attacks, mixed\l"
	.string "as the battle needs.\p"
	.string "Show me you can read it.\n"
	.string "Begin!$"

Act2_Text_KaiWins:
	.string "Ha! Got you this time!$"

Act2_Text_KaiAfterLoss:
	.string "KAI: Hey, nobody wins every time.\n"
	.string "That's what the club's for!$"

Act2_Text_MinaWins:
	.string "Yes! I finally won one!$"

Act2_Text_MinaAfterLoss:
	.string "MINA: Don't worry about it. We all\n"
	.string "lose sometimes. Even KAI!$"

Act2_Text_DaleWins:
	.string "Not this time.$"

Act2_Text_DaleAfterLoss:
	.string "DALE: You lost, but you kept your\n"
	.string "head. That counts for a lot.$"

Act2_Text_DalePassesAnyway:
	.string "DALE: Nobody wins every battle.\n"
	.string "You think like a TRAINER, and\l"
	.string "that's what I'm here to see.\p"
	.string "You passed. This is yours now.$"

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
	.string "ROWAN: Here he is: our examiner\n"
	.string "from the POKéMON LEAGUE.\p"
	.string "Win, and you get a real\n"
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
	.string "all four of us in the morning.\p"
	.string "ROWAN: The final project!$"

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
	.string "Catch three different FIRE\n"
	.string "POKéMON. Battle with them, and\l"
	.string "watch how FIRE moves work.\p"
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
	.string "System from anywhere, especially\l"
	.string "useful on these field assignments.\p"
	.string "Catch more POKéMON than you can\n"
	.string "carry, and they go to your boxes.\l"
	.string "Swap them out any time!\p"
	.string "If you find yourself weakened and\n"
	.string "unable to continue battling,\l"
	.string "I suggest you open your KEY ITEMS\l"
	.string "and DASH back to the POKéMON\l"
	.string "CENTER.\p"
	.string "Now, you'll need these.$"

Act2_Text_FireSheetIntro:
	.string "PROF. ASHFORD: Let's see your\n"
	.string "research sheet.$"

Act2_Text_OutOfBalls:
	.string "Out of POKé BALLS already?\n"
	.string "Here, the department has plenty.$"

Act2_Text_FireHint:
	.string "FIRE POKéMON live in the old\n"
	.string "mansion. So do its cold furnaces.$"

Act2_Text_FireResearchDone:
	.string "PROF. ASHFORD: Your research is\n"
	.string "complete! Wonderful notes.\p"
	.string "From all this, we can make a TM:\n"
	.string "FLAMETHROWER.$"

Act2_Text_FireTeachTM:
	.string "The TM is yours to keep. Talk to\n"
	.string "me when you're ready for my exam.$"

Act2_Text_ReadyForFireExam:
	.string "PROF. ASHFORD: Your research is\n"
	.string "done. Shall we begin the exam?$"

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
	.string "PROF. MARLOWE: Welcome to the WATER\n"
	.string "course. I trust your POKéMON are\l"
	.string "rested after last time.\p"
	.string "Water shapes everything it touches,\n"
	.string "and you'll see that for yourselves.\p"
	.string "For the WATER course, we're headed\n"
	.string "to VERMILION CITY.\p"
	.string "I'll explain the assignment when we\n"
	.string "arrive. Off we go!$"

Act2_Text_WaterBriefing:
	.string "PROF. MARLOWE: Welcome to VERMILION\n"
	.string "CITY, KANTO's great port.\p"
	.string "Its harbor is full of wild WATER\n"
	.string "POKéMON. A fine host for our field\l"
	.string "assignment.\p"
	.string "Catch three different WATER\n"
	.string "POKéMON. Battle with them, and\l"
	.string "watch how WATER moves work.\p"
	.string "Fish at the harbor, or look in the\n"
	.string "grass on ROUTE 6, to the north.\p"
	.string "Finish the sheet, and you may just\n"
	.string "be able to make a TM based on your\l"
	.string "newfound knowledge.\p"
	.string "Now, you'll need these.$"

Act2_Text_WaterBriefingEnd:
	.string "That GOOD ROD is yours to keep.\n"
	.string "Use it well!$"

Act2_Text_WaterSheetIntro:
	.string "PROF. MARLOWE: Your research sheet,\n"
	.string "please.$"

Act2_Text_WaterHint:
	.string "Fish at the harbor, or try the\n"
	.string "grass on ROUTE 6.$"

Act2_Text_WaterHintFire:
	.string "Fire on the docks, of all things!\p"
	.string "Your notes could make a TM strong\n"
	.string "enough to put it out.\p"
	.string "Finish your research quickly,\n"
	.string "{PLAYER}. Every minute counts!$"

Act2_Text_WaterLastFire:
	.string "PROF. MARLOWE: Now, to the docks!\n"
	.string "The fire at the end of the pier\l"
	.string "is still burning.\p"
	.string "BUBBLEBEAM should be strong enough\n"
	.string "to put it out.\p"
	.string "And hurry. I saw a POKéMON\n"
	.string "cowering from the flames there.$"

Act2_Text_WaterSeeLapras:
	.string "PROF. MARLOWE: The fires are out!\n"
	.string "Splendid work.\p"
	.string "But the exam can wait. That young\n"
	.string "LAPRAS at the end of the pier\l"
	.string "needs you first. Go to it!$"

Act2_Text_WaterAllDone:
	.string "PROF. MARLOWE: The docks are safe,\n"
	.string "and your research is complete.\p"
	.string "Splendid work, {PLAYER}!$"

Act2_Text_WaterResearchDone:
	.string "PROF. MARLOWE: Splendid research!\p"
	.string "From your notes, we can make a TM:\n"
	.string "BUBBLEBEAM.$"

Act2_Text_WaterTeachTM:
	.string "The TM is yours to keep. Teach\n"
	.string "BUBBLEBEAM to a WATER POKéMON.$"

Act2_Text_ReadyForWaterExam:
	.string "PROF. MARLOWE: Now, then.\n"
	.string "Shall we begin the exam?$"

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
	.string "course, your last one.\p"
	.string "GRASS POKéMON teach patience.\n"
	.string "Nothing here can be rushed.\p"
	.string "For the GRASS course, we're headed\n"
	.string "to VIRIDIAN CITY.\p"
	.string "I'll explain the assignment when we\n"
	.string "arrive. Let's go!$"

Act2_Text_GrassBriefing:
	.string "PROF. FERN: Welcome to VIRIDIAN\n"
	.string "CITY, the gateway to the forests.\p"
	.string "West of here, ROUTE 22's fields are\n"
	.string "home to wild GRASS POKéMON.\p"
	.string "Catch three different GRASS\n"
	.string "POKéMON. Battle with them, and\l"
	.string "watch how GRASS moves work.\p"
	.string "Parts of the field have withered.\n"
	.string "See what your POKéMON can do.\p"
	.string "Finish the sheet, and you may just\n"
	.string "be able to make a TM based on your\l"
	.string "newfound knowledge.\p"
	.string "Now, you'll need these.$"

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
	.string "The TM is yours to keep. Come\n"
	.string "back when you're ready to test.$"

Act2_Text_ReadyForGrassExam:
	.string "PROF. FERN: Are you ready?\n"
	.string "Shall we begin the exam?$"

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

Act2_Text_TaskComplete:
	.string "The last research task is done!\n"
	.string "Time to report to the professor.$"

Act2_Text_TaskDoneMoreToGo:
	.string "That research task is done!\n"
	.string "There's still more on the sheet.$"

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

Act2_Text_MansionJournalCold:
	.string "An old research journal.\p"
	.string "Its pages are cold and damp, stuck\n"
	.string "together. Maybe if the mansion\l"
	.string "were warm again…$"

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

Act2_Text_SmallDockFiresOut:
	.string "The small fires are out!\p"
	.string "But the big one at the end of the\n"
	.string "pier is still burning…$"

Act2_Text_DockFiresOutSeeLapras:
	.string "All the fires are out!\p"
	.string "The young LAPRAS is still at the\n"
	.string "end of the pier, watching\l"
	.string "{PLAYER}. Better go see it!$"

Act2_Text_BigDockFireTooStrong:
	.string "The flames at the end of the pier\n"
	.string "are too fierce to get near.\p"
	.string "A small WATER move won't touch\n"
	.string "this. It'll take something much\l"
	.string "stronger…$"

Act2_Text_LaprasSurfaces:
	.string "Something big surfaced by the end\n"
	.string "of the pier! It's a young LAPRAS!$"

Act2_Text_DockLapras:
	.string "The young LAPRAS was trapped by the\n"
	.string "fire. It looks so relieved!$"

Act2_Text_DockFireAlarm:
	.string "MINA: {PLAYER}! There you are!\p"
	.string "There's smoke over the harbor.\n"
	.string "The docks are on fire!\p"
	.string "Come on, quick!$"

Act2_Text_DockFireShow:
	.string "MINA: Look at it! The workers can't\n"
	.string "get anywhere near it.\p"
	.string "Your WATER POKéMON could put out\n"
	.string "the small fires.\p"
	.string "But that big one at the end of the\n"
	.string "pier… We'd need a much stronger\l"
	.string "WATER move for that.$"

Act2_Text_DockWorkerQuiet:
	.string "Busy day at the harbor.\n"
	.string "Watch your step on the pier!$"

Act2_Text_DockWorkerFire:
	.string "Fire! The pier's on fire!\p"
	.string "We can't get near it! Please,\n"
	.string "help us put it out!\p"
	.string "If only we had some WATER\n"
	.string "POKéMON around here…$"

Act2_Text_DockWorkerThanks:
	.string "You put out the fires!\n"
	.string "Thank you, kid!$"

Act2_Text_LaprasWantsToCome:
	.string "The LAPRAS nuzzles {PLAYER}.\n"
	.string "It seems to want to come along!\p"
	.string "Take the LAPRAS with you?$"

Act2_Text_LaprasWontLeave:
	.string "The LAPRAS won't leave\n"
	.string "{PLAYER}'s side…$"

Act2_Text_LaprasReportToProf:
	.string "The docks are safe, and the\n"
	.string "research sheet is complete.\p"
	.string "Time to report to PROF. MARLOWE!$"

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





