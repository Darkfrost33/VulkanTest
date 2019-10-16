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
	pFRC = new FrameRateController(60);
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
		pFRC->FrameStart();
		float deltaT = pFRC->GetFrameTime();
		t += deltaT*10.0f;
		pInputManager->Update();
		glfwPollEvents();
		//size_t ti = 0;
		//for (auto i: pComponentManager->activeComponents[TRANSFORM])
		//{
		//	if (i != 0) {
		//		pComponentManager->mTransforms[i].localPosition = glm::vec3(
		//			20 * cosf(glm::radians(360.0f / pComponentManager->activeComponents[TRANSFORM].size()*ti + t)),
		//			20 * sinf(glm::radians(360.0f / pComponentManager->activeComponents[TRANSFORM].size()*ti + t)),
		//			0);
		//		pComponentManager->mTransforms[i].localRotation *= 
		//			glm::quat(glm::radians(glm::vec3(0.0f, deltaT*60.0f, 0.0f)));
		//		pComponentManager->mTransforms[i].dirty = true;
		//		ti++;
		//	}
		//}
		pComponentManager->Update(deltaT);

		pGraphics->draw(deltaT);
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
	newComponent = pComponentManager->newComponent(newGameObjectID, BODY);
	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
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
	//{
	//	uint32_t newGameObjectID = pComponentManager->newComponent(ENTITY);
	//	uint32_t newComponent = pComponentManager->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.0f, 10.0f, 0.f);
	//	//newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	//	newComponent = pComponentManager->newComponent(newGameObjectID, MESH);
	//	pComponentManager->mMeshes[newComponent].BindMesh("sphere");
	//	newComponent = pComponentManager->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 10.0f;
	//	//ComponentManager::GetInstance()->mBodies[newComponent].vel = glm::vec3(10.0f, 0.0f, 0.0f);
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = pComponentManager->newComponent(newGameObjectID, COLLIDER);
	//	pComponentManager->mColliders[newComponent].type = SPHERE;
	//	pComponentManager->mColliders[newComponent].radius = 1.0f;
	//	pComponentManager->StartComponents(newGameObjectID);
	//}
	{
		uint32_t newGameObjectID = pComponentManager->newComponent(ENTITY);
		uint32_t newComponent = pComponentManager->newComponent(newGameObjectID, TRANSFORM);
		pComponentManager->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
		pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.0f, 20.0f, 0.f);
		newComponent = pComponentManager->newComponent(newGameObjectID, MESH);
		pComponentManager->mMeshes[newComponent].BindMesh("sphere");
		newComponent = pComponentManager->newComponent(newGameObjectID, BODY);
		//ComponentManager::GetInstance()->mBodies[newComponent].restitution = 0.4f;
		ComponentManager::GetInstance()->mBodies[newComponent].mass = 10.0f;
		ComponentManager::GetInstance()->mBodies[newComponent].vel = glm::vec3(0.0f,-10.0f,0.0f);
		newComponent = pComponentManager->newComponent(newGameObjectID, COLLIDER);
		pComponentManager->mColliders[newComponent].type = SPHERE;
		pComponentManager->mColliders[newComponent].radius = 5.0f;
		pComponentManager->StartComponents(newGameObjectID);
	}
	//{
	//	uint32_t newGameObjectID = pComponentManager->newComponent(ENTITY);
	//	uint32_t newComponent = pComponentManager->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, -9.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(10.0f, 10.0f, 10.0f);
	//	newComponent = pComponentManager->newComponent(newGameObjectID, MESH);
	//	pComponentManager->mMeshes[newComponent].BindMesh("sphere");
	//	newComponent = pComponentManager->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 200.0f;
	//	newComponent = pComponentManager->newComponent(newGameObjectID, COLLIDER);
	//	pComponentManager->mColliders[newComponent].type = SPHERE;
	//	pComponentManager->mColliders[newComponent].radius = 10.0f;
	//	pComponentManager->StartComponents(newGameObjectID);
	//}

	//for(int i=0;i<10;++i)
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	//newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, 5.1f*i, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(10.f, 5.0f, 10.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 200.0f-5*i;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 10.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 10.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 5.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	//newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(-15.f, -2.5f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(20.f, 5.0f, 15.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 15.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 20.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 5.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	//newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(15.f, -2.5f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(20.f, 5.0f, 15.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 15.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 20.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 5.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	//newComponent = pComponentManager->newComponent(newGameObjectID, CONTROLLER);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, 2.5f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(20.f, 5.0f, 15.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 50.0f;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 15.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 20.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 5.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	{
		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
		pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, -10.f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
		pComponentManager->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.f)));
		pComponentManager->mTransforms[newComponent].localScale = glm::vec3(100.0f,10.0f,100.0f);
		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
		ComponentManager::GetInstance()->mBodies[newComponent].mass = 1000.0f;
		ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
		ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
		ComponentManager::GetInstance()->mColliders[newComponent].length = 100.0f;
		ComponentManager::GetInstance()->mColliders[newComponent].width = 100.0f;
		ComponentManager::GetInstance()->mColliders[newComponent].height = 10.0f;
		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, 45.f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(50.0f, 1.0f, 50.0f);
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 1.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, 23.f, -25.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(50.0f, 1.0f, 50.0f);
	//	pComponentManager->mTransforms[newComponent].localRotation = glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 1.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(0.f, 23.f, 25.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(50.0f, 1.0f, 50.0f);
	//	pComponentManager->mTransforms[newComponent].localRotation = glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 1.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(-25.f, 23.f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(50.0f, 1.0f, 50.0f);
	//	pComponentManager->mTransforms[newComponent].localRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, -90.0f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 1.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	pComponentManager->mTransforms[newComponent].localPosition = glm::vec3(25.f, 23.f, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
	//	pComponentManager->mTransforms[newComponent].localScale = glm::vec3(50.0f, 1.0f, 50.0f);
	//	pComponentManager->mTransforms[newComponent].localRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, -90.0f)));
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//	ComponentManager::GetInstance()->mBodies[newComponent].mass = 100.0f;
	//	ComponentManager::GetInstance()->mBodies[newComponent].Kinematic = true;
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//	ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
	//	ComponentManager::GetInstance()->mColliders[newComponent].length = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].width = 50.0f;
	//	ComponentManager::GetInstance()->mColliders[newComponent].height = 1.0f;
	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}
}
