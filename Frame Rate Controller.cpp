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
	mTickStart = mTickEnd = mFrameTime = 0;

	if (0 != MaxFramerate)
		mNeededTicksPerFrame = 1000 / MaxFramerate;
	else
		mNeededTicksPerFrame = 0;
}

FrameRateController:: ~FrameRateController()
{

}

void FrameRateController::FrameStart()
{
	mTickStart = clock();
}

void FrameRateController::FrameEnd()
{
	mTickEnd = clock();
	while (mTickEnd - mTickStart < mNeededTicksPerFrame)
	{
		mTickEnd = clock();
	}
	mFrameTime = mTickEnd - mTickStart;
}

float FrameRateController::GetFrameTime()
{
	return mFrameTime*0.001f;
}

void FrameRateController::SetMaxFrameRate(uint32_t MaxFramerate) {
	if (0 != MaxFramerate)
		mNeededTicksPerFrame = 1000 / MaxFramerate;
	else
		mNeededTicksPerFrame = 0;
}

uint32_t FrameRateController::GetMaxFrameRate() {
	return mNeededTicksPerFrame * 1000;
}