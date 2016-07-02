#pragma once

int OnGameModeInit(lua_State* state);
int OnGameModeExit(lua_State* state);

int OnPlayerSpawn(lua_State * state, int playerid);