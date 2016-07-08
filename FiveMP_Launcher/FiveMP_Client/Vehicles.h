#pragma once
class GameVehicle
{
public:
	bool CreateVehicle(int vehicleid, char *modelname, Vector3 coords, float heading, int color1, int color2, bool respawn, int respawndelay);
};

