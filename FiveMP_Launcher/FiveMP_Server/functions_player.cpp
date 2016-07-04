#include "stdafx.h"

int GivePlayerWeapon(lua_State* state)
{
	int args = lua_gettop(state);

	printf("GivePlayerWeapon() was called with %d arguments:\n", args);

	int playerid = lua_tointeger(state, 1);
	const char *string = lua_tostring(state, 2);
	int ammo = lua_tointeger(state, 3);

	RakNet::RakString weaponid = string;

	RakNet::BitStream sGivePlayerWeapon;
	sGivePlayerWeapon.Write(playerid); //Probably do not need to send this
	sGivePlayerWeapon.Write(weaponid);
	sGivePlayerWeapon.Write(ammo);
	rpc.Signal("GivePlayerWeapon", &sGivePlayerWeapon, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, netPool.GetPlayerGUIDd(playerid), false, false);

	lua_pushnumber(state, 124);
	return 1;
}

int RemovePlayerWeapon(lua_State* state)
{
	int args = lua_gettop(state);

	printf("RemovePlayerWeapon() was called with %d arguments:\n", args);

	int playerid = lua_tointeger(state, 1);
	const char *string = lua_tostring(state, 2);

	RakNet::RakString weaponid = string;

	RakNet::BitStream sRemovePlayerWeapon;
	sRemovePlayerWeapon.Write(playerid);
	sRemovePlayerWeapon.Write(weaponid);
	rpc.Signal("RemovePlayerWeapon", &sRemovePlayerWeapon, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, netPool.GetPlayerGUIDd(playerid), false, false);

	//lua_pushnumber(state, 124); //IS THIS EVEN NEEDED?
	return 1;
}

int GivePlayerAmmo(lua_State* state)
{
	int args = lua_gettop(state);

	printf("GivePlayerAmmo() was called with %d arguments:\n", args);

	int playerid = lua_tointeger(state, 1);
	const char *string = lua_tostring(state, 2);
	int ammo = lua_tointeger(state, 3);

	RakNet::RakString weaponid = string;

	RakNet::BitStream sGivePlayerAmmo;
	sGivePlayerAmmo.Write(playerid);
	sGivePlayerAmmo.Write(weaponid);
	sGivePlayerAmmo.Write(ammo);
	rpc.Signal("GivePlayerAmmo", &sGivePlayerAmmo, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, netPool.GetPlayerGUIDd(playerid), false, false);

	return 1;
}

int RemovePlayerAmmo(lua_State* state)
{
	int args = lua_gettop(state);

	printf("RemovePlayerAmmo() was called with %d arguments:\n", args);

	int playerid = lua_tointeger(state, 1);
	const char *string = lua_tostring(state, 2);
	int ammo = lua_tointeger(state, 3);

	RakNet::RakString weaponid = string;

	RakNet::BitStream sRemovePlayerAmmo;
	sRemovePlayerAmmo.Write(playerid);
	sRemovePlayerAmmo.Write(weaponid);
	sRemovePlayerAmmo.Write(ammo);
	rpc.Signal("RemovePlayerAmmo", &sRemovePlayerAmmo, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, netPool.GetPlayerGUIDd(playerid), false, false);

	return 1;
}