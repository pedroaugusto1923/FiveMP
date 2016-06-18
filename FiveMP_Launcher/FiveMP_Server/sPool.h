#pragma once
class UserPool
{
public:
	int UserAmount;

	int AddToUserPool(char * username, RakNet::RakNetGUID * guid);

	void RemoveFromUserPool(RakNet::RakNetGUID * guid);
	
	int GetPlayerID(char *username);
	char *GetPlayerUsername(RakNet::RakNetGUID * guid);
	RakNet::RakNetGUID *GetPlayerGUID(char *username);
};
