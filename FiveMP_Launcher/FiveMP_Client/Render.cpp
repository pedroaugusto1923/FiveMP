#include "stdafx.h"

void CRender::RenderNametags()
{
	for (int i = 0; i < sizeof(playerData) / sizeof(*playerData); i++) {
		if (ENTITY::DOES_ENTITY_EXIST(playerData[i].pedPed)) {
			//if (ENTITY::IS_ENTITY_VISIBLE(playerData[i].pedPed)) {
			GRAPHICS::_WORLD3D_TO_SCREEN2D(playerData[i].x, playerData[i].y, playerData[i].z, &playerData[i].screen_x, &playerData[i].screen_y);
			draw_text(playerData[i].screen_x, playerData[i].screen_y, "User", { 255, 255, 255, 255 });
			//}
		}
	}
}
