#include "pch.h"
#include "Collider.h"
#include "transform.h"
#include "body.h"
#include "componentManager.h"
#include "vulkanAPI.h"
Collider::Collider():type(NONE), center(glm::vec3(0.0f)),radius(0.0f), length(0.0f), width(0.0f), height(0.f),
					colliderMatrix(nullptr),mTrans(nullptr), mBody(nullptr)
{
}

Collider::~Collider()
{
}

void Collider::Start()
{
	mTrans = &ComponentManager::GetInstance()->mTransforms[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM]];
	mBody = &ComponentManager::GetInstance()->mBodies[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[BODY]];

	colliderMatrix = VulkanAPI::GetInstance()->uboCollider[type].GetAvailableModel(this);
}

void Collider::Update(float deltaTime)
{
	center = mTrans->worldPosition;
	glm::mat4 scale = glm::mat4(1.0f);
	if (type == SPHERE)
	{
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(radius*2.0f, radius*2.0f, radius*2.0f));
	}
	else if (type == BOX)
	{
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(width, height, length));
	}
	*colliderMatrix = glm::translate(glm::mat4(1.0f), center)*
		glm::toMat4(mTrans->worldRotation)*
		scale;
}

void Collider::Serialize(XMLElement * _element)
{
}

void Collider::Initialize()
{
	Component::Initialize();
	center = glm::vec3(0.0f);
	radius = length = width = height = 0.f;
	VulkanAPI::GetInstance()->uboCollider[type].DeleteModel(colliderMatrix);
	colliderMatrix = nullptr;
	mTrans = nullptr;
	mBody = nullptr;
	type = NONE;
}
