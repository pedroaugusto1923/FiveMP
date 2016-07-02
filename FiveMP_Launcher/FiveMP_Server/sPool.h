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
	RakNet::RakNetGUID GetPlayerGUIDd(int playerid);
};

typedef DWORD Hash;

struct playerPool {
	int playerid;							// Player/Client ID
	char *playerusername;					// Player Username (set from client config)
	RakNet::RakNetGUID playerguid;			// Player GUID (client side)

	int pedType;							// Ped Type
	Hash pedModel;							// PedModel in hash
	int pedHealth;							// Ped Health

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