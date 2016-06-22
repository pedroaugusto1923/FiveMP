#ifndef MODBASEREQUIRED_H
#define MODBASEREQUIRED_H
#include <algorithm>
#include <array>

// NOTE: Must ALWAYS be unsigned
typedef unsigned short EntityId;

bool memory_compare(const BYTE *data, const BYTE *pattern, const char *mask);
UINT64 FindPattern(char *pattern, char *mask);
extern bool modbaseRequiredStarted;

#endif