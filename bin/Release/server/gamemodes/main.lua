function OnGameModeInit()
	print("My server has started.");
	return 1;
end

function OnGameModeExit()
	print("My server has stopped.");
	return 1;
end

function OnPlayerConnect(playerid)
	
	GivePlayerWeapon(playerid, "Firework", 10);
	ShowMessageToPlayer(playerid, "Welcome to this ~b~FiveMP ~w~server!");
	ShowMessageToPlayer(playerid, "Your player ID is ~b~" .. playerid);
	return 1;
end

function OnPlayerDisonnect(playerid)
    print(playerid .. " has disconnected");
    return 1;
end

