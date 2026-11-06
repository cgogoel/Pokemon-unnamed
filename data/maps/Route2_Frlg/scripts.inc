Route2_Frlg_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route2_OnTransition
	.byte 0

@ Act 2's final project: the Cut trees between Diglett's Cave and Viridian City are gone (Act2Road_IsRouteOpen)
Route2_OnTransition::
	specialvar VAR_RESULT, Act2Road_IsRouteOpenSpecial
	goto_if_eq VAR_RESULT, FALSE, Route2_EventScript_TreesStay
	setflag FLAG_TEMP_16 @ (18,26)
	setflag FLAG_TEMP_14 @ (16,62)
	setflag FLAG_TEMP_13 @ (15,69)
Route2_EventScript_TreesStay::
	end

Route2_EventScript_RouteSign::
	msgbox Route2_Text_RouteSign, MSGBOX_SIGN
	end

Route2_EventScript_DiglettsCaveSign::
	msgbox Route2_Text_DiglettsCave, MSGBOX_SIGN
	end

Route2_Text_RouteSign::
	.string "ROUTE 2\n"
	.string "VIRIDIAN CITY - PEWTER CITY$"

Route2_Text_DiglettsCave::
	.string "DIGLETT'S CAVE$"

