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
	ShowMessageToPlayer(playerid, "welcome to fivemp");
	return 1;
end

function OnPlayerDisonnect(playerid)
    print(playerid .. " disconnected");
    return 1;
end

