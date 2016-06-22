#include "stdafx.h"
#include "modbaserequired.h"
#include "StdInc.h"
#include <thread>

bool memory_compare(const BYTE *data, const BYTE *pattern, const char *mask)
{
	for (; *mask; ++mask, ++data, ++pattern)
	{
		if (*mask == 'x' && *data != *pattern)
		{
			return false;
		}
	}
	return (*mask) == NULL;
}
UINT64 FindPattern(char *pattern, char *mask)
{	//Edited, From YSF by Kurta999
	UINT64 i;
	UINT64 size;
	UINT64 address;

	MODULEINFO info = { 0 };

	address = (UINT64)GetModuleHandle(NULL);
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &info, sizeof(MODULEINFO));
	size = (UINT64)info.SizeOfImage;

	for (i = 0; i < size; ++i)
	{
		if (memory_compare((BYTE *)(address + i), (BYTE *)pattern, mask))
		{
			return (UINT64)(address + i);
		}
	}
	return 0;
}