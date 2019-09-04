#include "pch.h"
#include "camera.h"
#include "componentManager.h"
#include "VulkanAPI.h"
void Camera::Start()
{
	VulkanAPI::GetInstance()->cameraTransform = ComponentManager::GetInstance()->mTransforms[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM]].transMatrix;
	VulkanAPI::GetInstance()->cameraOffset = glm::vec3(0.0f,2.0f,0.0f);
	ComponentManager::GetInstance()->
		mTransforms[ComponentManager::GetInstance()->
		mEntities[mOwner].mComponents[TRANSFORM]].localPosition = glm::vec3(-2.0f,10.0f,100.0f);
	ComponentManager::GetInstance()->
		mTransforms[ComponentManager::GetInstance()->
		mEntities[mOwner].mComponents[TRANSFORM]].localScale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void Camera::Update(float deltaTime)
{
}

void Camera::Serialize(XMLElement * _element)
{
}

void Camera::Initialize()
{
	Component::Initialize();
	VulkanAPI::GetInstance()->cameraTransform = nullptr;
}
