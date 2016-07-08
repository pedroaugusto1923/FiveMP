#include "stdafx.h"

CNetworkManager::CNetworkManager()
{
	client = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
}

CNetworkManager::~CNetworkManager()
{
	Connected = false;
	Listening = false;
	Synchronized = false;

	playerid = -1;
	time_hour = NULL;
	time_minute = NULL;
	time_pause = NULL;

	RPCManager->UnRegisterRPCs();
}

bool CNetworkManager::Connect(char *serveraddress, char *port, char *clientport)
{
	if (serveraddress != NULL && port != NULL && clientport != NULL) {
		RakNet::SocketDescriptor socketDescriptor(atoi(clientport), 0);

		socketDescriptor.socketFamily = AF_INET;
		client->Startup(8, &socketDescriptor, 1);
		client->SetOccasionalPing(true);

		connection = client->Connect(serveraddress, atoi(port), "fivemp_dev", (int)strlen("fivemp_dev"));
		RakAssert(connection == RakNet::CONNECTION_ATTEMPT_STARTED);

		Listening = true;
		return true;
	}

	return false;
}

bool CNetworkManager::Disconnect()
{
	if (Listening) {
		client->Shutdown(300);

		Connected = false;
		Synchronized = false;
		Listening = false;

		player.ShowMessageAboveMap("Successfully disconnected!");
		return true;
	}

	return false;
}

void CNetworkManager::Handle()
{
	for (packet = client->Receive(); packet; client->DeallocatePacket(packet), packet = client->Receive()) {
		unsigned char packetIdentifier = GetPacketIdentifier(packet);

		RakNet::BitStream playerClientID(packet->data+1, packet->length+1, false);

		RakNet::BitStream bsPlayerSpawn;

		char testmessage[128];

		switch (packetIdentifier) {
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Connected = true;

			sprintf(testmessage, "Hi ~b~%s~w~, you have successfully connected to the server!", Config->client_username);
			player.ShowMessageAboveMap(testmessage);

			sprintf(testmessage, "GUID is: ~b~#%s", client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
			player.ShowMessageAboveMap(testmessage);
			break;

		case ID_CONNECTION_ATTEMPT_FAILED:
			Connected = false;
			Synchronized = false;

			player.ShowMessageAboveMap("~r~Could not connect to the server");
			Listening = false;
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Connected = false;
			Synchronized = false;

			player.ShowMessageAboveMap("~r~Server is full!");
			Listening = false;
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			Connected = false;
			Synchronized = false;

			player.ShowMessageAboveMap("~r~Connection closed!");
			Listening = false;
			break;

		case ID_CONNECTION_LOST:
			Connected = false;
			Synchronized = false;

			player.ShowMessageAboveMap("~r~Connection Lost!");
			Listening = false;
			break;

		case ID_CONNECTION_BANNED:
			Connected = false;
			Synchronized = false;

			player.ShowMessageAboveMap("~r~You're banned from this server!");
			Listening = false;
			break;

		case ID_REQUEST_SERVER_SYNC:
			TIME::SET_CLOCK_TIME(20, 00, 00);
			TIME::PAUSE_CLOCK(false);

			playerClientID.Read(playerid);

			playerClientID.Read(time_hour);
			playerClientID.Read(time_minute);
			playerClientID.Read(time_pause);

			TIME::ADVANCE_CLOCK_TIME_TO(time_hour, time_minute, 00);
			TIME::PAUSE_CLOCK(time_pause);

			bsPlayerSpawn.Write(playerid);
			rpc.Signal("PlayerConnect", &bsPlayerSpawn, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, UNASSIGNED_SYSTEM_ADDRESS, true, false);
			break;

		case ID_SEND_PLAYER_DATA:
			HandlePlayerSync(packet);
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

void CNetworkManager::HandlePlayerSync(Packet * p)
{
	RakNet::BitStream PlayerBitStream_receive(p->data+1, p->length+1, false);

	int tempplyrid;

	time_t temptimestamp;

	PlayerBitStream_receive.Read(tempplyrid);

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

	playerData[tempplyrid].tickssince = GetTickCount();

	//if (tempplyrid != playerid) {
		if (ENTITY::DOES_ENTITY_EXIST(playerData[tempplyrid].pedPed)) {
			/*float tempz;

			GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, &tempz, 1);

			if (SYSTEM::VDIST(playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].x, playerData[tempplyrid].y, tempz) > 5.0f) {
				ENTITY::SET_ENTITY_COORDS(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, 0, 0, 0, 0);
			}
			else {
				ENTITY::SET_ENTITY_COORDS(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, tempz, 0, 0, 0, 0);
				//AI::TASK_GO_STRAIGHT_TO_COORD(playerData[tempplyrid].pedPed, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].v, 1, playerData[tempplyrid].r, 0.0f);
			}
			ENTITY::SET_ENTITY_QUATERNION(playerData[tempplyrid].pedPed, playerData[tempplyrid].rx, playerData[tempplyrid].ry, playerData[tempplyrid].rz, playerData[tempplyrid].rw);*/
		} else {
			if (STREAMING::IS_MODEL_IN_CDIMAGE(playerData[tempplyrid].pedModel) && STREAMING::IS_MODEL_VALID(playerData[tempplyrid].pedModel))

				STREAMING::REQUEST_MODEL(playerData[tempplyrid].pedModel);
			while (!STREAMING::HAS_MODEL_LOADED(playerData[tempplyrid].pedModel))
				WAIT(0);
			playerData[tempplyrid].pedPed = PED::CREATE_PED(playerData[tempplyrid].pedType, playerData[tempplyrid].pedModel, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, 0.0f, false, true);
			STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(playerData[tempplyrid].pedModel);

			ENTITY::SET_ENTITY_NO_COLLISION_ENTITY(LocalPlayer->playerPed, playerData[tempplyrid].pedPed, false);
			ENTITY::SET_ENTITY_NO_COLLISION_ENTITY(playerData[tempplyrid].pedPed, LocalPlayer->playerPed, false);

			PED::SET_PED_FLEE_ATTRIBUTES(playerData[tempplyrid].pedPed, 0, 0);
			PED::SET_PED_COMBAT_ATTRIBUTES(playerData[tempplyrid].pedPed, 17, 1);
			PED::SET_PED_CAN_RAGDOLL(playerData[tempplyrid].pedPed, false);

			AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(playerData[tempplyrid].pedPed, true);

			playerData[tempplyrid].pedBlip = UI::ADD_BLIP_FOR_ENTITY(playerData[tempplyrid].pedPed);
			UI::SET_BLIP_AS_FRIENDLY(playerData[tempplyrid].pedBlip, true);
			UI::SET_BLIP_COLOUR(playerData[tempplyrid].pedBlip, 0);
			UI::SET_BLIP_SCALE(playerData[tempplyrid].pedBlip, 1.0f);
			UI::BEGIN_TEXT_COMMAND_SET_BLIP_NAME("STRING");
			UI::_ADD_TEXT_COMPONENT_STRING3("NAME test");
			UI::END_TEXT_COMMAND_SET_BLIP_NAME(playerData[tempplyrid].pedBlip);
		}
	//}
}

float ttlerp(float v0, float v1, float t) {
	return (1 - t)*v0 + t*v1;
}

void CNetworkManager::SyncOnFoot()
{
	for (int i = 0; i < 10; i++) {
		if (ENTITY::DOES_ENTITY_EXIST(playerData[i].pedPed)) {
			Vector3 curpos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerData[i].pedPed, 0.0, 0.0, 0.0);
			CVector3 curpos1;
			curpos1.fX = curpos.x;
			curpos1.fY = curpos.y;
			curpos1.fZ = curpos.z;

			CVector3 newpos;
			newpos.fX = playerData[i].x;
			newpos.fY = playerData[i].y;
			newpos.fZ = playerData[i].z;

			int now = GetTickCount();
			int elapsedTime = now - playerData[i].tickssince;
			int duration = playerData[i].tickssince+100 - playerData[i].tickssince;
			float progress = elapsedTime / duration;

			CVector3 updpos;
			updpos.fX = ttlerp(curpos1.fX, newpos.fX, progress);
			updpos.fY = ttlerp(curpos1.fY, newpos.fY, progress);
			updpos.fZ = ttlerp(curpos1.fZ, newpos.fZ, progress);

			printf("%f - %f/%f/%f\n", progress, updpos.fX, updpos.fY, updpos.fZ);

			ENTITY::SET_ENTITY_COORDS(playerData[i].pedPed, updpos.fX, updpos.fY, updpos.fZ, 0, 0, 0, 0);
		}
	}
}
