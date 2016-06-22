#include "stdafx.h"

int userAmount, vehicleAmount;
char userGuid;

int main(void)
{
	CIniReader iniReader(".\\FiveMP.ini");

	netConfig.ServerPort		= iniReader.ReadString("Connection", "port", "");
	netConfig.ServerName		= iniReader.ReadString("Details", "servername", "");

	netConfig.MaxPlayers		= iniReader.ReadInteger("Details", "maxplayers", 32);

	netConfig.ServerTimeHour	= iniReader.ReadInteger("Details", "hour", 12);
	netConfig.ServerTimeMinute	= iniReader.ReadInteger("Details", "min", 00);
	netConfig.ServerTimeFreeze	= iniReader.ReadBoolean("Details", "freeze", false);

	printf("\n%s running on Port: %s - time: %d - %d - %d\n", netConfig.ServerName, netConfig.ServerPort, netConfig.ServerTimeHour, netConfig.ServerTimeMinute, netConfig.ServerTimeFreeze);

	SetConsoleTitle("FiveMP - Server Console");

	netCode.Initialize();
	netCode.Connect(netConfig.ServerPort, netConfig.MaxPlayers);

	char message[2048];

	while (1)
	{
		RakSleep(30);

		netPacket.ReceivePacket(netCode.p, netCode.server);

		if (_kbhit())
		{
			Gets(message, sizeof(message));

			if (strcmp(message, "quit") == 0)
			{
				puts("Quitting.");
				break;
			}

			if (strcmp(message, "stat") == 0)
			{
				netCode.rss = netCode.server->GetStatistics(netCode.server->GetSystemAddressFromIndex(0));
				StatisticsToString(netCode.rss, message, 2);
				printf("%s", message);
				printf("Ping %i\n", netCode.server->GetAveragePing(netCode.server->GetSystemAddressFromIndex(0)));

				continue;
			}

			if (strcmp(message, "ping") == 0)
			{
				netCode.server->Ping(netCode.clientID);

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
				netCode.server->Ping(message, atoi(netConfig.ServerPort), false);

				continue;
			}

			if (strcmp(message, "kick") == 0)
			{
				netCode.server->CloseConnection(netCode.clientID, true, 0);

				continue;
			}

			if (strcmp(message, "getconnectionlist") == 0)
			{
				RakNet::SystemAddress systems[10];
				unsigned short numConnections = netConfig.MaxPlayers;
				netCode.server->GetConnectionList((RakNet::SystemAddress*) &systems, &numConnections);
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
				netCode.server->AddToBanList(message);
				printf("IP %s added to ban list.\n", message);

				continue;
			}

			if (strcmp(message, "update") == 0)
			{
				RakNet::BitStream upd_bitStream;

				upd_bitStream.Write((unsigned char)ID_UPDATE_CLIENT);
				upd_bitStream.Write(true);

				netCode.server->Send(&upd_bitStream, IMMEDIATE_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				continue;
			}

			char message2[2048];

			message2[0] = 0;
			const static char prefix[] = "Server: ";
			strncpy(message2, prefix, sizeof(message2));
			strncat(message2, message, sizeof(message2) - strlen(prefix) - 1);

			netCode.server->Send(message2, (const int)strlen(message2) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		}
	}

	netCode.server->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(netCode.server);
	Sleep(5000);
	return 0;
}