#include "pch.h"
#include "Collider.h"
#include "transform.h"
#include "body.h"
#include "componentManager.h"
#include "vulkanAPI.h"
Collider::Collider():type(NONE), offset(glm::vec3(0.0f)),radius(0.0f), length(0.0f), width(0.0f), height(0.f),
					colliderMatrix(nullptr),mTrans(nullptr), mBody(nullptr)
{
	scaleMatrix = glm::mat4(1.0f);
}

Collider::~Collider()
{
}

void Collider::Start()
{
	mTrans = &ComponentManager::GetInstance()->mTransforms[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM]];
	mBody = &ComponentManager::GetInstance()->mBodies[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[BODY]];

	colliderMatrix = VulkanAPI::GetInstance()->uboCollider[type].GetAvailableModel(this);
	if (type == BOX)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				for (int k = 0; k < 2; ++k)
				{
					vertices.push_back(glm::vec3(i-0.5f, j - 0.5f, k - 0.5f));
				}
			}
		}
		// compute inertia
		float mass = mBody->mass;
		glm::vec3 inertia(mass / 12.0f * (height * height + length * length),//width
									mass / 12.0f * (width * width + length * length),//height
									mass / 12.0f * (width * width + height * height));//length
		mBody->setInvInertia(inertia);

		scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(width, height, length));
	}
	else if (type == SPHERE)
	{
		float elem = 0.4f*mBody->mass*radius*radius;
		glm::vec3 inertia(elem, elem, elem);
		mBody->setInvInertia(inertia);

		scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(radius));
	}
	realVertices.resize(vertices.size());

	curCenter = mTrans->worldPosition + offset;
	*colliderMatrix = glm::translate(glm::mat4(1.0f), curCenter)*
		glm::toMat4(mTrans->worldRotation)*
		scaleMatrix;
	UpdateBoundingBox();
	m_proxyId = ComponentManager::GetInstance()->pCoManager->m_BVH.CreateProxy(boundingBox,this);
}

void Collider::Update(float deltaTime)
{
	preCenter = curCenter;
	preBoudingBox = boundingBox;
	curCenter = mTrans->worldPosition + offset;
	*colliderMatrix = glm::translate(glm::mat4(1.0f), curCenter)*
		glm::toMat4(mTrans->worldRotation)*
		scaleMatrix;
	UpdateBoundingBox();
	glm::vec3 displacement = curCenter - preCenter;
	AABB moveAABB;
	moveAABB.Combine(preBoudingBox, boundingBox);
	ComponentManager::GetInstance()->pCoManager->m_BVH.MoveProxy(m_proxyId, moveAABB, displacement);
}

void Collider::Serialize(XMLElement * _element)
{
}

void Collider::Initialize()
{
	Component::Initialize();
	offset = glm::vec3(0.0f);
	radius = length = width = height = 0.f;
	VulkanAPI::GetInstance()->uboCollider[type].DeleteModel(colliderMatrix);
	colliderMatrix = nullptr;
	scaleMatrix = glm::mat4(1.0f);
	mTrans = nullptr;
	mBody = nullptr;
	type = NONE;
	vertices.clear();
	realVertices.clear();
}

void Collider::UpdateBoundingBox()
{
	if (type == SPHERE)
	{
		boundingBox.lowerBound = curCenter - glm::vec3(radius);
		boundingBox.upperBound = boundingBox.lowerBound + 2.0f*glm::vec3(radius);
	}
	else if (type == BOX)
	{
		for (size_t i = 0; i < vertices.size(); ++i) {
			realVertices[i] = (*colliderMatrix) * glm::vec4(vertices[i], 1.0f);
		}
		glm::vec3 minVertex = realVertices[0];
		glm::vec3 maxVertex = realVertices[0];
		for (size_t i = 1; i < realVertices.size(); ++i)
		{
			if (realVertices[i].x < minVertex.x)
			{
				minVertex.x = realVertices[i].x;
			}
			if (realVertices[i].y < minVertex.y)
			{
				minVertex.y = realVertices[i].y;
			}
			if (realVertices[i].z < minVertex.z)
			{
				minVertex.z = realVertices[i].z;
			}
			// max
			if (realVertices[i].x > maxVertex.x)
			{
				maxVertex.x = realVertices[i].x;
			}
			if (realVertices[i].y > maxVertex.y)
			{
				maxVertex.y = realVertices[i].y;
			}
			if (realVertices[i].z > maxVertex.z)
			{
				maxVertex.z = realVertices[i].z;
			}
		}
		boundingBox.lowerBound = minVertex;
		boundingBox.upperBound = maxVertex;
	}
}
