#pragma once
#include "Collider.h"
#define nullNode (-1)
struct treeNode
{
	Collider* collider;
	bool IsLeaf() const
	{
		return child1 == nullNode;
	}

	/// Enlarged AABB
	AABB aabb;

	union
	{
		int32_t parent;
		int32_t next;
	};

	int32_t child1;
	int32_t child2;

	// leaf = 0, free node = -1
	int32_t height;
};

class DynamicTree
{
public:
	DynamicTree();
	~DynamicTree();
	int32_t CreateProxy(const AABB& aabb, Collider* collider);
	void DestroyProxy(int32_t proxyId);
	bool MoveProxy(int32_t proxyId, const AABB& aabb, const glm::vec3& displacement);
	Collider* GetCollider(int32_t proxyId) const;
	const AABB& GetFatAABB(int32_t proxyId) const;
	template <typename T>
	void Query(T* callback, const AABB& aabb) const;
	template <typename T>
	void Raycast(T* callback, const RayCastInput& input) const;
private:
	int32_t AllocateNode();
	void FreeNode(int32_t nodeId);
	void InsertLeaf(int32_t leaf);
	void RemoveLeaf(int32_t leaf);
	int32_t Balance(int32_t index);
	int32_t ComputeHeight() const;
	int32_t ComputeHeight(int32_t index) const;
	int32_t m_root;
	treeNode* m_nodes;
	int32_t m_nodeCount;
	int32_t m_nodeCapacity;
	int32_t m_freeList;
};

inline Collider* DynamicTree::GetCollider(int32_t proxyId) const
{
	return m_nodes[proxyId].collider;
}

inline const AABB& DynamicTree::GetFatAABB(int32_t proxyId) const
{
	return m_nodes[proxyId].aabb;
}

template <typename T>
inline void DynamicTree::Query(T* callback, const AABB& aabb) const
{
	std::stack<int32_t> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		int32_t nodeId = stack.top();
		stack.pop();
		if (nodeId == nullNode)
		{
			continue;
		}

		const treeNode* node = m_nodes + nodeId;

		if (TestAABBOverlap(node->aabb, aabb))
		{
			if (node->IsLeaf())
			{
				callback->QueryCallback(nodeId);
			}
			else
			{
				stack.push(node->child1);
				stack.push(node->child2);
			}
		}
	}
}

template<typename T>
inline void DynamicTree::Raycast(T * callback, const RayCastInput & input) const
{
	glm::vec3 p1 = input.p1;
	glm::vec3 p2 = input.p2;
	glm::vec3 r = p2 - p1;
	assert(glm::length2(r) > 0.0f);
	r.Normalize();

	// v is perpendicular to the segment.
	glm::vec3 v = b2Cross(1.0f, r);
	glm::vec3 abs_v = b2Abs(v);

	// Separating axis for segment (Gino, p80).
	// |dot(v, p1 - c)| > dot(|v|, h)

	float maxFraction = input.maxFraction;

	// Build a bounding box for the segment.
	AABB segmentAABB;
	{
		glm::vec3 t = p1 + maxFraction * (p2 - p1);
		segmentAABB.lowerBound = glm::vec3(std::min(p1.x,t.x), std::min(p1.y, t.y), std::min(p1.z, t.z));
		segmentAABB.upperBound = glm::vec3(std::max(p1.x, t.x), std::max(p1.y, t.y), std::max(p1.z, t.z));
	}

	std::stack<int32_t> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		int32_t nodeId = stack.top();
		stack.pop();
		if (nodeId == nullNode)
		{
			continue;
		}

		const treeNode* node = m_nodes + nodeId;

		if (TestAABBOverlap(node->aabb, segmentAABB) == false)
		{
			continue;
		}
		
		// test ray and boundingbox
		//...

		if (node->IsLeaf())
		{
			RayCastInput subInput;
			subInput.p1 = input.p1;
			subInput.p2 = input.p2;
			subInput.maxFraction = maxFraction;

			float value = callback->RayCastCallback(subInput, nodeId);

			if (value == 0.0f)
			{
				// The client has terminated the ray cast.
				return;
			}

			if (value > 0.0f)
			{
				// Update segment bounding box.
				maxFraction = value;
				glm::vec3 t = p1 + maxFraction * (p2 - p1);
				segmentAABB.lowerBound = glm::vec3(std::min(p1.x, t.x), std::min(p1.y, t.y), std::min(p1.z, t.z));
				segmentAABB.upperBound = glm::vec3(std::max(p1.x, t.x), std::max(p1.y, t.y), std::max(p1.z, t.z));
			}
		}
		else
		{
			stack.Push(node->child1);
			stack.Push(node->child2);
		}
	}
}
