#pragma once
class CConfig
{
public:
	char *server_ipaddress;
	char *server_port;

	char *client_port;
	char *client_username;
	bool client_steam_def;

	void Read();
};

