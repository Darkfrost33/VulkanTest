#pragma once

enum E_COMPONENT_TYPE
{
	ENTITY = 0,
	TRANSFORM,
	BODY,
	CONTROLLER,
	CAMERA,
	MESH,

	COMPONENT_NUM
};

enum E_COMPONENT_MASK : uint64_t
{
	ENTITY_NO_COMPONENT_MASK = 0,
	TRANSFORM_MASK = 1 << TRANSFORM,
	BODY_MASK = 1 << BODY,
	CONTROLLER_MASK = 1 << CONTROLLER,
	CAMERA_MASK = 1 << CAMERA,
	MESH_MASK = 1 <<MESH
};

class Entity
{
public:
	Entity() :active(false), name(""), entityMask(0){
		for (auto &cp : mComponents)
		{
			cp = -1;
		}
	}
	~Entity(){}

	void Initialize()
	{
		active = false;
		name = "";
		entityMask = 0;
		for (auto &cp : mComponents)
		{
			cp = -1;
		}
	}

	bool HasComponent(E_COMPONENT_MASK mask)
	{
		return (entityMask & std::bitset<64>(mask)).any();
	}

	uint32_t GetComponent(E_COMPONENT_TYPE type)
	{
		return mComponents[type];
	}
	bool active;
	std::string name;
	std::bitset<64> entityMask;
	std::array<uint32_t, COMPONENT_NUM> mComponents;
};