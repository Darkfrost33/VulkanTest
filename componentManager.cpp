#include "pch.h"
#include "componentManager.h"
#include "vulkanAPI.h"

ComponentManager::ComponentManager()
{
	for (int i = 0; i < COMPONENT_NUM; ++i)
	{
		newCreatedComponents[i].reserve(MED_SIZE);
	}

	std::vector<Component*> temp;
	componentsAddress.push_back(temp);
	for (auto& t : mTransforms)
	{
		temp.push_back(&t);
	}
	componentsAddress.push_back(temp);
	temp.clear();

	for (auto& t : mBodies)
	{
		temp.push_back(&t);
	}
	componentsAddress.push_back(temp);
	temp.clear();

	for (auto& t : mControllers)
	{
		temp.push_back(&t);
	}
	componentsAddress.push_back(temp);
	temp.clear();

	for (auto& t : mCameras)
	{
		temp.push_back(&t);
	}
	componentsAddress.push_back(temp);
	temp.clear();

	for (auto& t : mMeshes)
	{
		temp.push_back(&t);
	}
	componentsAddress.push_back(temp);
	temp.clear();

	addingGameObject = deletingGameObject = false;
}

ComponentManager::~ComponentManager()
{

}

uint32_t ComponentManager::newComponent(uint32_t type)
{
	uint32_t componentID = -1;
	if (!deletedIndex[type].empty())
	{
		componentID = deletedIndex[type].front();
		deletedIndex[type].pop();
	}
	else
	{
		if (lastIndex[type] > componentSize[type])
		{
			std::cout << "component full, type = " << type << std::endl;
			return -1;
		}
		componentID = lastIndex[type]++;
	}
	if (componentID != -1)
	{
		newCreatedComponents[type].push_back(componentID);
		addingGameObject = true;
	}
	return componentID;
}

uint32_t ComponentManager::newComponent(uint32_t entityID, uint32_t type)
{
	if (type == ENTITY)
	{
		return -1;
	}
	if ((mEntities[entityID].entityMask &  (std::bitset<64>(1) << type)).any())
	{
		std::cout << "already has this component!" << std::endl;
		return -1;
	}
	uint32_t componentID = newComponent(type);
	if (componentID != -1)
	{
		newCreatedComponents[type].push_back(componentID);
		addingGameObject = true;
		mEntities[entityID].entityMask |= (std::bitset<64>(1) << type);
		mEntities[entityID].mComponents[type] = componentID;
		componentsAddress[type][componentID]->mOwner = entityID;
	}
	return componentID;
}

void ComponentManager::deleteComponent(uint32_t type, uint32_t id)
{
	if (id != -1) {
		deletingGameObject = true;
		if (type == ENTITY) {
			mEntities[id].active = false;
			newDeletedComponents[type].push_back(id);
			for (int i = 1; i < COMPONENT_NUM; ++i)
			{
				if ((mEntities[id].entityMask &  (std::bitset<64>(1) << i)).any()) {
					newDeletedComponents[i].push_back(mEntities[id].mComponents[i]);
				}
			}
		}
		else {
			newDeletedComponents[type].push_back(id);
		}
	}
}

void ComponentManager::StartComponents(uint32_t entityID)
{
	for (int i = 0; i < COMPONENT_NUM; ++i)
	{
		if ((mEntities[entityID].entityMask &  (std::bitset<64>(1) << i)).any()) {
			uint32_t cpID = mEntities[entityID].mComponents[i];
			componentsAddress[i][cpID]->Start();
		}
	}
}

void ComponentManager::Update(float deltaTime)
{
	// merge
	registerNewComponents();

	for (auto cp : activeComponents[CONTROLLER]) {
		mControllers[cp].Update(deltaTime);
	}
	// update physics
	for (auto cp : activeComponents[BODY]) {
		mBodies[cp].Update(deltaTime);
	}
	// update worldposition
	for (auto cp : activeComponents[TRANSFORM]) {
		mTransforms[cp].Update(deltaTime);
	}
	// update collision

	// resolve collision
}

void ComponentManager::Delete()
{
	if (deletingGameObject)
	{
		if (!newDeletedComponents[0].empty())
		{
			for (auto id : newDeletedComponents[0]) {
				activeComponents[0].erase(id);
				mEntities[id].Initialize();
				deletedIndex[0].push(id);
			}
			newDeletedComponents[0].clear();
		}
		for (int i = 1; i < COMPONENT_NUM; ++i)
		{
			if (!newDeletedComponents[i].empty()) {
				for (auto id : newDeletedComponents[i]) {
					activeComponents[i].erase(id);
					componentsAddress[i][id]->Initialize();
					deletedIndex[i].push(id);
				}
				newDeletedComponents[i].clear();
			}
		}
		VulkanAPI::GetInstance()->flushCommandBuffer();
		deletingGameObject = false;
	}
}

void ComponentManager::registerNewComponents()
{
	if (addingGameObject) {
		for (int i = 0; i < COMPONENT_NUM; ++i)
		{
			if (!newCreatedComponents[i].empty()) {
				for (auto id : newCreatedComponents[i]) {
					activeComponents[i].insert(id);
				}
				newCreatedComponents[i].clear();
			}
		}
		VulkanAPI::GetInstance()->flushCommandBuffer();
		addingGameObject = false;
	}
}
