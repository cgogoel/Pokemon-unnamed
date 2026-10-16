@ Act 1 befriending (docs/hack/befriending.md)

@ befriendbattle: VAR_RESULT is the battle outcome afterwards (never TRUE or FALSE)
Common_EventScript_StartBefriendBattle::
	special StartBefriendBattle
	return

Common_EventScript_BefriendTakeHomeFirst::
	msgbox Common_Text_BefriendTakeHomeFirst, MSGBOX_DEFAULT
	setvar VAR_RESULT, BEFRIEND_RESULT_BLOCKED
	return

Common_Text_BefriendTakeHomeFirst:
	.string "You should take {STR_VAR_1} home\n"
	.string "to the farmyard first.$"
