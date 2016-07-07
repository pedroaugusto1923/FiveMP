#include "stdafx.h"

playerPool playerData[100];

CNetworkManager *NetworkManager;
CLocalPlayer	*LocalPlayer;
CConfig			*Config;

void InitGameScript() {
	Config = new CConfig;

	Config->Read();

	printf("\nIP: %s\nPort: %s\nClient Port: %s\n\n", Config->server_ipaddress, Config->server_port, Config->client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", Config->client_username, Config->client_steam_def);

	NetworkManager = new CNetworkManager;

	srand(GetTickCount());
	RunGameScript();
}

void RunGameScript() {
	bool HasInitialized = false;

	time_t lasttime;

	while (true)
	{
		if (!HasInitialized) {
			LocalPlayer = new CLocalPlayer;

			float groundz;

			player.DisableScripts();
			SCRIPT::SHUTDOWN_LOADING_SCREEN();
			CAM::DO_SCREEN_FADE_IN(500);

			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(391.4746f, -1637.9750f, 22.4998f, &groundz, 1);
			ENTITY::SET_ENTITY_COORDS(LocalPlayer->playerPed, 391.4746f, -1637.9750f, groundz + 1.0f, true, true, true, true);

			ENTITY::FREEZE_ENTITY_POSITION(LocalPlayer->playerPed, 0);
			ENTITY::SET_ENTITY_VISIBLE(LocalPlayer->playerPed, true, 0);

			UI::DISPLAY_RADAR(true);
			UI::DISPLAY_HUD(true);

			HasInitialized = true;
		}

		float rotation_x, rotation_y, rotation_z, rotation_w;

		ENTITY::GET_ENTITY_QUATERNION(LocalPlayer->playerPed, &rotation_x, &rotation_y, &rotation_z, &rotation_w);

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

		sprintf(blenddata, "%f", AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(LocalPlayer->playerPed));
		sprintf(velocitydata, "X = %f | Y = %f | Z = %f", LocalPlayer->GetVelocity().x, LocalPlayer->GetVelocity().y, LocalPlayer->GetVelocity().z);
		sprintf(coorddata, "X = %f | Y = %f | Z = %f", LocalPlayer->GetCoords().x, LocalPlayer->GetCoords().y, LocalPlayer->GetCoords().z);

		draw_text(0.002f, 0.002f, alphadata, { 255, 255, 255, 255 });

		draw_text(0.750f, 0.925f, blenddata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.950f, velocitydata, { 255, 255, 255, 255 });
		draw_text(0.750f, 0.975f, coorddata, { 255, 255, 255, 255 });

		if (NetworkManager->Listening) {
			NetworkManager->Handle();

			if (NetworkManager->Connected && NetworkManager->Synchronized) {
				for (int i = 0; i < sizeof(playerData) / sizeof(*playerData); i++) {
					if (ENTITY::IS_ENTITY_OCCLUDED(playerData[i].pedPed)) {
						draw_text(playerData[i].screen_x, playerData[i].screen_y, "User", { 255, 255, 255, 255 });
					}
				}

				RakNet::BitStream PlayerBitStream_send;

				PlayerBitStream_send.Write((MessageID)ID_SEND_PLAYER_DATA);
				
				PlayerBitStream_send.Write(NetworkManager->playerid);

				PlayerBitStream_send.Write(LocalPlayer->GetType());
				PlayerBitStream_send.Write(LocalPlayer->GetModel());
				PlayerBitStream_send.Write(LocalPlayer->GetHealth());

				PlayerBitStream_send.Write(LocalPlayer->GetCoords().x);
				PlayerBitStream_send.Write(LocalPlayer->GetCoords().y);
				PlayerBitStream_send.Write(LocalPlayer->GetCoords().z);

				PlayerBitStream_send.Write(LocalPlayer->GetHeading());
				PlayerBitStream_send.Write(rotation_x);
				PlayerBitStream_send.Write(rotation_y);
				PlayerBitStream_send.Write(rotation_z);
				PlayerBitStream_send.Write(rotation_w);

				PlayerBitStream_send.Write(AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(LocalPlayer->playerPed));
				PlayerBitStream_send.Write(LocalPlayer->GetVelocity().x);
				PlayerBitStream_send.Write(LocalPlayer->GetVelocity().y);
				PlayerBitStream_send.Write(LocalPlayer->GetVelocity().z);

				PlayerBitStream_send.Write(time(0));

				NetworkManager->client->Send(&PlayerBitStream_send, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
		}

		if (!NetworkManager->Synchronized && NetworkManager->Connected) {
			RakNet::BitStream requestid;

			RakNet::RakString playerUsername("%s", Config->client_username);

			requestid.Write((MessageID)ID_REQUEST_SERVER_SYNC);
			requestid.Write(playerUsername);

			NetworkManager->client->Send(&requestid, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

			player.ShowMessageAboveMap("Synchronizing with the server...");

			NetworkManager->Synchronized = true;
		}

		if (IsKeyJustUp(VK_F8) && !NetworkManager->Listening) {
			Config->Read();

			if (!NetworkManager->Connect(Config->server_ipaddress, Config->server_port, Config->client_port)) {
				player.ShowMessageAboveMap("An error occured while calling the ~~connect ~w~function");
			}
		}
		if (IsKeyJustUp(VK_F9)) {
			if (!NetworkManager->Disconnect()) {
				player.ShowMessageAboveMap("Could not disconnect: ~r~Not connected");
			}
		}
		if (IsKeyJustUp(VK_F10)) {
			Vector3 playerOffsetLocation = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(LocalPlayer->playerPed, 0.0, 3.0, 0.0);
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
