#include "stdafx.h"

// -- WEAPON --
void CScript::ShowMessageToPlayer(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char string[128];

	bitStream->Read(playerid);
	bitStream->Read(string);

	player.ShowMessageAboveMap(string);
}
void CScript::GivePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.GiveWeapon(weaponid, ammo);
}
void CScript::RemovePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];

	bitStream->Read(playerid);
	bitStream->Read(weaponid);

	weapon.RemoveWeapon(weaponid);
}
void CScript::GivePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.GiveAmmo(weaponid, ammo);
}
void CScript::RemovePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int playerid;
	char weaponid[20];
	int ammo;

	bitStream->Read(playerid);
	bitStream->Read(weaponid);
	bitStream->Read(ammo);

	weapon.RemoveAmmo(weaponid, ammo);
}