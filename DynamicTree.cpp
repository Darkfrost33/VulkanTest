#include "pch.h"
#include "DynamicTree.h"
#define aabbExtension		0.1f
#define aabbMultiplier		2.0f
DynamicTree::DynamicTree()
{
	m_root = nullNode;

	m_nodeCapacity = 64;
	m_nodeCount = 0;
	m_nodes = (treeNode*)malloc(m_nodeCapacity * sizeof(treeNode));
	memset(m_nodes, 0, m_nodeCapacity * sizeof(treeNode));

	// Build a linked list for the free list.
	for (int32_t i = 0; i < m_nodeCapacity - 1; ++i)
	{
		m_nodes[i].next = i + 1;
		m_nodes[i].height = -1;
	}
	m_nodes[m_nodeCapacity - 1].next = nullNode;
	m_nodes[m_nodeCapacity - 1].height = -1;
	m_freeList = 0;
}

DynamicTree::~DynamicTree()
{
	free(m_nodes);
}

int32_t DynamicTree::CreateProxy(const AABB & aabb, Collider * collider)
{
	int32_t proxyId = AllocateNode();

	// Fatten the aabb.
	glm::vec3 r(aabbExtension, aabbExtension, aabbExtension);
	m_nodes[proxyId].aabb.lowerBound = aabb.lowerBound - r;
	m_nodes[proxyId].aabb.upperBound = aabb.upperBound + r;
	m_nodes[proxyId].collider = collider;
	m_nodes[proxyId].height = 0;

	InsertLeaf(proxyId);

	return proxyId;
}

void DynamicTree::DestroyProxy(int32_t proxyId)
{
	assert(0 <= proxyId && proxyId < m_nodeCapacity);
	assert(m_nodes[proxyId].IsLeaf());

	RemoveLeaf(proxyId);
	FreeNode(proxyId);
}

bool DynamicTree::MoveProxy(int32_t proxyId, const AABB & aabb, const glm::vec3 & displacement)
{
	assert(0 <= proxyId && proxyId < m_nodeCapacity);

	assert(m_nodes[proxyId].IsLeaf());

	if (m_nodes[proxyId].aabb.Contains(aabb))
	{
		return false;
	}

	RemoveLeaf(proxyId);

	// Extend AABB.
	AABB b = aabb;
	glm::vec3 r(aabbExtension, aabbExtension, aabbExtension);
	b.lowerBound = b.lowerBound - r;
	b.upperBound = b.upperBound + r;

	// Predict AABB displacement.
	glm::vec3 d = aabbMultiplier * displacement;

	if (d.x < 0.0f)
	{
		b.lowerBound.x += d.x;
	}
	else
	{
		b.upperBound.x += d.x;
	}

	if (d.y < 0.0f)
	{
		b.lowerBound.y += d.y;
	}
	else
	{
		b.upperBound.y += d.y;
	}

	m_nodes[proxyId].aabb = b;

	InsertLeaf(proxyId);
	return true;
}

int32_t DynamicTree::AllocateNode()
{
	// Expand the node pool as needed.
	if (m_freeList == nullNode)
	{
		assert(m_nodeCount == m_nodeCapacity);

		// The free list is empty. Rebuild a bigger pool.
		treeNode* oldNodes = m_nodes;
		m_nodeCapacity *= 2;
		m_nodes = (treeNode*)malloc(m_nodeCapacity * sizeof(treeNode));
		memcpy(m_nodes, oldNodes, m_nodeCount * sizeof(treeNode));
		free(oldNodes);

		// Build a linked list for the free list. The parent
		// pointer becomes the "next" pointer.
		for (int32_t i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
		{
			m_nodes[i].next = i + 1;
			m_nodes[i].height = -1;
		}
		m_nodes[m_nodeCapacity - 1].next = nullNode;
		m_nodes[m_nodeCapacity - 1].height = -1;
		m_freeList = m_nodeCount;
	}

	// Peel a node off the free list.
	int32_t nodeId = m_freeList;
	m_freeList = m_nodes[nodeId].next;
	m_nodes[nodeId].parent = nullNode;
	m_nodes[nodeId].child1 = nullNode;
	m_nodes[nodeId].child2 = nullNode;
	m_nodes[nodeId].height = 0;
	m_nodes[nodeId].collider = nullptr;
	++m_nodeCount;
	return nodeId;
}

void DynamicTree::FreeNode(int32_t nodeId)
{
	assert(0 <= nodeId && nodeId < m_nodeCapacity);
	assert(0 < m_nodeCount);
	m_nodes[nodeId].next = m_freeList;
	m_nodes[nodeId].height = -1;
	m_freeList = nodeId;
	--m_nodeCount;
}

void DynamicTree::InsertLeaf(int32_t leaf)
{
	if (m_root == nullNode)
	{
		m_root = leaf;
		m_nodes[m_root].parent = nullNode;
		return;
	}

	// Find the best sibling for this node
	AABB leafAABB = m_nodes[leaf].aabb;
	int32_t index = m_root;
	while (m_nodes[index].IsLeaf() == false)
	{
		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		float area = m_nodes[index].aabb.GetArea();

		AABB combinedAABB;
		combinedAABB.Combine(m_nodes[index].aabb, leafAABB);
		float combinedArea = combinedAABB.GetArea();

		// Cost of creating a new parent for this node and the new leaf
		float cost = 2.0f * combinedArea;

		// Minimum cost of pushing the leaf further down the tree
		float inheritanceCost = 2.0f * (combinedArea - area);

		// Cost of descending into child1
		float cost1;
		if (m_nodes[child1].IsLeaf())
		{
			AABB aabb;
			aabb.Combine(leafAABB, m_nodes[child1].aabb);
			cost1 = aabb.GetArea() + inheritanceCost;
		}
		else
		{
			AABB aabb;
			aabb.Combine(leafAABB, m_nodes[child1].aabb);
			float oldArea = m_nodes[child1].aabb.GetArea();
			float newArea = aabb.GetArea();
			cost1 = (newArea - oldArea) + inheritanceCost;
		}

		// Cost of descending into child2
		float cost2;
		if (m_nodes[child2].IsLeaf())
		{
			AABB aabb;
			aabb.Combine(leafAABB, m_nodes[child2].aabb);
			cost2 = aabb.GetArea() + inheritanceCost;
		}
		else
		{
			AABB aabb;
			aabb.Combine(leafAABB, m_nodes[child2].aabb);
			float oldArea = m_nodes[child2].aabb.GetArea();
			float newArea = aabb.GetArea();
			cost2 = newArea - oldArea + inheritanceCost;
		}

		// Descend according to the minimum cost.
		if (cost < cost1 && cost < cost2)
		{
			break;
		}

		// Descend
		if (cost1 < cost2)
		{
			index = child1;
		}
		else
		{
			index = child2;
		}
	}

	int32_t sibling = index;

	// Create a new parent.
	int32_t oldParent = m_nodes[sibling].parent;
	int32_t newParent = AllocateNode();
	m_nodes[newParent].parent = oldParent;
	m_nodes[newParent].collider = nullptr;
	m_nodes[newParent].aabb.Combine(leafAABB, m_nodes[sibling].aabb);
	m_nodes[newParent].height = m_nodes[sibling].height + 1;

	if (oldParent != nullNode)
	{
		// The sibling was not the root.
		if (m_nodes[oldParent].child1 == sibling)
		{
			m_nodes[oldParent].child1 = newParent;
		}
		else
		{
			m_nodes[oldParent].child2 = newParent;
		}

		m_nodes[newParent].child1 = sibling;
		m_nodes[newParent].child2 = leaf;
		m_nodes[sibling].parent = newParent;
		m_nodes[leaf].parent = newParent;
	}
	else
	{
		// The sibling was the root.
		m_nodes[newParent].child1 = sibling;
		m_nodes[newParent].child2 = leaf;
		m_nodes[sibling].parent = newParent;
		m_nodes[leaf].parent = newParent;
		m_root = newParent;
	}

	// Walk back up the tree fixing heights and AABBs
	index = m_nodes[leaf].parent;
	while (index != nullNode)
	{
		index = Balance(index);

		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		assert(child1 != nullNode);
		assert(child2 != nullNode);

		m_nodes[index].height = 1 + std::max(m_nodes[child1].height, m_nodes[child2].height);
		m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);

		index = m_nodes[index].parent;
	}
}

void DynamicTree::RemoveLeaf(int32_t leaf)
{
	if (leaf == m_root)
	{
		m_root = nullNode;
		return;
	}

	int32_t parent = m_nodes[leaf].parent;
	int32_t grandParent = m_nodes[parent].parent;
	int32_t sibling;
	if (m_nodes[parent].child1 == leaf)
	{
		sibling = m_nodes[parent].child2;
	}
	else
	{
		sibling = m_nodes[parent].child1;
	}

	if (grandParent != nullNode)
	{
		// Destroy parent and connect sibling to grandParent.
		if (m_nodes[grandParent].child1 == parent)
		{
			m_nodes[grandParent].child1 = sibling;
		}
		else
		{
			m_nodes[grandParent].child2 = sibling;
		}
		m_nodes[sibling].parent = grandParent;
		FreeNode(parent);

		// Adjust ancestor bounds.
		int32_t index = grandParent;
		while (index != nullNode)
		{
			index = Balance(index);

			int32_t child1 = m_nodes[index].child1;
			int32_t child2 = m_nodes[index].child2;

			m_nodes[index].aabb.Combine(m_nodes[child1].aabb, m_nodes[child2].aabb);
			m_nodes[index].height = 1 + std::max(m_nodes[child1].height, m_nodes[child2].height);

			index = m_nodes[index].parent;
		}
	}
	else
	{
		m_root = sibling;
		m_nodes[sibling].parent = nullNode;
		FreeNode(parent);
	}
}

int32_t DynamicTree::Balance(int32_t iA)
{
	assert(iA != nullNode);

	treeNode* A = m_nodes + iA;
	if (A->IsLeaf() || A->height < 2)
	{
		return iA;
	}

	int32_t iB = A->child1;
	int32_t iC = A->child2;
	assert(0 <= iB && iB < m_nodeCapacity);
	assert(0 <= iC && iC < m_nodeCapacity);

	treeNode* B = m_nodes + iB;
	treeNode* C = m_nodes + iC;

	int32_t balance = C->height - B->height;

	// Rotate C up
	if (balance > 1)
	{
		int32_t iF = C->child1;
		int32_t iG = C->child2;
		treeNode* F = m_nodes + iF;
		treeNode* G = m_nodes + iG;
		assert(0 <= iF && iF < m_nodeCapacity);
		assert(0 <= iG && iG < m_nodeCapacity);

		// Swap A and C
		C->child1 = iA;
		C->parent = A->parent;
		A->parent = iC;

		// A's old parent should point to C
		if (C->parent != nullNode)
		{
			if (m_nodes[C->parent].child1 == iA)
			{
				m_nodes[C->parent].child1 = iC;
			}
			else
			{
				assert(m_nodes[C->parent].child2 == iA);
				m_nodes[C->parent].child2 = iC;
			}
		}
		else
		{
			m_root = iC;
		}

		// Rotate
		if (F->height > G->height)
		{
			C->child2 = iF;
			A->child2 = iG;
			G->parent = iA;
			A->aabb.Combine(B->aabb, G->aabb);
			C->aabb.Combine(A->aabb, F->aabb);

			A->height = 1 + std::max(B->height, G->height);
			C->height = 1 + std::max(A->height, F->height);
		}
		else
		{
			C->child2 = iG;
			A->child2 = iF;
			F->parent = iA;
			A->aabb.Combine(B->aabb, F->aabb);
			C->aabb.Combine(A->aabb, G->aabb);

			A->height = 1 + std::max(B->height, F->height);
			C->height = 1 + std::max(A->height, G->height);
		}

		return iC;
	}

	// Rotate B up
	if (balance < -1)
	{
		int32_t iD = B->child1;
		int32_t iE = B->child2;
		treeNode* D = m_nodes + iD;
		treeNode* E = m_nodes + iE;
		assert(0 <= iD && iD < m_nodeCapacity);
		assert(0 <= iE && iE < m_nodeCapacity);

		// Swap A and B
		B->child1 = iA;
		B->parent = A->parent;
		A->parent = iB;

		// A's old parent should point to B
		if (B->parent != nullNode)
		{
			if (m_nodes[B->parent].child1 == iA)
			{
				m_nodes[B->parent].child1 = iB;
			}
			else
			{
				assert(m_nodes[B->parent].child2 == iA);
				m_nodes[B->parent].child2 = iB;
			}
		}
		else
		{
			m_root = iB;
		}

		// Rotate
		if (D->height > E->height)
		{
			B->child2 = iD;
			A->child1 = iE;
			E->parent = iA;
			A->aabb.Combine(C->aabb, E->aabb);
			B->aabb.Combine(A->aabb, D->aabb);

			A->height = 1 + std::max(C->height, E->height);
			B->height = 1 + std::max(A->height, D->height);
		}
		else
		{
			B->child2 = iE;
			A->child1 = iD;
			D->parent = iA;
			A->aabb.Combine(C->aabb, D->aabb);
			B->aabb.Combine(A->aabb, E->aabb);

			A->height = 1 + std::max(C->height, D->height);
			B->height = 1 + std::max(A->height, E->height);
		}

		return iB;
	}

	return iA;
}

int32_t DynamicTree::ComputeHeight() const
{
	int32_t height = ComputeHeight(m_root);
	return height;
}

int32_t DynamicTree::ComputeHeight(int32_t nodeId) const
{
	assert(0 <= nodeId && nodeId < m_nodeCapacity);
	treeNode* node = m_nodes + nodeId;

	if (node->IsLeaf())
	{
		return 0;
	}

	int32_t height1 = ComputeHeight(node->child1);
	int32_t height2 = ComputeHeight(node->child2);
	return 1 + std::max(height1, height2);
}
