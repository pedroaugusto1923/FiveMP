#pragma once
class UserPool
{
public:
	int UserAmount;

	bool AddToUserPool(char *username, const char *guid);
	bool RemoveFromUserPool(const char *guid);
	
	int GetPlayerID(const char *guid);
	char *GetPlayerUsername(const char *guid);
	const char *GetPlayerGUID(char *username);
};
