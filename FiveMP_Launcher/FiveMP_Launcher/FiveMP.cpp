#include "stdafx.h"

using namespace std;

//Entry
int main(void) {
	SetConsoleTitle("FiveMP - Launcher Console");

	const char *dllname = "FiveMP.dll";
	const char *dllname2 = "ScriptHookV.dll";

	bool GameThread = false;
	char GamePath[MAX_PATH] = { 0 };
	char GameFullPath[MAX_PATH] = { 0 };
	char Params[] = "";

	printf("SEARCH: Attempting to search for %s.\n", dllname);

	if (!DoesFileExist(dllname)) {
		MessageBox(NULL, "Could not find the launcher DLL", "Fatal Error", MB_ICONERROR);
		return 0;
	}

	printf("SEARCH: Successfully found %s!\n\n", dllname);
	printf("SEARCH: Attempting to search for %s.\n", dllname2);

	if (!DoesFileExist(dllname2)) {
		MessageBox(NULL, "Could not find the scripthook DLL", "Fatal Error", MB_ICONERROR);
		return 0;
	}

	printf("SEARCH: Successfully found %s!\n\n", dllname2);

	printf("SEARCH: Attempting to search for GTA V's install directory.\n");

	if (!SharedUtils::Registry::Read(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\rockstar games\\Grand Theft Auto V", "InstallFolder", GamePath, MAX_PATH))
	{
		if (!SharedUtils::Registry::Read(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV", "InstallFolderSteam", GamePath, MAX_PATH))
		{
			// If we cannot find it - display an error
			// and close launcher
			// TODO: Custom game path selector
			MessageBox(NULL, "Cannot find game path in registry!", "Fatal Error", MB_ICONERROR);
			return 0;
		}
	}

	printf("SEARCH: Successfully found the install directory from the registry!\n\n");

	// Format game paths
	sprintf_s(GamePath, "%s", GamePath);
	sprintf_s(GameFullPath, "%s\\GTAVLauncher.exe", GamePath);

	// Predefine startup and process infos
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	printf("START: Attempting to start Grand Theft Auto V.\n");

	// Create game process
	if (!CreateProcess(GameFullPath, Params, NULL, NULL, true, CREATE_SUSPENDED, NULL, GamePath, &siStartupInfo, &piProcessInfo))
	{
		if (!ShellExecute(0, 0, "steam://run/271590", 0, 0, SW_SHOW)) {
			MessageBox(NULL, "Grand Theft Auto V was not able to start.", "Fatal Error", MB_ICONERROR);
			return 0;
		}
	}

	if (kill(0) == 0) {

	}

	printf("START: Successfully started Grand Theft Auto V!\n\n");

	printf("SCAN: Waiting for GTA5.exe to start.\n");

	// Resume game main thread
	ResumeThread(piProcessInfo.hThread);

	bool GameStarted			= false;
	bool Injected_ScriptHook	= false;
	bool Injected_FiveMP		= false;

	/*while (GameThread != true) {
		HWND hWnds = FindWindowA(NULL, "Grand Theft Auto V");

		if (hWnds != NULL) {
			if (GameStarted == false) {
				printf("SCAN: GTA5.exe has successfully started!\n\n");
				GameStarted = true;
			} else {
				Sleep(2500);*/
	while (GameThread == false) {
		if (_kbhit()) {
			if (_getch() == 'g') {

				if (InjectDLL("GTA5.exe", dllname2) == true) {
					printf("INJECT: Successfully injected %s into Grand Theft Auto V!\n\n", dllname2);
					Injected_ScriptHook = true;
				}
				if (InjectDLL("GTA5.exe", dllname) == true) {
					printf("INJECT: Successfully injected %s into Grand Theft Auto V!\n\n", dllname);

					Injected_FiveMP = true;
					GameThread = true;
					break;
				}
			}
			Sleep(500);
		}
	}
	Sleep(2500);
	//FindNativeTableAddress();
}