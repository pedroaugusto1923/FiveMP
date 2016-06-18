#pragma once

class sConfig
{
public:
	char *ServerPort;
	char *ServerName;
	char *MsgLength;

	int MaxPlayers;
	int ServerTimeHour;
	int ServerTimeMinute;

	bool ServerTimeFreeze = false;
};

