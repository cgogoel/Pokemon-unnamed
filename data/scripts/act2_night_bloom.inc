@ The Grass course's night bloom (docs/hack/act2_story.md, src/act2_night_bloom.c): VIRIDIAN FOREST at night, its
@ glowcaps and ODDISH, and the glade where the MOONVEIL blooms at dawn. State: VAR_ACT2_NIGHT_BLOOM (NIGHT_BLOOM_*).

@ ---------------------------------------------------------------------------------------------------------------
@ VIRIDIAN FOREST

ViridianForest_EventScript_Act2Objects::
	special Act2_SetupForestObjects
	return

ViridianForest_OnLoad::
	special Act2_DrawForestOpening
	end

@ Part 1, light the way: a dim glowcap wakes with a GRASS move (a GRASS POKéMON's, Act2_FindCourseMoveUser), and the
@ light widens. Its lit self is another object, at the same square.
Act2_EventScript_Glowcap1::
	setvar VAR_0x8004, 1
	goto Act2_EventScript_Glowcap

Act2_EventScript_Glowcap2::
	setvar VAR_0x8004, 2
	goto Act2_EventScript_Glowcap

Act2_EventScript_Glowcap3::
	setvar VAR_0x8004, 3
	goto Act2_EventScript_Glowcap

Act2_EventScript_Glowcap:
	lockall
	specialvar VAR_RESULT, Act2_FindCourseMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_GlowcapNeedsGrass
	msgbox Act2_Text_GlowcapAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_NightBloomRelease
	playse SE_M_ABSORB
	specialvar VAR_RESULT, Act2_WakeGlowcap
	copyvar VAR_0x8007, VAR_RESULT
	removeobject VAR_0x8005
	addobject VAR_0x8006
	waitse
	playse SE_M_MOONLIGHT
	special Act2_BrightenForest
	waitstate
	msgbox Act2_Text_GlowcapLit
	goto_if_eq VAR_0x8007, FALSE, Act2_EventScript_NightBloomRelease
	@ The last one: the first ODDISH comes out into the light
	addobject LOCALID_FOREST_ODDISH_1
	playmoncry SPECIES_ODDISH, CRY_MODE_NORMAL
	msgbox Act2_Text_GlowcapsAllLit
	waitmoncry
	releaseall
	end

Act2_EventScript_GlowcapNeedsGrass:
	msgbox Act2_Text_GlowcapNeedsGrass
	releaseall
	end

Act2_EventScript_GlowcapLitSign::
	msgbox Act2_Text_GlowcapLitSign, MSGBOX_SIGN
	end

Act2_EventScript_NightBloomRelease:
	releaseall
	end

@ Part 2, follow the ODDISH: too skittish to follow, until a POKéMON holds it still (SLEEP POWDER or STUN SPORE, or
@ another GRASS status move: Act2_FindHoldStillMoveUser). Then it shakes it off and toddles along the next leg, and
@ the next ODDISH waits where it stops. The third leads into the glade's hidden opening.
Act2_EventScript_Oddish1::
	setvar VAR_0x8004, 1
	goto Act2_EventScript_Oddish

Act2_EventScript_Oddish2::
	setvar VAR_0x8004, 2
	goto Act2_EventScript_Oddish

Act2_EventScript_Oddish3::
	setvar VAR_0x8004, 3
	goto Act2_EventScript_Oddish

Act2_EventScript_Oddish:
	lockall
	playmoncry SPECIES_ODDISH, CRY_MODE_NORMAL
	waitmoncry
	specialvar VAR_RESULT, Act2_FindHoldStillMoveUser
	goto_if_eq VAR_RESULT, FALSE, Act2_EventScript_OddishScurries
	msgbox Act2_Text_OddishAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Act2_EventScript_OddishScurries
	playse SE_M_POISON_POWDER
	msgbox Act2_Text_OddishHeld
	waitse
	call_if_eq VAR_0x8004, 1, Act2_EventScript_OddishLeg1Text
	call_if_eq VAR_0x8004, 2, Act2_EventScript_OddishLeg2Text
	call_if_eq VAR_0x8004, 3, Act2_EventScript_OddishLeg3Text
	closemessage
	call_if_eq VAR_0x8004, 1, Act2_EventScript_OddishLeg1
	call_if_eq VAR_0x8004, 2, Act2_EventScript_OddishLeg2
	call_if_eq VAR_0x8004, 3, Act2_EventScript_OddishLeg3
	specialvar VAR_RESULT, Act2_FollowOddish
	removeobject VAR_0x8005
	goto_if_eq VAR_RESULT, TRUE, Act2_EventScript_OddishFoundGlade
	addobject VAR_0x8006
	releaseall
	end

Act2_EventScript_OddishFoundGlade:
	playse SE_M_SWEET_SCENT
	special DrawWholeMapView
	msgbox Act2_Text_OddishFoundGlade
	waitse
	releaseall
	end

@ It darts away and back: no way to follow it like this
Act2_EventScript_OddishScurries:
	applymovement VAR_LAST_TALKED, Act2_Movement_OddishScurry
	waitmovement 0
	msgbox Act2_Text_OddishScurries
	releaseall
	end

Act2_EventScript_OddishLeg1Text:
	msgbox Act2_Text_OddishLeg1
	return

Act2_EventScript_OddishLeg2Text:
	msgbox Act2_Text_OddishLeg2
	return

Act2_EventScript_OddishLeg3Text:
	msgbox Act2_Text_OddishLeg3
	return

@ The legs (positions in Viridian Forest's map.json): (44,28) -> (30,17) -> (40,6) -> into the opening (40,1)
Act2_EventScript_OddishLeg1:
	applymovement LOCALID_FOREST_ODDISH_1, Act2_Movement_OddishLeg1
	waitmovement 0
	return

Act2_EventScript_OddishLeg2:
	applymovement LOCALID_FOREST_ODDISH_2, Act2_Movement_OddishLeg2
	waitmovement 0
	return

@ The opening is drawn first, so the ODDISH can be seen going in
Act2_EventScript_OddishLeg3:
	setvar VAR_0x8004, NIGHT_BLOOM_ODDISH_3
	special Act2_SetNightBloomBit
	special Act2_DrawForestOpening
	special DrawWholeMapView
	setvar VAR_0x8004, 3
	applymovement LOCALID_FOREST_ODDISH_3, Act2_Movement_OddishLeg3
	waitmovement 0
	return

Act2_Movement_OddishScurry:
	jump_in_place_down
	walk_in_place_faster_left
	walk_in_place_faster_right
	walk_in_place_faster_down
	step_end

Act2_Movement_OddishLeg1:
	walk_fast_left
	walk_fast_left
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	face_down
	step_end

Act2_Movement_OddishLeg2:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	face_down
	step_end

Act2_Movement_OddishLeg3:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

@ The opening (coord events, row 1 of the gap) and the way back
ViridianForest_EventScript_EnterGlade::
	lockall
	warp MAP_VIRIDIAN_FOREST_GLADE, 10, 13
	waitstate
	releaseall
	end

ViridianForest_Glade_EventScript_ToForest::
	lockall
	warp MAP_VIRIDIAN_FOREST, 40, 2
	waitstate
	releaseall
	end

@ ---------------------------------------------------------------------------------------------------------------
@ The glade

ViridianForest_Glade_OnTransition::
	special Act2_SetupGladeObjects
	setvar VAR_TEMP_1, 0
	specialvar VAR_RESULT, Act2_GetGladeScene
	call_if_ne VAR_RESULT, GLADE_SCENE_NONE, ViridianForest_Glade_EventScript_SceneDue
	end

ViridianForest_Glade_EventScript_SceneDue:
	setvar VAR_TEMP_1, 1
	return

ViridianForest_Glade_OnLoad::
	special Act2_DrawGladeHedges
	end

ViridianForest_Glade_OnFrame::
	map_script_2 VAR_TEMP_1, 1, ViridianForest_Glade_EventScript_Scene
	.2byte 0

ViridianForest_Glade_EventScript_Scene::
	lockall
	setvar VAR_TEMP_1, 0
	specialvar VAR_RESULT, Act2_GetGladeScene
	goto_if_eq VAR_RESULT, GLADE_SCENE_ARRIVAL, ViridianForest_Glade_EventScript_Arrival
	goto_if_eq VAR_RESULT, GLADE_SCENE_VIGIL, ViridianForest_Glade_EventScript_NextWave
	goto_if_eq VAR_RESULT, GLADE_SCENE_DAWN, ViridianForest_Glade_EventScript_Dawn
	goto_if_eq VAR_RESULT, GLADE_SCENE_MORNING, ViridianForest_Glade_EventScript_Morning
	releaseall
	end

@ The bud and the MORELULL beside it; then the scent draws the swarm
ViridianForest_Glade_EventScript_Arrival:
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox Act2_Text_GladeArrival
	playmoncry SPECIES_MORELULL, CRY_MODE_NORMAL
	msgbox Act2_Text_GladeMorelull
	waitmoncry
	setvar VAR_0x8004, NIGHT_BLOOM_ARRIVED
	special Act2_SetNightBloomBit
	playse SE_M_SWEET_SCENT
	msgbox Act2_Text_GladeScent
	waitse
	goto ViridianForest_Glade_EventScript_NextWave

@ Part 3, guard the bloom: three waves (west, north, east gaps), each fought, or kept out by a hedge
ViridianForest_Glade_EventScript_NextWave:
	specialvar VAR_0x8004, Act2_GetNextWave
	goto_if_eq VAR_0x8004, 0, ViridianForest_Glade_EventScript_Dawn
	call_if_eq VAR_0x8004, 1, ViridianForest_Glade_EventScript_ShowBug1
	call_if_eq VAR_0x8004, 2, ViridianForest_Glade_EventScript_ShowBug2
	call_if_eq VAR_0x8004, 3, ViridianForest_Glade_EventScript_ShowBug3
	playse SE_PIN
	call_if_eq VAR_0x8004, 1, ViridianForest_Glade_EventScript_Wave1Text
	call_if_eq VAR_0x8004, 2, ViridianForest_Glade_EventScript_Wave2Text
	call_if_eq VAR_0x8004, 3, ViridianForest_Glade_EventScript_Wave3Text
	specialvar VAR_RESULT, Act2_FindHedgeMoveUser
	goto_if_eq VAR_RESULT, FALSE, ViridianForest_Glade_EventScript_WaveBattle
	msgbox Act2_Text_HedgeAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianForest_Glade_EventScript_WaveBattle
	playse SE_M_GIGA_DRAIN
	specialvar VAR_RESULT, Act2_PlantHedge
	special DrawWholeMapView
	msgbox Act2_Text_HedgeGrew
	waitse
	removeobject VAR_0x8005
	goto ViridianForest_Glade_EventScript_NextWave

ViridianForest_Glade_EventScript_WaveBattle:
	msgbox Act2_Text_WaveBattle
	closemessage
	special StartAct2BloomWaveBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, ViridianForest_Glade_EventScript_WaveWon
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, ViridianForest_Glade_EventScript_WaveWon
	msgbox Act2_Text_WaveStillThere
	releaseall
	end

ViridianForest_Glade_EventScript_WaveWon:
	specialvar VAR_0x8004, Act2_GetNextWave
	specialvar VAR_RESULT, Act2_EndWave
	removeobject VAR_0x8005
	msgbox Act2_Text_WaveScattered
	goto ViridianForest_Glade_EventScript_NextWave

ViridianForest_Glade_EventScript_ShowBug1:
	clearflag FLAG_TEMP_6
	addobject LOCALID_GLADE_BUG_1
	return

ViridianForest_Glade_EventScript_ShowBug2:
	clearflag FLAG_TEMP_7
	addobject LOCALID_GLADE_BUG_2
	return

ViridianForest_Glade_EventScript_ShowBug3:
	clearflag FLAG_TEMP_8
	addobject LOCALID_GLADE_BUG_3
	return

ViridianForest_Glade_EventScript_Wave1Text:
	msgbox Act2_Text_Wave1
	return

ViridianForest_Glade_EventScript_Wave2Text:
	msgbox Act2_Text_Wave2
	return

ViridianForest_Glade_EventScript_Wave3Text:
	msgbox Act2_Text_Wave3
	return

@ Dawn: the hours go by (6 to 9 o'clock), the MOONVEIL opens, its dew is collected (the puzzle's end)
ViridianForest_Glade_EventScript_Dawn:
	msgbox Act2_Text_NightEnds
	closemessage
	setvar VAR_0x8004, 6
	special GameTime_ShowHour
	delay 45
	setvar VAR_0x8004, 7
	special GameTime_ShowHour
	delay 45
	setvar VAR_0x8004, 8
	special GameTime_ShowHour
	delay 45
	setvar VAR_0x8004, 9
	special GameTime_ShowHour
	delay 45
	special Act2_BreakDawn
	delay 30
	playse SE_M_PETAL_DANCE
	removeobject LOCALID_GLADE_BUD
	clearflag FLAG_TEMP_3
	addobject LOCALID_GLADE_BLOOM
	waitse
	msgbox Act2_Text_MoonveilOpens
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Act2_Text_GotDew
	waitmessage
	waitfanfare
	setvar VAR_0x8004, ACT2_RESEARCH_PUZZLE_DONE
	special Act2_SetResearchFlag
	call Act2_EventScript_TaskDone
	goto ViridianForest_Glade_EventScript_Morning

@ The MORELULL asks to come along (no won't do), then PROF. FERN arrives
ViridianForest_Glade_EventScript_Morning:
	goto_if_set FLAG_ACT2_GOT_MORELULL, ViridianForest_Glade_EventScript_FernArrives
	msgbox Act2_Text_MorelullComes
ViridianForest_Glade_EventScript_MorelullAsks:
	playmoncry SPECIES_MORELULL, CRY_MODE_NORMAL
	msgbox Act2_Text_MorelullWantsToCome, MSGBOX_YESNO
	waitmoncry
	goto_if_eq VAR_RESULT, NO, ViridianForest_Glade_EventScript_MorelullWontLeave
	specialvar VAR_RESULT, Act2_GiveGladeMorelull
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, ViridianForest_Glade_EventScript_MorelullToParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, ViridianForest_Glade_EventScript_MorelullToPC
	goto Common_EventScript_NoMoreRoomForPokemon

ViridianForest_Glade_EventScript_MorelullWontLeave:
	msgbox Act2_Text_MorelullWontLeave
	goto ViridianForest_Glade_EventScript_MorelullAsks

ViridianForest_Glade_EventScript_MorelullToParty:
	call ViridianForest_Glade_EventScript_MorelullJoined
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianForest_Glade_EventScript_FernArrives
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto ViridianForest_Glade_EventScript_FernArrives

ViridianForest_Glade_EventScript_MorelullToPC:
	call ViridianForest_Glade_EventScript_MorelullJoined
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianForest_Glade_EventScript_MorelullToPCDone
	call Common_EventScript_NameReceivedBoxMon
ViridianForest_Glade_EventScript_MorelullToPCDone:
	call Common_EventScript_TransferredToPC
	goto ViridianForest_Glade_EventScript_FernArrives

ViridianForest_Glade_EventScript_MorelullJoined:
	removeobject LOCALID_GLADE_MORELULL
	playfanfare MUS_LEVEL_UP
	message Act2_Text_MorelullJoined
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_MORELULL
	return

@ PROF. FERN runs up beside the player (as MINA on the docks), then makes the TM and gives the exam
ViridianForest_Glade_EventScript_FernArrives:
	setvar VAR_0x8004, NIGHT_BLOOM_FERN
	special Act2_SetNightBloomBit
	specialvar VAR_RESULT, Act2_FindSpotBesidePlayer
	goto_if_eq VAR_RESULT, FALSE, ViridianForest_Glade_EventScript_FernBeside
	setobjectxyperm LOCALID_GLADE_FERN, VAR_0x8005, VAR_0x8006
	clearflag FLAG_TEMP_5
	addobject LOCALID_GLADE_FERN
	setvar VAR_0x8009, LOCALID_GLADE_FERN
	call Act2_EventScript_FriendRunsUp
	goto ViridianForest_Glade_EventScript_FernTalks

ViridianForest_Glade_EventScript_FernBeside:
	clearflag FLAG_TEMP_5
	addobject LOCALID_GLADE_FERN
ViridianForest_Glade_EventScript_FernTalks:
	msgbox Act2_Text_FernAtDawn
	goto Act2_EventScript_ProfFernTalk

@ Talking to the bud: the vigil goes on (after a wave the player left)
ViridianForest_Glade_EventScript_Bud::
	lockall
	specialvar VAR_RESULT, Act2_GetGladeScene
	goto_if_eq VAR_RESULT, GLADE_SCENE_VIGIL, ViridianForest_Glade_EventScript_BudVigil
	msgbox Act2_Text_BudClosed
	releaseall
	end

ViridianForest_Glade_EventScript_BudVigil:
	msgbox Act2_Text_BudVigil
	goto ViridianForest_Glade_EventScript_NextWave

ViridianForest_Glade_EventScript_Bloom::
	msgbox Act2_Text_Bloom, MSGBOX_SIGN
	end

ViridianForest_Glade_EventScript_Morelull::
	lock
	faceplayer
	playmoncry SPECIES_MORELULL, CRY_MODE_NORMAL
	msgbox Act2_Text_MorelullGlows
	waitmoncry
	release
	end

ViridianForest_Glade_EventScript_Bug::
	end

@ ---------------------------------------------------------------------------------------------------------------

Act2_Text_GlowcapNeedsGrass:
	.string "A mushroom, sleeping in the dark.\n"
	.string "A GRASS POKéMON's move might\l"
	.string "wake it up.$"

Act2_Text_GlowcapAsk:
	.string "A dim GLOWCAP. Have {STR_VAR_1}\n"
	.string "use {STR_VAR_2} on it?$"

Act2_Text_GlowcapLit:
	.string "{STR_VAR_1} used {STR_VAR_2}!\p"
	.string "The GLOWCAP drank it in and lit\n"
	.string "up! The trail is brighter now.$"

Act2_Text_GlowcapsAllLit:
	.string "In the new light, something small\n"
	.string "rustles nearby… An ODDISH!$"

Act2_Text_GlowcapLitSign:
	.string "The GLOWCAP glows softly.$"

Act2_Text_OddishScurries:
	.string "The ODDISH darts off and back,\n"
	.string "too skittish to follow.\p"
	.string "If only something could make it\n"
	.string "hold still…$"

Act2_Text_OddishAsk:
	.string "The ODDISH won't hold still.\n"
	.string "Have {STR_VAR_1} use {STR_VAR_2}?$"

Act2_Text_OddishHeld:
	.string "{STR_VAR_1} used {STR_VAR_2}!\p"
	.string "The ODDISH froze, sniffing the\n"
	.string "air. It was heading somewhere…$"

Act2_Text_OddishLeg1:
	.string "It shook off the powder and\n"
	.string "toddled north, then west!$"

Act2_Text_OddishLeg2:
	.string "It shook off the powder and\n"
	.string "hurried north, then east!$"

Act2_Text_OddishLeg3:
	.string "It shook off the powder and\n"
	.string "pushed straight into the trees!$"

Act2_Text_OddishFoundGlade:
	.string "There's an opening between the\n"
	.string "trees! A sweet scent drifts out.$"

Act2_Text_GladeArrival:
	.string "Moonlight spills into a hidden\n"
	.string "glade.\p"
	.string "In the middle stands a pale bud,\n"
	.string "still closed: the MOONVEIL!$"

Act2_Text_GladeMorelull:
	.string "Beside it, a MORELULL glows\n"
	.string "softly, lighting the glade.$"

Act2_Text_GladeScent:
	.string "The bud's scent drifts into the\n"
	.string "dark forest…\p"
	.string "Something stirs out there.\n"
	.string "Hungry BUG POKéMON!\p"
	.string "Guard the MOONVEIL until dawn!$"

Act2_Text_Wave1:
	.string "Rustling at the west gap!\n"
	.string "A VENONAT is after the bud!$"

Act2_Text_Wave2:
	.string "Buzzing at the north gap!\n"
	.string "PARAS and VENONAT!$"

Act2_Text_Wave3:
	.string "A whole swarm at the east gap!\n"
	.string "VENOMOTH, BEEDRILL, VENONAT!$"

Act2_Text_HedgeAsk:
	.string "{STR_VAR_1} could grow a hedge in\n"
	.string "the gap with {STR_VAR_2}.\l"
	.string "Plant one?$"

Act2_Text_HedgeGrew:
	.string "{STR_VAR_1} used {STR_VAR_2}!\p"
	.string "A thick hedge sprang up in the gap!\n"
	.string "The BUG POKéMON buzz off.$"

Act2_Text_WaveBattle:
	.string "Here they come!$"

Act2_Text_WaveScattered:
	.string "The swarm scattered into the\n"
	.string "dark!$"

Act2_Text_WaveStillThere:
	.string "The BUG POKéMON are still\n"
	.string "circling the glade…\p"
	.string "Go back to the MOONVEIL when\n"
	.string "you're ready to guard it.$"

Act2_Text_BudClosed:
	.string "The MOONVEIL's bud, closed tight.\n"
	.string "It smells sweet.$"

Act2_Text_BudVigil:
	.string "The MOONVEIL's bud. The BUG\n"
	.string "POKéMON haven't given up yet…$"

Act2_Text_NightEnds:
	.string "The last of the swarm fled.\n"
	.string "The glade is quiet.\p"
	.string "Above the trees, the sky begins\n"
	.string "to pale…$"

Act2_Text_MoonveilOpens:
	.string "In the first light of dawn, the\n"
	.string "MOONVEIL opened!\p"
	.string "Dew sparkles on its petals.$"

Act2_Text_GotDew:
	.string "{PLAYER} collected the MOONVEIL's\n"
	.string "dew!$"

Act2_Text_MorelullComes:
	.string "The MORELULL that lit the glade\n"
	.string "all night drifts over to {PLAYER}.$"

Act2_Text_MorelullWantsToCome:
	.string "It seems to want to come along!\p"
	.string "Take the MORELULL with you?$"

Act2_Text_MorelullWontLeave:
	.string "The MORELULL won't leave\n"
	.string "{PLAYER}'s side…$"

Act2_Text_MorelullJoined:
	.string "The MORELULL joined {PLAYER}!$"

Act2_Text_MorelullGlows:
	.string "MORELULL: Lullu…\p"
	.string "It glows softly.$"

Act2_Text_FernAtDawn:
	.string "PROF. FERN: {PLAYER}! I followed\n"
	.string "the GLOWCAPS' light here.\p"
	.string "The MOONVEIL… in bloom! And you\n"
	.string "kept it safe all night.$"

Act2_Text_Bloom:
	.string "The MOONVEIL, in full bloom.\n"
	.string "It shines in the morning sun.$"
