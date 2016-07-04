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
	
	float r;								// Rotation (0-360)
	float rx;								// Rotation X coord
	float ry;								// Rotation Y coord
	float rz;								// Rotation Z coord
	float rw;								// Rotation W coord

	/*float old_x;							// Old Position X coord
	float old_y;							// Old Position Y coord
	float old_z;							// Old Position Z coord

	float old_rx;							// Old Rotation X coord
	float old_ry;							// Old Rotation Y coord
	float old_rz;							// Old Rotation Z coord
	float old_rw;							// Old Rotation W coord*/

	float v;								// Velocity
	float vx;								// Velocity X coord
	float vy;								// Velocity Y coord
	float vz;								// Velocity Z coord

	float lerp = 0.0;

	float screen_x;							// 2D X coord
	float screen_y;							// 2D Y coord

	bool used = false;						// Whether the player slot is in use or not.
};
extern playerPool playerData[100];