#include "stdafx.h"

playerPool playerData[100];
vehiclePool vehicleData[100];

CNetworkManager *NetworkManager;
CRPCManager		*RPCManager;
CLocalPlayer	*LocalPlayer;
CConfig			*Config;
CRenderDebug	*RenderDebug;
CRender			*Render;

void InitGameScript() {
	Config = new CConfig;

	Config->Read();

	printf("\nIP: %s\nPort: %s\nClient Port: %s\n\n", Config->server_ipaddress, Config->server_port, Config->client_port);
	printf("Username: %s\nUsing Steam: %d\n\n", Config->client_username, Config->client_steam_def);

	NetworkManager	= new CNetworkManager;
	RPCManager		= new CRPCManager;

	RPCManager->RegisterRPCs();

	srand(GetTickCount());
	RunGameScript();
}

void RunGameScript() {
	LocalPlayer = new CLocalPlayer;

	while (true)
	{
		LocalPlayer->Initialize();
		LocalPlayer->OnTick();

		RenderDebug->RenderDate();
		RenderDebug->RenderBlend();
		RenderDebug->RenderVelocity();
		RenderDebug->RenderCoords();

		if (NetworkManager->Listening) {
			NetworkManager->Handle();

			if (NetworkManager->Connected) {
				if (!NetworkManager->Synchronized) {
					LocalPlayer->SendSyncRequest();
				} else {
					Render->RenderNametags();
					LocalPlayer->SendOnFootData();
				}
			}
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
