#pragma once

#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include <string>
#include <map>
#include <string.h>
#include <vector>
#include <sstream>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <timeapi.h>
#include <time.h>
#include <tlhelp32.h>
#include <math.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "winmm.lib")

// RakNet
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetStatistics.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakSleep.h"
#include "PacketLogger.h"
#include "Kbhit.h"
#include "Gets.h"
#include "WindowsIncludes.h"
#include "GetTime.h"
#include "RPC4Plugin.h"
using namespace RakNet;

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

// Game Functions
#include "Player.h"
#include "Weapon.h"
#include "Vehicles.h"

// Math
#include "CMath.h"
#include "CVector3.h"

// Modification (FiveMP)
#include "Config.h"

// Net
#include "NetworkManager.h"
#include "PlayerPed.h"
#include "LocalPlayer.h"
#include "RPCManager.h"
#include "Script.h"
#include "cPool.h"

// Interface
#include "RenderDebug.h"
#include "Render.h"

namespace {
	// Classes (scripthook)
	class ScriptHook hook;

	// Classes (scripts)
	class GamePlayer player;
	class GameWeapon weapon;
}

extern MODULEINFO g_MainModuleInfo;

extern CNetworkManager	*NetworkManager;
extern CRPCManager		*RPCManager;
extern CLocalPlayer		*LocalPlayer;
extern CConfig			*Config;
extern CRenderDebug		*RenderDebug;
extern CRender			*Render;