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
	//Teleport to waypoint.
	static bool bNumpad0Pressed = false;
	if (isKeyPressedOnce(bNumpad0Pressed, VK_NUMPAD0))
	{
		Entity e = PLAYER::PLAYER_PED_ID();;
		if (PED::IS_PED_IN_ANY_VEHICLE(e, FALSE))
			e = PED::GET_VEHICLE_PED_IS_USING(e);
		bool bBlipFound = false;
		static Vector3 coords, oldLocation;
		if (bBlipFound == false)
		{
			Blip blipIterator = UI::_GET_BLIP_INFO_ID_ITERATOR();
			for (Blip i = UI::GET_FIRST_BLIP_INFO_ID(blipIterator); UI::DOES_BLIP_EXIST(i) != 0; i = UI::GET_NEXT_BLIP_INFO_ID(blipIterator))
			{
				if (UI::GET_BLIP_INFO_ID_TYPE(i) == 4)
				{
					coords = UI::GET_BLIP_INFO_ID_COORD(i);
					bBlipFound = true;
					oldLocation = ENTITY::GET_ENTITY_COORDS(e, FALSE);
					break;
				}
			}
		}
		if (bBlipFound)
		{
			bool groundFound = false;
			static float groundHeights[] = { 300.0, 100.0, 150.0, 50.0, 0.0, 200.0, 250.0, 300.0, 350.0, 400.0, 450.0, 500.0, 550.0, 600.0, 650.0, 700.0, 750.0, 800.0 };
			for each (float groundHeight in groundHeights)
			{
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, groundHeight, FALSE, FALSE, TRUE);
				if (GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(coords.x, coords.y, groundHeight, &coords.z, 0) == TRUE)
				{
					groundFound = true;
					ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, coords.x, coords.y, coords.z + 3, FALSE, FALSE, TRUE);
					break;
				}
				WAIT(0);
			}
			if (!groundFound)
			{
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, oldLocation.x, oldLocation.y, oldLocation.z, FALSE, FALSE, TRUE);
			}
		}
	}
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
	VEHICLE::SET_ALL_LOW_PRIORITY_VEHICLE_GENERATORS_ACTIVE(true);
	STREAMING::SET_VEHICLE_POPULATION_BUDGET(0);
	STREAMING::SET_PED_POPULATION_BUDGET(0);
	VEHICLE::SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);

	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE))
		playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

	// Test that drawing works.
	draw_menu_line("FiveMP Alpha", 15.0f, 50.0f, 570.0f, 6.0f, 5.0f, false, false, false);

	// Disable Apartment Radio (use documents one)
	SetApartmentRadioStation(AUDIO::GET_RADIO_STATION_NAME(RADIO_SELFRADIO));

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