#pragma once
#include "component.h"
#include "transform.h"
#include "physicsManager.h"
class Manifold;
class Body :public Component
{
public:
	Body();
	~Body();
	void Start() override;
	void Update(float deltaTime) override;
	void Serialize(XMLElement* _element) override;
	void Initialize() override;
	glm::vec3 force, torque, vel, angularVel, invInertia;
	glm::mat3 invInertiaTensor;
	float mass, invMass, gravityForce;
	float linearDamping, angularDamping;
	float friction, restitution;
	bool Kinematic;
	Transform* mTrans;

	uint32_t mBodyID;
	std::map<uint32_t, Manifold*> m_collisionList;

	inline glm::vec3 getVelocity(const glm::vec3& rePos) const {
		return vel + glm::cross(angularVel, rePos);
	}

	void applyImpulse(const glm::vec3& impulse, const glm::vec3& rel_pos);

	void updateInertiaTensor();

	inline void setInvInertia(glm::vec3 inertia)
	{
		invInertia = glm::vec3(inertia.x != 0.0f ? 1.0f / inertia.x : 0.0f,
								inertia.y != 0.0f ? 1.0f / inertia.y : 0.0f,
								inertia.z != 0.0f ? 1.0f / inertia.z : 0.0f);
		updateInertiaTensor();
	}
	inline const glm::vec3 LocalToGlobal(const glm::vec3& p) const
	{
		return mTrans->rotationMatrix*p + mTrans->worldPosition;
	}

	inline const glm::vec3 GlobalToLocal(const glm::vec3& p) const
	{
		return mTrans->invRotationMatrix*(p - mTrans->worldPosition);
	}
};