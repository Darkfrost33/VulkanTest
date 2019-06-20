#pragma once
#include "stdint.h"
class XMLElement;
class Component
{
public:
	Component():mOwner(-1){}
	virtual ~Component(){}
	virtual void Start(){}
	virtual void Update(float deltaTime) = 0;
	virtual void Serialize(XMLElement* _element) {}
	virtual void Initialize() { mOwner = -1; };
	uint32_t mOwner;
};