#pragma once

#include "stdafx_raknet.h"

void InitGameScript();
void RunGameScript();

unsigned char GetPacketIdentifier(RakNet::Packet * p);