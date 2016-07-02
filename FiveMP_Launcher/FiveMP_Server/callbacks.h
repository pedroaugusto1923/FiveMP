#pragma once

int OnGameModeInit(lua_State* state);
int OnGameModeExit(lua_State* state);

int OnPlayerConnect(lua_State * state, int playerid);