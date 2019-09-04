#pragma once
#include "component.h"

enum COLLIDER_TYPE
{
	NONE = -1,
	SPHERE,
	BOX,

	COLLIDER_TYPE_NUM
};

class Slab
{
public:
	Slab(glm::vec3 _N, float _d0, float _d1) :N(_N), d0(_d0), d1(_d1) {}
	glm::vec3 N;
	float d0, d1;
};

struct AABB
{
	glm::vec3 GetCenter() const
	{
		return 0.5f*(lowerBound + upperBound);
	}
	glm::vec3 GetExtends() const
	{
		return 0.5f*(upperBound - lowerBound);
	}
	float GetArea() const
	{
		glm::vec3 d = upperBound - lowerBound;
		return 2.0f*(d.x*d.y + d.y*d.z + d.z*d.x);
	}
	void Combine(const AABB& aabb)
	{
		lowerBound = glm::vec3(std::min(lowerBound.x, aabb.lowerBound.x), std::min(lowerBound.y, aabb.lowerBound.y), std::min(lowerBound.z, aabb.lowerBound.z));
		upperBound = glm::vec3(std::max(upperBound.x, aabb.upperBound.x), std::max(upperBound.y, aabb.upperBound.y), std::max(upperBound.z, aabb.upperBound.z));
	}
	void Combine(const AABB& aabb1, const AABB& aabb2)
	{
		lowerBound = glm::vec3(std::min(aabb1.lowerBound.x, aabb2.lowerBound.x), std::min(aabb1.lowerBound.y, aabb2.lowerBound.y), std::min(aabb1.lowerBound.z, aabb2.lowerBound.z));
		upperBound = glm::vec3(std::max(aabb1.upperBound.x, aabb2.upperBound.x), std::max(aabb1.upperBound.y, aabb2.upperBound.y), std::max(aabb1.upperBound.z, aabb2.upperBound.z));
	}
	/// Does this aabb contain the provided AABB.
	bool Contains(const AABB& aabb) const
	{
		bool result = true;
		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		return result;
	}
	bool RayCast() const { return false; };
	glm::vec3 lowerBound;
	glm::vec3 upperBound;
};
inline bool TestAABBOverlap(const AABB & a, const AABB & b)
{
	if (a.upperBound.x < b.lowerBound.x ||
		a.upperBound.y < b.lowerBound.y ||
		a.upperBound.z < b.lowerBound.z)
	{
		return false;
	}
	if (a.lowerBound.x > b.upperBound.x ||
		a.lowerBound.y > b.upperBound.y ||
		a.lowerBound.z > b.upperBound.z)
	{
		return false;
	}
	return true;
}
/// Ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
struct RayCastInput
{
	glm::vec3 p1, p2;
	float maxFraction;
};

/// Ray-cast output data. The ray hits at p1 + fraction * (p2 - p1), where p1 and p2
/// come from b2RayCastInput.
struct RayCastOutput
{
	glm::vec3 normal;
	float fraction;
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
	void UpdateBoundingBox();
	COLLIDER_TYPE type;
	glm::vec3 offset;
	float radius, length, width, height;
	glm::mat4 scaleMatrix;
	glm::mat4* colliderMatrix;
	glm::vec3 curCenter;
	glm::vec3 preCenter;

	Transform* mTrans;
	Body* mBody;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> realVertices;
	//bool computeVerteices;
	AABB boundingBox;
	AABB preBoudingBox;
	uint32_t m_proxyId;
};
