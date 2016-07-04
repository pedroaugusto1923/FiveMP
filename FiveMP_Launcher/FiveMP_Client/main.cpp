#include "stdafx.h"

RPC4 rpc;
RakNet::RakPeerInterface *client;
RakNet::Packet* packet;
RakNet::SystemAddress clientID;
RakNet::ConnectionAttemptResult car;

bool Player_IsConnected = false;
bool Player_NetListen = false;
bool Player_Synchronized = false;

int Player_ClientID;
int Server_Time_Hour;
int Server_Time_Minute;
bool Server_Time_Pause;

playerPool playerData[100];

void ShowMessageToPlayer(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char string[128];

	bitStream->Read(playerid);
	bitStream->Read(string);

	player.ShowMessageAboveMap(string);
}

void GivePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.GiveWeapon(weaponid, ammo);
}

void RemovePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];

	bitStream->Read(playerid);
	bitStream->Read(weaponid);

	weapon.RemoveWeapon(weaponid);
}

void GivePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.GiveAmmo(weaponid, ammo);
}

void RemovePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.RemoveAmmo(weaponid, ammo);
}

void InitGameScript() {
	CIniReader iniReader(".\\FiveMP.ini");

	server_ipaddress	= iniReader.ReadString("Connection", "ip", "");
	server_port			= iniReader.ReadString("Connection", "port", "");
	client_port			= iniReader.ReadString("Connection", "clientport", "");

	client_username		= iniReader.ReadString("Details", "username", "");
	client_steam_def	= iniReader.ReadBoolean("Details", "steam", "");

	printf("\nIP: %s\nPort: %s\nClient Port: %s\n\n", server_ipaddress, server_port, client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", client_username, client_steam_def);

	srand(GetTickCount());
	RunGameScript();
}

void RunGameScript() {
	bool HasInitialized = false;

	client		= RakNet::RakPeerInterface::GetInstance();
	clientID	= RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	time_t lasttime;

	client->AttachPlugin(&rpc);
	rpc.RegisterSlot("ShowMessageToPlayer", ShowMessageToPlayer, 0);
	rpc.RegisterSlot("GivePlayerWeapon", GivePlayerWeapon, 0);
	rpc.RegisterSlot("RemovePlayerWeapon", RemovePlayerWeapon, 0);
	rpc.RegisterSlot("GivePlayerAmmo", GivePlayerAmmo, 0);
	rpc.RegisterSlot("RemovePlayerAmmo", RemovePlayerAmmo, 0);

	while (true)
	{
		float rotation_x, rotation_y, rotation_z, rotation_w;
		char animDicti[24];
		char animNamei[24];

		Ped		playerPed		= PLAYER::PLAYER_PED_ID();
		Hash	playerModel		= ENTITY::GET_ENTITY_MODEL(playerPed);
		Vector3 playerCoords	= ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 0.0, 0.0);
		Vector3 playerVelocity	= ENTITY::GET_ENTITY_VELOCITY(playerPed);
		int		playerType		= PED::GET_PED_TYPE(playerPed);
		int		playerHealth	= ENTITY::GET_ENTITY_HEALTH(playerPed);
		float	playerHeading	= ENTITY::GET_ENTITY_HEADING(playerPed);
		float	playerAnimTime	= ENTITY::_GET_ANIM_DURATION(animDicti, animNamei);

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

		sprintf(alphadata, "FiveMP Alpha | %s - %s", __DATE__, __TIME__);

		sprintf(blenddata, "%f", AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(playerPed));
		sprintf(velocitydata, "X = %f | Y = %f | Z = %f", playerVelocity.x, playerVelocity.y, playerVelocity.z);
		sprintf(coorddata, "X = %f | Y = %f | Z = %f", playerCoords.x, playerCoords.y, playerCoords.z);

		draw_text(0.002f, 0.002f, alphadata, { 255, 255, 255, 255 });

		draw_text(0.750f, 0.925f, blenddata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.950f, velocitydata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.975f, coorddata, { 255, 255, 255, 255 });

		if (Player_NetListen) {
			if (Player_IsConnected && Player_Synchronized) {	
				/*for (int i = 0; i < 100; i++) {
					if (playerData[i].pedPed != playerPed && playerData[i].playerid < 2 && playerData[i].playerid != Player_ClientID) {
						UpdatePlayerPositionData(i);
					}
				}*/

				for (int i; i < sizeof(playerData); i++) {
					if (ENTITY::IS_ENTITY_OCCLUDED(playerData[i].pedPed)) {
						draw_text(playerData[i].screen_x, playerData[i].screen_y, "User", { 255, 255, 255, 255 });
					}
				}

				RakNet::BitStream PlayerBitStream_send;

				PlayerBitStream_send.Write((MessageID)ID_SEND_PLAYER_DATA);
				
				PlayerBitStream_send.Write(Player_ClientID);

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

				client->Send(&PlayerBitStream_send, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}

			for (packet = client->Receive(); packet; client->DeallocatePacket(packet), packet = client->Receive()) {
				unsigned char packetIdentifier = GetPacketIdentifier(packet);

				RakNet::BitStream playerClientID(packet->data + 1, 64, false);

				RakNet::BitStream PlayerBitStream_receive(packet->data + 1, 128, false);

				RakNet::BitStream bsPlayerSpawn;

				char testmessage[128];

				switch (packetIdentifier) {
				case ID_CONNECTION_REQUEST_ACCEPTED:
					Player_IsConnected = true;

					sprintf(testmessage, "Hi %s, you have successfully connected to the server!", client_username);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "GUID is: #%s", client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
					player.ShowMessageAboveMap(testmessage);
					break;

				case ID_CONNECTION_ATTEMPT_FAILED:
					Player_IsConnected = false;
					Player_Synchronized = false;

					player.ShowMessageAboveMap("Failed to connect!");
					Player_NetListen = false;
					break;

				case ID_NO_FREE_INCOMING_CONNECTIONS:
					Player_IsConnected = false;
					Player_Synchronized = false;

					player.ShowMessageAboveMap("Server is full!");
					Player_NetListen = false;
					break;

				case ID_DISCONNECTION_NOTIFICATION:
					Player_IsConnected = false;
					Player_Synchronized = false;

					player.ShowMessageAboveMap("Disconnected!");
					Player_NetListen = false;
					break;

				case ID_CONNECTION_LOST:
					Player_IsConnected = false;
					Player_Synchronized = false;

					player.ShowMessageAboveMap("Connection Lost!");
					Player_NetListen = false;
					break;

				case ID_CONNECTION_BANNED:
					Player_IsConnected = false;
					Player_Synchronized = false;

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

					printf("TIME: Hour %d - Minute %d - Freeze Time %d\n", Server_Time_Hour, Server_Time_Minute, Server_Time_Pause);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "CLIENTID: %d\n", Player_ClientID);
					player.ShowMessageAboveMap(testmessage);

					TIME::ADVANCE_CLOCK_TIME_TO(Server_Time_Hour, Server_Time_Minute, 00);
					TIME::PAUSE_CLOCK(Server_Time_Pause);

					bsPlayerSpawn.Write(Player_ClientID);
					rpc.Signal("PlayerConnect", &bsPlayerSpawn, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, false);
					break;

				case ID_SEND_PLAYER_DATA:
					int tempplyrid;

					time_t temptimestamp;

					PlayerBitStream_receive.Read(tempplyrid);

					tempplyrid++;
					
					playerData[tempplyrid].playerid = tempplyrid;

					/*playerData[tempplyrid].old_x = playerData[tempplyrid].x;
					playerData[tempplyrid].old_y = playerData[tempplyrid].y;
					playerData[tempplyrid].old_z = playerData[tempplyrid].z;

					playerData[tempplyrid].old_rx = playerData[tempplyrid].rx;
					playerData[tempplyrid].old_ry = playerData[tempplyrid].ry;
					playerData[tempplyrid].old_rz = playerData[tempplyrid].rz;
					playerData[tempplyrid].old_rw = playerData[tempplyrid].rw;*/

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
							ENTITY::SET_ENTITY_COORDS(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, 0, 0, 0, 0);
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
						}
					//}
					break;

				default:
					sprintf(testmessage, "%s", packet->data);
					player.ShowMessageAboveMap(testmessage);

					sprintf(testmessage, "Exception from %s\n", packet->data);
					client->Send(testmessage, (int)strlen(testmessage) + 1, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);
					break;
				}
			}
		}

		if (!Player_Synchronized && Player_IsConnected) {
			RakNet::BitStream requestid;

			RakNet::RakString playerUsername("%s", client_username);

			requestid.Write((MessageID)ID_REQUEST_SERVER_SYNC);
			requestid.Write(playerUsername);

			client->Send(&requestid, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

			player.ShowMessageAboveMap("Synchronizing with the server...");

			Player_Synchronized = true;
		}

		if (IsKeyDown(VK_F8) && !Player_NetListen) {
			RakNet::SocketDescriptor socketDescriptor(atoi(client_port), 0);

			socketDescriptor.socketFamily = AF_INET;
			client->Startup(8, &socketDescriptor, 1);
			client->SetOccasionalPing(true);

			car = client->Connect(server_ipaddress, atoi(server_port), "fivemp_dev", (int)strlen("fivemp_dev"));
			RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);

			Player_NetListen = true;
		}
		if (IsKeyDown(VK_F9)) {
			if (Player_NetListen) {
				client->Shutdown(300);

				Player_IsConnected = false;
				Player_NetListen = false;
				Player_Synchronized = false;

				player.ShowMessageAboveMap("Successfully disconnected!");
			}
		}
		if (IsKeyDown(VK_F10)) {
			Vector3 playerOffsetLocation = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 3.0, 0.0);
			Hash playermodel1 = GAMEPLAY::GET_HASH_KEY("a_f_y_skater_01");
			Ped player33;

			//PED::CREATE_RANDOM_PED(playerOffsetLocation.x, playerOffsetLocation.y, playerOffsetLocation.z);

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
