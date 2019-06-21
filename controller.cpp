#include "pch.h"
#include "componentManager.h"
#include "Input Manager.h"

const float controllerSpeed = 5.0f;
const float viewSpeedX = 50.0f;
const float viewSpeedY = 20.0f;

Controller::Controller()
{
	mTrans = nullptr;
	mBody = nullptr;
}

Controller::~Controller()
{
}

void Controller::Start()
{
	mTrans = &ComponentManager::GetInstance()->mTransforms[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM]];
}

void Controller::Update(float deltaTime)
{
	if (Input_Manager::GetInstance()->IsPressed(VK_W))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)) * controllerSpeed* deltaTime);
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_S))
	{
		mTrans->localPosition -= glm::vec3(((*mTrans->transMatrix) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)) * controllerSpeed* deltaTime);
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_A))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)) * controllerSpeed* deltaTime);
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_D))
	{
		mTrans->localPosition -= glm::vec3(((*mTrans->transMatrix) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)) * controllerSpeed* deltaTime);
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_E))
	{
		mTrans->localPosition.y += controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_C))
	{
		mTrans->localPosition.y -= controllerSpeed * deltaTime;
	}
	glm::vec2 mouseMove = Input_Manager::GetInstance()->GetMouseMovedRight();
	//std::cout << mouseMove.x << ", " << mouseMove.y << std::endl;
	mTrans->localRotation = mTrans->localRotation 
		* glm::quat(glm::radians(glm::vec3(mouseMove.y*viewSpeedY*deltaTime, mouseMove.x*viewSpeedX*deltaTime, 0.0f)));
	mTrans->dirty = true;

	if (Input_Manager::GetInstance()->IsTriggered(VK_P))
	{
		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
		ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size()*5 * cosf(0.0f),
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size()*5 * sinf(0.0f),
			0);

		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");

		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	}
	if (Input_Manager::GetInstance()->IsTriggered(VK_O))
	{
		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
		ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * cosf(0.0f),
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * sinf(0.0f),
			0);
		ComponentManager::GetInstance()->mTransforms[newComponent].localScale = glm::vec3(2.0f, 2.0f, 2.0f);
		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("sphere");

		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	}

	if (Input_Manager::GetInstance()->IsTriggered(VK_I))
	{
		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
		ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * cosf(0.0f),
			ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * sinf(0.0f),
			0);
		ComponentManager::GetInstance()->mTransforms[newComponent].localScale = glm::vec3(4.0f, 4.0f, 4.0f);
		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/chalet.jpg");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("house");

		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	}

	if (Input_Manager::GetInstance()->IsTriggered(VK_L))
	{
		//uint32_t index = std::max(std::rand()%ComponentManager::GetInstance()->activeComponents[TRANSFORM].size(), (uint64_t)1);
		if (ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() <= 1)
			return;
		auto ii = ComponentManager::GetInstance()->activeComponents[TRANSFORM].begin();
		ii++;
		uint32_t index = *ii;
		ComponentManager::GetInstance()->deleteComponent(ENTITY, index);
	}
}

void Controller::Serialize(XMLElement * _element)
{
}

void Controller::Initialize()
{
	mTrans = nullptr;
}
