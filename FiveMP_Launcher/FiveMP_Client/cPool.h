#pragma once

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

	float v;								// Velocity
	float vx;								// Velocity X coord
	float vy;								// Velocity Y coord
	float vz;								// Velocity Z coord

	float screen_x;							// 2D X coord
	float screen_y;							// 2D Y coord

	float lastone;
	int tickssince;

	bool used = false;						// Whether the player slot is in use or not.
};
extern playerPool playerData[128];

struct vehiclePool {
	int vehicleid;							// Vehicle ID (server)
	int *localvehicleid;					// Vehicle ID (client)

	Vehicle vehicleVehicle;					// Used to spawn the vehicle and such.
	Blip vehicleBlip;						// Vehicle Blip
	Hash vehicleModel;						// Vehicle Model in Hash
	int vehicleHealth;						// Vehicle Health

	int vehicleColor1;						// Vehicle Color 1
	int vehicleColor2;						// Vehicle Color 2

	float x;								// Position X coord
	float y;								// Position Y coord
	float z;								// Position Z coord

	float r;								// Rotation (0-360)
	float rx;								// Rotation X coord
	float ry;								// Rotation Y coord
	float rz;								// Rotation Z coord
	float rw;								// Rotation W coord

	float screen_x;							// 2D X coord
	float screen_y;							// 2D Y coord

	bool used = false;						// Whether the vehicle slot is in use or not.
};
extern vehiclePool vehicleData[100];