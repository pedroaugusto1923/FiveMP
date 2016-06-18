#pragma once

class cNetCode {
public:
	unsigned char GetPacketIdentifier(RakNet::Packet * p);

	bool Player_IsConnected = false;
	bool Player_HasID = false;
	bool Player_ShouldDisconnect = false;
	bool Player_NetListen = false;
	bool Player_Disconnected = false;

	int Player_ClientID;
	int Server_Time_Hour;
	int Server_Time_Minute;
	int Server_Time_Pause;

	RakNet::RakPeerInterface *client;
	RakNet::Packet* p;
	RakNet::SystemAddress clientID;

	unsigned char packetIdentifier;

	void Connect(char *, char *, char *);
	void Disconnect(void);
	void Initialize(void);
};