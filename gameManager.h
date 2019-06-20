#pragma once

class VulkanAPI;
class FrameRateController;
class Input_Manager;
class ComponentManager;
class ResourceManager;
class GameManager
{
public:
	GameManager();
	~GameManager();

	void initiate();
	void mainLoop();
private:
	ResourceManager* pResourceManager;
	VulkanAPI* pGraphics;
	FrameRateController* pFRC;
	Input_Manager* pInputManager;
	ComponentManager* pComponentManager;

	void CreateGameObjects();
};