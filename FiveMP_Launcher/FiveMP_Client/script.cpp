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
	#ifdef __DEBUG
	static bool bQuit, F12 = false;
	if (isKeyPressedOnce(F12, VK_F12)){ bQuit = true; }
	if (bQuit) { return; }
	#endif
	//Run your natives here.
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	if (ENTITY::DOES_ENTITY_EXIST(playerPed) == TRUE)
	{
		Hash currentWeapon;
		Vehicle playerVeh = NULL;

		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, FALSE))
			playerVeh = PED::GET_VEHICLE_PED_IS_USING(playerPed);

		//Test that drawing works.
		draw_menu_line("m0d-s0biet-v Redux by gir489", 15.0f, 50.0f, 50.0f, 550.0f, 5.0f, false, false, false);

		//Fuck you, Soulwax. Go die.
		SetApartmentRadioStation(AUDIO::GET_RADIO_STATION_NAME(RADIO_SELFRADIO));

		//Godmode
		if (!PLAYER::GET_PLAYER_INVINCIBLE(player))
		{
			DEBUGOUT("Setting godmode");
			PLAYER::SET_PLAYER_INVINCIBLE(player, true);
			ENTITY::SET_ENTITY_PROOFS(playerPed, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
			PED::SET_PED_CAN_RAGDOLL(playerPed, FALSE);
			PED::SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(playerPed, FALSE);
		}

		//Max armor.
		PED::ADD_ARMOUR_TO_PED(playerPed, PLAYER::GET_PLAYER_MAX_ARMOUR(player) - PED::GET_PED_ARMOUR(playerPed));

		static bool bMenuActive, bF3Pressed = false;
		static int iFreeze = -1;
		if (isKeyPressedOnce(bF3Pressed, VK_F3))
		{
			iFreeze = -1;
			bMenuActive = !bMenuActive;
		}
		bool bReset = false;
		if (bMenuActive)
		{
			static int iSelectedPlayer = 0;
			static bool bPgUpPressed, bPgDwnPressed = false;
			if (isKeyPressedOnce(bPgUpPressed, VK_PRIOR))
			{
				iSelectedPlayer--;
				CheckPlayer(iSelectedPlayer, false);
			}
			if (isKeyPressedOnce(bPgDwnPressed, VK_NEXT))
			{
				iSelectedPlayer++;
				CheckPlayer(iSelectedPlayer, true);
			}
			CheckPlayer(iSelectedPlayer, !bPgUpPressed);
			int iLineNum = 0;
			Ped selectedPed = NULL;
			for (Player playerIterator = 0; playerIterator < MAX_PLAYERS; playerIterator++)
			{
				bool bSelectedPed = (playerIterator == iSelectedPlayer);
				Ped pedIterator = PLAYER::GET_PLAYER_PED(playerIterator);
				if (bSelectedPed)
					selectedPed = pedIterator;
				if (ENTITY::DOES_ENTITY_EXIST(pedIterator))
				{
					char chStringToDraw[50];
					strcpy_s(chStringToDraw, PLAYER::GET_PLAYER_NAME(playerIterator));
					if (bSelectedPed)
					{
						sprintf_s(chStringToDraw, "%s M:%i", chStringToDraw, moneyPlayer == iSelectedPlayer);
						if (selectedPed != playerPed && !IsPlayerFriend(playerIterator))
						{
							sprintf_s(chStringToDraw, "%s F:%i", chStringToDraw, freezePlayer[playerIterator] != NULL);
						}
					}
					draw_menu_line(chStringToDraw, 150.0f, 4.0f, 25.0f + iLineNum * 13.0f, 350.0f, 0.0f, false, false, bSelectedPed, false);
					iLineNum++;
				}
			}

			static bool bDividePressed = false;
			if (isKeyPressedOnce(bDividePressed, VK_DIVIDE))
			{
				GiveAllWeaponsToPed(selectedPed, WEAPONTINT_LSPD, selectedPed == playerPed);
			}

			static bool bDecimalPressed = false;
			if (isKeyPressedOnce(bDecimalPressed, VK_DECIMAL))
			{
				if (selectedPed == playerPed)
				{
					if (IsKeyPressed(VK_RCONTROL))
					{
						//This will cycle our PED entity to remove any attached garbage to us. Select an outfit to fix your model.
						PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(), ENTITY::GET_ENTITY_MODEL(playerPed));
					}
					else
					{
						Vehicle vehicle = PED::GET_VEHICLE_PED_IS_IN(playerPed, TRUE);
						for (int i = SEAT_BACKPASSENGER; i >= SEAT_DRIVER; i--)
						{
							PED::SET_PED_INTO_VEHICLE(playerPed, vehicle, i);
						}
					}
				}
				else
				{
					if (PED::IS_PED_IN_ANY_VEHICLE(selectedPed, FALSE))
					{
						Vehicle selectedVehicle = PED::GET_VEHICLE_PED_IS_USING(selectedPed);
						for (int i = SEAT_BACKPASSENGER; i >= SEAT_DRIVER; i--)
						{
							PED::SET_PED_INTO_VEHICLE(playerPed, selectedVehicle, i);
						}
					}
				}
			}

			//Teleport to selected player on the menu.
			static bool bNumpad0Pressed = false;
			if (isKeyPressedOnce(bNumpad0Pressed, VK_NUMPAD0) && selectedPed != playerPed)
			{
				Entity e = playerPed;
				if (playerVeh != NULL)
					e = playerVeh;

				Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(selectedPed, FALSE);
				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, playerPosition.x, playerPosition.y, playerPosition.z + 1, FALSE, FALSE, TRUE);
			}

			static bool bNumpad2Pressed = false;
			if (isKeyPressedOnce(bNumpad2Pressed, VK_NUMPAD2))
			{
				Vehicle clonedVeh = ClonePedVehicle(selectedPed);
				BoostBaseVehicleStats(clonedVeh); //Gotta go fast
				WAIT(0); //We need to wait for the game to assign a random radio station to the car first before changing it.
				AUDIO::SET_VEH_RADIO_STATION(playerVeh, AUDIO::GET_RADIO_STATION_NAME(RADIO_SELFRADIO));
			}

			
			static bool bMinusPressed = false;
			if (isKeyPressedOnce(bMinusPressed, VK_SUBTRACT))
			{
				if (moneyPlayer != iSelectedPlayer)
					moneyPlayer = iSelectedPlayer;
				else
					moneyPlayer = -1;
			}

			if (IsPlayerFriend(iSelectedPlayer) == FALSE)
			{
				if (IsKeyPressed(VK_NUMPAD1))
				{
					if (IsKeyPressed(VK_RCONTROL))
					{
						Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(selectedPed, FALSE);
						if (selectedPed != playerPed)
						{
							AI::CLEAR_PED_TASKS_IMMEDIATELY(selectedPed);
							FIRE::ADD_OWNED_EXPLOSION(selectedPed, playerPosition.x, playerPosition.y, playerPosition.z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f);
						}
						else
						{
							FIRE::ADD_EXPLOSION(playerPosition.x, playerPosition.y, playerPosition.z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f);
						}
					}
					else if (IsKeyPressed(VK_RMENU))
					{
						Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPed, FALSE);
						FIRE::ADD_OWNED_EXPLOSION(selectedPed, playerPosition.x, playerPosition.y, playerPosition.z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f);
					}
					else
					{
						static int playerIterator = 0;
						for (int i = 0; i < 5; i++)
						{
							Ped playerPedIterator = PLAYER::GET_PLAYER_PED(playerIterator);
							if (ENTITY::DOES_ENTITY_EXIST(playerPedIterator) && playerPedIterator != playerPed) //If the iteration exists, and they're alive, and they're not me.
							{
								if (IsPlayerFriend(playerIterator) == FALSE && selectedPed != playerPedIterator)
								{
									try
									{
										AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPedIterator); //If they're in a jet, or something. Toss them out.
										Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPedIterator, FALSE);
										FIRE::ADD_OWNED_EXPLOSION(selectedPed, playerPosition.x, playerPosition.y, playerPosition.z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f); //We can blame anyone for the explosion. Whoever is selected in the menu will be blamed.
									}
									catch (...) { Log::Error("Crashed"); break; } //IDK why, but if you call these functions too many times per tick, it causes a crash. We can just toss the exception. Hopefully this fixes the crash...
								}
							}
							playerIterator++;
							if (playerIterator > MAX_PLAYERS)
								playerIterator = 0;
						}
					}
				}

				static bool bNumpad3Pressed = false;
				if (isKeyPressedOnce(bNumpad3Pressed, VK_NUMPAD3))
				{
					if (freezePlayer[iSelectedPlayer] == NULL && iSelectedPlayer != player && !IsPlayerFriend(iSelectedPlayer))
					{
						freezePlayer[iSelectedPlayer] = GetNetworkHandle(iSelectedPlayer);
					}
					else
					{
						freezePlayer[iSelectedPlayer] = NULL;
					}
				}
			}
		}
		else
		{
			//Test IsPlayerFriend and give a sample player iteration
			if (IsKeyPressed(VK_NUMPAD1))
			{
				static int playerIterator = 0;
				for (int i = 0; i < 5; i++)
				{
					Ped playerPedIterator = PLAYER::GET_PLAYER_PED(playerIterator);
					if (ENTITY::DOES_ENTITY_EXIST(playerPedIterator) && playerPedIterator != playerPed) //If the iteration exists, and they're alive, and they're not me.
					{
						if (IsPlayerFriend(playerIterator) == FALSE )
						{
							try
							{
								AI::CLEAR_PED_TASKS_IMMEDIATELY(playerPedIterator);
								WEAPON::REMOVE_ALL_PED_WEAPONS(playerPedIterator, TRUE); //Why does this work? C'mon, Rockstar...
								Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPedIterator, FALSE);
								FIRE::ADD_OWNED_EXPLOSION(playerPedIterator, playerPosition.x, playerPosition.y, playerPosition.z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f); //We can blame anyone for the explosion. Whoever is selected in the menu will be blamed.
							}
							catch (...) { Log::Error("Crashed"); break; } //IDK why, but if you call these functions too many times per tick, it causes a crash. We can just toss the exception. Hopefully this fixes the crash...
						}
					}
				}
				playerIterator++;
				if (playerIterator > MAX_PLAYERS)
					playerIterator = 0;
			}

			//Spawn a test car.
			static bool bNumpad2Pressed, bWaitingForModelCar = false;
			if ((isKeyPressedOnce(bNumpad2Pressed, VK_NUMPAD2) || bWaitingForModelCar == true) )
			{
				if (playerVeh == NULL || bWaitingForModelCar == true)
				{
					Hash vehicleModelHash = VEHICLE_KURUMA2;
					if (IsKeyPressed(VK_RCONTROL))
						vehicleModelHash = VEHICLE_BTYPE;
					else if (IsKeyPressed(VK_RMENU))
						vehicleModelHash = VEHICLE_RUINER;
					else if (IsKeyPressed(VK_RIGHT))
						vehicleModelHash = VEHICLE_T20;
					else if (IsKeyPressed(VK_DOWN))
						vehicleModelHash = VEHICLE_INSURGENT;
					else if (IsKeyPressed(VK_LEFT))
						vehicleModelHash = VEHICLE_RHINO;
					else if (IsKeyPressed(VK_END))
						vehicleModelHash = VEHICLE_BTYPE2;
					STREAMING::REQUEST_MODEL(vehicleModelHash);
					if (STREAMING::HAS_MODEL_LOADED(vehicleModelHash) == TRUE)
					{
						Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPed, FALSE);
						playerVeh = VEHICLE::CREATE_VEHICLE(vehicleModelHash, playerPosition.x, playerPosition.y, playerPosition.z, ENTITY::GET_ENTITY_HEADING(playerPed), TRUE, TRUE);
						NETWORK::NETWORK_REQUEST_CONTROL_OF_ENTITY(playerVeh);
						PED::SET_PED_INTO_VEHICLE(playerPed, playerVeh, SEAT_DRIVER);
						BoostBaseVehicleStats(playerVeh);
						VEHICLE::SET_VEHICLE_ENGINE_ON(playerVeh, TRUE, TRUE, TRUE);
						if (vehicleModelHash == VEHICLE_KURUMA2) //Test that I can make a perfect 1:1 clone of my Kuruma with only calling natives.
						{
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_SPOILER, MOD_INDEX_ONE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTBUMPER, MOD_INDEX_TWO, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_SIDESKIRT, MOD_INDEX_FIVE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_EXHAUST, MOD_INDEX_ONE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HORNS, HORN_SADTROMBONE, FALSE);
							VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_HIGHEND);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_HIGHEND_SUPAGEE, TRUE); //TRUE because we want the Custom Tires.
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_XENONLIGHTS, TRUE);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_YELLOWONBLACK);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " 2FAST  ");
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_BLACK);
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_MATTE_BLACK, COLOR_CLASSIC_ULTRA_BLUE);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
							VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, 0, COLOR_CLASSIC_ULTRA_BLUE);
							for (int i = 0; i < NEON_BACK; i++) //This will turn on all the neon emitters except the back one. That shit's annoying when I'm trying to drive.
							{
								VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, TRUE);
							}
							VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_ELECTRICBLUE);
						}
						else if (vehicleModelHash == VEHICLE_T20)
						{
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_SIDESKIRT, MOD_INDEX_ONE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_EXHAUST, MOD_INDEX_ONE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HOOD, MOD_INDEX_TWO, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HORNS, HORN_CLASSICALLOOP2, FALSE);
							VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_HIGHEND);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_HIGHEND_SUPAGEE, TRUE);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_XENONLIGHTS, TRUE);
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_BLACK);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_YELLOWONBLUE);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " 2FAST  ");
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_ULTRA_BLUE, COLOR_MATTE_ICE_WHITE);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
							VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, COLOR_CLASSIC_ULTRA_BLUE, COLOR_CLASSIC_ULTRA_BLUE);
							for (int i = 0; i < NEON_BACK; i++) //This will turn on all the neon emitters except the back one. That shit's annoying when I'm trying to drive.
							{
								VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, TRUE);
							}
							VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_ELECTRICBLUE);
							AddClanLogoToVehicle(playerVeh, playerPed);
						}
						else if (vehicleModelHash == VEHICLE_INSURGENT)
						{
							VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_SPORT);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_SPORT_DEEPFIVE, TRUE);
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_MATTE_BLACK, COLOR_MATTE_FOREST_GREEN);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
							VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, 0, COLOR_MATTE_FOREST_GREEN);
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_GREEN);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, "GETFUCKD");
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_YELLOWONBLACK);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HORNS, HORN_TRUCK, FALSE);
						}
						else if (vehicleModelHash == VEHICLE_RUINER)
						{
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_ULTRA_BLUE, COLOR_MATTE_ICE_WHITE);
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_BLACK);
							VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(playerVeh, FALSE);
							VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_MUSCLE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_SPOILER, MOD_INDEX_TWO, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_EXHAUST, MOD_INDEX_ONE, FALSE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HORNS, HORN_SADTROMBONE, FALSE);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_XENONLIGHTS, TRUE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_MUSCLE_CLASSICFIVE, TRUE);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_YELLOWONBLUE);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " 2FAST  ");
							VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, COLOR_CLASSIC_ULTRA_BLUE, COLOR_CLASSIC_ULTRA_BLUE); //Adding the pearlescentColor makes it "metallic." So there really is no purpose to having the Metallic collor tab under Secondary, since it can not have a Pearlescent Color. Oh well, lazy Rockstar coders.
							VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_ELECTRICBLUE);
							for (int i = 0; i < NEON_BACK; i++)
							{
								VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, TRUE);
							}
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 2, TRUE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 3, FALSE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 7, TRUE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 10, TRUE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 11, TRUE);
							AddClanLogoToVehicle(playerVeh, playerPed);
						}
						else if (vehicleModelHash == VEHICLE_BTYPE)
						{
							srand((unsigned int)time(NULL));
							if (rand() % 2)
							{
								VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_METALS_BRUSHED_GOLD, COLOR_METALS_BRUSHED_ALUMINUM);
								VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_MUSCLE);
								VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
								VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
								VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_MUSCLE_CLASSICROD, TRUE);
								VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " GIR489 ");
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, COLOR_METALS_PEARLESCENT_GOLD, 0);
								VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_WHITE);
							}
							else
							{
								VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_LIME_GREEN, COLOR_CLASSIC_ULTRA_BLUE);
								VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_TUNER);
								VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
								VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_GREEN);
								VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_TUNER_OUTSIDER, FALSE);
								VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " 2FAST  ");
								VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, COLOR_CLASSIC_LIME_GREEN, COLOR_CLASSIC_LIME_GREEN);
								VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_LIMEGREEN);
							}
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_BLACK);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_BLUEONWHITE3);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_XENONLIGHTS, TRUE);
							VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(playerVeh, FALSE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 1, TRUE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 2, TRUE);
							VEHICLE::SET_VEHICLE_EXTRA(playerVeh, 3, FALSE);
							for (int i = 0; i < NEON_BACK; i++)
							{
								VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, TRUE);
							}
							AddClanLogoToVehicle(playerVeh, playerPed);
						}
						else if (vehicleModelHash == VEHICLE_BTYPE2)
						{
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_BLACK, COLOR_CLASSIC_BLACK);
							VEHICLE::SET_VEHICLE_WINDOW_TINT(playerVeh, WINDOWTINT_BLACK);
							VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(playerVeh, FALSE);
							VEHICLE::SET_VEHICLE_WHEEL_TYPE(playerVeh, WHEEL_TYPE_MUSCLE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_HOOD, MOD_INDEX_ONE, FALSE);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_TIRESMOKE, TRUE);
							VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(playerVeh, TIRESMOKE_COLOR_BLACK);
							VEHICLE::TOGGLE_VEHICLE_MOD(playerVeh, MOD_XENONLIGHTS, TRUE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_FRONTWHEELS, WHEEL_MUSCLE_CLASSICROD, TRUE);
							VEHICLE::SET_VEHICLE_MOD(playerVeh, MOD_BOBBLEHEAD, 20, FALSE);
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(playerVeh, " 2FAST  ");
							VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(playerVeh, PLATE_BLUEONWHITE2);
							VEHICLE::SET_VEHICLE_EXTRA_COLOURS(playerVeh, COLOR_CLASSIC_BLACK, COLOR_CLASSIC_BLACK);
							VEHICLE::_SET_VEHICLE_NEON_LIGHTS_COLOUR(playerVeh, NEON_COLOR_BACKLIGHT);
							for (int i = 0; i < NEON_BACK; i++)
							{
								VEHICLE::_SET_VEHICLE_NEON_LIGHT_ENABLED(playerVeh, i, TRUE);
							}
							AddClanLogoToVehicle(playerVeh, playerPed);
						}
						STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehicleModelHash);
						bWaitingForModelCar = false;
						WAIT(0); //We need to wait for the game to assign a random radio station to the car first before changing it.
						AUDIO::SET_VEH_RADIO_STATION(playerVeh, AUDIO::GET_RADIO_STATION_NAME(RADIO_SELFRADIO));
					}
					else
					{
						bWaitingForModelCar = true;
					}
				}
				else if ( playerVeh != NULL )
				{
					RequestControl(playerVeh);
					VEHICLE::SET_VEHICLE_FORWARD_SPEED(playerVeh, VEHICLE::_GET_VEHICLE_MODEL_MAX_SPEED(ENTITY::GET_ENTITY_MODEL(playerVeh)));
				}
			}

			//Spawn a test aircraft.
			static bool bNumpad3Pressed, bWaitingForModelAircraft = false;
			if (isKeyPressedOnce(bNumpad3Pressed, VK_NUMPAD3))
			{
				if (bWaitingForModelAircraft || playerVeh == NULL)
				{
					Hash vehicleModelHash = VEHICLE_CARGOPLANE;
					if (IsKeyPressed(VK_RCONTROL))
						vehicleModelHash = VEHICLE_VALKYRIE;
					else if (IsKeyPressed(VK_RIGHT))
						vehicleModelHash = VEHICLE_LAZER;
					STREAMING::REQUEST_MODEL(vehicleModelHash);
					if (STREAMING::HAS_MODEL_LOADED(vehicleModelHash) == TRUE)
					{
						Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPed, FALSE);
						playerVeh = VEHICLE::CREATE_VEHICLE(vehicleModelHash, playerPosition.x, playerPosition.y, playerPosition.z + 800, ENTITY::GET_ENTITY_HEADING(playerPed), TRUE, TRUE);
						PED::SET_PED_INTO_VEHICLE(playerPed, playerVeh, SEAT_DRIVER);
						if (vehicleModelHash == VEHICLE_CARGOPLANE)
						{
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_ICE_WHITE, COLOR_CLASSIC_ULTRA_BLUE);
						}
						else if (vehicleModelHash == VEHICLE_VALKYRIE)
						{
							VEHICLE::SET_VEHICLE_COLOURS(playerVeh, COLOR_CLASSIC_SHADOW_SILVER, COLOR_CLASSIC_SHADOW_SILVER);
							VEHICLE::SET_HELI_BLADES_FULL_SPEED(playerVeh);
						}
						BoostBaseVehicleStats(playerVeh);
						STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehicleModelHash);
						bWaitingForModelAircraft = false;
					}
					else
					{
						bWaitingForModelAircraft = true;
					}
				}
				if (playerVeh != NULL && (VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(playerVeh)) == TRUE || VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(playerVeh)) == TRUE ||
					ENTITY::GET_ENTITY_MODEL(playerVeh) == VEHICLE_BLIMP || ENTITY::GET_ENTITY_MODEL(playerVeh) == VEHICLE_BLIMP2 || ENTITY::GET_ENTITY_MODEL(playerVeh) == VEHICLE_TITAN) )
				{
					Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerVeh, FALSE);
					RequestControl(playerVeh);
					if (playerPosition.z < 350.0f)
						ENTITY::SET_ENTITY_COORDS_NO_OFFSET(playerVeh, playerPosition.x, playerPosition.y, playerPosition.z + 800, FALSE, FALSE, TRUE);
					VEHICLE::SET_VEHICLE_FORWARD_SPEED(playerVeh, 500.0f);
					BoostBaseVehicleStats(playerVeh);
				}
			}
		}

		//TP to mission objective.
		static bool bNumpad7Pressed = false;
		if (isKeyPressedOnce(bNumpad7Pressed, VK_NUMPAD7))
		{
			for (int i = 0; i <= 1000; i++)
			{
				Blip_t* blip = pBlipList->m_Blips[i].m_pBlip;
				if (blip)
				{
					if ((blip->dwColor == BLIPCOLOR_MISSION && blip->iIcon == BLIP_CIRCLE) ||
						(blip->dwColor == BLIPCOLOR_YELLOWMISSION && blip->iIcon == BLIP_CIRCLE) ||
						(blip->dwColor == BLIPCOLOR_YELLOWMISSION2 && (blip->iIcon == BLIP_CIRCLE || blip->iIcon == BLIP_DOLLARSIGN)) ||
						(blip->dwColor == BLIPCOLOR_NONE && blip->iIcon == BLIP_RACEFLAG) ||
						(blip->dwColor == BLIPCOLOR_GREEN && blip->iIcon == BLIP_CIRCLE) ||
						(blip->iIcon == BLIP_SPECIALCRATE))
					{
						Entity e = playerPed;
						if (playerVeh)
							e = playerVeh;
						RequestControl(playerVeh);
						ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, blip->x, blip->y, blip->z, FALSE, FALSE, TRUE);
						break; //During a race there's sometimes 2 yellow markers. We want the first one.
					}
				}
			}
		}

		//Emulated am_cr_securityvan money drop script.
		/*if (moneyPlayer != -1)
		{
			Ped moneyPed = PLAYER::GET_PLAYER_PED(moneyPlayer);
			if (ENTITY::DOES_ENTITY_EXIST(moneyPed) && !(PED::IS_PED_IN_ANY_VEHICLE(moneyPed, TRUE)))
			{
				static DWORD time = 0;
				if ((timeGetTime() - time) > 2500)
				{
					STREAMING::REQUEST_MODEL(PROP_MONEY_BAG_01);
					if(!STREAMING::HAS_MODEL_LOADED(PROP_MONEY_BAG_01))
						WAIT(0);
					else
					{
						int numBags = GAMEPLAY::GET_RANDOM_INT_IN_RANGE(2, 6);
						int cashMoneyBaby = (GAMEPLAY::GET_RANDOM_INT_IN_RANGE(4000, 12001) / numBags);
						cashMoneyBaby = (int)round(cashMoneyBaby);
						for (int i = 0; i < numBags; i++)
						{
							Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(moneyPed, FALSE);
							OBJECT::CREATE_AMBIENT_PICKUP(PICKUP_MONEY_CASE, playerPosition.x, playerPosition.y, playerPosition.z + 0.5f, 0, cashMoneyBaby, PROP_MONEY_BAG_01, FALSE, TRUE);
						}
						STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(PROP_MONEY_BAG_01);
						time = timeGetTime();
					}
				}
			}
		}*/

		//Freeze memes in place.
		static int iFreezeCounter = 0;
		for (Player playerIterator = 0; playerIterator < MAX_PLAYERS; playerIterator++)
		{
			try
			{
				if (GetNetworkHandle(playerIterator) == freezePlayer[playerIterator])
				{
					Ped ped = PLAYER::GET_PLAYER_PED(playerIterator);
					if (ENTITY::DOES_ENTITY_EXIST(ped) && !ENTITY::IS_ENTITY_DEAD(ped))
					{
						if (iFreezeCounter >= 4)
						{
							WEAPON::REMOVE_ALL_PED_WEAPONS(ped, TRUE);
							AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);
							iFreezeCounter = 0;
						}
					}
				}
				else
				{
					freezePlayer[playerIterator] = NULL;
				}
			}
			catch (...) { freezePlayer[playerIterator] = NULL; }
		}
		iFreezeCounter++;

		//Shoot all spaghettios (Fuck Deliver EMP)
		if (IsKeyPressed(VK_NUMPAD9))
		{
			for (int i = 0; i <= 1000; i++)
			{
				Blip_t* blip = pBlipList->m_Blips[i].m_pBlip;
				if (blip)
				{
					if (blip->dwColor != BLIPCOLOR_BLUE) //Don't hit friendlies.
					{
						if (blip->iIcon == BLIP_COP /*cop*/ || blip->iIcon == BLIP_SPAGHETTIO || (blip->iIcon == BLIP_CIRCLE && blip->dwColor == BLIPCOLOR_RED))
						{
							static bool bShoot = false;
							bShoot = !bShoot;
							if (bShoot)
							{
								static Hash weaponList[] = { WEAPON_ADVANCEDRIFLE, WEAPON_APPISTOL, WEAPON_ASSAULTRIFLE, WEAPON_ASSAULTSMG, WEAPON_CARBINERIFLE, WEAPON_COMBATMG, WEAPON_COMBATPDW, WEAPON_COMBATPISTOL, WEAPON_HEAVYPISTOL, WEAPON_HEAVYSNIPER, WEAPON_MARKSMANRIFLE, WEAPON_MG, WEAPON_MICROSMG, WEAPON_PISTOL, WEAPON_PISTOL50, WEAPON_SMG, WEAPON_SNIPERRIFLE, WEAPON_SNSPISTOL, WEAPON_SPECIALCARBINE, WEAPON_MINIGUN };
								if (blip->fScale >= 1.0f)
									FIRE::ADD_OWNED_EXPLOSION(playerPed, blip->x, blip->y, blip->z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f);
								else
								{
									srand((unsigned int)time(NULL));
									GAMEPLAY::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(blip->x + 0.1f, blip->y, blip->z - 0.15f, blip->x - 0.1f, blip->y, blip->z + 1, 1000, TRUE, weaponList[rand() % (sizeof(weaponList) / 4)], playerPed, TRUE, TRUE, 1.0f); //FWARRRRRRAING! ~benji Alaska 2277
								}
							}
						}
						if ((blip->dwColor == BLIPCOLOR_NONE && (blip->iIcon == BLIP_HELIBLADESENEMY || blip->iIcon == BLIP_COPHELICOPTER)) ||
							((blip->dwColor == BLIPCOLOR_RED || blip->dwColor == BLIPCOLOR_REDMISSION) && (blip->iIcon == BLIP_PLANE || blip->iIcon == BLIP_MOTORCYCLE || blip->iIcon == BLIP_CAR || blip->iIcon == BLIP_HELICOPTER || blip->iIcon == BLIP_JET2 || blip->iIcon == BLIP_HELICOPTERBLADES || blip->iIcon == BLIP_PLANEVEHICLE)))
						{
							FIRE::ADD_OWNED_EXPLOSION(playerPed, blip->x, blip->y, blip->z, EXPLOSION_TANKER, 1000.0f, FALSE, TRUE, 0.0f);
						}
					}
				}
			}
		}

		//Infinite Ammo
		if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &currentWeapon, 1))
		{
			if (WEAPON::IS_WEAPON_VALID(currentWeapon))
			{
				int maxAmmo;
				if (WEAPON::GET_MAX_AMMO(playerPed, currentWeapon, &maxAmmo))
				{
					WEAPON::SET_PED_AMMO(playerPed, currentWeapon, maxAmmo);
				}
				WEAPON::SET_PED_INFINITE_AMMO_CLIP(playerPed, (WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, currentWeapon, TRUE) == 1) && !(IsKeyPressed(0x52)));
			}
		}

		//Force full reload animation on weapon. If you want a quicker reload, just quickly tap R. The 0x1 state seems to be time sensitive.
		if (GetAsyncKeyState(0x52) & 0x1)
		{
			if (WEAPON::IS_WEAPON_VALID(currentWeapon) && WEAPON::GET_WEAPONTYPE_GROUP(currentWeapon) == WEAPON_TYPE_GROUP_PISTOL && playerVeh == NULL) //Only pistols need this. #ONLY90SKIDSREMBERTHINGSFROMTHE90S
			{
				int clipazine, maxAmmo;
				WEAPON::GET_AMMO_IN_CLIP(playerPed, currentWeapon, &clipazine); //m8 do you even 30 caliber assault magazine clips?
				maxAmmo = WEAPON::GET_MAX_AMMO_IN_CLIP(playerPed, currentWeapon, TRUE);
				if (clipazine != maxAmmo && clipazine > 0)
				{
					keybd_event(0x52, 0, KEYEVENTF_KEYUP, 0); //We need to jitter the game's listener for the R key for one frame.
					WEAPON::SET_AMMO_IN_CLIP(playerPed, currentWeapon, 0);
				}
			}
		}

		//Increase wanted level.
		static bool bAddPressed = false;
		if (isKeyPressedOnce(bAddPressed, VK_ADD))
		{
			if (PLAYER::GET_PLAYER_WANTED_LEVEL(player) < 5)
			{
				PLAYER::SET_PLAYER_WANTED_LEVEL(player, PLAYER::GET_PLAYER_WANTED_LEVEL(player) + 1, FALSE);
				PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, FALSE);
			}
		}

		//Clear Wanted Level
		if (IsKeyPressed(VK_MULTIPLY))
		{
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, FALSE);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, FALSE);
		}

		//Fix player.
		static bool bDecimalPressed = false;
		if (isKeyPressedOnce(bDecimalPressed, VK_DECIMAL))
		{
			if (ENTITY::DOES_ENTITY_EXIST(playerVeh) && !ENTITY::IS_ENTITY_DEAD(playerVeh))
			{
				RequestControl(playerVeh); //Can't hurt to try.
				VEHICLE::SET_VEHICLE_FIXED(playerVeh);
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(playerVeh);
				BoostBaseVehicleStats(playerVeh);
			}
			PED::CLEAR_PED_BLOOD_DAMAGE(playerPed);
			MakeEventCarPurchaseable(VEHICLE_BTYPE2);
			ClearMentalState();
		}

		static bool F10Pressed = false;
		if (isKeyPressedOnce(F10Pressed, VK_F10) && playerVeh != NULL )
		{
			DumpVehicleStats(playerVeh);
		}

		static bool bF11Pressed = false;
		if (isKeyPressedOnce(bF11Pressed, VK_F11))
		{
			for (int i = 0; i <= 1000; i++)
			{
				Blip_t* blip = pBlipList->m_Blips[i].m_pBlip;
				if (blip)
				{
					if (blip->bFocused & 0x40)
						Log::Msg("Blip%i ID: %i ID2: %i Scale: %f Icon: %i Color: 0x%X Message: %s", i, blip->iID, blip->iID2, blip->fScale, blip->iIcon, blip->dwColor, blip->szMessage == NULL ? "" : blip->szMessage);
				}
			}
		}

		DWORD processIdFocused;
		GetWindowThreadProcessId(GetForegroundWindow(), &processIdFocused);
		if (processIdFocused == GetCurrentProcessId())
		{
			static bool bMouse5Pressed = false;
			if (isKeyPressedOnce(bMouse5Pressed, VK_XBUTTON1))
			{
				static BOOL bThermalVision = FALSE;
				bThermalVision = !bThermalVision;
				GRAPHICS::SET_SEETHROUGH(bThermalVision);
			}

			static bool b1pressed = false;
			if (isKeyPressedOnce(b1pressed, 0x31))
			{
				if (currentWeapon == WEAPON_UNARMED)
					CheckAndSelectWeapon(playerPed, WEAPON_KNIFE);
				else
					CheckAndSelectWeapon(playerPed, WEAPON_UNARMED);
			}
			static bool b2pressed = false;
			if (isKeyPressedOnce(b2pressed, 0x32))
			{
				if (playerVeh == NULL)
				{
					if (currentWeapon == WEAPON_HEAVYPISTOL)
						CheckAndSelectWeapon(playerPed, WEAPON_MARKSMANPISTOL);
					else
						CheckAndSelectWeapon(playerPed, WEAPON_HEAVYPISTOL);
				}
				else if (currentWeapon == WEAPON_MICROSMG)
					CheckAndSelectWeapon(playerPed, WEAPON_FLAREGUN);
				else
					CheckAndSelectWeapon(playerPed, WEAPON_MICROSMG);
			}
			static bool b3pressed = false;
			if (isKeyPressedOnce(b3pressed, 0x33))
			{
				if (currentWeapon == WEAPON_ADVANCEDRIFLE)
					CheckAndSelectWeapon(playerPed, WEAPON_CARBINERIFLE);
				else
					CheckAndSelectWeapon(playerPed, WEAPON_ADVANCEDRIFLE);
			}
			static bool b4pressed = false;
			if (isKeyPressedOnce(b4pressed, 0x34))
			{
				CheckAndSelectWeapon(playerPed, WEAPON_MINIGUN);
			}
			static bool b5pressed = false;
			if (isKeyPressedOnce(b5pressed, 0x35))
			{
				if (currentWeapon == WEAPON_HEAVYSNIPER)
					CheckAndSelectWeapon(playerPed, WEAPON_MARKSMANRIFLE);
				else
					CheckAndSelectWeapon(playerPed, WEAPON_HEAVYSNIPER);
			}
			static bool b6pressed = false;
			if (isKeyPressedOnce(b6pressed, 0x36))
			{
				if (playerVeh)
					CheckAndSelectWeapon(playerPed, WEAPON_STICKYBOMB);
				else
					CheckAndSelectWeapon(playerPed, WEAPON_HOMINGLAUNCHER);
			}
		}

		//How about some ESP? Commented out because it sucks.
		/*for (Player playerIterator = 0; playerIterator < MAX_PLAYERS; playerIterator++)
		{
			Ped playerPedIterator = PLAYER::GET_PLAYER_PED(playerIterator);
			if (ENTITY::DOES_ENTITY_EXIST(playerPedIterator) && !ENTITY::IS_ENTITY_DEAD(playerPedIterator) && playerPedIterator != playerPed)
			{
				//Remove this if you want to see everyone or people behind walls.
				if (ENTITY::HAS_ENTITY_CLEAR_LOS_TO_ENTITY(playerPed, playerPedIterator))
				{
					Vector3 playerPosition = ENTITY::GET_ENTITY_COORDS(playerPedIterator, FALSE);
					float screenX, screenY;
					if (GRAPHICS::_WORLD3D_TO_SCREEN2D(playerPosition.x, playerPosition.y, playerPosition.z +.5, &screenX, &screenY) == TRUE) //If we can see them.
					{
						//DEBUGOUT("Drawing on %s %f %f", PLAYER::GET_PLAYER_NAME(playerIterator), screenX, screenY);
						
					}
				}
			}
		}*/
	}

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