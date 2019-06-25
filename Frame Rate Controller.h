/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name: <Frame Rate Controller.h>
Purpose: <Define Frame rate controller class and its functions>
Language: <C++>
Platform: <VS, PC, Win10>
Project: <CS529_daxiao.final>
Author: <Daxiao Ge, daxiao.g, 60001118>
Creation date: <10/12/2018>
History:
- End Header --------------------------------------------------------*/
#ifndef FRC_H
#define FRC_H
#include "stdint.h"
#include "time.h"

class FrameRateController
{
public:
	FrameRateController(uint32_t MaxFramerate);
	~FrameRateController();

	void FrameStart();
	void FrameEnd();
	float GetFrameTime();
	void SetMaxFrameRate(uint32_t MaxFramerate);
	uint32_t GetMaxFrameRate();
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> mTickStart, mTickEnd;
	float mFrameTime;
	float mNeededTicksPerFrame;
};

#endif
