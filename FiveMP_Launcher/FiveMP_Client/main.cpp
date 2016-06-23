#include "stdafx.h"

void InitGameScript() {
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
	bool HasInitialized = false;

	RakNet::RakPeerInterface *client = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* packet;
	RakNet::SystemAddress clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;;
	RakNet::ConnectionAttemptResult car;

	bool Player_IsConnected = false;
	bool Player_HasID = false;
	bool Player_ShouldDisconnect = false;
	bool Player_NetListen = false;
	bool Player_Disconnected = false;
	bool Player_Synchronized = false;

	int Player_ClientID;
	int Server_Time_Hour;
	int Server_Time_Minute;
	bool Server_Time_Pause;

	while (true)
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		Vector3 playerCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 0.0, 0.0);
		//STREAMING::LOAD_SCENE(391.4746f, -1637.9750f, 22.4998f);

		if (HasInitialized == false) {
			float groundz;

			player.DisableScripts();
			SCRIPT::SHUTDOWN_LOADING_SCREEN();
			CAM::DO_SCREEN_FADE_IN(500);

			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(391.4746f, -1637.9750f, 22.4998f, &groundz, 1);
			ENTITY::SET_ENTITY_COORDS(playerPed, 391.4746f, -1637.9750f, groundz + 1.0f, true, true, true, true);
			
			ENTITY::FREEZE_ENTITY_POSITION(playerPed, 0);
			ENTITY::SET_ENTITY_VISIBLE(playerPed, true, 0);

			UI::DISPLAY_RADAR(true);
			UI::DISPLAY_HUD(true);

			HasInitialized = true;
		}

		GAMEPLAY::SET_TIME_SCALE(1.0);

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

		char coorddata[128];

		sprintf(coorddata, "X = %f | Y = %f | Z = %f", playerCoords.x, playerCoords.y, playerCoords.z);

		draw_text(0.002f, 0.002f, "FiveMP Alpha - 22-6-16", { 255, 255, 255, 255 });
		draw_text(0.750f, 0.900f, coorddata, { 255, 255, 255, 255 });

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

		if (Player_Synchronized == false && Player_IsConnected == true) {
			RakNet::BitStream RequestID;

			char playerUsernamePacket[64];

			sprintf(playerUsernamePacket, "%s", client_username);

			RequestID.Write((unsigned char)ID_REQUEST_SERVER_SYNC);
			RequestID.Write(playerUsernamePacket);

			client->Send(&RequestID, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);

			player.ShowMessageAboveMap("Synchronizing with the server...");

			Player_Synchronized = true;
		}

		if (Player_NetListen == true) {
			for (packet = client->Receive(); packet; client->DeallocatePacket(packet), packet = client->Receive()) {
				unsigned char packetIdentifier = GetPacketIdentifier(packet);

				RakNet::BitStream playerClientID(packet->data + 1, 32, false);

				char testmessage[128];

				switch (packetIdentifier) {
				case ID_CONNECTION_REQUEST_ACCEPTED:
					Player_IsConnected = true;
					Player_ShouldDisconnect = false;

					sprintf(testmessage, "Hi %s, you have successfully connected to the server!", player.GetPlayerSocialClubName());
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "GUID is: #%s", client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
					player.ShowMessageAboveMap(testmessage);
					break;

				case ID_CONNECTION_ATTEMPT_FAILED:
					Player_IsConnected = false;
					Player_ShouldDisconnect = true;

					player.ShowMessageAboveMap("Failed to connect!");
					Player_NetListen = false;
					break;

				case ID_NO_FREE_INCOMING_CONNECTIONS:
					Player_IsConnected = false;
					Player_ShouldDisconnect = true;

					player.ShowMessageAboveMap("Server is full!");
					Player_NetListen = false;
					break;

				case ID_DISCONNECTION_NOTIFICATION:
					Player_IsConnected = false;
					Player_ShouldDisconnect = true;

					player.ShowMessageAboveMap("Disconnected!");
					Player_NetListen = false;
					break;

				case ID_CONNECTION_LOST:
					Player_IsConnected = false;
					Player_ShouldDisconnect = true;

					player.ShowMessageAboveMap("Connection Lost!");
					Player_NetListen = false;
					break;

				case ID_CONNECTION_BANNED:
					Player_IsConnected = false;
					Player_ShouldDisconnect = true;

					player.ShowMessageAboveMap("You're banned from the server!");
					Player_NetListen = false;
					break;

				case ID_REQUEST_SERVER_SYNC:
					TIME::SET_CLOCK_TIME(20, 00, 00);
					TIME::PAUSE_CLOCK(false);

					playerClientID.Read(Player_ClientID);

					playerClientID.Read(Server_Time_Hour);
					playerClientID.Read(Server_Time_Minute);
					playerClientID.Read(Server_Time_Pause);

					printf("%d - %d - %d", Server_Time_Hour, Server_Time_Minute, Server_Time_Pause);

					sprintf(testmessage, "Client ID: %d\n", Player_ClientID);
					player.ShowMessageAboveMap(testmessage);

					TIME::SET_CLOCK_TIME(Server_Time_Hour, Server_Time_Minute, 00);
					TIME::PAUSE_CLOCK(Server_Time_Pause);

					Player_HasID = true;
					Player_ShouldDisconnect = false;
					break;

				default:
					sprintf(testmessage, "%s", packet->data);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "Exception from %s\n", packet->data);
					client->Send(testmessage, (int)strlen(testmessage) + 1, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);

					Player_ShouldDisconnect = false;
					break;
				}
			}
		}

		if (IsKeyDown(VK_F8)) {
			RakNet::SocketDescriptor socketDescriptor(atoi(client_port), 0);

			socketDescriptor.socketFamily = AF_INET;
			client->Startup(8, &socketDescriptor, 1);
			client->SetOccasionalPing(true);

			car = client->Connect(server_ipaddress, atoi(server_port), "fivemp_dev", (int)strlen("fivemp_dev"));
			RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);

			Player_NetListen = true;
		}
		if (IsKeyDown(VK_F9)) {
			if (Player_NetListen == true) {
				client->Shutdown(300);

				Player_IsConnected = false;
				Player_HasID = false;
				Player_ShouldDisconnect = false;
				Player_NetListen = false;
				Player_Disconnected = true;
				Player_Synchronized = false;

				player.ShowMessageAboveMap("Successfully disconnected!");
			}

			Player_NetListen = false;
		}

		WAIT(0); // Don't remove or you'll crash your game. :x
	}
}

unsigned char GetPacketIdentifier(RakNet::Packet *p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char)p->data[0];
}
