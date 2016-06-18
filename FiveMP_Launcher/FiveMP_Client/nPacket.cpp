#include "stdafx.h"

void cPacket::ReceivePacket(RakNet::Packet *packets, RakNet::RakPeerInterface *clients) {
	for (packets = clients->Receive(); packets; clients->DeallocatePacket(packets), packets = clients->Receive()) {
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

			sprintf(testmessage, "GUID is: #%s", clients->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
			player.ShowMessageAboveMap(testmessage);

			/*char playerUsernamePacket[64];
			playerUsernamePacket[0] = 0;

			strncat(playerUsernamePacket, client_username, sizeof(playerUsernamePacket));

			RequestID.Write((unsigned char)ID_REQUEST_SERVER_SYNC);
			RequestID.Write(playerUsernamePacket);

			clients->Send(&RequestID, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, false);*/
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

		case ID_SET_CLIENT_ID:
			playerClientID.Read(netCode.Player_ClientID);

			playerClientID.Read(netCode.Server_Time_Hour);
			playerClientID.Read(netCode.Server_Time_Minute);
			playerClientID.Read(netCode.Server_Time_Pause);

			sprintf(testmessage, "Client ID: %d\n", netCode.Player_ClientID);
			player.ShowMessageAboveMap(testmessage);

			TIME::SET_CLOCK_TIME(netCode.Server_Time_Hour, netCode.Server_Time_Minute, 01);
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