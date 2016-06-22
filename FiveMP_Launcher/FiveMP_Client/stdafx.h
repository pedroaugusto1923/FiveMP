#pragma once

#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include <string>
#include <vector>
#include <sstream>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <timeapi.h>
#include <time.h>

#pragma comment(lib, "winmm.lib")

extern MODULEINFO g_MainModuleInfo;

// ScriptHook
#include "natives.h"
#include "types.h"
#include "enums.h"
#include "sh_main.h"
#include "keyboard.h"

// Rockstar
#include "Log.h"
#include "RAGEHelper.h"
#include "DrawHelper.h"
#include "Pattern.h"

// INI Reader
#include "IniWriter.h"
#include "IniReader.h"

// Client
#include "main.h"
#include "ScriptHook.h"

// Net Code
#include "nConnect.h"
#include "nPacket.h"

// Game Functions
#include "player.h"

namespace {
	// Classes (net)
	class cNetCode netCode;
	class cPacket netPacket;
	
	// Classes (scripthook)
	class ScriptHook hook;

	// Classes (scripts)
	class GamePlayer player;

	// INI (server)
	char *server_ipaddress;
	char *server_port;

	// INI (client)
	char *client_port;
	char *client_username;
	bool client_steam_def;
}