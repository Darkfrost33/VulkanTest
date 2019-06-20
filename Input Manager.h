/* Start Header -------------------------------------------------------
Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name: <Input Manager.h>
Purpose: <Define input manager class and its functions>
Language: <C++>
Platform: <VS, PC, Win10>
Project: <CS529_daxiao.final>
Author: <Daxiao Ge, daxiao.g, 60001118>
Creation date: <10/12/2018>
History:
- End Header --------------------------------------------------------*/
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include "stdint.h"
#include "Singleton.h"
class Input_Manager: public Singleton<Input_Manager>
{
public:
	~Input_Manager();

	void Update();

	bool IsPressed(unsigned int KeyScanCode); // From SDL_Scancode
	bool IsTriggered(unsigned int KeyScanCode); // From SDL_Scancode
	bool IsReleased(unsigned int KeyScanCode); // From SDL_Scancode
	glm::vec2 GetMouseMovedRight();

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_move_callback(GLFWwindow* window, double x, double y);
private:
	friend class Singleton<Input_Manager>;
	Input_Manager();
	uint8_t mCurrentState[256];
	uint8_t mPreviousState[256];
	bool rightClick;
	double prevX, prevY, X, Y;
};


#endif
