#include "pch.h"
#include "componentManager.h"
#include "Input Manager.h"

const float controllerSpeed = 50.0f;
const float viewSpeedX = 50.0f;
const float viewSpeedY = 20.0f;
int index = 5;
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
	mBody = &ComponentManager::GetInstance()->mBodies[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[BODY]];
}

void Controller::Update(float deltaTime)
{
	if (Input_Manager::GetInstance()->IsPressed(VK_W))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)) * controllerSpeed*deltaTime);
		//mTrans->localPosition.z -= controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_S))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)) * controllerSpeed*deltaTime);
		//mTrans->localPosition.z += controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_A))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)) * controllerSpeed*deltaTime);
		//mTrans->localPosition.x -= controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_D))
	{
		mTrans->localPosition += glm::vec3(((*mTrans->transMatrix) * glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f)) * controllerSpeed*deltaTime);
		//mTrans->localPosition.x += controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_E))
	{
		mTrans->localPosition.y += controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_C))
	{
		mTrans->localPosition.y += -controllerSpeed * deltaTime;
	}
	if (Input_Manager::GetInstance()->IsTriggered(VK_Q))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(-deltaTime*30.0f, 0.0f, 0.0f)));

	}
	if (Input_Manager::GetInstance()->IsTriggered(VK_R))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(deltaTime*30.0f, 0.0f, 0.0f)));
		//glm::vec3 impulse = glm::vec3(0.0f,0.0f,1.0f);
		//glm::vec3 rel_pos = glm::vec3(0.5f,-0.0f,0.5f);
		//mBody->applyImpulse(impulse,rel_pos);
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_Z))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, -deltaTime * 30.0f, 0.0f)));
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_X))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, deltaTime * 30.0f, 0.0f)));
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_V))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, -deltaTime * 30.0f)));
	}
	if (Input_Manager::GetInstance()->IsPressed(VK_F))
	{
		mTrans->localRotation *= glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, deltaTime * 30.0f)));
	}
	glm::vec2 mouseMove = Input_Manager::GetInstance()->GetMouseMovedRight();
	//std::cout << mouseMove.x << ", " << mouseMove.y << std::endl;
	mTrans->localRotation *= 
		glm::quat(glm::radians(glm::vec3(mouseMove.y*viewSpeedY*deltaTime, mouseMove.x*viewSpeedX*deltaTime, 0.0f)));
	mTrans->dirty = true;

	if (Input_Manager::GetInstance()->IsTriggered(VK_SPACE))
	{
		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
		glm::vec3 offset = mTrans->rotationMatrix*glm::vec3(0.0f, 0.0f, -5.0f);
		ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = mTrans->worldPosition + offset;
		ComponentManager::GetInstance()->mTransforms[newComponent].localScale = glm::vec3(5.0f,5.0f,5.0f);
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("sphere");
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
		//ComponentManager::GetInstance()->mBodies[newComponent].restitution = 1.0f;
		ComponentManager::GetInstance()->mBodies[newComponent].mass = 50.0f;
		ComponentManager::GetInstance()->mBodies[newComponent].vel += offset * 50.0f;
		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
		ComponentManager::GetInstance()->mColliders[newComponent].type = SPHERE;
		ComponentManager::GetInstance()->mColliders[newComponent].radius = 5.0f;
		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	}
	if (Input_Manager::GetInstance()->IsTriggered(VK_P))
	{
			uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
			uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
			ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(0.f, 5.1f*index, 0.f); // glm::vec3(0.0f, 2.f, 0.f);
			ComponentManager::GetInstance()->mTransforms[newComponent].localScale = glm::vec3(10.f, 5.0f, 10.f); // glm::vec3(0.0f, 2.f, 0.f);
			ComponentManager::GetInstance()->mTransforms[newComponent].localRotation *= glm::quat(glm::radians(glm::vec3(60.0f, 0.0f, 0.f)));
			ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/texture.jpg");
			newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
			ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("box");
			newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
			ComponentManager::GetInstance()->mBodies[newComponent].mass = 500.0f-30*index;
			ComponentManager::GetInstance()->mBodies[newComponent].vel = glm::vec3(0.0f, -100.0f, 0.0f);
			newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
			ComponentManager::GetInstance()->mColliders[newComponent].type = BOX;
			ComponentManager::GetInstance()->mColliders[newComponent].length = 10.0f;
			ComponentManager::GetInstance()->mColliders[newComponent].width = 10.0f;
			ComponentManager::GetInstance()->mColliders[newComponent].height = 5.0f;
			ComponentManager::GetInstance()->StartComponents(newGameObjectID);
			index++;
	}
	//if (Input_Manager::GetInstance()->IsTriggered(VK_O))
	//{
	//	for (int i = 0; i < 10; ++i)
	//	{
	//		uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//		uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//		ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/2k_earth.jpg");
	//		ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(0.5f*i, 2.0f*i, 0.0f);
	//		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//		ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("sphere");
	//		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, BODY);
	//		//ComponentManager::GetInstance()->mBodies[newComponent].restitution = 1.0f;
	//		ComponentManager::GetInstance()->mBodies[newComponent].mass = 5.0f;
	//		newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, COLLIDER);
	//		ComponentManager::GetInstance()->mColliders[newComponent].type = SPHERE;
	//		ComponentManager::GetInstance()->mColliders[newComponent].radius = 1.0f;
	//		ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//	}
	//}

	//if (Input_Manager::GetInstance()->IsTriggered(VK_I))
	//{
	//	uint32_t newGameObjectID = ComponentManager::GetInstance()->newComponent(ENTITY);
	//	uint32_t newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, TRANSFORM);
	//	ComponentManager::GetInstance()->mTransforms[newComponent].localPosition = glm::vec3(
	//		ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * cosf(0.0f),
	//		ComponentManager::GetInstance()->activeComponents[TRANSFORM].size() * 5 * sinf(0.0f),
	//		0);
	//	ComponentManager::GetInstance()->mTransforms[newComponent].localScale = glm::vec3(4.0f, 4.0f, 4.0f);
	//	ComponentManager::GetInstance()->mTransforms[newComponent].GetTextureID("Textures/chalet.jpg");
	//	newComponent = ComponentManager::GetInstance()->newComponent(newGameObjectID, MESH);
	//	ComponentManager::GetInstance()->mMeshes[newComponent].BindMesh("house");

	//	ComponentManager::GetInstance()->StartComponents(newGameObjectID);
	//}

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
	Component::Initialize();
	mTrans = nullptr;
}
