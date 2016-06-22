#include "stdafx.h"

void ScriptHook::Initialize(HMODULE hModule)
{
	scriptRegister(hModule, InitGameScript);
	keyboardHandlerRegister(OnKeyboardMessage);
}

void ScriptHook::NoIntro()
{
	// CREDITS: CitizenMP

	//Disable logos since they add loading time
	UINT64 logos = FindPattern("platform:/movies/rockstar_logos", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	if (logos != 0)
	{
		//memset((void*)(logos + 0x11), 0x00, 0x0E);
		memcpy((void*)logos, "./nonexistingfilenonexistingfil", 32);

		//DisableLegalMessagesCompletely();
		DWORD64 dwSplashScreen = Pattern::Scan(g_MainModuleInfo, "72 1F E8 ? ? ? ? 8B 0D");
		if (dwSplashScreen == NULL)  //If the module is still encrypted at the time of injection, run the No Intro code.
		{
			while (dwSplashScreen == NULL)
			{
				Sleep(10);
				dwSplashScreen = Pattern::Scan(g_MainModuleInfo, "72 1F E8 ? ? ? ? 8B 0D");
			}

			if (dwSplashScreen != NULL)
				*(unsigned short*)(dwSplashScreen) = 0x9090; //NOP out the check to make it think it's time to stop.

			DWORD64 dwRockStarLogo = Pattern::Scan(g_MainModuleInfo, "70 6C 61 74 66 6F 72 6D 3A");
			int iCounter = 0;
			while (dwRockStarLogo == NULL)
			{
				Sleep(10);
				dwRockStarLogo = Pattern::Scan(g_MainModuleInfo, "70 6C 61 74 66 6F 72 6D 3A");
			}

			if (dwRockStarLogo != NULL)
				*(unsigned char*)(dwRockStarLogo) = 0x71; //Replace the P with some garbage so it won't find the file.

			Sleep(15000); //Wait until the logo code has finished running.
						  //Restore the EXE to its original state.
			*(unsigned char*)(dwRockStarLogo) = 0x70;
			*(unsigned short*)(dwSplashScreen) = 0x1F72;
		}
	}
}

void ScriptHook::StartConsole()
{
	if (AllocConsole()) {
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);

		SetConsoleTitle(L"FiveMP - Client Console");
	}
}

void ScriptHook::StopConsole()
{
	FreeConsole();
}
