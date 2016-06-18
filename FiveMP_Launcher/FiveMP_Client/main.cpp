#include "stdafx.h"

void RunGameScript() {
	char *playerUsername = SOCIALCLUB::_SC_GET_NICKNAME();

	Ped playerPed = PLAYER::PLAYER_PED_ID();

	Vehicle playerVeh = NULL;

	VEHICLE::SET_GARBAGE_TRUCKS(false);
	VEHICLE::SET_RANDOM_BOATS(false);
	VEHICLE::SET_RANDOM_TRAINS(false);
	VEHICLE::SET_FAR_DRAW_VEHICLES(false);

	VEHICLE::SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
	VEHICLE::SET_NUMBER_OF_PARKED_VEHICLES(-1);
	VEHICLE::SET_ALL_LOW_PRIORITY_VEHICLE_GENERATORS_ACTIVE(false);
	STREAMING::SET_VEHICLE_POPULATION_BUDGET(0);
	STREAMING::SET_PED_POPULATION_BUDGET(0);
	VEHICLE::SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);

	color_t test;
	test.red = 255;
	test.green = 255;
	test.blue = 255;
	test.alpha = 255;

	draw_text(0.005f, 0.050f, "FiveMP Alpha - 18|6|16", test);

	if (netCode.Player_NetListen == true) {
		netPacket.ReceivePacket(netCode.p, netCode.client);
	}

	if (IsKeyPressed(VK_F8)) {
		netCode.Connect(server_ipaddress, server_port, client_port);
		netCode.Player_NetListen = true;
	}
	if (IsKeyPressed(VK_F9)) {
		netCode.Disconnect();
		netCode.Player_NetListen = false;
	}
}

void RunMainScript() {
	netCode.Initialize();

	CIniReader iniReader("C:\\FiveMP\\FiveMP.ini");

	server_ipaddress	= iniReader.ReadString("Connection", "ip", "");
	server_port			= iniReader.ReadString("Connection", "port", "");
	client_port			= iniReader.ReadString("Connection", "clientport", "");

	client_username		= iniReader.ReadString("Details", "username", "");
	client_steam_def	= iniReader.ReadBoolean("Details", "steam", "");

	printf("IP: %s\nPort: %s\nClient Port: %s\n\n", server_ipaddress, server_port, client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", client_username, client_steam_def);
}