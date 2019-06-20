#include "pch.h"
#include "transform.h"
#include "vulkanAPI.h"
#include "componentManager.h"

Transform::Transform():parent(-1), worldPosition(0.0f), localPosition(0.0f), worldScale(1.0f), localScale(1.0f)
{
	worldRotation = localRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	dirty = true;
	mMesh = nullptr;
	transMatrix = nullptr;
}

void Transform::Start()
{
	if (ComponentManager::GetInstance()->mEntities[mOwner].HasComponent(MESH_MASK)) {
		uint32_t meshIndex = ComponentManager::GetInstance()->mEntities[mOwner].GetComponent(MESH);
		mMesh = ComponentManager::GetInstance()->mMeshes[meshIndex].mMesh;
		transMatrix = VulkanAPI::GetInstance()->uboDynamic[mMesh].GetAvailableModel(this);
	}
	else
	{
		transMatrix = new glm::mat4(1.0f);
	}
}

void Transform::Update(float deltaTime)
{
	if (dirty) {
		worldPosition = localPosition;
		worldRotation = localRotation;
		worldScale = localScale;
		*transMatrix = glm::translate(glm::mat4(1.0f), worldPosition)*
			glm::toMat4(worldRotation)*
			glm::scale(glm::mat4(1.0f), worldScale);
		dirty = false;
	}
}

void Transform::Serialize(XMLElement* _element)
{

}

void Transform::Initialize()
{
	Component::Initialize();
	parent = -1;
	childrenID.clear();
	worldPosition = localPosition = glm::vec3(0.0f);
	worldRotation = localRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	worldScale = localScale = glm::vec3(1.0f);
	if (nullptr != mMesh) {
		VulkanAPI::GetInstance()->uboDynamic[mMesh].DeleteModel(transMatrix);
		transMatrix = nullptr;
		mMesh = nullptr;
	}
	else
	{
		delete transMatrix;
		transMatrix = nullptr;
	}
	dirty = true;
}
