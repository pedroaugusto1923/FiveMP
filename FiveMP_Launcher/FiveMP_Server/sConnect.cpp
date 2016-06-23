#include "stdafx.h"

void sNetCode::Initialize()
{
	server = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	sNetCode::server->SetIncomingPassword("fivemp_dev", (int)strlen("fivemp_dev"));
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

void sNetCode::ReceivePacket() {
	for (sNetCode::p = sNetCode::server->Receive(); sNetCode::p; sNetCode::server->DeallocatePacket(sNetCode::p), sNetCode::p = sNetCode::server->Receive())
	{
		netCode.packetIdentifier = netCode.GetPacketIdentifier(sNetCode::p);

		RakNet::BitStream pid_bitStream;
		RakNet::BitStream pid_request(sNetCode::p->data + 1, 128, false);

		switch (netCode.packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			printf("ID_DISCONNECTION_NOTIFICATION from %s\n", sNetCode::p->systemAddress.ToString(true));;
			netPool.UserAmount--;
			break;

		case ID_NEW_INCOMING_CONNECTION:
			printf("Incoming connection (IP|PORT: %s - GUID: %s)\n", sNetCode::p->systemAddress.ToString(true), sNetCode::p->guid.ToString());
			netCode.clientID = sNetCode::p->systemAddress;

			printf("%s - %d\n", sNetCode::p->guid.ToString(), netPool.UserAmount);
			//PlayerInfo[userAmount][name] = userAmount;

			//callback.OnPlayerConnect(netPool.UserAmount);

			netPool.UserAmount++;
			break;

		case ID_REQUEST_SERVER_SYNC:
			printf("received syncrequest\n");
			
			/*CIniReader iniReader(".\\FiveMP.ini");

			netConfig.ServerPort = iniReader.ReadString("Connection", "port", "");
			netConfig.ServerName = iniReader.ReadString("Details", "servername", "");

			netConfig.MaxPlayers = iniReader.ReadInteger("Details", "maxplayers", 32);

			netConfig.ServerTimeHour = iniReader.ReadInteger("Details", "hour", 12);
			netConfig.ServerTimeMinute = iniReader.ReadInteger("Details", "min", 00);
			netConfig.ServerTimeFreeze = iniReader.ReadBoolean("Details", "freeze", false);*/

			bool notused;

			pid_request.Read(notused);

			printf("%d is username from syncrequest", notused);

			pid_bitStream.Write((unsigned char)ID_REQUEST_SERVER_SYNC);

			pid_bitStream.Write(netPool.UserAmount);

			pid_bitStream.Write(netConfig.ServerTimeHour);
			pid_bitStream.Write(netConfig.ServerTimeMinute);
			pid_bitStream.Write(netConfig.ServerTimeFreeze);

			sNetCode::server->Send(&pid_bitStream, IMMEDIATE_PRIORITY, RELIABLE, 0, sNetCode::p->systemAddress, false);
			break;

		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;

		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", sNetCode::p->systemAddress.ToString(true));
			break;

		case ID_CONNECTION_LOST:
			printf("ID_CONNECTION_LOST from %s\n", sNetCode::p->systemAddress.ToString(true));;
			netPool.UserAmount--;
			break;

		default:
			printf("%s\n", sNetCode::p->data);

			sprintf(netConfig.MsgLength, "%s", sNetCode::p->data);
			sNetCode::server->Send(netConfig.MsgLength, (const int)strlen(netConfig.MsgLength) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, sNetCode::p->systemAddress, true);

			break;
		}
	}
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