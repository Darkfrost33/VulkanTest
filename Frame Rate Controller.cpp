/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name: <Frame Rate Controller.cpp>
Purpose: <Implement frame rate controller functions>
Language: <C++>
Platform: <VS, PC, Win10>
Project: <CS529_daxiao.final>
Author: <Daxiao Ge, daxiao.g, 60001118>
Creation date: <10/12/2018>
History:
- End Header --------------------------------------------------------*/
#include "pch.h"
#include "Frame Rate Controller.h"

FrameRateController::FrameRateController(uint32_t MaxFramerate)
{
	mFrameTime = 0;

	if (0 != MaxFramerate)
		mNeededTicksPerFrame = 1.0f / MaxFramerate;
	else
		mNeededTicksPerFrame = 0;
}

FrameRateController:: ~FrameRateController()
{

}

void FrameRateController::FrameStart()
{
	mTickStart = std::chrono::high_resolution_clock::now();
}

void FrameRateController::FrameEnd()
{
	mTickEnd = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(mTickEnd - mTickStart).count();
	while (time < mNeededTicksPerFrame)
	{
		mTickEnd = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(mTickEnd - mTickStart).count();
	}
	mFrameTime = time;
}

float FrameRateController::GetFrameTime()
{
	return mFrameTime;
}

void FrameRateController::SetMaxFrameRate(uint32_t MaxFramerate) {
	if (0 != MaxFramerate)
		mNeededTicksPerFrame = 1.0f / MaxFramerate;
	else
		mNeededTicksPerFrame = 0;
}

uint32_t FrameRateController::GetMaxFrameRate() {
	return 1.0f/mNeededTicksPerFrame;
}