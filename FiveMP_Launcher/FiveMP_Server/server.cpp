#include "stdafx.h"

RakNet::RakPeerInterface *server;
RakNet::RakNetStatistics *rss;
RakNet::Packet* p;
RakNet::SystemAddress clientID;
RakNet::SocketDescriptor socketDescriptors[2];

int userAmount, vehicleAmount;
char userGuid;

RPC4 rpc;

playerPool playerData[100];

void PlayerConnect(RakNet::BitStream *bitStream, RakNet::Packet *packet) {
	int tempplayer;
	bitStream->Read(tempplayer);

	OnPlayerConnect(sLUA, tempplayer);
};

int main(void)
{
	unsigned char GetPacketIdentifier(RakNet::Packet * p);

	unsigned char packetIdentifier;

	CIniReader iniReader(".\\FiveMP.ini");

	netConfig.ServerPort		= iniReader.ReadString("Connection", "port", "");

	netConfig.ScriptGameMode	= iniReader.ReadString("Scripts", "gamemode", "");

	netConfig.ServerName		= iniReader.ReadString("Details", "servername", "");
	netConfig.MaxPlayers		= iniReader.ReadInteger("Details", "maxplayers", 32);

	netConfig.ServerTimeHour	= iniReader.ReadInteger("Details", "hour", 12);
	netConfig.ServerTimeMinute	= iniReader.ReadInteger("Details", "min", 00);
	netConfig.ServerTimeFreeze	= iniReader.ReadBoolean("Details", "freeze", false);

	printf("\n%s starting on Port: %s - time: %d - %d - %d\n", netConfig.ServerName, netConfig.ServerPort, netConfig.ServerTimeHour, netConfig.ServerTimeMinute, netConfig.ServerTimeFreeze);

	SetConsoleTitle("FiveMP - Server Console");

	server = RakNet::RakPeerInterface::GetInstance();
	clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

	server->SetIncomingPassword("fivemp_dev", (int)strlen("fivemp_dev"));
	server->SetTimeoutTime(15000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	puts("Starting server.");

	char tempgamemode[64];

	sprintf(tempgamemode, "gamemodes//%s.lua", netConfig.ScriptGameMode);

	sLUA = luaL_newstate();
	luaL_openlibs(sLUA);
	luaL_dofile(sLUA, tempgamemode);

	lua_register(sLUA, "SetPlayerUsername", SetPlayerUsername);
	lua_register(sLUA, "ShowMessageToPlayer", ShowMessageToPlayer);
	lua_register(sLUA, "GivePlayerWeapon", GivePlayerWeapon);
	lua_register(sLUA, "RemovePlayerWeapon", RemovePlayerWeapon);
	lua_register(sLUA, "GivePlayerAmmo", GivePlayerAmmo);
	lua_register(sLUA, "RemovePlayerAmmo", RemovePlayerAmmo);

	OnGameModeInit(sLUA);

	socketDescriptors[0].port = atoi(netConfig.ServerPort);
	socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
	socketDescriptors[1].port = atoi(netConfig.ServerPort);
	socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
	bool b = server->Startup(netConfig.MaxPlayers, socketDescriptors, 2) == RakNet::RAKNET_STARTED;
	server->SetMaximumIncomingConnections(netConfig.MaxPlayers);
	if (!b)
	{
		b = server->Startup(netConfig.MaxPlayers, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
		if (!b)
		{
			puts("Server failed to start.  Terminating.");
			exit(1);
		}
	}
	
	server->AttachPlugin(&rpc);

	rpc.RegisterSlot("PlayerConnect", PlayerConnect, 0);

	server->SetOccasionalPing(true);
	server->SetUnreliableTimeout(1000);

	char message[2048];
	int tempid;

	while (1)
	{
		RakSleep(100);

		for (p = server->Receive(); p; server->DeallocatePacket(p), p = server->Receive())
		{
			packetIdentifier = GetPacketIdentifier(p);

			RakNet::BitStream pid_bitStream;
			RakNet::BitStream pid_request(p->data + 1, 128, false);

			RakNet::BitStream PlayerBitStream_send;
			RakNet::BitStream PlayerBitStream_receive(p->data + 1, 128, false);

			switch (packetIdentifier)
			{
			case ID_DISCONNECTION_NOTIFICATION:
				printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString(true));;
				OnPlayerDisconnect(sLUA, netPool.GetPlayerID(p->guid));

				netPool.RemoveFromUserPool(p->guid);
				netPool.UserAmount--;
				break;

			case ID_NEW_INCOMING_CONNECTION:
				printf("Incoming connection (IP|PORT: %s - GUID: %s)\n", p->systemAddress.ToString(true), p->guid.ToString());
				clientID = p->systemAddress;

				netPool.UserAmount++;
				break;

			case ID_REQUEST_SERVER_SYNC:
				char tempname[64];

				pid_request.Read(tempname);

				tempid = netPool.AddToUserPool(tempname, p->guid);

				pid_bitStream.Write((unsigned char)ID_REQUEST_SERVER_SYNC);

				pid_bitStream.Write(tempid);

				pid_bitStream.Write(netConfig.ServerTimeHour);
				pid_bitStream.Write(netConfig.ServerTimeMinute);
				pid_bitStream.Write(netConfig.ServerTimeFreeze);

				server->Send(&pid_bitStream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, false);
				break;

			case ID_SEND_PLAYER_DATA:
				// Receive data
				int tempplyrid;
				time_t temptimestamp;

				PlayerBitStream_receive.Read(tempplyrid);

				PlayerBitStream_receive.Read(playerData[tempplyrid].pedType);
				PlayerBitStream_receive.Read(playerData[tempplyrid].pedModel);
				PlayerBitStream_receive.Read(playerData[tempplyrid].pedHealth);

				PlayerBitStream_receive.Read(playerData[tempplyrid].x);
				PlayerBitStream_receive.Read(playerData[tempplyrid].y);
				PlayerBitStream_receive.Read(playerData[tempplyrid].z);

				PlayerBitStream_receive.Read(playerData[tempplyrid].r);
				PlayerBitStream_receive.Read(playerData[tempplyrid].rx);
				PlayerBitStream_receive.Read(playerData[tempplyrid].ry);
				PlayerBitStream_receive.Read(playerData[tempplyrid].rz);
				PlayerBitStream_receive.Read(playerData[tempplyrid].rw);

				PlayerBitStream_receive.Read(playerData[tempplyrid].v);
				PlayerBitStream_receive.Read(playerData[tempplyrid].vx);
				PlayerBitStream_receive.Read(playerData[tempplyrid].vy);
				PlayerBitStream_receive.Read(playerData[tempplyrid].vz);

				PlayerBitStream_receive.Read(temptimestamp);

				// Send to other users.

				PlayerBitStream_send.Write((unsigned char)ID_SEND_PLAYER_DATA);

				PlayerBitStream_send.Write(tempplyrid);

				//RakNet::RakString tempusername = netPool.GetPlayerUsername(p->guid);

				//PlayerBitStream_send.Write(tempusername);

				PlayerBitStream_send.Write(playerData[tempplyrid].pedType);
				PlayerBitStream_send.Write(playerData[tempplyrid].pedModel);
				PlayerBitStream_send.Write(playerData[tempplyrid].pedHealth);

				PlayerBitStream_send.Write(playerData[tempplyrid].x);
				PlayerBitStream_send.Write(playerData[tempplyrid].y);
				PlayerBitStream_send.Write(playerData[tempplyrid].z);

				PlayerBitStream_send.Write(playerData[tempplyrid].r);
				PlayerBitStream_send.Write(playerData[tempplyrid].rx);
				PlayerBitStream_send.Write(playerData[tempplyrid].ry);
				PlayerBitStream_send.Write(playerData[tempplyrid].rz);
				PlayerBitStream_send.Write(playerData[tempplyrid].rw);

				PlayerBitStream_send.Write(playerData[tempplyrid].v);
				PlayerBitStream_send.Write(playerData[tempplyrid].vx);
				PlayerBitStream_send.Write(playerData[tempplyrid].vy);
				PlayerBitStream_send.Write(playerData[tempplyrid].vz);

				PlayerBitStream_send.Write(temptimestamp);

				server->Send(&PlayerBitStream_send, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

				//printf("%s | %d - %x | %f, %f, %f | %f, %f, %f, %f\n", playerData[tempplyrid].playerusername, playerData[tempplyrid].pedType, playerData[tempplyrid].pedModel, playerData[tempplyrid].x, playerData[tempplyrid].y, playerData[tempplyrid].z, playerData[tempplyrid].rx, playerData[tempplyrid].ry, playerData[tempplyrid].rz, playerData[tempplyrid].rw);
				break;

			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				printf("ID_INCOMPATIBLE_PROTOCOL_VERSION\n");
				break;

			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
				printf("Ping from %s\n", p->systemAddress.ToString(true));
				break;

			case ID_CONNECTION_LOST:
				printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString(true));;
				OnPlayerDisconnect(sLUA, netPool.GetPlayerID(p->guid));
				netPool.UserAmount--;

				netPool.RemoveFromUserPool(p->guid);
				break;

			default:
				printf("%s\n", p->data);

				sprintf(netConfig.MsgLength, "%s", p->data);
				server->Send(netConfig.MsgLength, (const int)strlen(netConfig.MsgLength) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);

				break;
			}
		}

		if (_kbhit())
		{
			Gets(message, sizeof(message));

			if (strcmp(message, "quit") == 0)
			{
				puts("Quitting.");
				break;
			}

			if (strcmp(message, "playertest") == 0)
			{
				printf("%d - %s - %s", playerData[0].playerid, playerData[0].playerusername, playerData[0].playerguid);
				break;
			}

			if (strcmp(message, "stat") == 0)
			{
				rss = server->GetStatistics(server->GetSystemAddressFromIndex(0));
				StatisticsToString(rss, message, 2);
				printf("%s", message);
				printf("Ping %i\n", server->GetAveragePing(server->GetSystemAddressFromIndex(0)));

				continue;
			}

			if (strcmp(message, "ping") == 0)
			{
				server->Ping(clientID);

				continue;
			}

			if (strcmp(message, "pingip") == 0)
			{
				printf("Enter IP: ");
				Gets(message, sizeof(message));
				printf("Enter port: ");
				Gets(netConfig.ServerPort, sizeof(netConfig.ServerPort));
				if (netConfig.ServerPort[0] == 0)
					strcpy(netConfig.ServerPort, "1234");
				server->Ping(message, atoi(netConfig.ServerPort), false);

				continue;
			}

			if (strcmp(message, "kick") == 0)
			{
				server->CloseConnection(clientID, true, 0);

				continue;
			}

			if (strcmp(message, "getconnectionlist") == 0)
			{
				RakNet::SystemAddress systems[10];
				unsigned short numConnections = netConfig.MaxPlayers;
				server->GetConnectionList((RakNet::SystemAddress*) &systems, &numConnections);
				for (int i = 0; i < numConnections; i++)
				{
					printf("%i. %s\n", i + 1, systems[i].ToString(true));
				}
				continue;
			}

			if (strcmp(message, "ban") == 0)
			{
				printf("Enter IP to ban.  You can use * as a wildcard\n");
				Gets(message, sizeof(message));
				server->AddToBanList(message);
				printf("IP %s added to ban list.\n", message);

				continue;
			}

			if (strcmp(message, "update") == 0)
			{
				RakNet::BitStream upd_bitStream;

				//upd_bitStream.Write((unsigned char)ID_UPDATE_CLIENT);
				//upd_bitStream.Write(true);

				//server->Send(&upd_bitStream, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				continue;
			}

			char message2[2048];

			message2[0] = 0;
			const static char prefix[] = "Server: ";
			strncpy(message2, prefix, sizeof(message2));
			strncat(message2, message, sizeof(message2) - strlen(prefix) - 1);

			server->Send(message2, (const int)strlen(message2) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		}
	}

	OnGameModeExit(sLUA);
	lua_close(sLUA);
	server->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(server);
	Sleep(2500);
	return 0;
}

unsigned char GetPacketIdentifier(RakNet::Packet *p)
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