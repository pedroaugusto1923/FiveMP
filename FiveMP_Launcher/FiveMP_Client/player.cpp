#pragma once

#include "stdafx.h"

void GamePlayer::ShowMessageAboveMap(char* msg) {
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(msg);
	UI::_DRAW_NOTIFICATION(FALSE, FALSE); // = DRAW ABOVE MAP

	printf("%s\n", msg);
}

char *GamePlayer::GetPlayerSocialClubName() {
	return SOCIALCLUB::_SC_GET_NICKNAME();
}