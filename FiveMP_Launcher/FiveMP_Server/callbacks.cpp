#include "stdafx.h"

int OnGameModeInit(lua_State* state)
{
	int result;

	printf("OnGameModeInit() was called\n");

	lua_getglobal(state, "OnGameModeInit");

	lua_call(state, 0, 1);

	result = (int)lua_tointeger(state, -1);
	lua_pop(state, 1);

	return result;
}

int OnGameModeExit(lua_State * state)
{
	int result;

	printf("OnGameModeExit() was called\n");

	lua_getglobal(state, "OnGameModeExit");

	lua_call(state, 0, 1);

	result = (int)lua_tointeger(state, -1);
	lua_pop(state, 1);

	return result;
}

int OnPlayerSpawn(lua_State * state, int playerid)
{
	int result;

	printf("OnPlayerSpawn() was called with %d\n", playerid);

	lua_getglobal(state, "OnPlayerSpawn");

	lua_pushnumber(state, playerid);

	lua_call(state, 1, 1);

	result = (int)lua_tointeger(state, -1);
	lua_pop(state, 1);

	return result;
}