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

// Mine
#include "Log.h"
#include "Pattern.h"

// Rockstar
#include "Types.h"
#include "pgCollection.h"
#include "scrThread.h"
#include "Hashes.h"
#include "VehicleValues.h"
#include "natives.h"
#include "RAGEHelper.h"
#include "DrawHelper.h"
#include "KeyHelper.h"
#include "NetworkHelper.h"
#include "VehicleHelper.h"
#include "StatScripts.h"
#include "RadioScripts.h"
#include "WeaponScripts.h"

// Custom
#include "IniWriter.h"
#include "IniReader.h"

// Main
#include "script.h"
#include "main.h"

// Net Code
#include "nConnect.h"
#include "nPacket.h"

// Game Functions
#include "player.h"

namespace {
	// Classes (net)
	class cNetCode netCode;
	class cPacket netPacket;

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

void Tick();
void Run();
void RunUnreliable();