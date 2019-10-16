#include "pch.h"
#include "componentManager.h"
#include "transform.h"
#define GRAVITY_ACC 10.0f
#define SIMD_EPSILON  2.2204460492503131e-016
#define LINEAR_VEL_THRESHOLD 2.0f
#define LINEAR_VEL_THRESHOLDSQR (LINEAR_VEL_THRESHOLD*LINEAR_VEL_THRESHOLD)
Body::Body() :force(0.0f), torque(0.0f), vel(0.0f), angularVel(0.0f), invInertia(0.0f), mass(0.0f), invMass(0.0f), gravityForce(0.0f),
linearDamping(0.5f), angularDamping(0.5f),
friction(0.00f),restitution(0.0f),
Kinematic(false), mTrans(nullptr)
{
}
Body::~Body() {

}

glm::quat VmultQ(glm::vec3 w, glm::quat q)
{
	return glm::quat(
		+w.x * q.w + w.y * q.z - w.z * q.y,
		+w.y * q.w + w.z * q.x - w.x * q.z,
		+w.z * q.w + w.x * q.y - w.y * q.x,
		-w.x * q.x - w.y * q.y - w.z * q.z);
}

void Body::Start()
{
	mTrans = &ComponentManager::GetInstance()->mTransforms[ComponentManager::GetInstance()->mEntities[mOwner].mComponents[TRANSFORM]];
	if (mass != 0.0f)
	{
		invMass = 1.0f / mass;
	}
	gravityForce = mass * GRAVITY_ACC;
}

void Body::Update(float deltaTime)
{
	if (Kinematic) {
		return;
	}
	force.y -= gravityForce;
	updateInertiaTensor();
	vel += force * invMass * deltaTime;
	angularVel += invInertiaTensor * torque*deltaTime;
	force = torque = glm::vec3(0.0f);
	if (glm::length2(vel) > SIMD_EPSILON) {
		mTrans->dirty = true;
		glm::vec3 translate = deltaTime * vel;
		if (glm::dot(translate, translate) > LINEAR_VEL_THRESHOLDSQR)
		{
			float ratio = LINEAR_VEL_THRESHOLD / glm::length(translate);
			vel *= ratio;
		}
		mTrans->localPosition += vel * deltaTime;
	}
	float fAngle2 = glm::length2(angularVel);
	if(fAngle2 > SIMD_EPSILON)
	{
		mTrans->dirty = true;
		//Exponential map
		glm::vec3 axis;
		float fAngle = sqrtf(fAngle2);
		//limit the angular motion
		static const float ANGULAR_MOTION_THRESHOLD = 0.25f * glm::pi<float>();
		if (fAngle * deltaTime > ANGULAR_MOTION_THRESHOLD)
		{
			fAngle = ANGULAR_MOTION_THRESHOLD / deltaTime;
		}

		if (fAngle < 0.001f)
		{
			// use Taylor's expansions of sync function
			axis = angularVel *( 0.5f * deltaTime - (deltaTime * deltaTime * deltaTime) * 0.020833333333f * fAngle * fAngle);
		}
		else
		{
			// sync(fAngle) = sin(c*fAngle)/t
			axis = angularVel * (sinf(0.5f * fAngle * deltaTime) / fAngle);
		}
		glm::quat dorn(axis);
		glm::quat orn0 = mTrans->localRotation;

		glm::quat predictedOrn = dorn * orn0;
		mTrans->localRotation = predictedOrn;
		assert(glm::length2(predictedOrn) > SIMD_EPSILON);

		//if (glm::length2(predictedOrn) > SIMD_EPSILON) {
		//	mTrans->localRotation = glm::normalize(predictedOrn);
		//}

	}
	vel *= 1.0f / (1.0f + deltaTime * linearDamping);
	angularVel *= 1.0f / (1.0f + deltaTime * angularDamping);
}
void Body::Serialize(XMLElement* _element) {

}

void Body::Initialize()
{
	Component::Initialize();
	force = torque = vel = angularVel = invInertia = glm::vec3(0.0f);
	mass = invMass = gravityForce = restitution = 0.0f;
	linearDamping = 0.1f;
	angularDamping = 0.5f;
	friction = 0.005f;
	Kinematic = false;
	mTrans = nullptr;
}

void Body::applyImpulse(const glm::vec3 & impulse, const glm::vec3 & rel_pos)
{
	if (Kinematic) return;
	vel += impulse * invMass;
	angularVel += invInertiaTensor * glm::cross(rel_pos, impulse);
}

void Body::updateInertiaTensor()
{
	glm::mat3& transform = mTrans->rotationMatrix;
	glm::mat3 IneriaTensor = glm::mat3(invInertia.x, 0.0f,0.0f,
										0.0f,invInertia.y, 0.0f,
										0.0f,0.0f,invInertia.z);
	//glm::mat3 scaledTr = glm::mat3(
	//	transform[0][0] * invInertia.x, transform[1][0] * invInertia.y, transform[2][0] * invInertia.z,
	//	transform[0][1] * invInertia.x, transform[1][1] * invInertia.y, transform[2][1] * invInertia.z,
	//	transform[0][2] * invInertia.x, transform[1][2] * invInertia.y, transform[2][2] * invInertia.z);
	invInertiaTensor = transform*IneriaTensor * glm::transpose(transform);
}
