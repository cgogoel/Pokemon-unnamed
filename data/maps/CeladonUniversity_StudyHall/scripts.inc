@ Celadon University: the Study Hall, in the small building by the dorms (Act 2, docs/hack/celadon_university.md).

CeladonUniversity_StudyHall_MapScripts::
	.byte 0

CeladonUniversity_StudyHall_EventScript_Youngster::
	msgbox CeladonUniversity_StudyHall_Text_Youngster, MSGBOX_NPC
	end

CeladonUniversity_StudyHall_EventScript_Lass::
	msgbox CeladonUniversity_StudyHall_Text_Lass, MSGBOX_NPC
	end

CeladonUniversity_StudyHall_EventScript_CooltrainerF::
	msgbox CeladonUniversity_StudyHall_Text_CooltrainerF, MSGBOX_NPC
	end

CeladonUniversity_StudyHall_EventScript_FatMan::
	msgbox CeladonUniversity_StudyHall_Text_FatMan, MSGBOX_NPC
	end

CeladonUniversity_StudyHall_EventScript_NoticeBoard::
	msgbox CeladonUniversity_StudyHall_Text_NoticeBoard, MSGBOX_SIGN
	end

CeladonUniversity_StudyHall_EventScript_Shelves::
	msgbox CeladonUniversity_StudyHall_Text_Shelves, MSGBOX_SIGN
	end

CeladonUniversity_StudyHall_Text_Youngster::
	.string "I've read this chapter four times.\n"
	.string "I still mix up special moves and\l"
	.string "physical ones.$"

CeladonUniversity_StudyHall_Text_Lass::
	.string "The library shushes you for\n"
	.string "turning a page. Here you can\l"
	.string "at least whisper.$"

CeladonUniversity_StudyHall_Text_CooltrainerF::
	.string "Some seniors say there's a battle\n"
	.string "club at the stadium after dark.\p"
	.string "Nobody ever admits to going,\n"
	.string "though.$"

CeladonUniversity_StudyHall_Text_FatMan::
	.string "This machine ate my coins again.\n"
	.string "Third time today. I keep hoping.$"

CeladonUniversity_StudyHall_Text_NoticeBoard::
	.string "STUDY HALL\n"
	.string "Open all night during exams.\p"
	.string "Someone's pinned a lost GOLDEEN\n"
	.string "keychain to the corner.$"

CeladonUniversity_StudyHall_Text_Shelves::
	.string "Old exam papers, worn soft from\n"
	.string "years of last-minute studying.$"
