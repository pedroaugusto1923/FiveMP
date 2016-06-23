#include "stdafx.h"

struct playerPool {
	int playerid;					// Player/Client ID

	char *username;					// Player Username (socialclub)

	const char *playerguid;	// Player GUID (client side)

	float x;						// Position X coord
	float y;						// Position Y coord
	float z;						// Position Z coord
	float r;						// Rotation (0-360)

	bool used;
};
playerPool playerData[100];

bool UserPool::AddToUserPool(char *username, const char *guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].used == false) {
			playerData[i].playerid		= i;
			playerData[i].username		= username;
			playerData[i].playerguid	= guid;
			
			playerData[i].used = true;
			return true;
		}
	}
}

bool UserPool::RemoveFromUserPool(const char *guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			playerData[i].playerid = -1;
			playerData[i].username = NULL;
			playerData[i].playerguid = NULL;

			playerData[i].used = false;
			return true;
		}
	}
}

int UserPool::GetPlayerID(const char *guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].playerid;
		}
	}
}

char *UserPool::GetPlayerUsername(const char *guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].username;
		}
	}
}

const char *UserPool::GetPlayerGUID(char *username)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].username == username) {
			return playerData[i].playerguid;
		}
	}
}
