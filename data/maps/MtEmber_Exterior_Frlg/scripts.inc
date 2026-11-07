MtEmber_Exterior_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MtEmber_Exterior_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, MtEmber_Exterior_OnLoad
	.byte 0

MtEmber_Exterior_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_MT_EMBER_EXTERIOR
	end

MtEmber_Exterior_OnLoad::
	call_if_ge VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 4, MtEmber_Exterior_EventScript_OpenCave
	end

MtEmber_Exterior_EventScript_OpenCave::
	setmetatile 42, 39, METATILE_MtEmber_CaveEntrance, 0
	return

MtEmber_Exterior_EventScript_Logan::
	trainerbattle_single TRAINER_PKMN_RANGER_LOGAN, MtEmber_Exterior_Text_LoganIntro, MtEmber_Exterior_Text_LoganDefeat
	msgbox MtEmber_Exterior_Text_LoganPostBattle, MSGBOX_AUTOCLOSE
	end

MtEmber_Exterior_EventScript_Beth::
	trainerbattle_single TRAINER_PKMN_RANGER_BETH, MtEmber_Exterior_Text_BethIntro, MtEmber_Exterior_Text_BethDefeat
	msgbox MtEmber_Exterior_Text_BethPostBattle, MSGBOX_AUTOCLOSE
	end

MtEmber_Exterior_EventScript_Jocelyn::
	trainerbattle_single TRAINER_CRUSH_GIRL_JOCELYN, MtEmber_Exterior_Text_JocelynIntro, MtEmber_Exterior_Text_JocelynDefeat
	msgbox MtEmber_Exterior_Text_JocelynPostBattle, MSGBOX_AUTOCLOSE
	end

Text_ExplosionTeach::
	.string "Can you imagine?\n"
	.string "If this volcano were to erupt?\p"
	.string "The explosion would be the end of\n"
	.string "us. How terrifying is that?\p"
	.string "While we're terrified, would you\n"
	.string "like me to teach EXPLOSION?$"

Text_ExplosionDeclined::
	.string "Yeah, you're right.\n"
	.string "It is too terrifying.$"

Text_ExplosionWhichMon::
	.string "You're terribly brave!\p"
	.string "Which POKéMON should I teach\n"
	.string "EXPLOSION?$"

Text_ExplosionTaught::
	.string "Using EXPLOSION while on this\n"
	.string "volcano…\p"
	.string "What a terrifying thrill!$"

MtEmber_Exterior_Text_LoganIntro::
	.string "An active volcano is one that's\n"
	.string "erupting, or about to.$"

MtEmber_Exterior_Text_LoganDefeat::
	.string "Whew… That was hot!\n"
	.string "Sweat's erupting from me!$"

MtEmber_Exterior_Text_LoganPostBattle::
	.string "They say there're over 1,500\n"
	.string "active volcanoes in the world.\p"
	.string "Just thinking about them makes me\n"
	.string "feel all hot and gooey inside!$"

MtEmber_Exterior_Text_BethIntro::
	.string "Do you know how caves form inside\n"
	.string "volcanoes?$"

MtEmber_Exterior_Text_BethDefeat::
	.string "Atchah!\n"
	.string "You're shockingly good!$"

MtEmber_Exterior_Text_BethPostBattle::
	.string "When lava at the surface of magma\n"
	.string "hardens, and the magma inside flows\l"
	.string "out, caverns are left behind.$"

MtEmber_Exterior_Text_JocelynIntro::
	.string "I've been in training so I can mend\n"
	.string "my broken heart…\p"
	.string "It toughened me up physically at\n"
	.string "least! Hi-yah!$"

MtEmber_Exterior_Text_JocelynDefeat::
	.string "I think I tried to overdo it…$"

MtEmber_Exterior_Text_JocelynPostBattle::
	.string "The way you battle, it reminds me\n"
	.string "of the guy who dumped me…$"

