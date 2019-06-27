#pragma once
#include "component.h"

enum COLLIDER_TYPE
{
	NONE = -1,
	SPHERE,
	BOX,

	COLLIDER_TYPE_NUM
};
class Transform;
class Body;
class Collider :public Component
{
public:
	Collider();
	~Collider();
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;
	COLLIDER_TYPE type;
	glm::vec3 center;
	float radius, length, width, height;
	glm::mat4* colliderMatrix;

	Transform* mTrans;
	Body* mBody;
};
