#include "stdafx.h"

MODULEINFO g_MainModuleInfo = { 0 };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved )
{
	if (reason == DLL_PROCESS_ATTACH) {
		Log::Init(hModule);
		DEBUGOUT("FiveMP DLL loaded"); 

		if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(0), &g_MainModuleInfo, sizeof(g_MainModuleInfo))) {
			Log::Fatal("Unable to get MODULEINFO from GTA5.exe");
		}

		DEBUGOUT("GTA5 [0x%I64X][0x%X]", g_MainModuleInfo.lpBaseOfDll, g_MainModuleInfo.SizeOfImage);

		hook.StartConsole();
		hook.NoIntro();
		
		hook.Initialize(hModule);
	}
	if (reason == DLL_PROCESS_DETACH) {
		printf("reach end");
		scriptUnregister(hModule);
		printf("reach end 2");
		hook.StopConsole();
	}
	return TRUE;
}

