#include "stdafx.h"

int SetPlayerUsername(lua_State* state) // function has no effect, placeholder for now.
{
	int args = lua_gettop(state);

	printf("SetPlayerUsername() was called with %d arguments:\n", args);

	for (int n = 1; n <= args; ++n) {
		printf("  argument %d: '%s'\n", n, lua_tostring(state, n));
	}

	lua_pushnumber(state, 123);
	return 1;
}