#pragma once

class GamePlayer {
public:
	void ShowMessageAboveMap(char * msg);
	char *GetPlayerSocialClubName();

	void DisableScripts();

	bool usingChat = false;
};