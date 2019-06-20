#include "pch.h"
#include "componentManager.h"

static const float GRAVITY = 10.0f;

Body::Body():force(0.0f), acc(0.0f), vel(0.0f), mass(0.0f),invMass(0.0f), mTrans(-1)
{

}
Body::~Body() {

}

void Body::Start()
{
	mTrans = ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM];
}

void Body::Update(float deltaTime)
{
	force.y -= mass * GRAVITY;
	acc = force * invMass;
	vel += acc * deltaTime;

}
void Body::Serialize(XMLElement* _element) {

}

void Body::Initialize()
{
	Component::Initialize();
	force = acc = vel = glm::vec3(0.0f);
	mass = invMass = 0.0f;
	mTrans = -1;
}
