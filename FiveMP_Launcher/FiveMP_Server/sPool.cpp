#include "stdafx.h"

int UserPool::AddToUserPool(char *username, RakNet::RakNetGUID guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].used == false) {
			playerData[i].playerid				= i;
			playerData[i].playerusername		= username;
			playerData[i].playerguid			= guid;
			playerData[i].used					= true;

			printf("%s - %s\n", playerData[i].playerguid.ToString(), guid.ToString());
			return i;
		}
	}
	return -1;
}

void UserPool::RemoveFromUserPool(RakNet::RakNetGUID guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			playerData[i].used = false;
			return;
		}
	}
}

int UserPool::GetPlayerID(RakNet::RakNetGUID guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].playerid;
		}
	}
	return -1;
}

char *UserPool::GetPlayerUsername(RakNet::RakNetGUID guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].playerusername;
		}
	}
}

RakNet::RakNetGUID UserPool::GetPlayerGUID(char *username)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerusername == username) {
			return playerData[i].playerguid;
		}
	}
}

RakNet::RakNetGUID UserPool::GetPlayerGUIDd(int playerid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerid == playerid) {
			return playerData[i].playerguid;
		}
	}
}