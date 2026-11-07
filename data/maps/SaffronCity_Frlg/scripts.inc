SaffronCity_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SaffronCity_OnTransition
	.byte 0

SaffronCity_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_SAFFRON_CITY
	end

SaffronCity_EventScript_WorkerM::
	msgbox SaffronCity_Text_AlwaysBuildingInSaffron, MSGBOX_NPC
	end

SaffronCity_EventScript_Youngster::
	msgbox SaffronCity_Text_DadWorksAtSilph, MSGBOX_NPC
	end

SaffronCity_EventScript_Lass::
	msgbox SaffronCity_Text_AllowanceNeverLasts, MSGBOX_NPC
	end

SaffronCity_EventScript_Boy::
	msgbox SaffronCity_Text_CommuteOnPidgeot, MSGBOX_NPC
	end

SaffronCity_EventScript_Pidgeot::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PIDGEOT, CRY_MODE_NORMAL
	msgbox SaffronCity_Text_Pidgeot
	waitmoncry
	release
	end

SaffronCity_EventScript_Man::
	msgbox SaffronCity_Text_DojoMasterTrainsAtDawn, MSGBOX_NPC
	end

SaffronCity_EventScript_CitySign::
	msgbox SaffronCity_Text_CitySign, MSGBOX_SIGN
	end

SaffronCity_EventScript_DojoSign::
	msgbox SaffronCity_Text_FightingDojo, MSGBOX_SIGN
	end

SaffronCity_EventScript_GymSign::
	lockall
	famechecker FAMECHECKER_SABRINA, 0
	msgbox SaffronCity_Text_GymSign
	releaseall
	end

SaffronCity_EventScript_TrainerTips1::
	msgbox SaffronCity_Text_FullHealCuresStatus, MSGBOX_SIGN
	end

SaffronCity_EventScript_TrainerTips2::
	msgbox SaffronCity_Text_GreatBallImprovedCatchRate, MSGBOX_SIGN
	end

SaffronCity_EventScript_SilphCoSign::
	msgbox SaffronCity_Text_SilphCoSign, MSGBOX_SIGN
	end

SaffronCity_EventScript_MrPsychicsHouseSign::
	msgbox SaffronCity_Text_MrPsychicsHouse, MSGBOX_SIGN
	end

SaffronCity_EventScript_SilphProductSign::
	msgbox SaffronCity_Text_SilphsLatestProduct, MSGBOX_SIGN
	end

SaffronCity_EventScript_TrainerFanClubSign::
	msgbox SaffronCity_Text_TrainerFanClubSign, MSGBOX_SIGN
	end

SaffronCity_Text_AlwaysBuildingInSaffron::
	.string "Phew, what a shift!\p"
	.string "There's always something going up\n"
	.string "in SAFFRON. Offices, shops, towers…\p"
	.string "My MACHOP carries the steel beams.\n"
	.string "I just sign the paperwork!$"

SaffronCity_Text_DadWorksAtSilph::
	.string "My dad works at SILPH CO.\p"
	.string "He tests new POKé BALLS all day.\n"
	.string "He says most of them wobble funny.\p"
	.string "Someday I'll work there too!$"

SaffronCity_Text_AllowanceNeverLasts::
	.string "I take the bus to SAFFRON every\n"
	.string "weekend just for the shops.\p"
	.string "My whole allowance goes on cute\n"
	.string "POKé DOLLS. It never lasts!$"

SaffronCity_Text_CommuteOnPidgeot::
	.string "I fly in to work on my PIDGEOT.\p"
	.string "The streets are packed every\n"
	.string "morning, but the sky's wide open!$"

SaffronCity_Text_Pidgeot::
	.string "PIDGEOT: Bi bibii!$"

SaffronCity_Text_DojoMasterTrainsAtDawn::
	.string "The KARATE MASTER at the DOJO\n"
	.string "trains before sunrise every day.\p"
	.string "I hear his shouts on my way to\n"
	.string "the office. Better than an alarm!$"

SaffronCity_Text_CitySign::
	.string "SAFFRON CITY\n"
	.string "Shining, Golden Land of Commerce$"

SaffronCity_Text_FightingDojo::
	.string "FIGHTING DOJO$"

SaffronCity_Text_GymSign::
	.string "SAFFRON CITY POKéMON GYM\n"
	.string "LEADER: SABRINA\l"
	.string "The Master of PSYCHIC POKéMON!$"

SaffronCity_Text_FullHealCuresStatus::
	.string "TRAINER TIPS\p"
	.string "FULL HEAL cures all ailments like\n"
	.string "burns, paralysis, poisoning,\l"
	.string "freezing, and sleep.\p"
	.string "It costs a bit more, but it's more\n"
	.string "convenient than buying other items.$"

SaffronCity_Text_GreatBallImprovedCatchRate::
	.string "TRAINER TIPS\p"
	.string "The new GREAT BALL offers improved\n"
	.string "catching rates.\p"
	.string "Try it on those hard-to-catch\n"
	.string "POKéMON.$"

SaffronCity_Text_SilphCoSign::
	.string "SILPH CO. OFFICE BUILDING$"

SaffronCity_Text_MrPsychicsHouse::
	.string "MR. PSYCHIC'S HOUSE$"

SaffronCity_Text_SilphsLatestProduct::
	.string "SILPH's latest product!\n"
	.string "Release to be determined…$"

SaffronCity_Text_TrainerFanClubSign::
	.string "POKéMON TRAINER FAN CLUB\p"
	.string "Many TRAINERS have scribbled their\n"
	.string "names on this sign.$"

SaffronCity_Text_HowCanClubNotRecognizeLance::
	.string "This FAN CLUB…\n"
	.string "No one here has a clue!\p"
	.string "How could they not recognize\n"
	.string "the brilliance that is LANCE?\p"
	.string "He stands for justice!\n"
	.string "He's cool, and yet passionate!\l"
	.string "He's the greatest, LANCE!$"

