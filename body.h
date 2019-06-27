#pragma once
#include "component.h"
class Transform;
class Body :public Component
{
public:
	Body();
	~Body();
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;
	glm::vec3 force, acc, vel;
	float mass, invMass;

	Transform* mTrans;
};