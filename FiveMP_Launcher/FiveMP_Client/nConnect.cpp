#pragma once

#include "stdafx.h"

class GamePlayer playernetcode;

void cNetCode::Initialize()
{
	client = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;
}

void cNetCode::Connect(char *ipaddress, char *port, char *clientport)
{
	RakNet::SocketDescriptor socketDescriptor(atoi(clientport), 0);

	socketDescriptor.socketFamily = AF_INET;
	cNetCode::client->Startup(8, &socketDescriptor, 1);
	cNetCode::client->SetOccasionalPing(true);

	RakNet::ConnectionAttemptResult car = cNetCode::client->Connect(ipaddress, atoi(port), "Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
	RakAssert(car == RakNet::CONNECTION_ATTEMPT_STARTED);
}

void cNetCode::Disconnect(void)
{
	if (cNetCode::Player_NetListen == true) {
		cNetCode::client->Shutdown(300);

		cNetCode::Player_IsConnected = false;
		cNetCode::Player_HasID = false;
		cNetCode::Player_ShouldDisconnect = false;
		cNetCode::Player_NetListen = false;
		cNetCode::Player_Disconnected = true;

		playernetcode.ShowMessageAboveMap("Successfully disconnected!");
	}
}

unsigned char cNetCode::GetPacketIdentifier(RakNet::Packet *p)
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