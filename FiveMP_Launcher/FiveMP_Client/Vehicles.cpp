#include "stdafx.h"

bool GameVehicle::CreateVehicle(int vehicleid, char *modelname, Vector3 coords, float heading, int color1, int color2, bool respawn, int respawndelay)
{
	Hash vehiclehash = GAMEPLAY::GET_HASH_KEY(modelname);

	if (STREAMING::IS_MODEL_IN_CDIMAGE(vehiclehash) && STREAMING::IS_MODEL_VALID(vehiclehash))
	{
		STREAMING::REQUEST_MODEL(vehiclehash);
		while (!STREAMING::HAS_MODEL_LOADED(vehiclehash))
			WAIT(0);
		vehicleData[vehicleid].vehicleVehicle = VEHICLE::CREATE_VEHICLE(vehiclehash, coords.x, coords.y, coords.z, heading, false, true);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehiclehash);

		VEHICLE::SET_VEHICLE_COLOURS(vehicleData[vehicleid].vehicleVehicle, color1, color2);

		vehicleData[vehicleid].used = true;
		return 1;
	}
	return 0;
}
