#include "stdafx.h"

void initpls() {
	CIniReader iniReader(".\\FiveMP.ini");

	netConfig.ServerPort = iniReader.ReadString("Connection", "port", "");
	netConfig.ServerName = iniReader.ReadString("Details", "servername", "");

	netConfig.MaxPlayers = iniReader.ReadInteger("Details", "maxplayers", 32);

	netConfig.ServerTimeHour = iniReader.ReadInteger("Details", "hour", 12);
	netConfig.ServerTimeMinute = iniReader.ReadInteger("Details", "min", 00);
	netConfig.ServerTimeFreeze = iniReader.ReadBoolean("Details", "freeze", false);
}

void sPacket::ReceivePacket(RakNet::Packet *packets, RakNet::RakPeerInterface *servers) {
	for (packets = servers->Receive(); packets; servers->DeallocatePacket(packets), packets = servers->Receive())
	{
		netCode.packetIdentifier = netCode.GetPacketIdentifier(packets);

		RakNet::BitStream pid_bitStream;
		RakNet::BitStream pid_request(packets->data + 1, 32, false);

		switch (netCode.packetIdentifier)
		{
		case ID_DISCONNECTION_NOTIFICATION:
			printf("ID_DISCONNECTION_NOTIFICATION from %s\n", packets->systemAddress.ToString(true));;
			netPool.UserAmount--;
			break;

		case ID_NEW_INCOMING_CONNECTION:
			printf("Incoming connection (IP|PORT: %s - GUID: %s)\n", packets->systemAddress.ToString(true), packets->guid.ToString());
			netCode.clientID = packets->systemAddress;

			printf("%s - %d\n", packets->guid.ToString(), netPool.UserAmount);
			//PlayerInfo[userAmount][name] = userAmount;

			//callback.OnPlayerConnect(netPool.UserAmount);

			netPool.UserAmount++;
			break;

		case ID_REQUEST_SERVER_SYNC:
			printf("received syncrequest\n");
			initpls();

			char *notused;

			pid_request.Read(notused);

			pid_bitStream.Write((unsigned char)ID_REQUEST_SERVER_SYNC);

			pid_bitStream.Write(netPool.UserAmount);

			pid_bitStream.Write(netConfig.ServerTimeHour);
			pid_bitStream.Write(netConfig.ServerTimeMinute);
			pid_bitStream.Write(netConfig.ServerTimeFreeze);

			servers->Send(&pid_bitStream, IMMEDIATE_PRIORITY, RELIABLE, 0, packets->systemAddress, false);
			break;

		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
			break;

		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
			printf("Ping from %s\n", packets->systemAddress.ToString(true));
			break;

		case ID_CONNECTION_LOST:
			printf("ID_CONNECTION_LOST from %s\n", packets->systemAddress.ToString(true));;
			netPool.UserAmount--;
			break;

		default:
			printf("%s\n", packets->data);

			sprintf(netConfig.MsgLength, "%s", packets->data);
			servers->Send(netConfig.MsgLength, (const int)strlen(netConfig.MsgLength) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packets->systemAddress, true);

			break;
		}
	}
}