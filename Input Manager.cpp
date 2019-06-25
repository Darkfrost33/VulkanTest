/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name: <Input Manager.cpp>
Purpose: <Implement input manager functions>
Language: <C++>
Platform: <VS, PC, Win10>
Project: <CS529_daxiao.final>
Author: <Daxiao Ge, daxiao.g, 60001118>
Creation date: <10/12/2018>
History:
- End Header --------------------------------------------------------*/
#include "pch.h"
#include "Input manager.h"
Input_Manager::Input_Manager()
{
	memset(mCurrentState, 0, 256 * sizeof(uint8_t));
	rightClick = false;
	prevX = prevY = X = Y = 0;
}
Input_Manager::~Input_Manager()
{
}


void Input_Manager::Update()
{
	memcpy(mPreviousState, mCurrentState, 256 * sizeof(uint8_t));
	prevX = X;
	prevY = Y;
	for (int i = 0; i < 256; ++i) {
		mCurrentState[i] = (uint8_t)(GetAsyncKeyState(i) >> 8);
	}
}

bool Input_Manager::IsPressed(unsigned int KeyScanCode)
{
	if(KeyScanCode >=256)
	return false;

	if (mCurrentState[KeyScanCode])
		return true;

	return false;
}

bool Input_Manager::IsTriggered(unsigned int KeyScanCode)
{
	if (KeyScanCode >= 256)
		return false;
	if (mCurrentState[KeyScanCode] && !mPreviousState[KeyScanCode])
		return true;
	return false;
}

bool Input_Manager::IsReleased(unsigned int KeyScanCode)
{
	if (KeyScanCode >= 256)
		return false;
	if (!mCurrentState[KeyScanCode] && mPreviousState[KeyScanCode])
		return true;
	return false;
}

glm::vec2 Input_Manager::GetMouseMovedRight()
{
	if (!rightClick)
	{
		return glm::vec2(0.0f,0.0f);
	}
	//std::cout << "prevX: " << Input_Manager::GetInstance()->prevX << "  prevY: " << Input_Manager::GetInstance()->prevY << ",  x: " << Input_Manager::GetInstance()->X << "  y: " << Input_Manager::GetInstance()->Y << std::endl;
	return glm::vec2(X-prevX, Y-prevY);
}

void Input_Manager::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			//printf("Mosue left button clicked!\n");
			Input_Manager::GetInstance()->leftClick = true;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			//printf("Mosue middle button clicked!\n");
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			//printf("Mosue right button clicked!\n");
			Input_Manager::GetInstance()->rightClick = true;
			break;
		default:
			return;
	}
	else if (action == GLFW_RELEASE) switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			//printf("Mosue left button released!\n");
			Input_Manager::GetInstance()->leftClick = false;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			//printf("Mosue middle button released!\n");
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			//printf("Mosue right button released!\n");
			Input_Manager::GetInstance()->rightClick = false;
			break;
		default:
			return;
	}
}

void Input_Manager::mouse_move_callback(GLFWwindow* window, double x, double y)
{
	//printf("Mouse position move to [%d:%d]\n", int(x), int(y));
	Input_Manager::GetInstance()->X = x;
	Input_Manager::GetInstance()->Y = y;
	//std::cout << "prevX: " << Input_Manager::GetInstance()->prevX << "  prevY: " << Input_Manager::GetInstance()->prevY << ",  x: " << x << "  y: " << y << std::endl;
}
