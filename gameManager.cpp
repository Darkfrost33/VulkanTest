#include "pch.h"
#include "gameManager.h"
#include "vulkanAPI.h"
#include "Frame Rate Controller.h"
#include "Input Manager.h"
#include "componentManager.h"
#include "ResourceManager.h"

float t = 0.0f;
GameManager::GameManager()
{
	pGraphics = nullptr;
	pFRC = nullptr;
	pInputManager = nullptr;
	pComponentManager = nullptr;
	pResourceManager = nullptr;
}

GameManager::~GameManager()
{
	pGraphics->cleanup();
	delete pFRC;
}

void GameManager::initiate()
{
	std::srand(std::time(nullptr));
	pResourceManager = ResourceManager::GetInstance();
	pResourceManager->Initialize_Models();
	pFRC = new FrameRateController(0);
	pInputManager = Input_Manager::GetInstance();
	pComponentManager = ComponentManager::GetInstance();
	pGraphics = VulkanAPI::GetInstance();
	pGraphics->initWindow();
	pGraphics->initVulkan();
	pResourceManager->Initialize_Textures();
	CreateGameObjects();
}

void GameManager::mainLoop()
{
	while (!glfwWindowShouldClose(pGraphics->pWindow)) {
		t += pFRC->GetFrameTime()*10;
		pFRC->FrameStart();
		pInputManager->Update();
		glfwPollEvents();
		size_t ti = 0;
		for (auto i: pComponentManager->activeComponents[TRANSFORM])
		{
			if (i != 0) {
				pComponentManager->mTransforms[i].localPosition = glm::vec3(
					20 * cosf(glm::radians(360.0f / pComponentManager->activeComponents[TRANSFORM].size()*ti + t)),
					20 * sinf(glm::radians(360.0f / pComponentManager->activeComponents[TRANSFORM].size()*ti + t)),
					0);
				pComponentManager->mTransforms[i].dirty = true;
				ti++;
			}
		}
		pComponentManager->Update(pFRC->GetFrameTime());

		pGraphics->draw();
		pComponentManager->Delete();
		pComponentManager->registerNewComponents();
		pFRC->FrameEnd();
		//printf("FrameRate: %f\n", pFRC->GetFrameTime());
	}
}

void GameManager::CreateGameObjects()
{
	uint32_t newGameObjectID = pComponentManager->newComponent(ENTITY);
	uint32_t newComponent = pComponentManager->newComponent(newGameObjectID, TRANSFORM);
	newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	newComponent = pComponentManager->newComponent(newGameObjectID, CAMERA);
	pComponentManager->StartComponents(newGameObjectID);

	//for (int i = 0; i < 32; ++i)
	//{
	//	uint32_t newGameObjectID = pComponentManager->newComponent(ENTITY);
	//	uint32_t newComponent = pComponentManager->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(
	//			20*cosf(glm::radians(360.0f / 32*i)),
	//			20* sinf(glm::radians(360.0f / 32 *i)),
	//			0);

	//	newComponent = pComponentManager->newComponent(newGameObjectID, MESH);
	//	pComponentManager->mMeshes[newComponent].BindMesh("box");

	//	pComponentManager->StartComponents(newGameObjectID);
	//}
}
