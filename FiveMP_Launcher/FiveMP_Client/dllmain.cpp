#include "stdafx.h"

MODULEINFO g_MainModuleInfo = { 0 };

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		Log::Init(hModule);
		DEBUGOUT("dll loaded"); 

		if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(0), &g_MainModuleInfo, sizeof(g_MainModuleInfo))) {
			Log::Fatal("Unable to get MODULEINFO from GTA5.exe");
		}

		DEBUGOUT("GTA5 [0x%I64X][0x%X]", g_MainModuleInfo.lpBaseOfDll, g_MainModuleInfo.SizeOfImage);

		NoIntro();							// Disable stupid intro, wastes your times. I just don't see the need.
		SpawnScriptHook();					// I guess this says enough, this allows us to use the game's natives.
		BypassOnlineModelRequestBlock();	// No idea what the heck this is, could probably remove it.

		if (AllocConsole()) {
			freopen("CONIN$", "rb", stdin);
			freopen("CONOUT$", "wb", stdout);
			freopen("CONOUT$", "wb", stderr);

			SetConsoleTitle("FiveMP - Client Console");
		}
	}
	if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		FreeConsole();
	}
	return TRUE;
}

