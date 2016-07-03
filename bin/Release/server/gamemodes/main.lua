function OnGameModeInit()
	print("My server has started.");
	return 1;
end

function OnGameModeExit()
	print("My server has stopped.");
	return 1;
end

function OnPlayerConnect(playerid)
	ShowMessageToPlayer(playerid, "welcome to fivemp");
	return 1;
end
