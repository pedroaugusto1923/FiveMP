#pragma once
class UserPool
{
public:
	int UserAmount;
};

typedef DWORD Hash;

struct playerPool {
	int playerid;							// Player/Client ID
	char *playerusername;					// Player Username (socialclub)

	Ped pedPed;								// Used to spawn the ped and such.
	Blip pedBlip;
	Hash pedModel;							// PedModel in hash
	int pedHealth;							// Ped Health
	int pedType;							// Ped Type

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