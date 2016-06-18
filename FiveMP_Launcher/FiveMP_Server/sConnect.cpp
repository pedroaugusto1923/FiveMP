#include "stdafx.h"

void sNetCode::Initialize()
{
	server = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	sNetCode::server->SetIncomingPassword("Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
	sNetCode::server->SetTimeoutTime(30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	//	RakNet::PacketLogger packetLogger;
	//	server->AttachPlugin(&packetLogger);
}

void sNetCode::Connect(char *port, int maxplayers)
{
	puts("Starting server.");

	sNetCode::socketDescriptors[0].port = atoi(port);
	sNetCode::socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
	sNetCode::socketDescriptors[1].port = atoi(port);
	sNetCode::socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
	bool b = sNetCode::server->Startup(maxplayers, sNetCode::socketDescriptors, 2) == RakNet::RAKNET_STARTED;
	sNetCode::server->SetMaximumIncomingConnections(maxplayers);
	if (!b)
	{
		b = sNetCode::server->Startup(maxplayers, sNetCode::socketDescriptors, 1) == RakNet::RAKNET_STARTED;
		if (!b)
		{
			puts("Server failed to start.  Terminating.");
			exit(1);
		}
	}

	sNetCode::server->SetOccasionalPing(true);
	sNetCode::server->SetUnreliableTimeout(1000);

	printf("\nMy GUID is %s\n", sNetCode::server->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
	puts("'quit' to quit. 'stat' to show stats. 'ping' to ping.\n'pingip' to ping an ip address\n'ban' to ban an IP from connecting.\n'kick to kick the first connected player.\nType to talk.");
}

unsigned char sNetCode::GetPacketIdentifier(RakNet::Packet *p)
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