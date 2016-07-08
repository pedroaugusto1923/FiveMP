#pragma once
class CScript
{
public:
	// Weapon
	void ShowMessageToPlayer(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void GivePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void RemovePlayerWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void GivePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet);
	void RemovePlayerAmmo(RakNet::BitStream *bitStream, RakNet::Packet *packet);

};

