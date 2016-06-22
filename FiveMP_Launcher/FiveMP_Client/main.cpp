#include "stdafx.h"

void InitGameScript() {
	netCode.Initialize();

	CIniReader iniReader(".\\FiveMP.ini");

	server_ipaddress = iniReader.ReadString("Connection", "ip", "");
	server_port = iniReader.ReadString("Connection", "port", "");
	client_port = iniReader.ReadString("Connection", "clientport", "");

	client_username = iniReader.ReadString("Details", "username", "");
	client_steam_def = iniReader.ReadBoolean("Details", "steam", "");

	printf("\nIP: %s\nPort: %s\nClient Port: %s\n\n", server_ipaddress, server_port, client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", client_username, client_steam_def);

	srand(GetTickCount());
	RunGameScript();
}

void RunGameScript() {
	while (true)
	{
		VEHICLE::SET_GARBAGE_TRUCKS(false);
		VEHICLE::SET_RANDOM_BOATS(false);
		VEHICLE::SET_RANDOM_TRAINS(false);
		VEHICLE::SET_FAR_DRAW_VEHICLES(false);
		VEHICLE::_0xF796359A959DF65D(false);
		VEHICLE::DELETE_ALL_TRAINS();

		MOBILE::DESTROY_MOBILE_PHONE();

		VEHICLE::SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
		VEHICLE::SET_NUMBER_OF_PARKED_VEHICLES(-1);
		VEHICLE::SET_ALL_LOW_PRIORITY_VEHICLE_GENERATORS_ACTIVE(false);
		STREAMING::SET_VEHICLE_POPULATION_BUDGET(0);
		STREAMING::SET_PED_POPULATION_BUDGET(0);
		VEHICLE::SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);

		CONTROLS::DISABLE_CONTROL_ACTION(2, 19, true);

		//color_t test{ 255, 255, 255, 255 };

		//draw_text(0.002f, 0.002f, "FiveMP Alpha - 20-6-16", test);

		Ped playerPed = PLAYER::PLAYER_PED_ID();

		if (IsKeyDown(VK_F10)) {
			Vector3 playerOffsetLocation = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 3.0, 0.0);
			Hash playermodel1 = GAMEPLAY::GET_HASH_KEY("a_f_y_skater_01");
			Ped player33;

			PED::CREATE_RANDOM_PED(playerOffsetLocation.x, playerOffsetLocation.y, playerOffsetLocation.z);

			if (STREAMING::IS_MODEL_IN_CDIMAGE(playermodel1) && STREAMING::IS_MODEL_VALID(playermodel1))

				STREAMING::REQUEST_MODEL(playermodel1);
			while (!STREAMING::HAS_MODEL_LOADED(playermodel1))
				WAIT(0);
			player33 = PED::CREATE_PED(4, playermodel1, playerOffsetLocation.x, playerOffsetLocation.y, playerOffsetLocation.z, 0.0f, true, true);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(playermodel1);

			PED::SET_PED_FLEE_ATTRIBUTES(player33, 0, 0);
			PED::SET_PED_COMBAT_ATTRIBUTES(player33, 17, 1);
			PED::SET_PED_CAN_RAGDOLL(player33, false);

			AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(player33, true);
		}

		WAIT(0); // Don't remove or you'll crash your game. :x
	}
}