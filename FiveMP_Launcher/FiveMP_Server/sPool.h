#pragma once
class UserPool
{
public:
	int UserAmount;

	int AddToUserPool(char *username, RakNet::RakNetGUID guid);
	void RemoveFromUserPool(RakNet::RakNetGUID guid);
	
	int GetPlayerID(RakNet::RakNetGUID guid);
	char *GetPlayerUsername(RakNet::RakNetGUID guid);
	RakNet::RakNetGUID GetPlayerGUID(char *username);
};

struct playerPool {
	int playerid;					// Player/Client ID

	char *playerusername;					// Player Username (socialclub)

	RakNet::RakNetGUID playerguid;			// Player GUID (client side)

	float x;						// Position X coord
	float y;						// Position Y coord
	float z;						// Position Z coord
	float r;						// Rotation (0-360)

	bool used = false;				// Whether the player slot is in use or not.
};
extern playerPool playerData[100];