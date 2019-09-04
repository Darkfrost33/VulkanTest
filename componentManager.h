#pragma once
#include "transform.h"
#include "body.h"
#include "entity.h"
#include "Singleton.h"
#include "controller.h"
#include "camera.h"
#include "Mesh.h"
#include "Collider.h"

#define LONG_SIZE 2048
#define MED_SIZE 512
#define SHORT_SIZE 32
class CollisionManager;
const std::array<uint32_t, COMPONENT_NUM> componentSize = {
	LONG_SIZE,
	LONG_SIZE,
	LONG_SIZE,
	SHORT_SIZE,
	SHORT_SIZE,
	LONG_SIZE,
	LONG_SIZE
};

class ComponentManager:public Singleton<ComponentManager>
{
public:
	~ComponentManager();

	std::array<Entity, LONG_SIZE> mEntities;
	std::array<Transform, LONG_SIZE> mTransforms;
	std::array<Body, LONG_SIZE> mBodies;
	std::array<Controller, SHORT_SIZE> mControllers;
	std::array<Camera, SHORT_SIZE> mCameras;
	std::array<Mesh, LONG_SIZE> mMeshes;
	std::array<Collider, LONG_SIZE> mColliders;

	std::array<std::set<uint32_t>, COMPONENT_NUM> activeComponents;

	uint32_t newComponent(uint32_t type);
	uint32_t newComponent(uint32_t entityID, uint32_t type);
	void deleteComponent(uint32_t type, uint32_t id);
	void StartComponents(uint32_t entityID);
	void Update(float deltaTime);
	void Delete();
	void registerNewComponents();

	CollisionManager* pCoManager;
private:
	friend class Singleton< ComponentManager>;
	ComponentManager();

	std::array<uint32_t, COMPONENT_NUM> lastIndex;
	std::array<std::queue<uint32_t>, COMPONENT_NUM> deletedIndex;

	std::array<std::vector<uint32_t>, COMPONENT_NUM> newCreatedComponents;
	std::array<std::vector<uint32_t>, COMPONENT_NUM> newDeletedComponents;
	std::vector<std::vector<Component*>> componentsAddress;
	bool addingGameObject;
	bool deletingGameObject;
};