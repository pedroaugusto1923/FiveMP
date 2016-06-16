#include "stdafx.h"

int freezePlayer[MAX_PLAYERS+1];
Player moneyPlayer = -1;

BlipList* pBlipList;
GtaThread_VTable gGtaThreadOriginal;
GtaThread_VTable gGtaThreadNew;

HANDLE mainFiber;
DWORD wakeAt;

void WAIT(DWORD ms)
{
#ifndef __DEBUG
	wakeAt = timeGetTime() + ms;
	SwitchToFiber(mainFiber);
#endif
}

void RequestControl(Entity e)
{
	NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(e);
	if (!NETWORK::NETWORK_HAS_CONTROL_OF_ENTITY(e))
		WAIT(0);
	NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(e);
}

eThreadState Trampoline(GtaThread* This)
{
	rage::scrThread* runningThread = GetActiveThread();
	SetActiveThread(This);
	#ifdef  __DEBUG
	Run(); //We don't want to also call RunUnlireable, since it's expecting WAIT() to work, which it doesn't in debug mode. #depechemode
	#else
	Tick();
	#endif
	SetActiveThread(runningThread);
	return gGtaThreadOriginal.Run(This);
}

void __stdcall ReliableScriptFunction(LPVOID lpParameter)
{
	try
	{
		while (1)
		{
			Run();
			SwitchToFiber(mainFiber);
		}
	}
	catch (...)
	{
		Log::Fatal("Failed scriptFiber");
	}
}

void __stdcall HeavyWaitFunction(LPVOID lpParameter)
{
	try
	{
		while (1)
		{
			RunUnreliable();
			SwitchToFiber(mainFiber);
		}
	}
	catch (...)
	{
		Log::Fatal("Failed scriptFiber");
	}
}

void Tick()
{
	if (mainFiber == nullptr)
		mainFiber = ConvertThreadToFiber(nullptr);

	static HANDLE reliableFiber;
	if (reliableFiber)
		SwitchToFiber(reliableFiber);
	else
		reliableFiber = CreateFiber(NULL, ReliableScriptFunction, nullptr);

	if (timeGetTime() < wakeAt)
		return;

	static HANDLE scriptFiber;
	if (scriptFiber)
		SwitchToFiber(scriptFiber);
	else
		scriptFiber = CreateFiber(NULL, HeavyWaitFunction, nullptr);
}

void RunUnreliable() //Put functions that don't really need to be run every frame that can cause heavy wait times for the function here.
{
	
}

void Run() //Only call WAIT(0) here. The Tick() function will ignore wakeAt and call this again regardless of the specified wakeAt time.
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	Vehicle playerVeh = NULL;

	VEHICLE::SET_GARBAGE_TRUCKS(false);
	VEHICLE::SET_RANDOM_BOATS(false);
	VEHICLE::SET_RANDOM_TRAINS(false);
	VEHICLE::SET_FAR_DRAW_VEHICLES(false);

	VEHICLE::SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
	VEHICLE::SET_NUMBER_OF_PARKED_VEHICLES(-1);
	VEHICLE::SET_ALL_LOW_PRIORITY_VEHICLE_GENERATORS_ACTIVE(false);
	STREAMING::SET_VEHICLE_POPULATION_BUDGET(0);
	STREAMING::SET_PED_POPULATION_BUDGET(0);
	VEHICLE::SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);

	color_t test;
	test.red = 255;
	test.green = 255;
	test.blue = 255;
	test.alpha = 255;

	draw_text(0.005f, 0.050f, "FiveMP Alpha", test);
	return;
}

bool AttemptScriptHook()
{
	rage::pgPtrCollection<GtaThread>* threadCollection = GetGtaThreadCollection(pBlipList);

	if (!threadCollection) {
		return false;
	}

	for (UINT16 i = 0; i < threadCollection->count(); i++) {
		GtaThread* pThread = threadCollection->at(i);

		if (!pThread)
			continue;

		//s0biet originally had some junk thread that was called for like 2 seconds then died. This thread is better.
		if (pThread->GetContext()->ScriptHash != MAIN_PERSISTENT) {
			continue;
		}

		// Now what? We need to find a target thread and hook its "Tick" function
		if (gGtaThreadOriginal.Deconstructor == NULL) {
			memcpy(&gGtaThreadOriginal, (DWORD64*)((DWORD64*)pThread)[0], sizeof(gGtaThreadOriginal)); //Create a backup of the original table so we can call the original functions from our hook.
			memcpy(&gGtaThreadNew, &gGtaThreadOriginal, sizeof(GtaThread_VTable)); //Construct our VMT replacement table.

			gGtaThreadNew.Run = Trampoline; //Replace the .Run method in the new table with our method.
		}

		if (((DWORD64*)pThread)[0] != (DWORD64)&gGtaThreadNew) { //If the table is not VMT Hooked.
			DEBUGOUT("Hooking thread [%i] (0x%X)", pThread->GetContext()->ThreadId, pThread->GetContext()->ScriptHash);
			((DWORD64*)pThread)[0] = (DWORD64)&gGtaThreadNew; //Replace the VMT pointer with a pointer to our new VMT.
			DEBUGOUT("Hooked thread [%i] (0x%X)", pThread->GetContext()->ThreadId, pThread->GetContext()->ScriptHash);
			return true;
		}
	}
	return false;
}

DWORD WINAPI lpHookScript(LPVOID lpParam) {
	while (!AttemptScriptHook()) {
		Sleep(100);
	}

	return 0; //We no longer need the lpHookScript thread because our Trampoline function will now be the hip and or hop hang out spot for the KewlKidzKlub®.
}

void SpawnScriptHook() {
	CreateThread(0, 0, lpHookScript, 0, 0, 0);
}