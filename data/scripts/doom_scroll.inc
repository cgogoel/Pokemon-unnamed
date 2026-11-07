@ The DOOM SCROLL (src/game_time.c): wait until night or day. From Act 2's final project on, unless the story
@ holds the time (a club night, the Grass trip).
EventScript_DoomScroll::
	lockall
	specialvar VAR_RESULT, GameTime_GetDoomScrollState
	goto_if_eq VAR_RESULT, DOOM_SCROLL_STORY_TIME, EventScript_DoomScrollStoryTime
	goto_if_eq VAR_RESULT, DOOM_SCROLL_TOO_EARLY, EventScript_DoomScrollTooEarly
	specialvar VAR_RESULT, GameTime_IsNight
	goto_if_eq VAR_RESULT, TRUE, EventScript_DoomScrollTillDay
	msgbox Text_DoomScrollTillNight, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DoomScrollEnd
	goto EventScript_DoomScrollAway

EventScript_DoomScrollTillDay:
	msgbox Text_DoomScrollTillDay, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DoomScrollEnd
EventScript_DoomScrollAway:
	closemessage
	fadescreen FADE_TO_BLACK
	msgbox Text_DoomScrolled
	closemessage
	special GameTime_ScrollToOtherTime
	waitstate
	releaseall
	end

EventScript_DoomScrollStoryTime:
	msgbox Text_DoomScrollStoryTime
	releaseall
	end

EventScript_DoomScrollTooEarly:
	msgbox Text_DoomScrollTooEarly
EventScript_DoomScrollEnd:
	releaseall
	end

Text_DoomScrollTillNight:
	.string "It's daytime.\n"
	.string "Scroll until night?$"

Text_DoomScrollTillDay:
	.string "It's nighttime.\n"
	.string "Scroll until morning?$"

Text_DoomScrolled:
	.string "{PLAYER} scrolled for hours…\p"
	.string "…………\p"
	.string "Where did the time go?$"

Text_DoomScrollStoryTime:
	.string "No time for scrolling now.\n"
	.string "The story isn't waiting!$"

Text_DoomScrollTooEarly:
	.string "There's coursework to do.\n"
	.string "Scrolling will have to wait.$"
