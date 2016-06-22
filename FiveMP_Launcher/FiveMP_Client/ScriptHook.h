#pragma once

class ScriptHook {
public:
	void Initialize(HMODULE hModule);
	void NoIntro();
	void StartConsole();
	void StopConsole();
};