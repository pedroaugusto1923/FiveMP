#pragma once

bool memory_compare(const BYTE *data, const BYTE *pattern, const char *mask);
UINT64 FindPattern(char *pattern, char *mask);