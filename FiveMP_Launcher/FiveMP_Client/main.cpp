#include "stdafx.h"

playerPool playerData[100];
CNetworkManager *NetworkManager;

void InitGameScript() {
	CIniReader iniReader(".\\FiveMP.ini");

	server_ipaddress	= iniReader.ReadString("Connection", "ip", "");
	server_port			= iniReader.ReadString("Connection", "port", "");
	client_port			= iniReader.ReadString("Connection", "clientport", "");

	client_username		= iniReader.ReadString("Details", "username", "");
	client_steam_def	= iniReader.ReadBoolean("Details", "steam", "");

	printf("\nIP: %s\nPort: %s\nClient Port: %s\n\n", server_ipaddress, server_port, client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", client_username, client_steam_def);

	NetworkManager = new CNetworkManager;

	srand(GetTickCount());
	RunGameScript();
}

void RunGameScript() {
	bool HasInitialized = false;

	time_t lasttime;

	while (true)
	{
		float rotation_x, rotation_y, rotation_z, rotation_w;

		Ped		playerPed		= PLAYER::PLAYER_PED_ID();
		Hash	playerModel		= ENTITY::GET_ENTITY_MODEL(playerPed);
		Vector3 playerCoords	= ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 0.0, 0.0);
		Vector3 playerVelocity	= ENTITY::GET_ENTITY_VELOCITY(playerPed);
		int		playerType		= PED::GET_PED_TYPE(playerPed);
		int		playerHealth	= ENTITY::GET_ENTITY_HEALTH(playerPed);
		float	playerHeading	= ENTITY::GET_ENTITY_HEADING(playerPed);

		ENTITY::GET_ENTITY_QUATERNION(playerPed, &rotation_x, &rotation_y, &rotation_z, &rotation_w);

		if (!HasInitialized) {
			float groundz;

			player.DisableScripts();
			SCRIPT::SHUTDOWN_LOADING_SCREEN();
			CAM::DO_SCREEN_FADE_IN(500);

			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(391.4746f, -1637.9750f, 22.4998f, &groundz, 1);
			ENTITY::SET_ENTITY_COORDS(playerPed, 391.4746f, -1637.9750f, groundz+1.0f, true, true, true, true);
			
			ENTITY::FREEZE_ENTITY_POSITION(playerPed, 0);
			ENTITY::SET_ENTITY_VISIBLE(playerPed, true, 0);

			UI::DISPLAY_RADAR(true);
			UI::DISPLAY_HUD(true);

			HasInitialized = true;
		}

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

		char alphadata[128];

		char blenddata[32];
		char velocitydata[64];
		char coorddata[64];

		sprintf(alphadata, "Five~r~MP~w~ Alpha | %s - %s", __DATE__, __TIME__);

		sprintf(blenddata, "%f", AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(playerPed));
		sprintf(velocitydata, "X = %f | Y = %f | Z = %f", playerVelocity.x, playerVelocity.y, playerVelocity.z);
		sprintf(coorddata, "X = %f | Y = %f | Z = %f", playerCoords.x, playerCoords.y, playerCoords.z);

		draw_text(0.002f, 0.002f, alphadata, { 255, 255, 255, 255 });

		draw_text(0.750f, 0.925f, blenddata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.950f, velocitydata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.975f, coorddata, { 255, 255, 255, 255 });

		if (NetworkManager->Listening) {
			if (NetworkManager->Connected && NetworkManager->Synchronized) {
				for (int i; i < sizeof(playerData) / sizeof(*playerData); i++) {
					if (ENTITY::IS_ENTITY_OCCLUDED(playerData[i].pedPed)) {
						draw_text(playerData[i].screen_x, playerData[i].screen_y, "User", { 255, 255, 255, 255 });
					}
				}

				RakNet::BitStream PlayerBitStream_send;

				PlayerBitStream_send.Write((MessageID)ID_SEND_PLAYER_DATA);
				
				PlayerBitStream_send.Write(NetworkManager->playerid);

				PlayerBitStream_send.Write(playerType);
				PlayerBitStream_send.Write(playerModel);
				PlayerBitStream_send.Write(playerHealth);

				PlayerBitStream_send.Write(playerCoords.x);
				PlayerBitStream_send.Write(playerCoords.y);
				PlayerBitStream_send.Write(playerCoords.z);

				PlayerBitStream_send.Write(playerHeading);
				PlayerBitStream_send.Write(rotation_x);
				PlayerBitStream_send.Write(rotation_y);
				PlayerBitStream_send.Write(rotation_z);
				PlayerBitStream_send.Write(rotation_w);

				PlayerBitStream_send.Write(AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(playerPed));
				PlayerBitStream_send.Write(playerVelocity.x);
				PlayerBitStream_send.Write(playerVelocity.y);
				PlayerBitStream_send.Write(playerVelocity.z);

				PlayerBitStream_send.Write(time(0));

				NetworkManager->client->Send(&PlayerBitStream_send, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}

			for (NetworkManager->packet = NetworkManager->client->Receive(); NetworkManager->packet; NetworkManager->client->DeallocatePacket(NetworkManager->packet), NetworkManager->packet = NetworkManager->client->Receive()) {
				unsigned char packetIdentifier = GetPacketIdentifier(NetworkManager->packet);

				RakNet::BitStream playerClientID(NetworkManager->packet->data + 1, 64, false);

				RakNet::BitStream PlayerBitStream_receive(NetworkManager->packet->data + 1, 128, false);

				RakNet::BitStream bsPlayerSpawn;

				char testmessage[128];

				switch (packetIdentifier) {
				case ID_CONNECTION_REQUEST_ACCEPTED:
					NetworkManager->Connected = true;

					sprintf(testmessage, "Hi ~b~%s~w~, you have successfully connected to the server!", client_username);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "GUID is: ~b~#%s", NetworkManager->client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
					player.ShowMessageAboveMap(testmessage);
					break;

				case ID_CONNECTION_ATTEMPT_FAILED:
					NetworkManager->Connected = false;
					NetworkManager->Synchronized = false;

					player.ShowMessageAboveMap("~r~Could not connect to the server");
					NetworkManager->Listening = false;
					break;

				case ID_NO_FREE_INCOMING_CONNECTIONS:
					NetworkManager->Connected = false;
					NetworkManager->Synchronized = false;

					player.ShowMessageAboveMap("~r~Server is full!");
					NetworkManager->Listening = false;
					break;

				case ID_DISCONNECTION_NOTIFICATION:
					NetworkManager->Connected = false;
					NetworkManager->Synchronized = false;

					player.ShowMessageAboveMap("~r~Connection closed!");
					NetworkManager->Listening = false;
					break;

				case ID_CONNECTION_LOST:
					NetworkManager->Connected = false;
					NetworkManager->Synchronized = false;

					player.ShowMessageAboveMap("~r~Connection Lost!");
					NetworkManager->Listening = false;
					break;

				case ID_CONNECTION_BANNED:
					NetworkManager->Connected = false;
					NetworkManager->Synchronized = false;

					player.ShowMessageAboveMap("~r~You're banned from this server!");
					NetworkManager->Listening = false;
					break;

				case ID_REQUEST_SERVER_SYNC:
					TIME::SET_CLOCK_TIME(20, 00, 00);
					TIME::PAUSE_CLOCK(false);

					playerClientID.Read(NetworkManager->playerid);

					playerClientID.Read(NetworkManager->time_hour);
					playerClientID.Read(NetworkManager->time_minute);
					playerClientID.Read(NetworkManager->time_pause);

					printf("TIME: Hour ~b~%d ~w~- Minute ~b~%d ~w~- Freeze Time ~b~%d", NetworkManager->time_hour, NetworkManager->time_minute, NetworkManager->time_pause);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "CLIENTID: ~b~%d", NetworkManager->playerid);
					player.ShowMessageAboveMap(testmessage);

					TIME::ADVANCE_CLOCK_TIME_TO(NetworkManager->time_hour, NetworkManager->time_minute, 00);
					TIME::PAUSE_CLOCK(NetworkManager->time_pause);

					bsPlayerSpawn.Write(NetworkManager->playerid);
					NetworkManager->rpc.Signal("PlayerConnect", &bsPlayerSpawn, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, false);
					break;

				case ID_SEND_PLAYER_DATA:
					int tempplyrid;

					time_t temptimestamp;

					PlayerBitStream_receive.Read(tempplyrid);

					tempplyrid++;
					
					playerData[tempplyrid].playerid = tempplyrid;

					PlayerBitStream_receive.Read(playerData[tempplyrid].pedType);
					PlayerBitStream_receive.Read(playerData[tempplyrid].pedModel);
					PlayerBitStream_receive.Read(playerData[tempplyrid].pedHealth);

					PlayerBitStream_receive.Read(playerData[tempplyrid].x);
					PlayerBitStream_receive.Read(playerData[tempplyrid].y);
					PlayerBitStream_receive.Read(playerData[tempplyrid].z);

					PlayerBitStream_receive.Read(playerData[tempplyrid].r);
					PlayerBitStream_receive.Read(playerData[tempplyrid].rx);
					PlayerBitStream_receive.Read(playerData[tempplyrid].ry);
					PlayerBitStream_receive.Read(playerData[tempplyrid].rz);
					PlayerBitStream_receive.Read(playerData[tempplyrid].rw);

					PlayerBitStream_receive.Read(playerData[tempplyrid].v);
					PlayerBitStream_receive.Read(playerData[tempplyrid].vx);
					PlayerBitStream_receive.Read(playerData[tempplyrid].vy);
					PlayerBitStream_receive.Read(playerData[tempplyrid].vz);

					PlayerBitStream_receive.Read(temptimestamp);

					playerData[tempplyrid].lerp = 0.0f;

					//if (tempplyrid != Player_ClientID) {
						//printf("%s | %d - %x | %f, %f, %f | %f, %f, %f, %f\n", playerData[tempplyrid].playerusername, playerData[tempplyrid].pedType, playerData[tempplyrid].pedModel, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].rx, playerData[tempplyrid].ry, playerData[tempplyrid].rz, playerData[tempplyrid].rw);

						if (ENTITY::DOES_ENTITY_EXIST(playerData[tempplyrid].pedPed)) {
							float tempz;

							GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, &tempz, 1);

							if (SYSTEM::VDIST(playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].x, playerData[tempplyrid].y, tempz) > 5.0f) {
								ENTITY::SET_ENTITY_COORDS(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, 0, 0, 0, 0);
							}
							else {
								ENTITY::SET_ENTITY_COORDS(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, tempz, 0, 0, 0, 0);
								//AI::TASK_GO_STRAIGHT_TO_COORD(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].v, 1, playerData[tempplyrid].r, 0.0f);
							}
							ENTITY::SET_ENTITY_QUATERNION(playerData[tempplyrid].pedPed, playerData[tempplyrid].rx, playerData[tempplyrid].ry, playerData[tempplyrid].rz, playerData[tempplyrid].rw);

							GRAPHICS::_WORLD3D_TO_SCREEN2D(playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, &playerData[tempplyrid].screen_x, &playerData[tempplyrid].screen_y);
						} else {
							if (STREAMING::IS_MODEL_IN_CDIMAGE(playerData[tempplyrid].pedModel) && STREAMING::IS_MODEL_VALID(playerData[tempplyrid].pedModel))

								STREAMING::REQUEST_MODEL(playerData[tempplyrid].pedModel);
							while (!STREAMING::HAS_MODEL_LOADED(playerData[tempplyrid].pedModel))
								WAIT(0);
							playerData[tempplyrid].pedPed = PED::CREATE_PED(playerData[tempplyrid].pedType, playerData[tempplyrid].pedModel, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, 0.0f, false, true);
							STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(playerData[tempplyrid].pedModel);

							ENTITY::SET_ENTITY_NO_COLLISION_ENTITY(playerPed, playerData[tempplyrid].pedPed, false);
							ENTITY::SET_ENTITY_NO_COLLISION_ENTITY(playerData[tempplyrid].pedPed, playerPed, false);

							ENTITY::SET_ENTITY_ALPHA(playerData[tempplyrid].pedPed, 120, false);

							PED::SET_PED_FLEE_ATTRIBUTES(playerData[tempplyrid].pedPed, 0, 0);
							PED::SET_PED_COMBAT_ATTRIBUTES(playerData[tempplyrid].pedPed, 17, 1);
							PED::SET_PED_CAN_RAGDOLL(playerData[tempplyrid].pedPed, false);

							AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(playerData[tempplyrid].pedPed, true);

							playerData[tempplyrid].pedBlip = UI::ADD_BLIP_FOR_ENTITY(playerData[tempplyrid].pedPed);
							UI::SET_BLIP_AS_FRIENDLY(playerData[tempplyrid].pedBlip, true);
							UI::SET_BLIP_COLOUR(playerData[tempplyrid].pedBlip, 0);
							UI::SET_BLIP_SCALE(playerData[tempplyrid].pedBlip, 1.0f);
							UI::SET_BLIP_NAME_FROM_TEXT_FILE(playerData[tempplyrid].pedBlip, "FiveMP placeholder");
						}
					//}
					break;

				default:
					sprintf(testmessage, "%s", NetworkManager->packet->data);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "Exception from %s\n", NetworkManager->packet->data);
					NetworkManager->client->Send(testmessage, (int)strlen(testmessage) + 1, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);
					break;
				}
			}
		}

		if (!NetworkManager->Synchronized && NetworkManager->Connected) {
			RakNet::BitStream requestid;

			RakNet::RakString playerUsername("%s", client_username);

			requestid.Write((MessageID)ID_REQUEST_SERVER_SYNC);
			requestid.Write(playerUsername);

			NetworkManager->client->Send(&requestid, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

			player.ShowMessageAboveMap("Synchronizing with the server...");

			NetworkManager->Synchronized = true;
		}

		if (IsKeyJustUp(VK_F8) && !NetworkManager->Listening) {
			if (!NetworkManager->Connect(server_ipaddress, server_port, client_port)) {
				player.ShowMessageAboveMap("An error occured while calling the ~~connect ~w~function");
			}
		}
		if (IsKeyJustUp(VK_F9)) {
			if (!NetworkManager->Disconnect()) {
				player.ShowMessageAboveMap("Could not disconnect: ~r~Not connected");
			}
		}
		if (IsKeyJustUp(VK_F10)) {
			Vector3 playerOffsetLocation = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 3.0, 0.0);
			Hash playermodel1 = GAMEPLAY::GET_HASH_KEY("a_f_y_skater_01");
			Ped player33;

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

		WAIT(0);
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
