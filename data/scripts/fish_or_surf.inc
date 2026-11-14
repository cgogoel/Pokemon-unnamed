@ Pressing A facing water with a fishing rod in the bag (GetInteractedWaterScript, src/field_control_avatar.c).
@ When SURF can be used: SURF, FISH or CANCEL, SURF first. Otherwise: fish here? Fishing casts the best rod in the
@ bag (StartFishingWithBestRod), once the script is over. Without a rod, A on water is as before (the SURF prompt).
EventScript_FishOrSurf::
	lockall
	message Text_FishOrSurf
	waitmessage
	multichoice 21, 7, MULTI_SURF_FISH, FALSE
	goto_if_eq VAR_RESULT, 0, EventScript_FishOrSurf_Surf
	goto_if_eq VAR_RESULT, 1, EventScript_FishOrSurf_Fish
	closemessage
	releaseall
	end

EventScript_FishOrSurf_Surf:
	checkfieldmove FIELD_MOVE_SURF
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_FishOrSurf_Release
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	setfieldeffectargument 0, VAR_RESULT
	msgbox gText_PlayerUsedSurf, MSGBOX_DEFAULT
	dofieldeffect FLDEFF_USE_SURF
	releaseall
	end

EventScript_FishOrSurf_Fish:
	closemessage
	releaseall
	special StartFishingWithBestRod
	end

EventScript_FishOrSurf_Release:
	closemessage
	releaseall
	end

EventScript_AskToFish::
	lockall
	specialvar VAR_RESULT, GetBestFishingRodItem
	bufferitemname STR_VAR_1, VAR_RESULT
	msgbox Text_WantToFish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_FishOrSurf_Fish
	releaseall
	end

Text_FishOrSurf:
	.string "The water is dyed a deep blue…\n"
	.string "What would you like to do?$"

Text_WantToFish:
	.string "The water is dyed a deep blue…\n"
	.string "Fish with the {STR_VAR_1}?$"
