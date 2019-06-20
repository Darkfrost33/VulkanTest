#pragma once
#include "component.h"

class Camera :public Component
{
public:
	Camera(){}
	~Camera(){}
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;
}; 
