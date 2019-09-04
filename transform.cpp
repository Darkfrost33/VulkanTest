#include "pch.h"
#include "transform.h"
#include "vulkanAPI.h"
#include "componentManager.h"
#include "ResourceManager.h"

Transform::Transform():parent(-1), worldPosition(0.0f), localPosition(0.0f), worldScale(1.0f), localScale(1.0f)
{
	worldRotation = localRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	rotationMatrix = invRotationMatrix = glm::mat3(1.0f);
	dirty = true;
	mMesh = nullptr;
	transMatrix = nullptr;
	textureID = 0;
}

Transform::~Transform()
{
	if (nullptr == mMesh) 
	{
		delete transMatrix;
		transMatrix = nullptr;
	}
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
	Update(0.0f);
}

void Transform::Update(float deltaTime)
{
	if (dirty) {
		worldPosition = localPosition;
		//std::cout << worldPosition.x <<", "<< worldPosition.y <<", "<< worldPosition.z << std::endl;
		worldRotation = localRotation;
		worldScale = localScale;
		rotationMatrix = glm::toMat3(worldRotation);
		invRotationMatrix = glm::inverse(rotationMatrix);
		*transMatrix = glm::translate(glm::mat4(1.0f), worldPosition)*
			glm::mat4(rotationMatrix)*
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
	rotationMatrix = invRotationMatrix = glm::mat3(1.0f);
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
	textureID = 0;
	dirty = true;
}

void Transform::GetTextureID(std::string path)
{
	textureID = ResourceManager::GetInstance()->LoadTexture(path);
}
