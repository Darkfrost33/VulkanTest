#pragma once
#include "component.h"

struct MeshData;
class Transform :public Component
{
public:
	Transform();
	~Transform(){}
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;

	uint32_t parent;
	std::set<unsigned> childrenID;
	bool dirty;
	glm::vec3 worldPosition;
	glm::vec3 localPosition;
	glm::quat worldRotation;
	glm::quat localRotation;
	glm::vec3 worldScale;
	glm::vec3 localScale;
	glm::mat4* transMatrix;

	MeshData* mMesh;
};