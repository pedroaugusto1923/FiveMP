#pragma once
class CNetworkManager
{
public:
	CNetworkManager();
	~CNetworkManager();

	bool Connect(char *serveraddress, char *port, char *clientport);
	bool Disconnect();
	void Handle();
	void HandlePlayerSync(Packet *p);
	void DropPlayer(Packet *p);

	RPC4 rpc;
	RakNet::RakPeerInterface *client;
	RakNet::Packet* packet;
	RakNet::SystemAddress clientID;
	RakNet::ConnectionAttemptResult connection;

	bool Connected = false;
	bool Listening = false;
	bool Synchronized = false;

	int playerid;
	int time_hour;
	int time_minute;
	bool time_pause;
};

