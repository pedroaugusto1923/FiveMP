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

	RakNet::ConnectionAttemptResult car = cNetCode::client->Connect(ipaddress, atoi(port), "fivemp_dev", (int)strlen("fivemp_dev"));
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

void cNetCode::ReceivePacket(RakNet::Packet *packets, RakNet::RakPeerInterface *clients) {
	for (packets = cNetCode::client->Receive(); packets; cNetCode::client->DeallocatePacket(packets), packets = cNetCode::client->Receive()) {
		netCode.packetIdentifier = netCode.GetPacketIdentifier(packets);

		RakNet::BitStream playerClientID(packets->data + 1, 32, false);

		RakNet::BitStream RequestID;

		char testmessage[128];

		switch (netCode.packetIdentifier) {
		case ID_CONNECTION_REQUEST_ACCEPTED:
			netCode.Player_IsConnected = true;
			netCode.Player_ShouldDisconnect = false;

			sprintf(testmessage, "Hi %s, you have successfully connected to the server!", player.GetPlayerSocialClubName());
			player.ShowMessageAboveMap(testmessage);

			sprintf(testmessage, "GUID is: #%s", cNetCode::client->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
			player.ShowMessageAboveMap(testmessage);

			//char playerUsernamePacket[64];
			//playerUsernamePacket[0] = 0;

			//sprintf(playerUsernamePacket, "%s", client_username);

			RequestID.Write((unsigned char)ID_REQUEST_SERVER_SYNC);
			RequestID.Write(true);

			cNetCode::client->Send(&RequestID, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);

			printf("Requesting sync from server.\n");
			break;

		case ID_CONNECTION_ATTEMPT_FAILED:
			netCode.Player_IsConnected = false;
			netCode.Player_ShouldDisconnect = true;

			player.ShowMessageAboveMap("Failed to connect!");
			netCode.Player_NetListen = false;
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			netCode.Player_IsConnected = false;
			netCode.Player_ShouldDisconnect = true;

			player.ShowMessageAboveMap("Server is full!");
			netCode.Player_NetListen = false;
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			netCode.Player_IsConnected = false;
			netCode.Player_ShouldDisconnect = true;

			player.ShowMessageAboveMap("Disconnected!");
			netCode.Player_NetListen = false;
			break;

		case ID_CONNECTION_LOST:
			netCode.Player_IsConnected = false;
			netCode.Player_ShouldDisconnect = true;

			player.ShowMessageAboveMap("Connection Lost!");
			netCode.Player_NetListen = false;
			break;

		case ID_CONNECTION_BANNED:
			netCode.Player_IsConnected = false;
			netCode.Player_ShouldDisconnect = true;

			player.ShowMessageAboveMap("You're banned from the server!");
			netCode.Player_NetListen = false;
			break;

		case ID_REQUEST_SERVER_SYNC:
			TIME::SET_CLOCK_TIME(20, 00, 00);
			TIME::PAUSE_CLOCK(false);

			playerClientID.Read(netCode.Player_ClientID);

			playerClientID.Read(netCode.Server_Time_Hour);
			playerClientID.Read(netCode.Server_Time_Minute);
			playerClientID.Read(netCode.Server_Time_Pause);

			printf("%d - %d - %d", netCode.Server_Time_Hour, netCode.Server_Time_Minute, netCode.Server_Time_Pause);

			sprintf(testmessage, "Client ID: %d\n", netCode.Player_ClientID);
			player.ShowMessageAboveMap(testmessage);

			TIME::SET_CLOCK_TIME(netCode.Server_Time_Hour, netCode.Server_Time_Minute, 00);
			TIME::PAUSE_CLOCK(netCode.Server_Time_Pause);

			netCode.Player_HasID = true;
			netCode.Player_ShouldDisconnect = false;
			break;

		default:
			sprintf(testmessage, "%s", packets->data);
			player.ShowMessageAboveMap(testmessage);

			sprintf(testmessage, "Exception from %s\n", packets->data);
			clients->Send(testmessage, (int)strlen(testmessage) + 1, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);

			netCode.Player_ShouldDisconnect = false;
			break;
		}
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
