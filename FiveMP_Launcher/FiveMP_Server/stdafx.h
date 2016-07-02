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
#include <direct.h>

#pragma comment(lib, "winmm.lib")

extern MODULEINFO g_MainModuleInfo;

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

// Netcode
#include "sPool.h"
#include "sConfig.h"

// INI Reader
#include "IniWriter.h"
#include "IniReader.h"

// scripting (LUA)
#include <lua-5.3.3/lua.hpp>

namespace {
	// Classes (net)
	class sConfig	netConfig;

	// Pools (net)
	class UserPool netPool;
}