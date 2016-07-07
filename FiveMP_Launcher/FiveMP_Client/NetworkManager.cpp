#include "stdafx.h"

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

CNetworkManager::CNetworkManager()
{
	client = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	client->AttachPlugin(&rpc);
	rpc.RegisterSlot("ShowMessageToPlayer", ShowMessageToPlayer, 0);
	rpc.RegisterSlot("GivePlayerWeapon", GivePlayerWeapon, 0);
	rpc.RegisterSlot("RemovePlayerWeapon", RemovePlayerWeapon, 0);
	rpc.RegisterSlot("GivePlayerAmmo", GivePlayerAmmo, 0);
	rpc.RegisterSlot("RemovePlayerAmmo", RemovePlayerAmmo, 0);
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
