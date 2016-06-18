#include "stdafx.h"

struct playerPool {
	int playerid;					// Player/Client ID

	char *username;					// Player Username (socialclub)

	RakNet::RakNetGUID *playerguid;	// Player GUID (client side)

	float x;						// Position X coord
	float y;						// Position Y coord
	float z;						// Position Z coord
	float r;						// Rotation (0-360)

	bool used;
};
playerPool playerData[100];

int UserPool::AddToUserPool(char *username, RakNet::RakNetGUID *guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].used == false) {
			playerData[i].playerid		= i;
			playerData[i].username		= username;
			playerData[i].playerguid	= guid;
			
			playerData[i].used = true;
			return i;
		}
	}
}

void UserPool::RemoveFromUserPool(RakNet::RakNetGUID * guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			playerData[i].playerid = -1;
			playerData[i].username = NULL;
			playerData[i].playerguid = NULL;

			playerData[i].used = false;
		}
	}
}

int UserPool::GetPlayerID(char * username)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].username == username) {
			return playerData[i].playerid;
		}
	}
}

char * UserPool::GetPlayerUsername(RakNet::RakNetGUID * guid)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].playerguid == guid) {
			return playerData[i].username;
		}
	}
}

RakNet::RakNetGUID * UserPool::GetPlayerGUID(char * username)
{
	for (int i; i < sizeof(playerData); i++)
	{
		if (playerData[i].username == username) {
			return playerData[i].playerguid;
		}
	}
}
