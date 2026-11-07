LavenderTown_House1_Frlg_MapScripts::
	.byte 0

LavenderTown_House1_EventScript_CooltrainerF::
	lock
	faceplayer
	msgbox LavenderTown_House1_Text_AdoptedCuboneFromFuji
	release
	end

LavenderTown_House1_EventScript_Cubone::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_CUBONE, CRY_MODE_NORMAL
	msgbox LavenderTown_House1_Text_Cubone
	waitmoncry
	release
	end

LavenderTown_House1_Text_Cubone::
	.string "CUBONE: Kyarugoo!$"

LavenderTown_House1_Text_AdoptedCuboneFromFuji::
	.string "I adopted this CUBONE from MR.\n"
	.string "FUJI's house down the road.\p"
	.string "It was so shy at first. It hid\n"
	.string "under its skull for weeks.\p"
	.string "Now it follows me everywhere,\n"
	.string "even into the bath!$"

