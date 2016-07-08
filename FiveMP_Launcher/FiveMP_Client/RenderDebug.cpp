#include "stdafx.h"

void CRenderDebug::RenderDate()
{
	char alphadata[128];
	sprintf(alphadata, "Five~r~MP~w~ Alpha | %s - %s", __DATE__, __TIME__);
	draw_text(0.002f, 0.002f, alphadata, { 255, 255, 255, 255 });
}

void CRenderDebug::RenderBlend()
{
	char blenddata[32];
	sprintf(blenddata, "%f", AI::GET_PED_DESIRED_MOVE_BLEND_RATIO(LocalPlayer->playerPed));
	draw_text(0.750f, 0.925f, blenddata, { 255, 255, 255, 255 });
}

void CRenderDebug::RenderVelocity()
{
	char velocitydata[64];
	sprintf(velocitydata, "X = %f | Y = %f | Z = %f", LocalPlayer->GetVelocity().x, LocalPlayer->GetVelocity().y, LocalPlayer->GetVelocity().z);
	draw_text(0.750f, 0.950f, velocitydata, { 255, 255, 255, 255 });
}

void CRenderDebug::RenderCoords()
{
	char coorddata[64];
	sprintf(coorddata, "X = %f | Y = %f | Z = %f", LocalPlayer->GetCoords().x, LocalPlayer->GetCoords().y, LocalPlayer->GetCoords().z);
	draw_text(0.750f, 0.975f, coorddata, { 255, 255, 255, 255 });
}
