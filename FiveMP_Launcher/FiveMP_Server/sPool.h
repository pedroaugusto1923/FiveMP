#pragma once
class UserPool
{
public:
	int UserAmount;

	void AddToUserPool(char *username, const char *guid);
	void RemoveFromUserPool(const char *guid);
	
	int GetPlayerID(const char *guid);
	char *GetPlayerUsername(const char *guid);
	const char *GetPlayerGUID(char *username);
};
