#pragma once
#include "component.h"
class Transform;
class Body;
class Controller :public Component
{
public:
	Controller();
	~Controller();
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;

	Transform* mTrans;
	Body* mBody;
};