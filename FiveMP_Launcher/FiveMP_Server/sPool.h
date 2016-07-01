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

typedef DWORD Hash;

struct playerPool {
	int playerid;							// Player/Client ID
	char *playerusername;					// Player Username (socialclub)
	RakNet::RakNetGUID playerguid;			// Player GUID (client side)

	int pedType;							// Ped Type
	Hash pedModel;							// Player Model in hash

	float x;								// Position X coord
	float y;								// Position Y coord
	float z;								// Position Z coord
	
	float rx;								// Rotation X coord
	float ry;								// Rotation Y coord
	float rz;								// Rotation Z coord
	float rw;								// Rotation W coord

	bool used = false;						// Whether the player slot is in use or not.
};
extern playerPool playerData[100];