#pragma once

#include "stdafx.h"

void GameWorld::CleanUp() {

	for (int i = 0; i < sizeof(playerData) / sizeof(*playerData); i++) {
		if (ENTITY::DOES_ENTITY_EXIST(playerData[i].pedPed)) {
			ENTITY::DELETE_ENTITY(&playerData[i].pedPed);
			UI::REMOVE_BLIP(&playerData[i].pedBlip);
		}
	}
}