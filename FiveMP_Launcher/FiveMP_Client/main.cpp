#include "stdafx.h"

class cNetCode netCode;
class GamePlayer player;

unsigned char packetIdentifier;

char *server_ipaddress;
char *server_port;

char *client_port;
char *client_username;
bool client_steam_def;

bool netListen = false;

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

	char test2[120];
	char test3[120];
	char stupidbs[128];

	//float test4 = ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(playerPed, test2, test3);
	//sprintf(stupidbs, "%f | %s | %s", test4, test2, test3);
	//draw_text(0.005f, 0.050f, stupidbs, test);

	if (IsKeyPressed(VK_F8)) {
		netCode.Connect(server_ipaddress, server_port, client_port);
		netListen = true;
	}
	if (IsKeyPressed(VK_F9)) {
		netCode.Disconnect();
		netListen = false;
	}

	if(netListen == true) {
		for (netCode.p = netCode.client->Receive(); netCode.p; netCode.client->DeallocatePacket(netCode.p), netCode.p = netCode.client->Receive()) {
			packetIdentifier = netCode.GetPacketIdentifier(netCode.p);

			RakNet::BitStream playerClientID(netCode.p->data + 1, 32, false);

			RakNet::BitStream RequestID;

			char testmessage[128];

			switch (packetIdentifier) {
			case ID_CONNECTION_REQUEST_ACCEPTED:
				netCode.Player_IsConnected = true;
				netCode.Player_ShouldDisconnect = false;

				sprintf(testmessage, "Hi %s, you have successfully connected to the server!", player.GetPlayerSocialClubName());
				player.ShowMessageAboveMap(testmessage);

				sprintf(testmessage, "GUID is: #%s", netCode.client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
				player.ShowMessageAboveMap(testmessage);

				char playerUsernamePacket[64];
				playerUsernamePacket[0] = 0;

				strncat(playerUsernamePacket, playerUsername, sizeof(playerUsernamePacket));

				RequestID.Write((unsigned char)ID_REQUEST_SERVER_SYNC);
				RequestID.Write(playerUsernamePacket);

				netCode.client->Send(&RequestID, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);
				break;

			case ID_CONNECTION_ATTEMPT_FAILED:
				netCode.Player_IsConnected = false;
				netCode.Player_ShouldDisconnect = true;

				player.ShowMessageAboveMap("Failed to connect!");
				netListen = false;
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				netCode.Player_IsConnected = false;
				netCode.Player_ShouldDisconnect = true;

				player.ShowMessageAboveMap("Server is full!");
				netListen = false;
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				netCode.Player_IsConnected = false;
				netCode.Player_ShouldDisconnect = true;

				player.ShowMessageAboveMap("Disconnected!");
				netListen = false;
				break;

			case ID_CONNECTION_LOST:
				netCode.Player_IsConnected = false;
				netCode.Player_ShouldDisconnect = true;

				player.ShowMessageAboveMap("Connection Lost!");
				netListen = false;
				break;

			case ID_CONNECTION_BANNED:
				netCode.Player_IsConnected = false;
				netCode.Player_ShouldDisconnect = true;

				player.ShowMessageAboveMap("You're banned from the server!");
				netListen = false;
				break;

			case ID_SET_CLIENT_ID:
				playerClientID.Read(netCode.Player_ClientID);

				playerClientID.Read(netCode.Server_Time_Hour);
				playerClientID.Read(netCode.Server_Time_Minute);
				playerClientID.Read(netCode.Server_Time_Pause);

				sprintf(testmessage, "Client ID: %d\n", netCode.Player_ClientID);
				player.ShowMessageAboveMap(testmessage);

				TIME::SET_CLOCK_TIME(netCode.Server_Time_Hour, netCode.Server_Time_Minute, 01);
				TIME::PAUSE_CLOCK(netCode.Server_Time_Pause);

				netCode.Player_HasID = true;
				netCode.Player_ShouldDisconnect = false;
				break;

			default:
				sprintf(testmessage, "%s", netCode.p->data);
				player.ShowMessageAboveMap(testmessage);

				sprintf(testmessage, "Exception from %s\n", netCode.p->data);
				netCode.client->Send(testmessage, (int)strlen(testmessage) + 1, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);

				netCode.Player_ShouldDisconnect = false;
				break;
			}
		}
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
	printf("Username: %s\nUsing Steam:%d\n\n", client_username, client_steam_def);
}