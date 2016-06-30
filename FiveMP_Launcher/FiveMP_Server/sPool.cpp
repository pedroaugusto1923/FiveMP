#include "stdafx.h"

struct playerPool {
	int playerid;					// Player/Client ID

	char *username;					// Player Username (socialclub)

	const char *playerguid;			// Player GUID (client side)

	float x;						// Position X coord
	float y;						// Position Y coord
	float z;						// Position Z coord
	float r;						// Rotation (0-360)

	bool used = false;				// Whether the player slot is in use or not.
};
playerPool playerData[100];

void UserPool::AddToUserPool(char *username, const char *guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].used == false) {
			playerData[i].playerid		= i;
			playerData[i].username		= username;
			playerData[i].playerguid	= guid;
			
			playerData[i].used = true;
			return;
		}
	}
	return;
}

void UserPool::RemoveFromUserPool(const char *guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			playerData[i].playerid		= -1;
			playerData[i].username		= NULL;
			playerData[i].playerguid	= NULL;

			playerData[i].used = false;
			return;
		}
	}
	return;
}

int UserPool::GetPlayerID(const char *guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].playerid;
		}
	}
	return -1;
}

char *UserPool::GetPlayerUsername(const char *guid)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].username;
		}
	}
}

const char *UserPool::GetPlayerGUID(char *username)
{
	for (int i = 0; i < sizeof(playerData); i++)
	{
		if (playerData[i].username == username) {
			return playerData[i].playerguid;
		}
	}
}
