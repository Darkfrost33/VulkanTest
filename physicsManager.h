#pragma once
#include "Collider.h"
#include "body.h"
#include "DynamicTree.h"
#define MAX_CONTACT_NUM 4
class ContactPoint
{
public:
	glm::vec3 globalPositionA;
	glm::vec3 globalPositionB;
	glm::vec3 localPositionA;
	glm::vec3 localPositionB;
	glm::vec3 contactNormal;
	glm::vec3 contactTangent1, contactTangent2;
	float penatationDepth;

	float normalImpulseSum;
	float tangentImpulseSum1;
	float tangentImpulseSum2;

	uint32_t m_lifeTime;// increase when exist

	ContactPoint():normalImpulseSum(0.0f), tangentImpulseSum1(0.0f), tangentImpulseSum2(0.0f), m_lifeTime(0)
	{}
};

class Manifold
{
public:
	Body* bodyA;
	Body* bodyB;
	bool alreadyCheck;
	ContactPoint m_contactList[MAX_CONTACT_NUM];
	ContactPoint* m_contactListPtr[MAX_CONTACT_NUM];
	int m_contactsNum;
	int32_t m_lifeTime;// decrease
	Manifold():alreadyCheck(true),m_contactsNum(0), m_lifeTime(60){
		for (int i = 0; i < MAX_CONTACT_NUM; ++i)
		{
			m_contactListPtr[i] = &m_contactList[i];
		}
	}
	inline void refreshLifeTime()
	{
		m_lifeTime = 60;
		alreadyCheck = true;
	}
	void CheckContactPointValidate();
	void AddnewContacntPoint(ContactPoint* cp);
	void RemoveContactPoint(int index);
	void CheckNewContactPoint(ContactPoint* cp);
	int SortContactPoint(ContactPoint* cp);
};

class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	void Reset();
	void flushManifolds();
	void QueryCallback(uint32_t proxyId);
	void CheckBVH(const Collider& pCo);
	// generate to push into the contacts
	void CheckConllisionAndGenerateContact(const Collider& pCo1, const Collider& pCo2);
	void Resolve(float deltaTime);
	std::vector<Manifold*> removeList;
	std::list<Manifold*> mManifolds;
	DynamicTree m_BVH;
	uint32_t m_queryProxyId;
};